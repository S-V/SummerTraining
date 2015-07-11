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
#include <Renderer/Vertex.h>

//#include <Renderer/Shaders/HLSL/_common.h>
#if 0

#include <Scripting/Scripting.h>
#include <Scripting/Lua_Helpers.h>

extern "C"{

static int Wrap_SetColorTarget( lua_State* L )
{
	const int numArgs = lua_gettop( L );
	mxASSERT(numArgs == 3);

	mxASSERT(lua_islightuserdata(L,1));
	//luaL_checktype(L, 1/*index*/, LUA_TUSERDATA);
	RenderContext* ctx = static_cast< RenderContext* >( lua_touserdata(L, 1) );

	mxASSERT(lua_isinteger(L,2));
	int slot = lua_tointeger(L, 2);

	mxASSERT(lua_isstring(L,3));
	size_t len;
	const char* targetID = lua_tolstring(L, 3, &len);


	if(strlen(targetID)==0)
	{
		mxASSERT(slot==0);
		ctx->viewState.colorTargets[slot].SetDefault();
	}
	else
	{
		UNDONE;
	}

	return 0;
}

static int Wrap_SetDepthTarget( lua_State* L )
{
	const int numArgs = lua_gettop( L );
	mxASSERT(numArgs == 2);

	mxASSERT(lua_islightuserdata(L,1));
	RenderContext* ctx = static_cast< RenderContext* >( lua_touserdata(L, 1) );

	mxASSERT(lua_isstring(L,2));
	size_t len;
	const char* targetID = lua_tolstring(L, 2, &len);

	if(strlen(targetID)==0)
	{
		ctx->viewState.depthTarget.SetDefault();
	}
	else
	{
		UNDONE;
	}

	return 0;
}

static int Wrap_SetFlags( lua_State* L )
{
	const int numArgs = lua_gettop( L );
	mxASSERT(numArgs == 2);

	mxASSERT(lua_islightuserdata(L,1));
	RenderContext* ctx = static_cast< RenderContext* >( lua_touserdata(L, 1) );

	mxASSERT(lua_isinteger(L,2));
	int flags = lua_tointeger(L, 2);

	ctx->viewState.flags = flags;

	return 0;
}

static int Wrap_SetState( lua_State* L )
{
	const int numArgs = lua_gettop( L );
	mxASSERT(numArgs == 2);

	mxASSERT(lua_islightuserdata(L,1));
//	RenderContext* ctx = static_cast< RenderContext* >( lua_touserdata(L, 1) );

	mxASSERT(lua_isstring(L,2));
	size_t len;
//	const char* renderStateID = lua_tolstring(L, 2, &len);

	//ctx->viewState.flags = flags;
	UNDONE;

	return 0;
}

}//extern "C"

static const luaL_Reg gs_functions[] =
{
	{ "SetColorTarget", &Wrap_SetColorTarget },
	{ "SetDepthTarget", &Wrap_SetDepthTarget },
	{ "SetFlags", &Wrap_SetFlags },
	{ "SetState", &Wrap_SetState },
};

#endif

namespace Rendering
{
	static bool DependsOnBackBufferSize( const FxRenderTargetBase& renderTarget )
	{
		return renderTarget.sizeX.relative || renderTarget.sizeY.relative;
	}

	static UINT32 CalculateRenderTargetDimension( INT32 screenDimension, const FxRenderTargetSize& textureSize )
	{
		float sizeValue = textureSize.size;
		if( textureSize.relative ) {
			sizeValue *= screenDimension;
		}
		return (UINT32) sizeValue;
	}

	static UINT32 CalculateRenderTargetArea( UINT16 screenWidth, UINT16 screenHeight, const FxRenderTargetBase& renderTarget )
	{
		UINT32 sizeX = CalculateRenderTargetDimension( screenWidth, renderTarget.sizeX );
		UINT32 sizeY = CalculateRenderTargetDimension( screenHeight, renderTarget.sizeY );
		return sizeX * sizeY;
	}

