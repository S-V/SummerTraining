/*
=============================================================================
	File:	Client.cpp
	Desc:	
=============================================================================
*/
#include <Core/Core_PCH.h>
#pragma hdrstop
#include <Core/Client.h>

mxBEGIN_REFLECT_ENUM( GameStateID )
#define DECLARE_GAME_STATE( name, description )		mxREFLECT_ENUM_ITEM( name, GameStates::name ),
	#include <Core/GameStates.inl>
#undef DECLARE_GAME_STATE
mxREFLECT_ENUM_ITEM( ALL, GameStates::ALL ),
mxEND_REFLECT_ENUM

mxBEGIN_FLAGS( GameStateF )
#define DECLARE_GAME_STATE( name, description )		mxREFLECT_ENUM_ITEM( name, BIT(GameStates::name) ),
	#include <Core/GameStates.inl>
#undef DECLARE_GAME_STATE
mxREFLECT_BIT( ALL, GameStates::ALL ),
mxEND_FLAGS

namespace GameActions {

}//namespace GameActions

mxBEGIN_REFLECT_ENUM( GameActionID )
#define DECLARE_GAME_ACTION( name, description )	mxREFLECT_ENUM_ITEM( name, GameActions::name ),
	#include <Core/GameActions.inl>
#undef DECLARE_GAME_ACTION
mxEND_REFLECT_ENUM

/*
-----------------------------------------------------------------------------
	KeyBind
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS(KeyBind);
mxBEGIN_REFLECTION(KeyBind)
	mxMEMBER_FIELD(key),
	mxMEMBER_FIELD(when),
	mxMEMBER_FIELD(with),
	mxMEMBER_FIELD(action),
mxEND_REFLECTION;
KeyBind::KeyBind()
{
	key = EKeyCode::KEY_Unknown;
	when = IS_Pressed;
	with = KeyModifier_None;
	action = GameActions::NONE;
}

/*
-----------------------------------------------------------------------------
	AxisBind
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS(AxisBind);
mxBEGIN_REFLECTION(AxisBind)
	mxMEMBER_FIELD(axis),
	mxMEMBER_FIELD(action),
mxEND_REFLECTION;
AxisBind::AxisBind()
{
	axis = MouseAxisX;
	action = GameActions::NONE;
}

/*
-----------------------------------------------------------------------------
	InputContext
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS(InputContext);
mxBEGIN_REFLECTION(InputContext)
	mxMEMBER_FIELD(name),
	mxMEMBER_FIELD(key_binds),
	mxMEMBER_FIELD(axis_binds),
	mxMEMBER_FIELD(state_mask),
mxEND_REFLECTION;
InputContext::InputContext()
{
	state_mask = GameStates::ALL;
}

namespace ClientCallbacks
{
	static TPtr< Data >	gs_Data;
	static NiftyCounter	gs_InitCounter;

	ERet Initialize()
	{
		if( gs_InitCounter.IncRef() )
		{
			gs_Data.ConstructInPlace();
		}
		return ALL_OK;
	}
	void Shutdown()
	{
		if( gs_InitCounter.DecRef() )
		{
			gs_Data.Destruct();
		}
	}
	Data& GetData()
	{
		return *gs_Data;
	}

	//ERet BindKey( EKeyCode key, const char* action )
	//{
	//	UNDONE;
	//	return ERR_UNSUPPORTED_FEATURE;
	//	//chkRET_X_IF_NOT(key >= 0 && key < mxCOUNT_OF(g_keys),ERR_INVALID_PARAMETER);
	//	//g_keys[key].action = action;
	//	//return ALL_OK;
	//}
	//const char* GetKeyBinding( EKeyCode key )
	//{
	//	//return g_keys[key].action.ToPtr();
	//	UNDONE;
	//	return NULL;
	//}

}//namespace ClientCallbacks

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
