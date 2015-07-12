/*
	Light Indexed Deferred Renderin

	This approach simply assigns each light a unique index and then stores this index at each 
fragment the light hits, rather than storing all the light or material properties per fragment. These 
indexes can then be used in a fragment shader to lookup into a lighting properties table for data to 
light the fragment.
	1) Render depth only pre-pass
	2) Disable depth writes (depth testing only) and render light volumes into a light index texture. 
	Standard deferred lighting / shadow volume techniques can be used to find what fragments 
	are hit by each light volume.
	3) Render geometry using standard forward rendering – lighting is done using the light index 
	texture to access lighting properties in each shader.  
*/

// http://www.neogaf.com/forum/showthread.php?t=995252&page=2

#include "Renderer/Renderer_PCH.h"
#pragma hdrstop

// for std::sort()
#include <algorithm>

#include <Core/Util/Tweakable.h>

#include <Graphics/Effects.h>

#include <Renderer/Mesh.h>
#include <Renderer/Model.h>
#include <Renderer/Texture.h>
#include <Renderer/Material.h>
#include <Renderer/Renderer.h>
#include <Renderer/Light.h>
#include <Renderer/Vertex.h>
//#include <Renderer/Shaders/HLSL/_common.h>
#include <Renderer/_common.h>
#include <Renderer/ForwardPlus.h>