	struct SortItem
	{
		void *	o;	// FxColorTarget or FxDepthTarget
		UINT32	size;
		bool	isColor;
	};

	ERet CreateRenderTargets(
		const Clump & clump,
		UINT16 screenWidth, UINT16 screenHeight,		
		bool createOnlyThoseDependentOnBackBuffer )
	{
		// create largest render targets first
		TLocalArray< SortItem, 32 >	sortedItems;

		{
			TObjectIterator< FxColorTarget >	colorTargetIt( clump );
			while( colorTargetIt.IsValid() )
			{
				FxColorTarget& colorTarget = colorTargetIt.Value();
				if( createOnlyThoseDependentOnBackBuffer && !DependsOnBackBufferSize( colorTarget ) ) {
					continue;
				}
				SortItem& sortItem = sortedItems.Add();
				sortItem.o = &colorTarget;
				sortItem.size = CalculateRenderTargetArea( screenWidth, screenHeight, colorTarget ) * PixelFormat::BitsPerPixel(colorTarget.format);
				sortItem.isColor = true;
				colorTargetIt.MoveToNext();
			}
		}
		{
			TObjectIterator< FxDepthTarget >	depthTargetIt( clump );
			while( depthTargetIt.IsValid() )
			{
				FxDepthTarget& depthTarget = depthTargetIt.Value();
				if( createOnlyThoseDependentOnBackBuffer && !DependsOnBackBufferSize( depthTarget ) ) {
					continue;
				}
				SortItem& sortItem = sortedItems.Add();
				sortItem.o = &depthTarget;
				sortItem.size = CalculateRenderTargetArea( screenWidth, screenHeight, depthTarget ) * DepthStencilFormat::BitsPerPixel(depthTarget.format);
				sortItem.isColor = false;
				depthTargetIt.MoveToNext();
			}
		}

		struct CompareRenderTargetsByArea {
			inline bool operator () ( const SortItem& a, const SortItem& b ) const { return a.size > b.size; }
		};
		std::stable_sort( sortedItems.ToPtr(), sortedItems.ToPtr() + sortedItems.Num(), CompareRenderTargetsByArea() );

		ColorTargetDescription	colorTargetDescription;
		DepthTargetDescription	depthTargetDescription;
		for( UINT32 iRenderTarget = 0; iRenderTarget < sortedItems.Num(); iRenderTarget++ )
		{
			SortItem& item = sortedItems[ iRenderTarget ];
			if( item.isColor )
			{
				FxColorTarget& colorTarget = *static_cast< FxColorTarget* >( item.o );

				colorTargetDescription.name.SetReference( colorTarget.name );
				colorTargetDescription.format = colorTarget.format;
				colorTargetDescription.width = CalculateRenderTargetDimension( screenWidth, colorTarget.sizeX );
				colorTargetDescription.height = CalculateRenderTargetDimension( screenHeight, colorTarget.sizeY );

				colorTarget.handle = llgl::CreateColorTarget( colorTargetDescription );
			}
			else
			{
				FxDepthTarget& depthTarget = *static_cast< FxDepthTarget* >( item.o );

				depthTargetDescription.name.SetReference( depthTarget.name );
				depthTargetDescription.format = depthTarget.format;
				depthTargetDescription.width = CalculateRenderTargetDimension( screenWidth, depthTarget.sizeX );
				depthTargetDescription.height = CalculateRenderTargetDimension( screenHeight, depthTarget.sizeY );
				depthTargetDescription.sample = depthTarget.sample;

				depthTarget.handle = llgl::CreateDepthTarget( depthTargetDescription );
			}
		}
		return ALL_OK;
	}

