// This is a precompiled header.  Include a bunch of common stuff.
// This is kind of ugly in that it adds a bunch of dependency where it isn't needed.
// But on balance, the compile time is much lower (even incrementally) once the precompiled
// headers contain these headers.

#pragma once

#include <Base/Base.h>
#include <Base/Math/Math.h>

#include <Core/Core.h>
#if MX_AUTOLINK
	#pragma comment( lib, "Core.lib" )
#endif

#include <Graphics/Device.h>
#if MX_AUTOLINK
	#pragma comment( lib, "Graphics.lib" )
#endif

/*
// Deferred viewports resizing is used as an optimization when resizing multiple viewports

struct ResizeViewportCommand
{
	RenderViewport*	viewport;
	UINT			newWidth;
	UINT			newHeight;
	UINT			timeStamp;	// relative time, in milliseconds
};

typedef TStaticList_InitZeroed
<
	ResizeViewportCommand,
	rxMAX_VIEWPORTS
>
ResizeViewportCommands;

// batches up resize commands
//
class ResizeViewportsUtil
{
	ResizeViewportCommands	viewportsToResize;

	UINT	lastTimeViewportAdded;	// in milliseconds
	UINT	delayInMilliSeconds;	// in milliseconds

public:
	ResizeViewportsUtil()
	{
		lastTimeViewportAdded = 0;
		delayInMilliSeconds = 1000;
	}
	UINT CurrentTimeMilliseconds() const
	{
		return mxGetTimeInMilliseconds();
	}
	void SetDelayMilliseconds( UINT delayMSec )
	{
		delayInMilliSeconds = delayMSec;
	}
	void Add( RenderViewport* viewport, UINT newWidth, UINT newHeight )
	{
		ResizeViewportCommand* newCmd = nil;

		for( UINT iViewport = 0; iViewport < viewportsToResize.Num(); iViewport++ )
		{
			ResizeViewportCommand & cmd = viewportsToResize[ iViewport ];
			if ( cmd.viewport == viewport )
			{
				newCmd = &cmd;
				break;
			}
		}

		if( newCmd == nil )
		{
			newCmd = &viewportsToResize.Add();
		}

		newCmd->viewport = viewport;
		newCmd->newWidth = newWidth;
		newCmd->newHeight = newHeight;
		newCmd->timeStamp = CurrentTimeMilliseconds();

		lastTimeViewportAdded = CurrentTimeMilliseconds();
	}
	void Remove( RenderViewport* viewport )
	{
		for( UINT iViewport = 0; iViewport < viewportsToResize.Num(); iViewport++ )
		{
			ResizeViewportCommand & cmd = viewportsToResize[ iViewport ];
			if ( cmd.viewport == viewport )
			{
				viewportsToResize.RemoveAt( iViewport );
			}
		}
	}
	bool ShouldResizeViewports() const
	{
		return (viewportsToResize.Num() > 0)
			&& (CurrentTimeMilliseconds() - lastTimeViewportAdded >= delayInMilliSeconds)
			;
	}
	void Clear()
	{
		viewportsToResize.Empty();
	}
	ResizeViewportCommand* ToPtr()
	{
		return viewportsToResize.ToPtr();
	}
	UINT Num() const
	{
		return viewportsToResize.Num();
	}
};
*/
