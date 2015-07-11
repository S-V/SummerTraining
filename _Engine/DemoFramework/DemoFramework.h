#pragma once

#if MX_AUTOLINK
#pragma comment( lib, "DemoFramework.lib" )
#endif //MX_AUTOLINK

#include <Core/ObjectModel.h>

#include <Renderer/Renderer.h>
#include <Renderer/Mesh.h>
#include <Meshok/Meshok.h>

#include <EngineUtil/EngineUtil.h>
#include <EngineUtil/ImGUI_Renderer.h>

#include <GameUtil/GameUtil.h>

#include <EditorSupport/DevAssetFolder.h>

namespace DemoUtil
{

//ERet CreateEffectFromFile( const char* filepath, int width, int height, Clump &_clump );

rxMesh* CreateMesh( Clump * clump, const TArray<FatVertex1>& vertices, const TArray<int>& indices );

void DBG_PrintClump( const Clump& _clump );

struct DemoApp : SingleInstance< DemoApp >
{
	DevAssetFolder	m_AssetFolder;

	MainMode		mainMode;	// main game mode
	//MenuState		menuState;	// shown when escape is pressed (<= app-specific)
	DevConsoleState	devConsole;	// shown when tilde is pressed

	StateManager	stateMgr;

public:
	void ChangeGameState( AClientState* state );

	// Input bindings
	virtual void attack1( GameActionID action, EInputState status, float value );
	virtual void attack2( GameActionID action, EInputState status, float value );
	virtual void attack3( GameActionID action, EInputState status, float value );
	virtual void take_screenshot( GameActionID action, EInputState status, float value );
	virtual void dev_toggle_console( GameActionID action, EInputState status, float value );
	virtual void dev_toggle_menu( GameActionID action, EInputState status, float value );
	virtual void dev_reset_state( GameActionID action, EInputState status, float value );
	virtual void dev_reload_shaders( GameActionID action, EInputState status, float value );
	virtual void dev_toggle_raytrace( GameActionID action, EInputState status, float value );
	virtual void dev_save_state( GameActionID action, EInputState status, float value );
	virtual void dev_load_state( GameActionID action, EInputState status, float value );
	virtual void dev_do_action1( GameActionID action, EInputState status, float value );
	virtual void dev_do_action2( GameActionID action, EInputState status, float value );
	virtual void dev_do_action3( GameActionID action, EInputState status, float value );
	virtual void dev_do_action4( GameActionID action, EInputState status, float value );
	virtual void request_exit( GameActionID action, EInputState status, float value );

protected:
	DemoApp();
	virtual ~DemoApp();

	ERet __Initialize();
	void __Shutdown();

	virtual void Update(const double deltaSeconds);
};

}//namespace DemoUtil