	ERet ReleaseRenderTargets( const Clump& _clump )
	{
		{
			TObjectIterator< FxColorTarget >	colorTargetIt( _clump );
			while( colorTargetIt.IsValid() )
			{
				FxColorTarget& colorTarget = colorTargetIt.Value();
				if( colorTarget.handle.IsValid() )
				{
					llgl::DeleteColorTarget( colorTarget.handle );
					colorTarget.handle.SetNil();
				}				
				colorTargetIt.MoveToNext();
			}
		}
		{
			TObjectIterator< FxDepthTarget >	depthTargetIt( _clump );
			while( depthTargetIt.IsValid() )
			{
				FxDepthTarget& depthTarget = depthTargetIt.Value();
				if( depthTarget.handle.IsValid() )
				{
					llgl::DeleteDepthTarget( depthTarget.handle );
					depthTarget.handle.SetNil();
				}
				depthTargetIt.MoveToNext();
			}
		}
		return ALL_OK;
	}

	ERet ReleaseResourcesDependentOnBackBuffer( const Clump& _clump )
	{
		{
			TObjectIterator< FxColorTarget >	colorTargetIt( _clump );
			while( colorTargetIt.IsValid() )
			{
				FxColorTarget& colorTarget = colorTargetIt.Value();
				if( DependsOnBackBufferSize( colorTarget ) && colorTarget.handle.IsValid() ) {
					llgl::DeleteColorTarget( colorTarget.handle );
					colorTarget.handle.SetNil();
				}
				colorTargetIt.MoveToNext();
			}
		}
		{
			TObjectIterator< FxDepthTarget >	depthTargetIt( _clump );
			while( depthTargetIt.IsValid() )
			{
				FxDepthTarget& depthTarget = depthTargetIt.Value();
				if( DependsOnBackBufferSize( depthTarget ) && depthTarget.handle.IsValid() ) {
					llgl::DeleteDepthTarget( depthTarget.handle );
					depthTarget.handle.SetNil();
				}
				depthTargetIt.MoveToNext();
			}
		}
		return ALL_OK;
	}

	ERet RecreateResourcesDependentOnBackBuffer( const Clump& _clump, UINT16 screenWidth, UINT16 screenHeight )
	{
		mxDO(CreateRenderTargets(_clump, screenWidth, screenHeight, true));
		return ALL_OK;
	}

	ERet CreateRenderResources( const Clump& _clump )
	{
		// Ideally, render states should be sorted by state deltas (so that changes between adjacent states are minimized).
		{
			TObjectIterator< FxSamplerState >	samplerStateIt( _clump );
			while( samplerStateIt.IsValid() )
			{
				FxSamplerState& samplerState = samplerStateIt.Value();
				samplerState.handle = llgl::CreateSamplerState( samplerState );
				samplerStateIt.MoveToNext();
			}
		}
		{
			TObjectIterator< FxDepthStencilState >	depthStencilStateIt( _clump );
			while( depthStencilStateIt.IsValid() )
			{
				FxDepthStencilState& depthStencilState = depthStencilStateIt.Value();
				depthStencilState.handle = llgl::CreateDepthStencilState( depthStencilState );
				depthStencilStateIt.MoveToNext();
			}
		}
		{
			TObjectIterator< FxRasterizerState >	rasterizerStateIt( _clump );
			while( rasterizerStateIt.IsValid() )
			{
				FxRasterizerState& rasterizerState = rasterizerStateIt.Value();
				rasterizerState.handle = llgl::CreateRasterizerState( rasterizerState );
				rasterizerStateIt.MoveToNext();
			}
		}
		{
			TObjectIterator< FxBlendState >	blendStateIt( _clump );
			while( blendStateIt.IsValid() )
			{
				FxBlendState& blendState = blendStateIt.Value();
				blendState.handle = llgl::CreateBlendState( blendState );
				blendStateIt.MoveToNext();
			}
		}

		FxRenderResources* renderStates = FindSingleInstance<FxRenderResources>(_clump);
		if(renderStates)
		{
			for( UINT32 iStateBlock = 0; iStateBlock < renderStates->state_blocks.Num(); iStateBlock++ )
			{
				FxStateBlock* stateBlock = renderStates->state_blocks[ iStateBlock ];
				stateBlock->blendState		= renderStates->blend_states[ stateBlock->blendState.id ]->handle;
				stateBlock->rasterizerState	= renderStates->rasterizer_states[ stateBlock->rasterizerState.id ]->handle;
				stateBlock->depthStencilState= renderStates->depth_stencil_states[ stateBlock->depthStencilState.id ]->handle;
			}
		}

		return ALL_OK;
	}

