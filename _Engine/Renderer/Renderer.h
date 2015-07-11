/*
=============================================================================
	File:	Renderer.h
	Desc:	High-level renderer interface.
=============================================================================
*/
#pragma once

#if MX_AUTOLINK
	#pragma comment( lib, "Renderer.lib" )
#endif

#include <Core/Editor.h>
#include <Graphics/Device.h>
#include <Graphics/Effects.h>
#include <Renderer/Vertex.h>

#define mxDO2( X )\
	mxMACRO_BEGIN\
		static bool showError = true;\
		const ERet result = (X);\
		if( mxFAILED(result) )\
		{\
			if( showError ){\
			const char* errorMessage = EReturnCode_To_Chars( result );\
			mxGetLog().PrintF( LL_Error, "%s(%d): '%s' failed with '%s'\n", __FILE__, __LINE__, #X, errorMessage );\
			showError = false;}\
		}\
		else{\
			showError = true;\
		}\
	mxMACRO_END

class Clump;
class rxMaterial;

struct RenderContext
{
	llgl::ViewState	viewState;
};

struct SceneView
{
	Float4x4	viewMatrix;
	Float4x4	projectionMatrix;

	Float4x4	viewProjectionMatrix;

	Float3	worldSpaceCameraPos;

	float	viewportWidth, viewportHeight;

	float nearClip, farClip;
};

namespace Rendering
{
	ERet RegisterClasses();

	// Globals
	extern HInputLayout		g_inputLayouts[VTX_MAX];
	extern HSamplerState	g_samplers[Sampler_MAX];

	ERet InitializeGlobals( const Clump& rendererData );
	void DestroyGlobals();

	// this gets called before the main viewport has been deallocated for resizing
	ERet ReleaseResourcesDependentOnBackBuffer( const Clump& _clump );

	// this gets called after the main viewport has been reallocated
	ERet RecreateResourcesDependentOnBackBuffer( const Clump& _clump, UINT16 screenWidth, UINT16 screenHeight );

}//namespace Rendering

class RendererBase
	: SingleInstance< RendererBase > // prevent double instantiation
{
protected:
	Clump *			m_rendererData;

	HContext		m_hRenderContext;

	HBuffer			m_hCBPerFrame;
	HBuffer			m_hCBPerCamera;
	HBuffer			m_hCBPerObject;

	// floats to prevent int->float conversions
	float	m_viewportWidth, m_viewportHeight;

protected:
	RendererBase();
	~RendererBase();
	ERet Initialize( Clump* rendererData );
	void Shutdown();

public:
	ERet DrawFullScreenTriangle( FxShader* shader );
	ERet DrawFullScreenQuad( FxShader* shader );
	ERet DrawScreenQuad( FxShader* shader );

	void DBG_Draw_Models_With_Custom_Shader(
		const SceneView& sceneView,
		const Clump& sceneData,
		const FxShader& shader,
		TopologyT overrideTopology = Topology::Undefined
	);

	void DBG_Draw_Models_Wireframe(
		const SceneView& sceneView,
		const Clump& sceneData,
		const FxShader& shader
	);

	void SetGlobalUniformBuffers( llgl::DrawCall *batch );
	void BindMaterial( const rxMaterial* material, llgl::DrawCall *batch );
};

class SimpleRenderer : RendererBase
{
public:
	typedef RendererBase Super;
	SimpleRenderer();
	~SimpleRenderer();

	ERet Initialize( Clump* rendererData );
	void Shutdown();

	ERet RenderScene( const SceneView& sceneView, const Clump& sceneData );

	ERet ResizeBuffers( UINT16 width, UINT16 height, bool fullscreen );
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
