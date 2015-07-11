#pragma once

#include <Core/Input.h>

// low-level system-generated event
enum ESystemEventType
{
	SysEvent_None = 0,
	SysEvent_Key,		// A key was pressed or released
	SysEvent_MouseMove,	// The mouse cursor moved
	SysEvent_Joystick,
	SysEvent_Gamepad,
	SysEvent_Window,	// Application events
};

enum EWindowEventType
{
	WinEvent_Exiting,	// user-requested quit, the application is terminating
};

struct WindowEvent
{
	EWindowEventType	type;
};

//
// low-level system-generated event
//
struct SystemEvent
{
	ESystemEventType	type;	// EInputEventType
	//UINT32	timestamp;	// timestamp of the event
	union
	{
		//InputEventBase		common;
		KeyButtonEvent		button;
		MouseEventBase		mouse;
		MouseMotionEvent	motion;
		JoystickEvent		joystick;
		GamepadEvent		gamepad;
		TextInputEvent		text;
		WindowEvent			window;	// SysEvent_Window
	};
};

namespace EventSystem
{
	void Initialize();
	void Shutdown();

	// event generation
	void		PostEvent( const SystemEvent& _event );
	void		ClearEvents();
	SystemEvent	GetEvent();
}//namespace EventSystem