	void ReleaseRenderResources( const Clump& _clump )
	{
#if 0
		for( UINT32 iProgram = 0; iProgram < m_programs.Num(); iProgram++ )
		{
			FxProgram& program = m_programs[ iProgram ];
			llgl::DeleteProgram(program.handle);
			program.handle.SetNil();
		}

		for( UINT32 shaderType = 0; shaderType < mxCOUNT_OF(m_shaders); shaderType++ )
		{
			for( UINT32 iShader = 0; iShader < m_shaders[shaderType].Num(); iShader++ )
			{
				llgl::DeleteShader( m_shaders[shaderType][iShader] );
				m_shaders[shaderType][iShader].SetNil();
			}
		}

		for( UINT32 iTechnique = 0; iTechnique < m_techniques.Num(); iTechnique++ )
		{
			FxShader& technique = m_techniques[ iTechnique ];
			for( UINT32 iCB = 0; iCB < technique.locals.Num(); iCB++ )
			{
				FxCBuffer& rCB = technique.locals[ iCB ];
				llgl::DeleteBuffer( rCB.handle );
				rCB.handle.SetNil();
			}
		}

		for( UINT32 iCB = 0; iCB < m_globalCBuffers.Num(); iCB++ )
		{
			FxCBuffer& rCB = m_globalCBuffers[ iCB ];
			llgl::DeleteBuffer( rCB.handle );
			rCB.handle.SetNil();
		}
		//for( UINT32 iSR = 0; iSR < shaderResources.Num(); iSR++ )
		//{
		//	//XShaderResource& rSR = shaderResources[ iSR ];
		//	UNDONE;
		//}
#endif
#if 0
		for( UINT32 iStateBlock = 0; iStateBlock < m_stateBlocks.Num(); iStateBlock++ )
		{
			FxStateBlock& stateBlock = m_stateBlocks[ iStateBlock ];
			stateBlock.rasterizerState.SetNil();
			stateBlock.depthStencilState.SetNil();
			stateBlock.blendState.SetNil();
		}
		m_stateBlocks.DestroyAndEmpty();
#endif

		{
			TObjectIterator< FxBlendState >	blendStateIt( _clump );
			while( blendStateIt.IsValid() )
			{
				FxBlendState& blendState = blendStateIt.Value();
				llgl::DeleteBlendState( blendState.handle );
				blendState.handle.SetNil();
				blendStateIt.MoveToNext();
			}
		}
		{
			TObjectIterator< FxRasterizerState >	rasterizerStateIt( _clump );
			while( rasterizerStateIt.IsValid() )
			{
				FxRasterizerState& rasterizerState = rasterizerStateIt.Value();
				llgl::DeleteRasterizerState( rasterizerState.handle );
				rasterizerState.handle.SetNil();
				rasterizerStateIt.MoveToNext();
			}
		}
		{
			TObjectIterator< FxDepthStencilState >	depthStencilStateIt( _clump );
			while( depthStencilStateIt.IsValid() )
			{
				FxDepthStencilState& depthStencilState = depthStencilStateIt.Value();
				llgl::DeleteDepthStencilState( depthStencilState.handle );
				depthStencilState.handle.SetNil();
				depthStencilStateIt.MoveToNext();
			}
		}
		{
			TObjectIterator< FxSamplerState >	samplerStateIt( _clump );
			while( samplerStateIt.IsValid() )
			{
				FxSamplerState& samplerState = samplerStateIt.Value();
				llgl::DeleteSamplerState( samplerState.handle );
				samplerState.handle.SetNil();
				samplerStateIt.MoveToNext();
			}
		}
	}

