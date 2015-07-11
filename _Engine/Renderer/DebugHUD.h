#pragma once

#include <Graphics/Device.h>

namespace DebugHUD
{
	ERet Initialize( FxLibrary* library );
	void Shutdown();

	// x and y are in viewport coordinates of the text sprite's top left corner
//	ERet SubmitText( utl::SpriteFont* font, UINT16 x, UINT16 y, const char* text, UINT16 length );

	void RenderBatchedItems( UINT16 viewportWidth, UINT16 viewportHeight );

}//namespace DebugHUD

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
