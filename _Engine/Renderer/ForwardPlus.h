/*
=============================================================================
	File:	
	Desc:	
=============================================================================
*/
#pragma once

#include <Renderer/Renderer.h>

class ForwardPlusRenderer : RendererBase
{

public:
	typedef RendererBase Super;
	ForwardPlusRenderer();
	~ForwardPlusRenderer();

	ERet Initialize( Clump* rendererData );
	void Shutdown();

	ERet RenderScene( const SceneView& sceneView, const Clump& sceneData );

	ERet ResizeBuffers( UINT16 width, UINT16 height, bool fullscreen );
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