	ERet RegisterClasses()
	{
		ShaderSystem_RegisterClasses();

		{
			Assets::AssetMetaType& meshType = Assets::gs_assetTypes[AssetTypes::MESH];
			meshType.loadData = &rxMesh::Load;
			meshType.finalize = &rxMesh::Online;
			meshType.bringOut = &rxMesh::Offline;
			meshType.freeData = &rxMesh::Destruct;
		}
		{
			Assets::AssetMetaType& assetCallbacks = Assets::gs_assetTypes[AssetTypes::TEXTURE];
			assetCallbacks.loadData = &rxTexture::Loader;
			assetCallbacks.finalize = &rxTexture::Online;
			assetCallbacks.bringOut = &rxTexture::Offline;
			assetCallbacks.freeData = &rxTexture::Unloader;
		}
		{
			Assets::AssetMetaType& assetCallbacks = Assets::gs_assetTypes[AssetTypes::MATERIAL];
			assetCallbacks.loadData = &rxMaterial::Load;
			assetCallbacks.finalize = &rxMaterial::Online;
			assetCallbacks.bringOut = &rxMaterial::Offline;
			assetCallbacks.freeData = &rxMaterial::Destruct;
		}

		//LuaModule	module;
		//{
		//	module.name = "GL";
		//	module.functions = gs_functions;
		//	module.numFunctions = mxCOUNT_OF(gs_functions);
		//}
		//Lua_RegisterModule( Scripting::GetLuaState(), module );

		return ALL_OK;
	}

	HInputLayout	g_inputLayouts[VTX_MAX];
	HSamplerState	g_samplers[Sampler_MAX];

	ERet InitializeGlobals( const Clump& rendererData )
	{
		{
			VertexDescription	vertexDescription;
			{
				vertexDescription.Begin();
				vertexDescription.Add(AttributeType::Float, 4, VertexAttribute::Position);
				vertexDescription.End();
				g_inputLayouts[VTX_Pos4F] = llgl::CreateInputLayout(vertexDescription,"Pos4F");
			}
			{
				DrawVertex::BuildVertexDescription( vertexDescription );
				g_inputLayouts[VTX_Draw] = llgl::CreateInputLayout(vertexDescription,"DrawVertex");
			}
		}
		{
			FxSamplerState* samplerState;

			mxDO(GetByName(rendererData,"Point",samplerState));
			mxASSERT(samplerState->handle.IsValid());
			g_samplers[PointSampler] = samplerState->handle;

			mxDO(GetByName(rendererData,"Bilinear",samplerState));
			mxASSERT(samplerState->handle.IsValid());
			g_samplers[BilinearSampler] = samplerState->handle;

			mxDO(GetByName(rendererData,"Trilinear",samplerState));
			mxASSERT(samplerState->handle.IsValid());
			g_samplers[TrilinearSampler] = samplerState->handle;

			mxDO(GetByName(rendererData,"Anisotropic",samplerState));
			mxASSERT(samplerState->handle.IsValid());
			g_samplers[AnisotropicSampler] = samplerState->handle;


			g_samplers[DiffuseMapSampler] = g_samplers[TrilinearSampler];
		}

		return ALL_OK;
	}

	void DestroyGlobals()
	{
		for( int i = 0; i < mxCOUNT_OF(g_inputLayouts); i++ )
		{
			llgl::DeleteInputLayout(g_inputLayouts[i]);
			g_inputLayouts[i].SetNil();
		}		
	}

}//namespace Rendering

RendererBase::RendererBase()
{
	m_rendererData = nil;
	m_viewportWidth = 0;
	m_viewportHeight = 0;
}
RendererBase::~RendererBase()
{

}

