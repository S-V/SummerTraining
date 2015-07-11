/*
=============================================================================
	File:	Input.cpp
	Desc:	
	References:
	http://www.gamedev.net/page/resources/_/technical/game-programming/designing-a-robust-input-handling-system-for-games-r2975
	http://www.gamedev.net/page/resources/_/technical/general-programming/proper-input-implementation-r3959
	http://www.gamedev.net/blog/355/entry-2250186-designing-a-robust-input-handling-system-for-games/
	http://molecularmusings.wordpress.com/2011/09/05/properly-handling-keyboard-input/
	http://www.gamedev.net/topic/658794-game-engine-entity-component-design-handling-input/
=============================================================================
*/

#include <Core/Core_PCH.h>
#pragma hdrstop
#include <Core/Core.h>
#include <Core/Text/NameTable.h>
#include <Core/Input.h>

mxBEGIN_REFLECT_ENUM( KeyCodeT )
	#define mxDEFINE_KEYCODE( name )	mxREFLECT_ENUM_ITEM( name, KEY_##name )
	#include "KeyCodes.inl"
	#undef mxDEFINE_KEYCODE
	// for convenience when binding input actions in scripts
	mxREFLECT_ENUM_ITEM( LMB, KEY_MOUSE1 ),
	mxREFLECT_ENUM_ITEM( RMB, KEY_MOUSE2 ),
	mxREFLECT_ENUM_ITEM( MMB, KEY_MOUSE3 ),
mxEND_REFLECT_ENUM

const char* EKeyCode_To_Chars( EKeyCode keyCode )
{
	return GetEnumMembers< EKeyCode >().items[keyCode].name;
}
EKeyCode FindKeyByName(const char* keyname)
{
	return (EKeyCode) GetTypeOf_KeyCodeT().GetValueByString(keyname);
}

mxBEGIN_REFLECT_ENUM( InputKeyEventT )
	mxREFLECT_ENUM_ITEM( Pressed, EInputKeyEvent::IA_Pressed ),
	mxREFLECT_ENUM_ITEM( Released, EInputKeyEvent::IA_Released ),
	mxREFLECT_ENUM_ITEM( Held, EInputKeyEvent::IA_Repeated ),
	mxREFLECT_ENUM_ITEM( DoubleClicked, EInputKeyEvent::IA_DoubleClicked ),
mxEND_REFLECT_ENUM

mxBEGIN_FLAGS( FInputStateT )
	mxREFLECT_BIT( Pressed, (1 << EInputKeyEvent::IA_Pressed) ),
	mxREFLECT_BIT( Released, (1 << EInputKeyEvent::IA_Released) ),
	mxREFLECT_BIT( Held, (1 << EInputKeyEvent::IA_Repeated) ),
mxEND_FLAGS;

mxBEGIN_FLAGS( FKeyModifiersT )
	mxREFLECT_BIT( None, FKeyModifiers::KeyModifier_None ),
	mxREFLECT_BIT( LCtrl, FKeyModifiers::KeyModifier_LCtrl ),
	mxREFLECT_BIT( LShift, FKeyModifiers::KeyModifier_LShift ),
	mxREFLECT_BIT( LAlt, FKeyModifiers::KeyModifier_LAlt ),
	mxREFLECT_BIT( LWin, FKeyModifiers::KeyModifier_LWin ),
	mxREFLECT_BIT( RCtrl, FKeyModifiers::KeyModifier_RCtrl ),
	mxREFLECT_BIT( RShift, FKeyModifiers::KeyModifier_RShift ),
	mxREFLECT_BIT( RAlt, FKeyModifiers::KeyModifier_RAlt ),
	mxREFLECT_BIT( RWin, FKeyModifiers::KeyModifier_RWin ),
	mxREFLECT_BIT( NumLock, FKeyModifiers::KeyModifier_NumLock ),
	mxREFLECT_BIT( CapsLock, FKeyModifiers::KeyModifier_CapsLock ),
	mxREFLECT_BIT( ScrollLock, FKeyModifiers::KeyModifier_ScrollLock ),
	// Don't those names them in scripts to avoid confusion!
	//mxREFLECT_BIT( Ctrl, FKeyModifiers::KeyModifier_Ctrl ),
	//mxREFLECT_BIT( Shift, FKeyModifiers::KeyModifier_Shift ),
	//mxREFLECT_BIT( Alt, FKeyModifiers::KeyModifier_Alt ),
	//mxREFLECT_BIT( Win, FKeyModifiers::KeyModifier_Win ),
	//mxREFLECT_BIT( Modifiers, FKeyModifiers::KeyModifier_Modifiers ),
	//mxREFLECT_BIT( LockKeys, FKeyModifiers::KeyModifier_LockKeys ),
mxEND_FLAGS;

const Chars EMouseButton_To_Chars( EMouseButton button )
{
	switch( button ) {
		case MouseButton_1	:	return Chars("LMB");
		case MouseButton_2	:	return Chars("RMB");
		case MouseButton_3	:	return Chars("MMB");
		case MouseButton_4	:	return Chars("MB4");
		case MouseButton_5	:	return Chars("MB5");
		case MouseButton_6	:	return Chars("MB6");
		case MouseButton_7	:	return Chars("MB7");
		case MouseButton_8	:	return Chars("MB8");
	}
	return Chars("UNKNOWN");
}

