/*
=============================================================================
	File:	Driver.h
	Desc:
=============================================================================
*/
#pragma once

#if MX_AUTOLINK
	#pragma comment( lib, "Driver.lib" )
#endif

#include <Core/Input.h>

/*
=============================================================================
	
	PLATFORM DRIVER

=============================================================================
*/
namespace WindowsDriver
{
	struct Settings
	{
		const char* name;

		UINT16	screenWidth, screenHeight;
		bool	fullScreen;

		bool	resizable;

		Settings();
	};

	ERet Initialize(const Settings& options = Settings());
	void Shutdown();

	void GenerateSystemEvents( UINT32 states = -1 );
	int ProcessSystemEvents( UINT32 states = -1 );

	void* GetNativeWindowHandle();

	ERet SetRelativeMouseMode(bool enable);
	bool IsRelativeMouseModeEnabled();
	bool HasFocus();

	void RequestExit();
	bool IsAboutToExit();

	void GetWindowSize( int *x, int *y );
	void GetWindowPosition( int *x, int *y );

	const UINT8* GetKeyboardState();	// EKeyCode [MAX_KEYS]
	const UINT16 GetModifiers();		// FKeyModifiers mask
	int GetMouseState( int *x, int *y );// EMouseButton mask

	// get the number of seconds elapsed since the last frame
	double GetDeltaSeconds();

}//namespace WindowsDriver

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
