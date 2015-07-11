/*
=============================================================================
	File:	Client.h
	Desc:	
=============================================================================
*/
#pragma once

//@this is bad: TCallback<> is used for input action binding
#include <Base/Template/Delegate/Delegate.h>

#include <Core/Input.h>


namespace GameStates {
	enum EGameStateIDs {
#define DECLARE_GAME_STATE( name, description )		name,
#include <Core/GameStates.inl>
#undef DECLARE_GAME_STATE
		ALL = -1,
		MAX_STATES
	};
	mxSTATIC_ASSERT2(MAX_STATES < 32, Must_Fit_Into_Int32);
}//namespace GameStates

mxDECLARE_ENUM( GameStates::EGameStateIDs, UINT8, GameStateID );
mxDECLARE_FLAGS( GameStates::EGameStateIDs, UINT32, GameStateF );

/*
=============================================================================
	INPUT ACTION MAPPING
	(inspired by CryENGINE's approach)
=============================================================================
*/
namespace GameActions {
	enum EGameActionIDs {
#define DECLARE_GAME_ACTION( name, description )		name,
#include <Core/GameActions.inl>
#undef DECLARE_GAME_ACTION
		MAX_ACTIONS
	};
	mxSTATIC_ASSERT2(MAX_ACTIONS < 256, Some_Code_Relies_On_It);
}//namespace GameActions

mxDECLARE_ENUM( GameActions::EGameActionIDs, UINT8, GameActionID );

typedef TCallback< void ( GameActionID, EInputState, float ) >	ActionHandlerT;
typedef TKeyValue< GameActionID, ActionHandlerT >				ActionBindingT;

/*
-----------------------------------------------------------------------------
	KeyBind
-----------------------------------------------------------------------------
*/
struct KeyBind : public CStruct
{
	KeyCodeT		key;	//1 Name of the key (keyboard, mouse or joystick key) bind
	FInputStateT	when;	//1 bitmask (EInputKeyEvent)
	FKeyModifiersT	with;	//2 bitmask (FKeyModifiers) of keys that need to be held down for this key bind to activate
	GameActionID	action;	//1 Command to execute when this key bind is activated
public:
	mxDECLARE_CLASS(KeyBind,CStruct);
	mxDECLARE_REFLECTION;
	KeyBind();
};

/*
-----------------------------------------------------------------------------
	AxisBind
-----------------------------------------------------------------------------
*/
struct AxisBind : public CStruct
{
	InputAxisT		axis;	//1 mouse or joystick axis
	GameActionID	action;	//1 Command to execute when this binding is activated
public:
	mxDECLARE_CLASS(AxisBind,CStruct);
	mxDECLARE_REFLECTION;
	AxisBind();
};

/*
-----------------------------------------------------------------------------
	InputContext
	e.g. menu, in-game, PDA, vehicle, helicopter, cut scene, spectator, edit
-----------------------------------------------------------------------------
*/
struct InputContext : public CStruct
{
	String				name;
	TArray< KeyBind >	key_binds;
	TArray< AxisBind >	axis_binds;
	GameStateF			state_mask;
public:
	mxDECLARE_CLASS(InputContext,CStruct);
	mxDECLARE_REFLECTION;
	InputContext();
};


namespace ClientCallbacks
{
	struct Data
	{
		TArray< ActionBindingT >	handlers;
		TArray< InputContext >	contexts;
	};

	ERet Initialize();
	void Shutdown();

	Data& GetData();

	//ERet BindKey( EKeyCode key, const char* action );
	//const char* GetKeyBinding( EKeyCode key );

}//namespace ClientCallbacks

/*
-----------------------------------------------------------------------------
	AViewport
	interface to a platform-specific window implementation
-----------------------------------------------------------------------------
*/
class AViewport {
public:
	virtual void* VGetWindowHandle() = 0;

	virtual void VGetSize( int &_w, int &_h ) = 0;
	virtual bool VIsFullscreen() const = 0;

	virtual bool VIsKeyPressed( EKeyCode key ) const = 0;
	virtual void VGetMousePosition( int &x, int &y ) const = 0;

	virtual float VGetTabletPressure() const { return 0.0f; }

protected:
	virtual ~AViewport() {}
};

/*
-----------------------------------------------------------------------------
	game/editor/whatever render viewports classes can inherit
	from this abstract interface to receive messages from the window
	(this is used for e.g. GUI window -> renderer viewport communication)
-----------------------------------------------------------------------------
*/
class AViewportClient : public AObject {
public:
	// user input event handling
	virtual void VOnInputEvent( AViewport* _window, const SInputEvent& _event )
	{}

	//// The mouse cursor entered the area of the window
	//virtual void VOnMouseEntered( AViewport* _window )
	//{}
	//// The mouse cursor left the area of the window
	//virtual void VOnMouseLeft( AViewport* _window )
	//{}

	// 'The application will come to foreground.'
	virtual void VOnFocusGained( AViewport* _window )
	{}
	// 'The application will come to background.'
	virtual void VOnFocusLost( AViewport* _window )
	{}

	// this function gets called when the parent window's size changes and this viewport should be resized
	virtual void VOnResize( AViewport* _window, int _width, int _height, bool _fullScreen )
	{}

	// the containing widget is about to be closed; shutdown everything (but keep the object around).
	virtual void VOnClose( AViewport* _window )
	{}

	// the containing viewport widget requested redraw
	virtual void VOnRePaint( AViewport* _window )
	{}

	virtual ECursorShape VGetCursorShape( AViewport* _window, int _x, int _y )
	{
		return CursorShape_Arrow;
	}

protected:
	AViewportClient()
	{}
	virtual ~AViewportClient()
	{}
};

//
// e.g. 'Menu', 'MainGame', 'Vehicle'
//
class AClientState {
public:
	// Setup and destroy the state
//	virtual ERet Initialize() { return ALL_OK; }
//	virtual void Shutdown() {};

	// input events will be processed only if (current_states & state_mask) != 0
	virtual int GetStateMask()
	{ return GameStates::ALL; }

	virtual void Update( double deltaSeconds )
	{};

	// e.g. renders demo HUD
	virtual void RenderGUI()
	{};

	virtual bool NeedsRelativeMouseMode() const
	{return false;}

#if 0
	// Setup and destroy the state
	void Init();
	void Cleanup();

	// Used when temporarily transitioning to another state
	void Pause();
	void Resume();

	// The three important actions within a game loop
	void HandleEvents();
	void Update();
	void Draw();
#endif

protected:
	virtual ~AClientState() {}
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
