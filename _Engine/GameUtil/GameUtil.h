#pragma once

#if MX_AUTOLINK
#pragma comment( lib, "GameUtil.lib" )
#endif //MX_AUTOLINK

#include <EngineUtil/EngineUtil.h>
#include <EngineUtil/ImGUI_Renderer.h>

class MainMode : public AClientState {
public:
	ERet Initialize();
	void Shutdown();

	virtual int GetStateMask() override
	{ return BIT(GameStates::main); }

	virtual bool NeedsRelativeMouseMode() const override
	{return true;}
};

class MenuState : public AClientState {
public:
	virtual int GetStateMask() override
	{ return BIT(GameStates::escape_menu); }

	virtual void Update( double deltaSeconds ) override
	{};

	virtual void RenderGUI() override
	{};
};

class DevConsoleState : public AClientState {
public:
	virtual int GetStateMask() override
	{ return BIT(GameStates::dev_console); }

	virtual void Update( double deltaSeconds ) override;

	virtual void RenderGUI() override;
};

#if 0
class EscapeMenu : public AClientState {
	GUI_Renderer	m_gui;
public:
	ERet Initialize( Clump* shaders );
	void Shutdown();
	virtual int GetStateMask() { return BIT(GameStates::escape_menu); }
	virtual void Update( double deltaSeconds ) override;
	virtual void Render() override;
};
#endif
class StateManager
{
	TStaticList_InitZeroed< AClientState*, 8 >	m_stateStack;

public:
	StateManager() {}
	~StateManager() {}

	ERet Initialize();
	void Shutdown();

	// returns the top-level state
	AClientState* GetCurrentState();

	void PushState( AClientState* state );
	AClientState* PopState();

#if 0
        // Creating and destroying the state machine
        void Init();
        void Cleanup();

        // Transit between states
        void ChangeState(CGameState* state);
        void PushState(CGameState* state);
        void PopState();

        // The three important actions within a game loop
        // (these will be handled by the top state in the stack)
        void HandleEvents();
        void Update();
        void Draw();
#endif
};
