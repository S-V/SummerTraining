/*
=============================================================================
	File:	Input.h
	Desc:	Input system interfaces.
=============================================================================
*/
#pragma once

/*
=============================================================================
	INPUT SYSTEM
=============================================================================
*/
mxSWIPED("parts stolen from CryEngine SDK and Unreal Engine 4");

enum EInputDevice
{
	InputDevice_Unknown = 0,
	InputDevice_Keyboard,
	InputDevice_Mouse,
	InputDevice_Joystick,
	InputDevice_Gamepad,
};

/*
=============================================================================
	VIRTUAL KEY CODES AND KEYBOARD
=============================================================================
*/

//
//	EKeyCode - enumeration of virtual/logical key codes.
//
enum EKeyCode {
#define mxDEFINE_KEYCODE( name )		KEY_##name
#include "KeyCodes.inl"
#undef mxDEFINE_KEYCODE
	MAX_KEYS	// Marker. Do not use.
};
mxDECLARE_ENUM( EKeyCode, UINT8, KeyCodeT );
mxSTATIC_ASSERT2(MAX_KEYS < 256, Some_Code_Relies_On_It);

const char* EKeyCode_To_Chars( EKeyCode keyCode );
EKeyCode FindKeyByName(const char* keyname);

// input button events
enum EInputKeyEvent
{
	IA_Pressed			= 0,	// the key was pressed
	IA_Released			= 1,	// the key was released	
	IA_Repeated			= 2,	// the user is holding down the key (keyboard only)
	IA_DoubleClicked	= 3,	// the mouse button was double-clicked (mouse only)
};
mxDECLARE_ENUM( EInputKeyEvent, UINT8, InputKeyEventT );

// this is used for mapping (binding) input events to certain actions
enum EInputState
{
	IS_Pressed			= 0,	// the button was pressed
	IS_Released			= 1,	// the button was released
	IS_HeldDown			= 2,	// the button is being held down
	IS_Changed			= 3,	// e.g. mouse cursor coordinates changed
};
mxDECLARE_FLAGS( EInputState, UINT8, FInputStateT );

/*
-----------------------------------------------------------------------------
	FKeyModifiers
-----------------------------------------------------------------------------
*/
enum FKeyModifiers
{
	KeyModifier_None			= 0,
	KeyModifier_LCtrl			= BIT(0),	///< Is the left Control key pressed?
	KeyModifier_LShift			= BIT(1),	///< Is the left Shift key pressed?
	KeyModifier_LAlt			= BIT(2),	///< Is the left Alt key pressed?
	KeyModifier_LWin			= BIT(3),	///< Is the left Windows logo key pressed?
	KeyModifier_RCtrl			= BIT(4),	///< Is the right Control key pressed?
	KeyModifier_RShift			= BIT(5),	///< Is the right Shift key pressed?
	KeyModifier_RAlt			= BIT(6),	///< Is the right Alt key pressed?
	KeyModifier_RWin			= BIT(7),	///< Is the right Windows logo key pressed?
	KeyModifier_NumLock			= BIT(8),	///< Is the Num Lock key pressed?
	KeyModifier_CapsLock		= BIT(9),	///< Is the Caps Lock key pressed?
	KeyModifier_ScrollLock		= BIT(10),	///< Is the Scroll Lock key pressed?
	//These should only be used in C++ code to test for flag combinations:
	KeyModifier_Ctrl			= (KeyModifier_LCtrl | KeyModifier_RCtrl),	// True if ctrl was also pressed.
	KeyModifier_Shift			= (KeyModifier_LShift | KeyModifier_RShift),// True if shift was also pressed.
	KeyModifier_Alt				= (KeyModifier_LAlt | KeyModifier_RAlt),	// True if alt was also pressed.
	KeyModifier_Win				= (KeyModifier_LWin | KeyModifier_RWin),
	KeyModifier_Modifiers		= (KeyModifier_Ctrl | KeyModifier_Shift | KeyModifier_Alt | KeyModifier_Win),
	KeyModifier_LockKeys		= (KeyModifier_CapsLock | KeyModifier_NumLock | KeyModifier_ScrollLock)
};
mxDECLARE_FLAGS( FKeyModifiers, UINT16, FKeyModifiersT );

