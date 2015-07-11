#include <Base/Base.h>
#pragma hdrstop
#include <Driver/Driver.h>
#include <Graphics/Device.h>
#include <Graphics/Utils.h>
#include <EffectCompiler2/Effect_Compiler.h>
#include <DemoFramework/DemoFramework.h>

#include <TxTSupport/TxTSerializers.h>
#include <TxTSupport/TxTReader.h>

namespace DemoUtil
{

#if 0
ERet CreateEffectFromFile( const char* filepath, int width, int height, Clump &_clump )
{
	Resolution resolution = { width, height };

	String96	filename;
	Str::CopyS(filename, filepath);
	Str::StripPath(filename);

	FxOptions	options;
#if DEV_BUILD
	options.debugDumpPath.SetReference("R:/");
#endif

	ByteArrayT	output;
	mxDO(FxCompileEffectFromFile(filepath, output, options));

#if DEV_BUILD
	Util_SaveDataToFile(output.ToPtr(), output.Num(), Str::ComposeFilePath<String96>("R:",filename.ToPtr(), mxUSE_BINARY_EFFECT_FILE_FORMAT?".fxb":".son").ToPtr());
#endif

	{
		_clump.RemoveAll(FxRenderTargetBase::MetaClass());
		_clump.RemoveAll(FxDepthStencilState::MetaClass());
		_clump.RemoveAll(FxRasterizerState::MetaClass());
		_clump.RemoveAll(FxSamplerState::MetaClass());
		_clump.RemoveAll(FxBlendState::MetaClass());
		_clump.RemoveAll(FxStateBlock::MetaClass());
		_clump.RemoveAll(FxRenderResources::MetaClass());

		_clump.RemoveAll(FxBackingStore::MetaClass());
		_clump.RemoveAll(FxUniform::MetaClass());
		_clump.RemoveAll(FxCBuffer::MetaClass());
		_clump.RemoveAll(FxResource::MetaClass());

		_clump.RemoveAll(FxShader::MetaClass());
	}

	MemoryReader	effectStream(output.ToPtr(), output.Num());
	mxDO(FxUtil_LoadShaderLibrary(effectStream, _clump, resolution));

	return ALL_OK;
}
#endif

rxMesh* CreateMesh( Clump * clump, const TArray<FatVertex1>& vertices, const TArray<int>& indices )
{
	rxMesh* mesh = clump->New< rxMesh >();

	const UINT32 vertexBufferSize = vertices.Num() * sizeof(DrawVertex);
	const UINT32 indexBufferSize = indices.Num() * sizeof(UINT32);
	{
		ScopedStackAlloc	tempAlloc( gCore.frameAlloc );
		void* vertexData = tempAlloc.AllocA( vertexBufferSize );
		for( int i = 0; i < vertices.Num(); i++ )
		{
			const FatVertex1& src = vertices[i];
			DrawVertex & dst = ((DrawVertex*)vertexData)[i];

			dst.xyz = src.position;
			dst.st = Float2_To_Half2( src.texCoord );

			dst.N = PackNormal( src.normal );
			dst.T = PackNormal( src.tangent );
		}

		mesh->m_vertexBuffer = llgl::CreateBuffer( Buffer_Vertex, vertexBufferSize, vertexData );
		mesh->m_numVertices = vertices.Num();
	}
	{
		mesh->m_indexBuffer = llgl::CreateBuffer( Buffer_Index, indexBufferSize, indices.ToPtr() );
		mesh->m_indexStride = sizeof(UINT32);
		mesh->m_topology = Topology::TriangleList;
		mesh->m_numIndices = indices.Num();
	}
	{
		rxSubmesh& submesh = mesh->m_parts.Add();
		submesh.startIndex = 0;
		submesh.indexCount = indices.Num();
		submesh.baseVertex = 0;
		submesh.vertexCount = vertices.Num();
	}
	return mesh;
}

void DBG_PrintClump( const Clump& _clump )
{
	ObjectList::Head currentList = _clump.GetObjectLists();
	while( currentList != NULL )
	{
		const UINT32 objectCount = currentList->Num();
		const mxClass& objectType = currentList->GetType();

		DBGOUT("ObjectList: count=%u, type='%s'",objectCount,objectType.GetTypeName());

		//CStruct* objectsArray = currentList->GetArrayPtr();
		//const UINT32 arrayStride = currentList->GetStride();

		currentList = currentList->_next;
	}
}

void DemoApp::ChangeGameState( AClientState* state )
{
	//bool grabsMouse = state->GrabsMouseInput();
	//bool isMainMode = ((state->GetStateMask() & BIT(GameStates::main)) != 0);
	//bool isGuiMode = !isMainMode;
	bool grabsMouse = state->NeedsRelativeMouseMode();
	bool isGuiMode = !grabsMouse;

	bool allowModeSwitch = true;

	if( !isGuiMode )
	{
		// allow mode switch if we are in main game mode (e.g. fly, walk)
	}
	else
	{
		// If we're in GUI mode and interacting with a window, don't switch mode:
		if( ImGui::IsMouseHoveringAnyWindow() || ImGui::IsAnyItemActive() ) {
			allowModeSwitch = false;
		}
	}

	if( allowModeSwitch )
	{
		if( stateMgr.GetCurrentState() != state ) {
			stateMgr.PushState( state );
		} else {
			stateMgr.PopState();
		}

		AClientState* nextState = stateMgr.GetCurrentState();
		if( nextState ) {
			WindowsDriver::SetRelativeMouseMode(nextState->NeedsRelativeMouseMode());
		}
	}
	else
	{
		DBGOUT("allowModeSwitch = false, isGuiMode: %d",isGuiMode);
	}
}

DemoApp::DemoApp()
{
	struct MyExceptionCallback : AExceptionCallback
	{
		TLocalString< 2046 >	m_stackTrace;

		virtual void AddStackEntry( const char* text ) override
		{
			Str::AppendS( m_stackTrace, text );
		}
		virtual void OnException() override
		{
			if( m_stackTrace.Length() )
			{
				mxGetLog().VWrite(LL_Error, m_stackTrace.ToPtr(), m_stackTrace.Length());
				::MessageBoxA(NULL, m_stackTrace.ToPtr(), "ERROR", MB_OK);
			}
		}
	};
	static MyExceptionCallback myExceptionCallback;
	g_exceptionCallback = &myExceptionCallback;
}

DemoApp::~DemoApp()
{

}

ERet DemoApp::__Initialize()
{
	//mxDO(Scripting::Initialize());
	mxDO(ClientCallbacks::Initialize());

	{
		ClientCallbacks::Data& input = ClientCallbacks::GetData();
		SON::LoadFromFile(gUserINI->GetString("path_to_input_bindings","input_mappings.son"), input.contexts);

		input.handlers.Add( ActionBindingT(GameActions::attack1, mxBIND_MEMBER_FUNCTION(DemoApp,attack1,*this)) );
		input.handlers.Add( ActionBindingT(GameActions::attack2, mxBIND_MEMBER_FUNCTION(DemoApp,attack2,*this)) );
		input.handlers.Add( ActionBindingT(GameActions::attack3, mxBIND_MEMBER_FUNCTION(DemoApp,attack3,*this)) );

		input.handlers.Add( ActionBindingT(GameActions::take_screenshot, mxBIND_MEMBER_FUNCTION(DemoApp,take_screenshot,*this)) );

		input.handlers.Add( ActionBindingT(GameActions::dev_toggle_console, mxBIND_MEMBER_FUNCTION(DemoApp,dev_toggle_console,*this)) );
		input.handlers.Add( ActionBindingT(GameActions::dev_toggle_menu, mxBIND_MEMBER_FUNCTION(DemoApp,dev_toggle_menu,*this)) );

		input.handlers.Add( ActionBindingT(GameActions::dev_reload_shaders, mxBIND_MEMBER_FUNCTION(DemoApp,dev_reload_shaders,*this)) );
		input.handlers.Add( ActionBindingT(GameActions::dev_reset_state, mxBIND_MEMBER_FUNCTION(DemoApp,dev_reset_state,*this)) );
		input.handlers.Add( ActionBindingT(GameActions::dev_save_state, mxBIND_MEMBER_FUNCTION(DemoApp,dev_save_state,*this)) );
		input.handlers.Add( ActionBindingT(GameActions::dev_load_state, mxBIND_MEMBER_FUNCTION(DemoApp,dev_load_state,*this)) );

		input.handlers.Add( ActionBindingT(GameActions::dev_action0, mxBIND_MEMBER_FUNCTION(DemoApp,dev_toggle_raytrace,*this)) );
		input.handlers.Add( ActionBindingT(GameActions::dev_action1, mxBIND_MEMBER_FUNCTION(DemoApp,dev_do_action1,*this)) );
		input.handlers.Add( ActionBindingT(GameActions::dev_action2, mxBIND_MEMBER_FUNCTION(DemoApp,dev_do_action2,*this)) );
		input.handlers.Add( ActionBindingT(GameActions::dev_action3, mxBIND_MEMBER_FUNCTION(DemoApp,dev_do_action3,*this)) );
		input.handlers.Add( ActionBindingT(GameActions::dev_action4, mxBIND_MEMBER_FUNCTION(DemoApp,dev_do_action4,*this)) );

		//input.handlers.Add( ActionBindingT(GameActions::exit_app, mxBIND_FREE_FUNCTION(request_exit)) );
		input.handlers.Add( ActionBindingT(GameActions::exit_app, mxBIND_MEMBER_FUNCTION(DemoApp,request_exit,*this)) );
	}


	mxDO(mainMode.Initialize());
	//mxDO(escapeMenu.Initialize(&m_sceneData));

	mxDO(stateMgr.Initialize());

	stateMgr.PushState( &mainMode );


	const char* pathToAssets = gUserINI->GetString("path_to_compiled_assets");

	//if(BUILD_ASSETS)
	//{
	//	AssetPipeline::AssetCompiler	assetCompiler;

	//	mxDO(assetCompiler.Initialize());
	//	{
	//		mxDO(assetCompiler.BuildAssets( gUserINI->GetString("path_to_source_assets"), pathToAssets ));
	//	}
	//	assetCompiler.Shutdown();
	//}



	mxDO(m_AssetFolder.Initialize());
	mxDO(m_AssetFolder.Mount(pathToAssets));

	return ALL_OK;
}

void DemoApp::__Shutdown()
{
	m_AssetFolder.Unmount();
	m_AssetFolder.Shutdown();

	mainMode.Shutdown();

	stateMgr.Shutdown();

	ClientCallbacks::Shutdown();
	//Scripting::Shutdown();
}
void DemoApp::Update(const double deltaSeconds)
{
	AClientState* currentState = stateMgr.GetCurrentState();
	currentState->Update(deltaSeconds);

	AssetHotReloader callback;
	m_AssetFolder.ProcessChangedAssets(&callback);
}
// Input bindings
void DemoApp::attack1( GameActionID action, EInputState status, float value )
{

}
void DemoApp::attack2( GameActionID action, EInputState status, float value )
{

}
void DemoApp::attack3( GameActionID action, EInputState status, float value )
{

}
void DemoApp::take_screenshot( GameActionID action, EInputState status, float value )
{
}
void DemoApp::dev_toggle_console( GameActionID action, EInputState status, float value )
{
	ChangeGameState( &devConsole );
}
void DemoApp::dev_toggle_menu( GameActionID action, EInputState status, float value )
{
	//ChangeGameState( &menuState );
}
void DemoApp::dev_reset_state( GameActionID action, EInputState status, float value )
{
}
void DemoApp::dev_reload_shaders( GameActionID action, EInputState status, float value )
{
}
void DemoApp::dev_toggle_raytrace( GameActionID action, EInputState status, float value )
{
}
void DemoApp::dev_save_state( GameActionID action, EInputState status, float value )
{
}
void DemoApp::dev_load_state( GameActionID action, EInputState status, float value )
{
}
void DemoApp::dev_do_action1( GameActionID action, EInputState status, float value )
{
}
void DemoApp::dev_do_action2( GameActionID action, EInputState status, float value )
{
}
void DemoApp::dev_do_action3( GameActionID action, EInputState status, float value )
{
}
void DemoApp::dev_do_action4( GameActionID action, EInputState status, float value )
{
}
void DemoApp::request_exit( GameActionID action, EInputState status, float value )
{
	WindowsDriver::RequestExit();
}

}//namespace DemoUtil