ERet RendererBase::Initialize( Clump* rendererData )
{
	ptPRINT("RendererBase::Initialize");

	m_rendererData = rendererData;

	m_hRenderContext = llgl::GetMainContext();

	m_hCBPerFrame = llgl::CreateBuffer(Buffer_Uniform,sizeof(G_PerFrame));
	m_hCBPerCamera = llgl::CreateBuffer(Buffer_Uniform,sizeof(G_PerCamera));
	m_hCBPerObject = llgl::CreateBuffer(Buffer_Uniform,sizeof(G_PerObject));	

	// Initialization order:
	// 1) Render targets
	// 2) State objects
	// 3) Shaders
	// 4) Input layouts
	// 5) Everything else

	// render targets should be sorted by size
	// shader resources should be sorted by size
	// constant buffers should be sorted by size

//	mxDO(Rendering::CreateRenderTargets(screen, *m_rendererData, false));

	mxDO(Rendering::CreateRenderResources(*m_rendererData));

	mxDO(Rendering::InitializeGlobals(*m_rendererData));

	return ALL_OK;
}

void RendererBase::Shutdown()
{
	ptPRINT("RendererBase::Shutdown");

	Rendering::ReleaseRenderTargets(*m_rendererData);
	Rendering::ReleaseRenderResources(*m_rendererData);

	llgl::DeleteBuffer(m_hCBPerFrame);
	llgl::DeleteBuffer(m_hCBPerCamera);
	llgl::DeleteBuffer(m_hCBPerObject);
	
	Rendering::DestroyGlobals();
}
ERet RendererBase::DrawFullScreenTriangle( FxShader* shader )
{
	llgl::DrawCall	batch;
	batch.Clear();
	SetGlobalUniformBuffers( &batch );
	FxApplyShaderState(batch,*shader);

	batch.topology = Topology::TriangleList;
	batch.baseVertex = 0;
	batch.vertexCount = 3;
	batch.startIndex = 0;
	batch.indexCount = 0;

	llgl::Submit( m_hRenderContext, batch );

	return ALL_OK;
}
ERet RendererBase::DrawFullScreenQuad( FxShader* shader )
{
	llgl::DrawCall	batch;
	batch.Clear();
	SetGlobalUniformBuffers( &batch );
	FxApplyShaderState(batch,*shader);

	batch.topology = Topology::TriangleStrip;
	batch.baseVertex = 0;
	batch.vertexCount = 4;
	batch.startIndex = 0;
	batch.indexCount = 0;

	llgl::Submit( m_hRenderContext, batch );

	return ALL_OK;
}
void RendererBase::DBG_Draw_Models_With_Custom_Shader(
	const SceneView& sceneView,
	const Clump& sceneData,
	const FxShader& shader,
	TopologyT overrideTopology
	)
{
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


		llgl::DrawCall	batch;
		batch.Clear();

		SetGlobalUniformBuffers( &batch );
		FxApplyShaderState(batch, shader);

		batch.inputLayout = Rendering::g_inputLayouts[VTX_Draw];
		batch.topology = (overrideTopology != Topology::Undefined) ? overrideTopology : mesh->m_topology;

		batch.VB[0] = mesh->m_vertexBuffer;
		batch.IB = mesh->m_indexBuffer;
		batch.b32bit = (mesh->m_indexStride == sizeof(UINT32));

		batch.baseVertex = 0;
		batch.vertexCount = mesh->m_numVertices;
		batch.startIndex = 0;
		batch.indexCount = mesh->m_numIndices;

		llgl::Submit(m_hRenderContext, batch);


		modelIt.MoveToNext();
	}
}
void RendererBase::DBG_Draw_Models_Wireframe(
	const SceneView& sceneView,
	const Clump& sceneData,
	const FxShader& shader
)
{
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


		llgl::DrawCall	batch;
		batch.Clear();

		SetGlobalUniformBuffers( &batch );
		FxApplyShaderState(batch, shader);

		batch.inputLayout = Rendering::g_inputLayouts[VTX_Draw];
		batch.topology = mesh->m_topology;

		batch.VB[0] = mesh->m_vertexBuffer;
		batch.IB = mesh->m_indexBuffer;
		batch.b32bit = (mesh->m_indexStride == sizeof(UINT32));

		batch.baseVertex = 0;
		batch.vertexCount = mesh->m_numVertices;
		batch.startIndex = 0;
		batch.indexCount = mesh->m_numIndices;

		llgl::Submit(m_hRenderContext, batch);

		modelIt.MoveToNext();
	}
}
void RendererBase::SetGlobalUniformBuffers( llgl::DrawCall *batch )
{
	batch->CBs[G_PerFrame_Index] = m_hCBPerFrame;
	batch->CBs[G_PerCamera_Index] = m_hCBPerCamera;
	batch->CBs[G_PerObject_Index] = m_hCBPerObject;
}
void RendererBase::BindMaterial( const rxMaterial* material, llgl::DrawCall *batch )
{
	const FxShader* shader = material->m_shader;

	// update shader constant buffers with material data
	if( shader->localCBs.Num() )
	{
		mxASSERT(shader->localCBs.Num()==1);
		const ParameterBuffer& uniforms = material->m_uniforms;
		const FxCBuffer& rCB = shader->localCBs[0];
		llgl::UpdateBuffer(m_hRenderContext, rCB.handle, uniforms.GetDataSize(), uniforms.ToPtr() );
	}

	SetGlobalUniformBuffers( batch );

	const UINT32 numCBs = shader->CBs.Num();
	for( UINT32 iCB = 0; iCB < numCBs; iCB++ )
	{
		const FxCBufferBinding& binding = shader->CBs[ iCB ];
		const FxCBuffer& cbuffer = shader->localCBs[ binding.id ];
		batch->CBs[ binding.slot ] = cbuffer.handle;
	}
	const UINT32 numTSs = shader->SRs.Num();
	for( UINT32 iTS = 0; iTS < numTSs; iTS++ )
	{
		const TextureLayer& layer = material->m_textures[ iTS ]; 
		const rxTexture* texture = layer.texture;

		const FxTextureBinding& binding = shader->SRs[ iTS ];
		const FxResource& resource = shader->localSRs[ binding.id ];

		//batch->SRs[ binding.slot ] = resource.texture;
		//batch->SSs[ binding.slot ] = resource.sampler;
		batch->SRs[ binding.slot ] = texture ? texture->m_resource : resource.texture;
		batch->SSs[ binding.slot ] = Rendering::g_samplers[ layer.sampler ];
	}
	int shaderIndex = 0;
	batch->program = shader->programs[ shaderIndex ];
}