ForwardPlusRenderer::ForwardPlusRenderer()
{

}
ForwardPlusRenderer::~ForwardPlusRenderer()
{

}
ERet ForwardPlusRenderer::Initialize( Clump* rendererData )
{
	mxDO(Super::Initialize(rendererData));
	return ALL_OK;
}
void ForwardPlusRenderer::Shutdown()
{
	Super::Shutdown();
}
ERet ForwardPlusRenderer::RenderScene( const SceneView& sceneView, const Clump& sceneData )
{
	if( sceneView.viewportWidth != m_viewportWidth || sceneView.viewportHeight != m_viewportHeight )
	{
		this->ResizeBuffers( sceneView.viewportWidth, sceneView.viewportHeight, false );
		m_viewportWidth = sceneView.viewportWidth;
		m_viewportHeight = sceneView.viewportHeight;
	}

	FxColorTarget* pColorRT0;
	mxDO(GetByName(*m_rendererData, "GBufferTexture0", pColorRT0));

	FxColorTarget* pColorRT1;
	mxDO(GetByName(*m_rendererData, "GBufferTexture1", pColorRT1));

	FxDepthTarget* pDepthRT;
	mxDO(GetByName(*m_rendererData, "MainDepthStencil", pDepthRT));

	{
		llgl::ViewState	viewState;
		{
			viewState.Reset();
			viewState.colorTargets[0] = pColorRT0->handle;
			viewState.colorTargets[1] = pColorRT1->handle;
			viewState.targetCount = 2;
			viewState.depthTarget = pDepthRT->handle;
			viewState.flags = llgl::ClearAll;
		}
		llgl::SubmitView(m_hRenderContext, viewState);
	}


	G_PerCamera	cbPerView;
	{
		cbPerView.g_viewMatrix = sceneView.viewMatrix;
		cbPerView.g_viewProjectionMatrix = sceneView.viewMatrix * sceneView.viewProjectionMatrix;
		cbPerView.g_inverseViewMatrix = Matrix_OrthoInverse( sceneView.viewMatrix );
		cbPerView.g_projectionMatrix = sceneView.projectionMatrix;
		cbPerView.g_inverseProjectionMatrix = Matrix_Inverse( sceneView.viewProjectionMatrix );

		llgl::UpdateBuffer(llgl::GetMainContext(), m_hCBPerCamera, sizeof(cbPerView), &cbPerView);
	}

	mxDO(FxSlow_SetRenderState(m_hRenderContext, *m_rendererData, "Default"));





	// G-Buffer Stage: Render all solid objects to a very sparse G-Buffer

	G_PerObject	cbPerObject;


	TObjectIterator< rxModel >	modelIt( sceneData );
	while( modelIt.IsValid() )
	{
		const rxModel& model = modelIt.Value();

		const Float3x4* TRS = model.m_transform;

		{
			cbPerObject.g_worldMatrix = Float3x4_Unpack( *TRS );
			cbPerObject.g_worldViewMatrix = Matrix_Multiply(cbPerObject.g_worldMatrix, sceneView.viewMatrix);
			cbPerObject.g_worldViewProjectionMatrix = Matrix_Multiply(cbPerObject.g_worldMatrix, sceneView.viewProjectionMatrix);

			llgl::UpdateBuffer(m_hRenderContext, m_hCBPerObject, sizeof(cbPerObject), &cbPerObject);
		}

		const rxMesh* mesh = model.m_mesh;

		for( int iSubMesh = 0; iSubMesh < mesh->m_parts.Num(); iSubMesh++ )
		{
			const rxSubmesh& submesh = mesh->m_parts[iSubMesh];
			const rxMaterial* material = model.m_batches[iSubMesh];
			const FxShader* shader = material->m_shader;

			if( shader->localCBs.Num() )
			{
				mxASSERT(shader->localCBs.Num()==1);
				const ParameterBuffer& uniforms = material->m_uniforms;
				const FxCBuffer& rCB = shader->localCBs[0];
				llgl::UpdateBuffer(m_hRenderContext, rCB.handle, uniforms.GetDataSize(), uniforms.ToPtr() );
			}

			llgl::DrawCall	batch;
			batch.Clear();
UNDONE;
	#if 0
		{
				batch.CBs[G_PerCamera_Index] = m_hCBPerCamera;
				batch.CBs[G_PerObject_Index] = m_hCBPerObject;

				const UINT32 numCBs = shader->CBs.Num();
				for( UINT32 iCB = 0; iCB < numCBs; iCB++ )
				{
					const FxCBufferBinding& binding = shader->CBs[ iCB ];
					batch.CBs[ binding.slot ] = binding.resource->handle;
				}
				const UINT32 numTSs = shader->SRs.Num();
				for( UINT32 iTS = 0; iTS < numTSs; iTS++ )
				{
					const TextureLayer& layer = material->m_textures[ iTS ]; 
					const rxTexture* texture = layer.texture;
					const FxTextureBinding& binding = shader->SRs[ iTS ];						
					const FxResource* resource = binding.resource;

					batch.SRs[ binding.slot ] = texture ? texture->m_resource : resource->texture;
					//batch.SSs[ binding.slot ] = resource->sampler;
					batch.SSs[ binding.slot ] = Rendering::g_samplers[ layer.sampler ];
				}

				batch.program = shader->programs[ 0 ];
			}
#endif
			batch.inputLayout = Rendering::g_inputLayouts[VTX_Draw];
			batch.topology = mesh->m_topology;

			batch.VB[0] = mesh->m_vertexBuffer;
			batch.IB = mesh->m_indexBuffer;
			batch.b32bit = (mesh->m_indexStride == sizeof(UINT32));

			batch.baseVertex = submesh.baseVertex;
			batch.vertexCount = submesh.vertexCount;
			batch.startIndex = submesh.startIndex;
			batch.indexCount = submesh.indexCount;

			llgl::Submit(m_hRenderContext, batch);
		}

		modelIt.MoveToNext();
	}

	// Deferred Lighting Stage: Accumulate all lights as a screen space operation

	{
		llgl::ViewState	viewState;
		{
			viewState.Reset();
			viewState.colorTargets[0].SetDefault();
			viewState.targetCount = 1;
			viewState.flags = llgl::ClearAll;
		}
		llgl::SubmitView(m_hRenderContext, viewState);

		//{
		//	//

		//	TObjectIterator< rxGlobalLight >	lightIt( sceneData );
		//	while( lightIt.IsValid() )
		//	{
		//		rxGlobalLight& light = lightIt.Value();

		//		//

		//		lightIt.MoveToNext();
		//	}
		//}
	}
// Thursday, March 26, 2015 Implementing Weighted, Blended Order-Independent Transparency 
//http://casual-effects.blogspot.ru/2015/03/implemented-weighted-blended-order.html

// Forward+ notes
//http://bioglaze.blogspot.fi/2014/07/2048-point-lights-60-fps.html

	// Material Stage: Render all solid objects again while combining the lighting
	// from Stage 2 with certain material-properties (colors, reflections, glow, fog, etc.)
	// to produce the final image

	{
		llgl::ViewState	viewState;
		{
			viewState.Reset();
			viewState.colorTargets[0].SetDefault();
			viewState.targetCount = 1;
			viewState.flags = llgl::ClearAll;
		}
		llgl::SubmitView(m_hRenderContext, viewState);
	}

	{
		mxDO(FxSlow_SetRenderState(m_hRenderContext, *m_rendererData, "NoCulling"));

		FxShader* shader;
		mxDO(GetByName(*m_rendererData, "full_screen_triangle", shader));
		mxDO(FxSlow_SetResource(shader, "t_sourceTexture", llgl::AsResource(pColorRT1->handle), Rendering::g_samplers[PointSampler]));
		mxDO(FxSlow_Commit(m_hRenderContext,shader));

		DrawFullScreenTriangle(shader);
	}

	return ALL_OK;
}

ERet ForwardPlusRenderer::ResizeBuffers( UINT16 width, UINT16 height, bool fullscreen )
{
	mxDO(Rendering::ReleaseResourcesDependentOnBackBuffer( *m_rendererData ));

	mxDO(llgl::SetVideoMode( width, height ));
	mxDO(llgl::NextFrame());

	mxDO(Rendering::RecreateResourcesDependentOnBackBuffer( *m_rendererData, width, height ));

	return ALL_OK;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
