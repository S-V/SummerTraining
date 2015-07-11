/*
=============================================================================
	File:	Tweakable.h
	Desc:	Tweakable constants for debugging and rapid prototyping.
	See:

	http://blogs.msdn.com/b/shawnhar/archive/2009/05/01/motogp-tweakables.aspx

	http://www.gamedev.net/page/resources/_/technical/game-programming/tweakable-constants-r2731
	http://www.gamedev.net/topic/559658-tweakable-constants/

	https://mollyrocket.com/forums/viewtopic.php?p=3355
	https://mollyrocket.com/forums/viewtopic.php?t=556

	http://www.pouet.net/topic.php?which=7126&page=1&x=19&y=10

=============================================================================
*/
#pragma once

#include <Base/Template/Containers/HashMap/TPointerMap.h>
#include <Base/Template/Delegate/Delegate.h>

#include <Core/Editor.h>

//--------------------------------------------------
//	Definitions of useful macros.
//--------------------------------------------------

#if MX_EDITOR

	// NOTE: use only on static variables
	#define HOT_VAR(x)		TweakUtil::Tweak_Variable( &x, #x, __FILE__, __LINE__ )

#else

	#define HOT_VAR(x)

#endif // MX_EDITOR


/*
-----------------------------------------------------------------------------
	TweakUtil
-----------------------------------------------------------------------------
*/
namespace TweakUtil
{
	typedef TPointerMap< AProperty::Ref > PropertyMap;

	void Setup();
	void Close();

	void Tweak_Variable( bool * var, const char* expr, const char* file, int line );
	void Tweak_Variable( int * var, const char* expr, const char* file, int line );
	void Tweak_Variable( float * var, const char* expr, const char* file, int line );
	void Tweak_Variable( double * var, const char* expr, const char* file, int line );

	PropertyMap& GetAllTweakables();

	extern TCallback< void (AProperty*) >	OnNewTweakableAdded;
	extern TCallback< void (void) >			OnShutDown;

}//namespace TweakUtil

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