SimpleRenderer::SimpleRenderer()
{

}
SimpleRenderer::~SimpleRenderer()
{

}
ERet SimpleRenderer::Initialize( Clump* rendererData )
{
	mxDO(Super::Initialize(rendererData));
	return ALL_OK;
}
void SimpleRenderer::Shutdown()
{
	Super::Shutdown();
}
ERet SimpleRenderer::RenderScene( const SceneView& sceneView, const Clump& sceneData )
{
	if( sceneView.viewportWidth != m_viewportWidth || sceneView.viewportHeight != m_viewportHeight )
	{
		this->ResizeBuffers( sceneView.viewportWidth, sceneView.viewportHeight, false );
		m_viewportWidth = sceneView.viewportWidth;
		m_viewportHeight = sceneView.viewportHeight;
	}

	FxDepthTarget* pDepthRT;
	mxDO(GetByName(*m_rendererData, "MainDepthStencil", pDepthRT));

	llgl::ViewState	viewState;
	{
		viewState.Reset();
		viewState.colorTargets[0].SetDefault();
		viewState.targetCount = 1;
		viewState.depthTarget = pDepthRT->handle;
		viewState.flags = llgl::ClearAll;
	}
	llgl::SubmitView(m_hRenderContext, viewState);


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

			{
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

	return ALL_OK;
}

ERet SimpleRenderer::ResizeBuffers( UINT16 width, UINT16 height, bool fullscreen )
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
