/*
=============================================================================
	File:	
	Desc:	
=============================================================================
*/
#pragma once

#include <Renderer/Renderer.h>

class DeferredRenderer : public RendererBase
{
public:
	FxColorTarget* m_colorRT0;
	FxColorTarget* m_colorRT1;
	FxDepthTarget* m_depthRT;

public:
	typedef RendererBase Super;
	DeferredRenderer();
	~DeferredRenderer();

	ERet Initialize( Clump* rendererData );
	void Shutdown();

	ERet RenderScene( const SceneView& sceneView, const Clump& sceneData );

	ERet ResizeBuffers( UINT16 width, UINT16 height, bool fullscreen );

public:
	ERet BeginRender_GBuffer();
	ERet EndRender_GBuffer();
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