mxBEGIN_REFLECT_ENUM( InputAxisT )
	mxREFLECT_ENUM_ITEM( MouseAxisX, EInputAxis::MouseAxisX ),
	mxREFLECT_ENUM_ITEM( MouseAxisY, EInputAxis::MouseAxisY ),
mxEND_REFLECT_ENUM

SInputEvent::SInputEvent()
{
	mxZERO_OUT(*this);
}

namespace InputUtil
{
	//EKeyCode FindKeyByName(const char* keyname)
	//{
	//	const mxEnumType::MemberList keyNames = GetEnumMembers< EKeyCode >().items;
	//	for( int i = 0; i < mxCOUNT_OF(gs_keyNames); i++ )
	//	{
	//		if( 0==strcmp(gs_keyNames[i].buffer, keyname)) {
	//			return (EKeyCode) i;
	//		}
	//	}
	//	return KEY_Unknown;
	//}
	String64 KeyModifiersToString( UINT32 modifiers )
	{
		String64 result;

		if( modifiers & KeyModifier_LCtrl ) {
			Str::AppendS(result, "+LCtrl");
		}
		if( modifiers & KeyModifier_LShift ) {
			Str::AppendS(result, "+LShift");
		}
		if( modifiers & KeyModifier_LAlt ) {
			Str::AppendS(result, "+LAlt");
		}
		if( modifiers & KeyModifier_LWin ) {
			Str::AppendS(result, "+LWin");
		}

		if( modifiers & KeyModifier_RCtrl ) {
			Str::AppendS(result, "+RCtrl");
		}
		if( modifiers & KeyModifier_RShift ) {
			Str::AppendS(result, "+RShift");
		}
		if( modifiers & KeyModifier_RAlt ) {
			Str::AppendS(result, "+RAlt");
		}
		if( modifiers & KeyModifier_RWin ) {
			Str::AppendS(result, "+RWin");
		}

		if( modifiers & KeyModifier_NumLock ) {
			Str::AppendS(result, "+NumLock");
		}
		if( modifiers & KeyModifier_CapsLock ) {
			Str::AppendS(result, "+CapsLock");
		}
		if( modifiers & KeyModifier_ScrollLock ) {
			Str::AppendS(result, "+ScrollLock");
		}

		if( result.IsEmpty() ) {
			Str::Copy(result,"None");
		}

		return result;
	}

	String64 MouseButtonMaskToString( UINT8 buttonMask )
	{
		String64 result;

		if( buttonMask & BIT(MouseButton_1) ) {
			Str::AppendS(result, "+LMB");
		}
		if( buttonMask & BIT(MouseButton_2) ) {
			Str::AppendS(result, "+RMB");
		}
		if( buttonMask & BIT(MouseButton_3) ) {
			Str::AppendS(result, "+MMB");
		}
		if( buttonMask & BIT(MouseButton_4) ) {
			Str::AppendS(result, "+MB4");
		}
		if( buttonMask & BIT(MouseButton_5) ) {
			Str::AppendS(result, "+MB5");
		}
		if( buttonMask & BIT(MouseButton_6) ) {
			Str::AppendS(result, "+MB6");
		}
		if( buttonMask & BIT(MouseButton_7) ) {
			Str::AppendS(result, "+MB7");
		}
		if( buttonMask & BIT(MouseButton_8) ) {
			Str::AppendS(result, "+MB8");
		}

		return result;
	}
#if 0

	void PrintInputEvent( const SInputEvent& _event )
	{
		LogStream	log(LL_Info);
		if( _event.device == InputDevice_Keyboard )
		{
			if( _event.type == Event_KeyboardEvent )
			{
				log << "Key ";
				if( _event.keyboard.action == IA_Pressed ) {
					log << "pressed: ";
				} else if( _event.keyboard.action == IA_Released ) {
					log << "released: ";
				} else if( _event.keyboard.action == IA_Repeated ) {
					log << "repeated: ";
				}
				log << EKeyCode_To_Chars((EKeyCode)_event.keyboard.key);
				if( _event.keyboard.modifiers ) {
					log << ", modifiers: " << KeyModifiersToString(_event.keyboard.modifiers);
				}
				log << '\n';
			}
			if( _event.type == Event_TextEntered )
			{
				log << "Entered char: " << _event.text.character;
				log << '\n';
			}
		}
		if( _event.device == InputDevice_Mouse )
		{
			if( _event.type == Event_MouseButtonEvent )
			{
				log << "Mouse button ";
				if( _event.mouseButton.action == IA_Pressed ) {
					log << "pressed: ";
				} else if( _event.mouseButton.action == IA_Released ) {
					log << "released: ";
				} else if( _event.mouseButton.action == IA_DoubleClicked ) {
					log << "double-clicked: ";
				}
				log << EMouseButton_To_Chars((EMouseButton)_event.mouseButton.button);
				if( _event.mouseButton.buttonMask ) {
					log << ", pressed buttons: " << MouseButtonMaskToString(_event.mouseButton.buttonMask);
				}
				log << '\n';
			}
			if( _event.type == Event_MouseCursorMoved )
			{
				log << "Mouse cursor moved from " << _event.mouseMotion.x << ", " << _event.mouseMotion.y
					<< " by " << _event.mouseMotion.deltaX << ", " << _event.mouseMotion.deltaY << '\n';
			}
			if( _event.type == Event_MouseWheelMoved )
			{
				log << "Mouse wheeled by " << _event.mouseWheel.wheelDelta << '\n';
			}
		}
	}
#endif
}//namespace InputUtil

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
