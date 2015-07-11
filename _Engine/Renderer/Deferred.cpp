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
#include <Renderer/Shaders/HLSL/_common.h>
#include <Renderer/Deferred.h>

DeferredRenderer::DeferredRenderer()
{

}
DeferredRenderer::~DeferredRenderer()
{

}
ERet DeferredRenderer::Initialize( Clump* rendererData )
{
	mxDO(Super::Initialize(rendererData));

	mxDO(GetByName(*m_rendererData, "GBufferTexture0", m_colorRT0));
	mxDO(GetByName(*m_rendererData, "GBufferTexture1", m_colorRT1));
	mxDO(GetByName(*m_rendererData, "MainDepthStencil", m_depthRT));

	return ALL_OK;
}
void DeferredRenderer::Shutdown()
{
	Super::Shutdown();
}
ERet DeferredRenderer::RenderScene( const SceneView& sceneView, const Clump& sceneData )
{
	gfxMARKER(RenderScene);

	if( sceneView.viewportWidth != m_viewportWidth || sceneView.viewportHeight != m_viewportHeight )
	{
		this->ResizeBuffers( sceneView.viewportWidth, sceneView.viewportHeight, false );
		m_viewportWidth = sceneView.viewportWidth;
		m_viewportHeight = sceneView.viewportHeight;
	}

	mxDO(BeginRender_GBuffer());

	G_PerCamera	cbPerView;
	{
		cbPerView.g_viewMatrix = sceneView.viewMatrix;

		cbPerView.g_viewProjectionMatrix = sceneView.viewMatrix * sceneView.projectionMatrix;

		cbPerView.g_inverseViewMatrix = Matrix_OrthoInverse( sceneView.viewMatrix );
		cbPerView.g_projectionMatrix = sceneView.projectionMatrix;
		cbPerView.g_inverseProjectionMatrix = ProjectionMatrix_Inverse( sceneView.projectionMatrix );
		cbPerView.g_inverseViewProjectionMatrix = Matrix_Inverse( cbPerView.g_viewProjectionMatrix );

		cbPerView.g_WorldSpaceCameraPos = sceneView.worldSpaceCameraPos;

		float n = sceneView.nearClip;
		float f = sceneView.farClip;
		float x = 1 - f / n;
		float y = f / n;
		float z = x / f;
		float w = y / f;
		cbPerView.g_ZBufferParams = Float4_Set( x, y, z, w );

		cbPerView.g_ZBufferParams2 = Float4_Set( n, f, 1.0f/n, 1.0f/f );

		float H = sceneView.projectionMatrix[0][0];
		float V = sceneView.projectionMatrix[2][1];
		float A = sceneView.projectionMatrix[1][2];
		float B = sceneView.projectionMatrix[3][2];
		cbPerView.g_ProjParams = Float4_Set( H, V, A, B );

		// x = 1/H
		// y = 1/V
		// z = 1/B
		// w = -A/B
		cbPerView.g_ProjParams2 = Float4_Set( 1/H, 1/V, 1/B, -A/B );

		llgl::UpdateBuffer(llgl::GetMainContext(), m_hCBPerCamera, sizeof(cbPerView), &cbPerView);
	}


	// G-Buffer Stage: Render all solid objects to a very sparse G-Buffer
	{
		gfxMARKER(Fill_Geometry_Buffer);

		mxDO(FxSlow_SetRenderState(m_hRenderContext, *m_rendererData, "Default"));

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

				SetGlobalUniformBuffers( &batch );
				BindMaterial( material, &batch );

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
	}

	EndRender_GBuffer();

	// Deferred Lighting Stage: Accumulate all lights as a screen space operation
	{
		gfxMARKER(Deferred_Lighting);

		llgl::ViewState	viewState;
		{
			viewState.Reset();
			viewState.colorTargets[0].SetDefault();
			viewState.targetCount = 1;
			viewState.flags = llgl::ClearColor;
		}
		llgl::SubmitView(m_hRenderContext, viewState);

		{
			gfxMARKER(Directional_Lights);

			mxDO(FxSlow_SetRenderState(m_hRenderContext, *m_rendererData, "Deferred_Lighting"));

			FxShader* shader;
			mxDO(GetByName(*m_rendererData, "deferred_directional_light", shader));
			mxDO2(FxSlow_SetResource(shader, "GBufferTexture0", llgl::AsResource(m_colorRT0->handle), Rendering::g_samplers[PointSampler]));
			mxDO2(FxSlow_SetResource(shader, "GBufferTexture1", llgl::AsResource(m_colorRT1->handle), Rendering::g_samplers[PointSampler]));
			mxDO2(FxSlow_SetResource(shader, "DepthTexture", llgl::AsResource(m_depthRT->handle), Rendering::g_samplers[PointSampler]));

			TObjectIterator< rxGlobalLight >	lightIt( sceneData );
			while( lightIt.IsValid() )
			{
				rxGlobalLight& light = lightIt.Value();

				//mxDO(FxSlow_Commit(m_hRenderContext,shader));
				DirectionalLight lightData;
				{
					lightData.direction = Matrix_TransformNormal(sceneView.viewMatrix, light.m_direction);
					lightData.color = light.m_color;
				}
				mxDO2(FxSlow_UpdateUBO(m_hRenderContext,shader,"DATA",&lightData,sizeof(lightData)));

				DrawFullScreenTriangle(shader);

				lightIt.MoveToNext();
			}
		}


		{
			gfxMARKER(Point_Lights);

			mxDO(FxSlow_SetRenderState(m_hRenderContext, *m_rendererData, "Deferred_Lighting"));

			FxShader* shader;
			mxDO(GetAsset(shader,MakeAssetID("deferred_point_light.shader"),m_rendererData));
			mxDO2(FxSlow_SetResource(shader, "GBufferTexture0", llgl::AsResource(m_colorRT0->handle), Rendering::g_samplers[PointSampler]));
			mxDO2(FxSlow_SetResource(shader, "GBufferTexture1", llgl::AsResource(m_colorRT1->handle), Rendering::g_samplers[PointSampler]));
			mxDO2(FxSlow_SetResource(shader, "DepthTexture", llgl::AsResource(m_depthRT->handle), Rendering::g_samplers[PointSampler]));

			TObjectIterator< rxLocalLight >	lightIt( sceneData );
			while( lightIt.IsValid() )
			{
				rxLocalLight& light = lightIt.Value();

				//mxDO(FxSlow_Commit(m_hRenderContext,shader));
				PointLight lightData;
				{
					Float3 viewSpaceLightPosition = Matrix_TransformPoint(sceneView.viewMatrix, light.position);
					lightData.Position_InverseRadius = Float4_Set(viewSpaceLightPosition, 1.0f/light.radius);
					lightData.Color_Radius = Float4_Set(light.color, light.radius);
				}
				mxDO2(FxSlow_UpdateUBO(m_hRenderContext,shader,"DATA",&lightData,sizeof(lightData)));

				DrawFullScreenTriangle(shader);

				lightIt.MoveToNext();
			}
		}
	}


// Thursday, March 26, 2015 Implementing Weighted, Blended Order-Independent Transparency 
//http://casual-effects.blogspot.ru/2015/03/implemented-weighted-blended-order.html

// Forward+ notes
//http://bioglaze.blogspot.fi/2014/07/2048-point-lights-60-fps.html

	// Material Stage: Render all solid objects again while combining the lighting
	// from Stage 2 with certain material-properties (colors, reflections, glow, fog, etc.)
	// to produce the final image

#if 0
	{
		llgl::ViewState	viewState;
		{
			viewState.Reset();
			viewState.colorTargets[0].SetDefault();
			viewState.targetCount = 1;
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
#endif

#if 0
	{
		llgl::ViewState	viewState;
		{
			viewState.Reset();
			viewState.colorTargets[0].SetDefault();
			viewState.targetCount = 1;
		}
		llgl::SubmitView(m_hRenderContext, viewState);
	}

	{
		FxShader* shader;

		mxDO(FxSlow_SetRenderState(m_hRenderContext, *m_rendererData, "Default"));
		{
			mxDO(GetAsset(shader,MakeAssetID("debug_draw_colored.shader"),m_rendererData));
			mxDO(FxSlow_SetUniform(shader,"g_color",RGBAf::GRAY.ToPtr()));
			mxDO(FxSlow_Commit(m_hRenderContext,shader));
			DBG_Draw_Models_With_Custom_Shader(sceneView, sceneData, *shader);
		}

		mxDO(FxSlow_SetRenderState(m_hRenderContext, *m_rendererData, "NoCulling"));
		{
			mxDO(GetAsset(shader,MakeAssetID("debug_draw_normals.shader"),m_rendererData));
			float lineLength = 4.0f;
			mxDO(FxSlow_SetUniform(shader,"g_lineLength",&lineLength));
			mxDO(FxSlow_Commit(m_hRenderContext,shader));
			DBG_Draw_Models_With_Custom_Shader(sceneView, sceneData, *shader, Topology::PointList);
		}
	}
#endif

	return ALL_OK;
}

ERet DeferredRenderer::ResizeBuffers( UINT16 width, UINT16 height, bool fullscreen )
{
	mxDO(Rendering::ReleaseResourcesDependentOnBackBuffer( *m_rendererData ));

	mxDO(llgl::SetVideoMode( width, height ));
	mxDO(llgl::NextFrame());

	mxDO(Rendering::RecreateResourcesDependentOnBackBuffer( *m_rendererData, width, height ));

	return ALL_OK;
}
ERet DeferredRenderer::BeginRender_GBuffer()
{
	{
		llgl::ViewState	viewState;
		{
			viewState.Reset();
			viewState.colorTargets[0] = m_colorRT0->handle;
			viewState.colorTargets[1] = m_colorRT1->handle;
			viewState.targetCount = 2;
			viewState.depthTarget = m_depthRT->handle;
			viewState.depth = 1.0f;
			viewState.flags = llgl::ClearAll;
		}
		llgl::SubmitView(m_hRenderContext, viewState);
	}

	return ALL_OK;
}
ERet DeferredRenderer::EndRender_GBuffer()
{
	return ALL_OK;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