/*
=============================================================================
	MOUSE
=============================================================================
*/
enum EMouseButton
{
	MouseButton_1         = 0,
	MouseButton_2         = 1,
	MouseButton_3         = 2,
	MouseButton_4         = 3,	// XButton1
	MouseButton_5         = 4,	// XButton2
	MouseButton_6         = 5,
	MouseButton_7         = 6,
	MouseButton_8         = 7,
	MouseButton_Last      = MouseButton_8,

	MouseButton_Left      = MouseButton_1,
	MouseButton_Right     = MouseButton_2,
	MouseButton_Middle    = MouseButton_3,
	MouseButton_X1        = MouseButton_4,
	MouseButton_X2        = MouseButton_5,
};

const Chars EMouseButton_To_Chars( EMouseButton button );

enum EInputAxis
{
	MouseAxisX,
	MouseAxisY,
	//JoystickAxisX,
	//JoystickAxisY,
	//JoystickAxisZ,
	InputAxesMAX
};
mxDECLARE_ENUM( EInputAxis, UINT8, InputAxisT );

/*
=============================================================================
	INTERFACES
=============================================================================
*/
struct InputEventBase
{
	// Bit field describing which modifier keys were held down (FKeyModifiers mask).
	UINT16		modifiers;
};
// A key was pressed or released
struct KeyButtonEvent : InputEventBase
{
	EKeyCode		key;		// The virtual key that was pressed or released (EKeyCode).
	//UINT32		scan;		// The system-specific scan code of the key.
	EInputKeyEvent	action;		// pressed, released or held down?
};
// A character was entered
struct TextInputEvent : InputEventBase
{
	UINT32	character;	/// The UTF-32 unicode value of the character
};
struct MouseEventBase : InputEventBase
{
	INT32	x, y;		// the absolute position of the mouse cursor in window coordinates
	UINT16	buttonMask;	// currently pressed mouse buttons (MouseButton_*)
};
// The mouse cursor moved
struct MouseMotionEvent : MouseEventBase
{
	INT32	deltaX;	// The relative motion in the X direction
	INT32	deltaY;	// The relative motion in the Y direction
};
//Mouse wheel is treated like a special button (see KeyButtonEvent)
//struct MouseWheelEvent : MouseEventBase
//{
//	INT32	wheelDelta;	// The amount scrolled vertically, positive away from the user and negative toward the user
//};
struct JoystickEvent : InputEventBase
{
	enum Type {
		ButtonPressed, /// A joystick button was pressed
		ButtonReleased, /// A joystick button was released
		Moved, /// The joystick moved along an axis
		Connected, /// A joystick was connected
		Disconnected, /// A joystick was disconnected
	};
	//...
};
struct GamepadEvent : InputEventBase
{
	//...
};

//
// low-level system-generated input event
//
struct SInputEvent
{
	UINT8	type;	// EInputEventType
	UINT8	device;	// EInputDevice
	union
	{
		InputEventBase		common;
		KeyButtonEvent		keyboard;
		MouseEventBase		mouse;
		MouseMotionEvent	motion;
		JoystickEvent		joystick;
		TextInputEvent		text;
	};
public:
	SInputEvent();
};

// this is used to provide visual hints when moused over a hit proxy
enum ECursorShape
{
	CursorShape_Arrow,			// The standard arrow cursor.
	CursorShape_PointingHand,	// A pointing hand cursor that is typically used for clickable elements such as hyperlinks.
	CursorShape_OpenHand,		// A cursor representing an open hand, typically used to indicate that the area under the cursor is the visible part of a canvas that the user can click and drag in order to scroll around.
	CursorShape_ClosedHand,		// A cursor representing a closed hand, typically used to indicate that a dragging operation is in progress that involves scrolling.
	CursorShape_Cross,			// A crosshair cursor, typically used to help the user accurately select a point on the screen.
	CursorShape_SizeAll,		// A cursor used for elements that are used to resize top-level windows in any direction.
};

namespace InputUtil
{
	//EKeyCode FindKeyByName(const char* keyname);
	String64 KeyModifiersToString( UINT32 modifiers );
	String64 MouseButtonMaskToString( UINT8 buttonMask );
	void PrintInputEvent( const SInputEvent& _event );
}//namespace InputUtil

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
