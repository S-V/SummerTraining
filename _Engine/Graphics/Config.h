/*
=============================================================================
	Build configuration settings.
	These are compile time settings for the libraries.
=============================================================================
*/
#pragma once

#if MX_AUTOLINK
	#pragma comment( lib, "Graphics.lib" )
#endif

#define LLGL_Driver_Direct3D_11		(1)
#define LLGL_Driver_OpenGL_4plus	(2)

//	Defines
#define LLGL_Driver		LLGL_Driver_Direct3D_11
// enable multithreaded draw call submission
#define LLGL_MULTITHREADED				(0)

// 0..3
#if MX_DEBUG
	#define LL_DEBUG_LEVEL		(3)
#else
	#define LL_DEBUG_LEVEL		(0)
#endif

// Config (NOTE: don't make them too big to avoid data structures bloat)
#define LLGL_VALIDATE_PROGRAMS			(MX_DEBUG)
#define LLGL_ENABLE_PERF_HUD			(MX_DEBUG)
#define LLGL_MAX_DRAW_CALLS				(2048)
#define LLGL_CREATE_RENDER_THREAD		(0)
#define LLGL_COMMAND_BUFFER_SIZE		(1*mxMEBIBYTE)
#define LLGL_MAX_VERTEX_ATTRIBS			(8)
#define LLGL_MAX_TEXTURE_DIMENSIONS		(4096)
#define LLGL_MAX_TEXTURE_MIP_LEVELS		(14)
#define LLGL_MAX_TEXTURE_ARRAY_SIZE		(8)
#define LLGL_MAX_RENDER_TARGET_SIZE		(8192)
#define LLGL_MAX_BOUND_TARGETS			(4)

#define LLGL_SUPPORT_BINARY_GL_PROGRAMS		(0)

#define LLGL_SUPPORT_GEOMETRY_SHADERS		(1)
#define LLGL_SUPPORT_TESSELATION_SHADERS	(1)

//#define LLGL_MAX_BOUND_UNIFORM_BUFFERS	(14)
//#define LLGL_MAX_BOUND_SHADER_SAMPLERS	(16)
//#define LLGL_MAX_BOUND_SHADER_TEXTURES	(32)
#define LLGL_MAX_BOUND_UNIFORM_BUFFERS	(11)
#define LLGL_MAX_BOUND_SHADER_SAMPLERS	(16)
#define LLGL_MAX_BOUND_SHADER_TEXTURES	(16)

// maximum number of uniforms in a shader constant buffer
#define LLGL_MAX_CBUFFER_UNIFORMS		(4096)

#define LLGL_MAX_TEXTURE_UNITS			(8)

#define LLGL_MAX_VERTEX_STREAMS		(1)

#define LLGL_Driver_Is_Direct3D	(LLGL_Driver == LLGL_Driver_Direct3D_11)
#define LLGL_Driver_Is_OpenGL	(LLGL_Driver == LLGL_Driver_OpenGL_4plus)



//=====================================================================
//	MINI-MATH
//=====================================================================

#if LLGL_Driver_Is_Direct3D
	#define Matrix_Perspective	Matrix_PerspectiveD3D
#endif

#if LLGL_Driver_Is_OpenGL
	#define Matrix_Perspective	Matrix_PerspectiveOGL
#endif

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
