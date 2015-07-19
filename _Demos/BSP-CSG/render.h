//based on
/*
 * Copyright 2011-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */
#include <Base/Base.h>
#include <Core/Core.h>

#include "common.h"
#include "bgfx_utils.h"
#include "imgui/imgui.h"
#include "camera.h"
#include "bounds.h"

#define RENDER_PASS_GEOMETRY_ID       0
#define RENDER_PASS_LIGHT_ID          1
#define RENDER_PASS_GLOBAL_LIGHT      2
#define RENDER_PASS_COMBINE_ID        3
#define RENDER_PASS_DEBUG_LIGHTS_ID   4
#define RENDER_PASS_DEBUG_GBUFFER_ID  5
#define RENDER_PASS_FORWARD           7
#define RENDER_PASS_DEBUG_LINES_3D    12

extern float g_texelHalf;
extern bool g_originBottomLeft;


inline void mtxProj(float* _result, float _fovy, float _aspect, float _near, float _far)
{
	bx::mtxProj(_result, _fovy, _aspect, _near, _far, g_originBottomLeft);
}

struct PosNormalTangentTexcoordVertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_normal;
	uint32_t m_tangent;
	int16_t m_u;
	int16_t m_v;
	//24

	static void init()
	{
		ms_decl
			.begin()
			.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Normal,    4, bgfx::AttribType::Uint8, true, true)
			.add(bgfx::Attrib::Tangent,   4, bgfx::AttribType::Uint8, true, true)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true, true)
			.end();
	}

	static bgfx::VertexDecl ms_decl;
};

struct PosTexCoord0Vertex
{
	float m_x;
	float m_y;
	float m_z;
	float m_u;
	float m_v;

	static void init()
	{
		ms_decl
			.begin()
			.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.end();
	}

	static bgfx::VertexDecl ms_decl;
};

struct DebugVertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_abgr;

	static void init()
	{
		ms_decl
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
			.end();
	}

	static bgfx::VertexDecl ms_decl;
};

uint32_t packUint32(uint8_t _x, uint8_t _y, uint8_t _z, uint8_t _w);

uint32_t packF4u(float _x, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f);

extern PosNormalTangentTexcoordVertex s_cubeVertices[24];
extern const uint16_t s_cubeIndices[36];

ERet screenSpaceQuad(float _textureWidth, float _textureHeight, float _texelHalf, bool _originBottomLeft, float _width = 1.0f, float _height = 1.0f, float zz = 0.0f);


class Renderer
{
public:
	uint32_t width;
	uint32_t height;
	uint32_t debug;
	uint32_t reset;

	bgfx::VertexBufferHandle vbh;
	bgfx::IndexBufferHandle ibh;

	bgfx::UniformHandle s_texColor;
	bgfx::UniformHandle s_texNormal;
	bgfx::UniformHandle s_albedo;
	bgfx::UniformHandle s_normal;
	bgfx::UniformHandle s_depth;
	bgfx::UniformHandle s_light;

	bgfx::UniformHandle u_mtx;
	bgfx::UniformHandle u_lightPosRadius;
	bgfx::UniformHandle u_lightRgbInnerR;

	bgfx::UniformHandle u_lightVector;
	bgfx::UniformHandle u_lightColor;
	bgfx::UniformHandle u_inverseViewMat;


	bgfx::ProgramHandle geomProgram;
	bgfx::ProgramHandle lightProgram;
	bgfx::ProgramHandle combineProgram;
	bgfx::ProgramHandle debugProgram;
	bgfx::ProgramHandle lineProgram;
	bgfx::ProgramHandle forwardProgram;

	bgfx::ProgramHandle deferred_directional_light_program;

	bgfx::TextureHandle textureColor;
	bgfx::TextureHandle textureNormal;

	// G-buffer:
	// RT0 - hardware depth
	// RT1 - view-space normals
	// RT2 - diffuse color (albedo)
	enum {
		RT_DEPTH = 0,
		RT_ALBEDO = 1,
		RT_NORMALS = 2,
		//RT_EMISSIVE = 3,//RT_SPECULAR = 3,
	};

	bgfx::TextureHandle gbufferTex[3];
	bgfx::FrameBufferHandle gbuffer;
	bgfx::FrameBufferHandle lightBuffer;

public:
	Renderer();
	ERet Initialize();
	void Shutdown();

	ERet BeginFrame( uint32_t _width, uint32_t _height, uint32_t _reset, const float view[16], float time );
	ERet EndFrame();
};

extern bool animateMesh;
extern bool showScissorRects;
extern bool showGBuffer;
extern int32_t numLights;
extern float lightAnimationSpeed;
