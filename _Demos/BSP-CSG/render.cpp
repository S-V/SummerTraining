//based on
/*
 * Copyright 2011-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */
#include "render.h"

float g_texelHalf = 0;
bool g_originBottomLeft = false;

bgfx::VertexDecl PosNormalTangentTexcoordVertex::ms_decl;
bgfx::VertexDecl PosTexCoord0Vertex::ms_decl;
bgfx::VertexDecl DebugVertex::ms_decl;

static bgfx::VertexDecl g_AuxVertexDecl;

inline uint64_t ToBgfxTopology( const Topology::Enum topology )
{
	static const uint64_t g_bgfxState[] = {
/* Topology::Undefined */		0,
/* Topology::PointList */		BGFX_STATE_PT_POINTS,
/* Topology::LineList */		BGFX_STATE_PT_LINES,
/* Topology::LineStrip */		BGFX_STATE_PT_LINESTRIP,
/* Topology::TriangleList */	0,
/* Topology::TriangleStrip */	BGFX_STATE_PT_TRISTRIP,
/* Topology::TriangleFan */		0,	// UNSUPPORTED
	};
	uint64_t state = g_bgfxState[ topology ];
	return state;
}

PosNormalTangentTexcoordVertex s_cubeVertices[24] =
{
	{-1.0f,  1.0f,  1.0f, packF4u( 0.0f,  0.0f,  1.0f), 0,      0,      0 },
	{ 1.0f,  1.0f,  1.0f, packF4u( 0.0f,  0.0f,  1.0f), 0, 0x7fff,      0 },
	{-1.0f, -1.0f,  1.0f, packF4u( 0.0f,  0.0f,  1.0f), 0,      0, 0x7fff },
	{ 1.0f, -1.0f,  1.0f, packF4u( 0.0f,  0.0f,  1.0f), 0, 0x7fff, 0x7fff },
	{-1.0f,  1.0f, -1.0f, packF4u( 0.0f,  0.0f, -1.0f), 0,      0,      0 },
	{ 1.0f,  1.0f, -1.0f, packF4u( 0.0f,  0.0f, -1.0f), 0, 0x7fff,      0 },
	{-1.0f, -1.0f, -1.0f, packF4u( 0.0f,  0.0f, -1.0f), 0,      0, 0x7fff },
	{ 1.0f, -1.0f, -1.0f, packF4u( 0.0f,  0.0f, -1.0f), 0, 0x7fff, 0x7fff },
	{-1.0f,  1.0f,  1.0f, packF4u( 0.0f,  1.0f,  0.0f), 0,      0,      0 },
	{ 1.0f,  1.0f,  1.0f, packF4u( 0.0f,  1.0f,  0.0f), 0, 0x7fff,      0 },
	{-1.0f,  1.0f, -1.0f, packF4u( 0.0f,  1.0f,  0.0f), 0,      0, 0x7fff },
	{ 1.0f,  1.0f, -1.0f, packF4u( 0.0f,  1.0f,  0.0f), 0, 0x7fff, 0x7fff },
	{-1.0f, -1.0f,  1.0f, packF4u( 0.0f, -1.0f,  0.0f), 0,      0,      0 },
	{ 1.0f, -1.0f,  1.0f, packF4u( 0.0f, -1.0f,  0.0f), 0, 0x7fff,      0 },
	{-1.0f, -1.0f, -1.0f, packF4u( 0.0f, -1.0f,  0.0f), 0,      0, 0x7fff },
	{ 1.0f, -1.0f, -1.0f, packF4u( 0.0f, -1.0f,  0.0f), 0, 0x7fff, 0x7fff },
	{ 1.0f, -1.0f,  1.0f, packF4u( 1.0f,  0.0f,  0.0f), 0,      0,      0 },
	{ 1.0f,  1.0f,  1.0f, packF4u( 1.0f,  0.0f,  0.0f), 0, 0x7fff,      0 },
	{ 1.0f, -1.0f, -1.0f, packF4u( 1.0f,  0.0f,  0.0f), 0,      0, 0x7fff },
	{ 1.0f,  1.0f, -1.0f, packF4u( 1.0f,  0.0f,  0.0f), 0, 0x7fff, 0x7fff },
	{-1.0f, -1.0f,  1.0f, packF4u(-1.0f,  0.0f,  0.0f), 0,      0,      0 },
	{-1.0f,  1.0f,  1.0f, packF4u(-1.0f,  0.0f,  0.0f), 0, 0x7fff,      0 },
	{-1.0f, -1.0f, -1.0f, packF4u(-1.0f,  0.0f,  0.0f), 0,      0, 0x7fff },
	{-1.0f,  1.0f, -1.0f, packF4u(-1.0f,  0.0f,  0.0f), 0, 0x7fff, 0x7fff },
};
const uint16_t s_cubeIndices[36] =
{
	 0,  2,  1,
	 1,  2,  3,
	 4,  5,  6,
	 5,  7,  6,

	 8, 10,  9,
	 9, 10, 11,
	12, 13, 14,
	13, 15, 14,

	16, 18, 17,
	17, 18, 19,
	20, 21, 22,
	21, 23, 22,
};

uint32_t packUint32(uint8_t _x, uint8_t _y, uint8_t _z, uint8_t _w)
{
	union
	{
		uint32_t ui32;
		uint8_t arr[4];
	} un;

	un.arr[0] = _x;
	un.arr[1] = _y;
	un.arr[2] = _z;
	un.arr[3] = _w;

	return un.ui32;
}

uint32_t packF4u(float _x, float _y, float _z, float _w)
{
	const uint8_t xx = uint8_t(_x*127.0f + 128.0f);
	const uint8_t yy = uint8_t(_y*127.0f + 128.0f);
	const uint8_t zz = uint8_t(_z*127.0f + 128.0f);
	const uint8_t ww = uint8_t(_w*127.0f + 128.0f);
	return packUint32(xx, yy, zz, ww);
}

ERet screenSpaceTriangle(float _textureWidth, float _textureHeight, float _texelHalf, bool _originBottomLeft, float _width, float _height, float zz)
{
	if (bgfx::checkAvailTransientVertexBuffer(3, PosTexCoord0Vertex::ms_decl) )
	{
		bgfx::TransientVertexBuffer vb;
		bgfx::allocTransientVertexBuffer(&vb, 3, PosTexCoord0Vertex::ms_decl);
		PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)vb.data;

		const float minx = -_width;
		const float maxx =  _width;
		const float miny = 0.0f;
		const float maxy = _height*2.0f;

		const float texelHalfW = _texelHalf/_textureWidth;
		const float texelHalfH = _texelHalf/_textureHeight;
		const float minu = -1.0f + texelHalfW;
		const float maxu =  1.0f + texelHalfH;

		float minv = texelHalfH;
		float maxv = 2.0f + texelHalfH;

		if (_originBottomLeft)
		{
			float temp = minv;
			minv = maxv;
			maxv = temp;

			minv -= 1.0f;
			maxv -= 1.0f;
		}

		vertex[0].m_x = minx;
		vertex[0].m_y = miny;
		vertex[0].m_z = zz;
		vertex[0].m_u = minu;
		vertex[0].m_v = minv;

		vertex[1].m_x = maxx;
		vertex[1].m_y = miny;
		vertex[1].m_z = zz;
		vertex[1].m_u = maxu;
		vertex[1].m_v = minv;

		vertex[2].m_x = maxx;
		vertex[2].m_y = maxy;
		vertex[2].m_z = zz;
		vertex[2].m_u = maxu;
		vertex[2].m_v = maxv;

		bgfx::setVertexBuffer(&vb);

		return ALL_OK;
	}
	return ERR_OUT_OF_MEMORY;
}

ERet screenSpaceQuad(
					 float _textureWidth, float _textureHeight,
					 float _texelHalf, bool _originBottomLeft,
					 float _width, float _height, float zz
					 )
{
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;
	if (bgfx::allocTransientBuffers(&tvb, PosTexCoord0Vertex::ms_decl, 4, &tib, 6) )
	{
		const float minx = -_width;
		const float maxx = +_width;
		const float miny = -_height;
		const float maxy = +_height;

		const float texelHalfW = _texelHalf/_textureWidth;
		const float texelHalfH = _texelHalf/_textureHeight;
		const float minu = texelHalfW;
		const float maxu = 1.0f + texelHalfH;

		float minv = texelHalfH;
		float maxv = 1.0f + texelHalfH;

		if (_originBottomLeft)
		{
			UNDONE;
			//float temp = minv;
			//minv = maxv;
			//maxv = temp;

			//minv -= 1.0f;
			//maxv -= 1.0f;
		}

		PosTexCoord0Vertex* vertices = (PosTexCoord0Vertex*)tvb.data;

		// lower left
		vertices[0].m_x = minx;
		vertices[0].m_y = miny;
		vertices[0].m_z = zz;
		vertices[0].m_u = minu;
		vertices[0].m_v = maxv;

		// upper left
		vertices[1].m_x = minx;
		vertices[1].m_y = maxy;
		vertices[1].m_z = zz;
		vertices[1].m_u = minu;
		vertices[1].m_v = minv;

		// upper right
		vertices[2].m_x = maxx;
		vertices[2].m_y = maxy;
		vertices[2].m_z = zz;
		vertices[2].m_u = maxu;
		vertices[2].m_v = minv;

		// lower right
		vertices[3].m_x = maxx;
		vertices[3].m_y = miny;
		vertices[3].m_z = zz;
		vertices[3].m_u = maxu;
		vertices[3].m_v = maxv;

		uint16_t* indices = (uint16_t*)tib.data;
		*indices++ = 0;
		*indices++ = 1;
		*indices++ = 2;
		*indices++ = 0;
		*indices++ = 2;
		*indices++ = 3;

		bgfx::setVertexBuffer(&tvb);
		bgfx::setIndexBuffer(&tib);

		return ALL_OK;
	}
	return ERR_OUT_OF_MEMORY;
}

Renderer::Renderer()
{
}
ERet Renderer::Initialize()
{
	width = 1280;
	height = 720;
	debug = BGFX_DEBUG_TEXT;
	reset = BGFX_RESET_VSYNC;

	bgfx::init(
		bgfx::RendererType::Direct3D9,
		BGFX_PCI_ID_NONE,
		0,
		&callback
	);

	// Get renderer capabilities info.
	const bgfx::Caps* caps = bgfx::getCaps();

	if (2 > caps->maxFBAttachments)
	{
		// multiple render targets (MRT) is not supported by GPU
		return ERR_UNSUPPORTED_FEATURE;
	}

	const bgfx::RendererType::Enum renderer = bgfx::getRendererType();
	g_texelHalf = bgfx::RendererType::Direct3D9 == renderer ? 0.5f : 0.0f;
	g_originBottomLeft = bgfx::RendererType::OpenGL == renderer || bgfx::RendererType::OpenGLES == renderer;

	const char* rendererType = "Unknown";
	switch (bgfx::getRendererType() )
	{
	case bgfx::RendererType::Direct3D9:
		rendererType = "Direct3D 9";
		break;
	case bgfx::RendererType::Direct3D11:
		rendererType = "Direct3D 11";
		break;
	case bgfx::RendererType::Direct3D12:
		rendererType = "Direct3D 12";
		break;
	case bgfx::RendererType::OpenGL:
		rendererType = "OpenGL";
		break;
	}
	ptPRINT("Using renderer: '%s'", rendererType);


	bgfx::reset(width, height, reset);

	// Enable debug text.
	bgfx::setDebug(debug);

	// Set clear color palette for index 0
	bgfx::setClearColor(0, UINT32_C(0x00000000) );

	// Set clear color palette for index 1
	bgfx::setClearColor(1, UINT32_C(0x303030ff) );

	// Set geometry pass view clear state.
	bgfx::setViewClear(RENDER_PASS_GEOMETRY_ID
		, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
		, 1.0f
		, 0
		, 1
		);

	// Set light pass view clear state.
	bgfx::setViewClear(RENDER_PASS_LIGHT_ID
		, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
		, 1.0f
		, 0
		, 0
		);

	// Create vertex stream declaration.
	PosNormalTangentTexcoordVertex::init();
	PosTexCoord0Vertex::init();
	DebugVertex::init();

	g_AuxVertexDecl.begin()
		.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Normal,    4, bgfx::AttribType::Uint8, true, true)
		.add(bgfx::Attrib::Tangent,   4, bgfx::AttribType::Uint8, true, true)
		.add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
		.end();

	calcTangents(s_cubeVertices
		, BX_COUNTOF(s_cubeVertices)
		, PosNormalTangentTexcoordVertex::ms_decl
		, s_cubeIndices
		, BX_COUNTOF(s_cubeIndices)
		);

	// Create static vertex buffer.
	cubeVB = bgfx::createVertexBuffer(
		  bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices) )
		, PosNormalTangentTexcoordVertex::ms_decl
		);

	// Create static index buffer.
	cubeIB = bgfx::createIndexBuffer(bgfx::makeRef(s_cubeIndices, sizeof(s_cubeIndices) ) );

	//dynamicVB = bgfx::createDynamicVertexBuffer( PlatformRenderer::VB_SIZE/sizeof(AuxVertex), g_AuxVertexDecl, BGFX_BUFFER_ALLOW_RESIZE );
	//dynamicIB = bgfx::createDynamicIndexBuffer( PlatformRenderer::IB_SIZE/sizeof(UINT16), BGFX_BUFFER_ALLOW_RESIZE );

	// Create texture sampler uniforms.
	s_texColor  = bgfx::createUniform("s_texColor",  bgfx::UniformType::Int1);
	s_texNormal = bgfx::createUniform("s_texNormal", bgfx::UniformType::Int1);

	s_albedo = bgfx::createUniform("s_albedo", bgfx::UniformType::Int1);
	s_normal = bgfx::createUniform("s_normal", bgfx::UniformType::Int1);
	s_depth  = bgfx::createUniform("s_depth",  bgfx::UniformType::Int1);
	s_light  = bgfx::createUniform("s_light",  bgfx::UniformType::Int1);

	u_mtx            = bgfx::createUniform("u_mtx",            bgfx::UniformType::Mat4);
	u_lightPosRadius = bgfx::createUniform("u_lightPosRadius", bgfx::UniformType::Vec4);
	u_lightRgbInnerR = bgfx::createUniform("u_lightRgbInnerR", bgfx::UniformType::Vec4);

	u_lightVector    = bgfx::createUniform("u_lightVector",    bgfx::UniformType::Vec4);
	u_lightColor     = bgfx::createUniform("u_lightColor",     bgfx::UniformType::Vec4);
	u_inverseViewMat = bgfx::createUniform("u_inverseViewMat", bgfx::UniformType::Mat4);

	// Create program from shaders.
	geomProgram    = loadProgram("vs_deferred_geom",       "fs_deferred_geom");
	lightProgram   = loadProgram("vs_deferred_light",      "fs_deferred_light");
	combineProgram = loadProgram("vs_deferred_combine",    "fs_deferred_combine");
	debugProgram   = loadProgram("vs_deferred_debug",      "fs_deferred_debug");
	lineProgram    = loadProgram("vs_deferred_debug_line", "fs_deferred_debug_line");
	forwardProgram = loadProgram("vs_forward",             "fs_forward");

	deferred_directional_light_program = loadProgram(
		"vs_deferred_directional_light",
		"fs_deferred_directional_light"
		);

	// Load diffuse texture.
	colorMap  = loadTexture("tiles_WhiteAndGrey_01_diffuse.dds");
//	textureColor  = loadTexture("fieldstone-rgba.dds");

	// Load normal texture.
	normalMap = loadTexture("tiles_WhiteAndGrey_01_normal.dds");
//	textureNormal = loadTexture("fieldstone-n.dds");

	memset(gbufferTex, bgfx::invalidHandle, sizeof(gbufferTex));
	gbuffer.idx = bgfx::invalidHandle;
	lightBuffer.idx = bgfx::invalidHandle;

	return ALL_OK;
}
void Renderer::Shutdown()
{
	if (bgfx::isValid(gbuffer) )
	{
		bgfx::destroyFrameBuffer(gbuffer);
		bgfx::destroyFrameBuffer(lightBuffer);
	}

	//bgfx::destroyDynamicVertexBuffer(dynamicVB);
	//bgfx::destroyDynamicIndexBuffer(dynamicIB);

	bgfx::destroyIndexBuffer(cubeIB);
	bgfx::destroyVertexBuffer(cubeVB);

	bgfx::destroyProgram(geomProgram);
	bgfx::destroyProgram(lightProgram);
	bgfx::destroyProgram(combineProgram);
	bgfx::destroyProgram(debugProgram);
	bgfx::destroyProgram(lineProgram);
	bgfx::destroyProgram(forwardProgram);

	bgfx::destroyTexture(colorMap);
	bgfx::destroyTexture(normalMap);
	bgfx::destroyUniform(s_texColor);
	bgfx::destroyUniform(s_texNormal);

	bgfx::destroyUniform(s_albedo);
	bgfx::destroyUniform(s_normal);
	bgfx::destroyUniform(s_depth);
	bgfx::destroyUniform(s_light);

	bgfx::destroyUniform(u_lightPosRadius);
	bgfx::destroyUniform(u_lightRgbInnerR);
	bgfx::destroyUniform(u_mtx);

	bgfx::destroyUniform(u_lightVector);
	bgfx::destroyUniform(u_lightColor);
	bgfx::destroyUniform(u_inverseViewMat);

	// Shutdown bgfx.
	bgfx::shutdown();
}

ERet Renderer::BeginFrame( uint32_t _width, uint32_t _height, uint32_t _reset, const float view[16], float time )
{
	if (_width  != width
	||  _height != height
	||  _reset  != reset
	||  !bgfx::isValid(gbuffer) )
	{
		// Recreate variable size render targets when resolution changes.
		width  = _width;
		height = _height;
		reset  = _reset;

		if (bgfx::isValid(gbuffer) )
		{
			bgfx::destroyFrameBuffer(gbuffer);
		}

		const uint32_t samplerFlags = 0
			| BGFX_TEXTURE_RT
			| BGFX_TEXTURE_MIN_POINT
			| BGFX_TEXTURE_MAG_POINT
			| BGFX_TEXTURE_MIP_POINT
			| BGFX_TEXTURE_U_CLAMP
			| BGFX_TEXTURE_V_CLAMP
			;
		gbufferTex[RT_DEPTH] = bgfx::createTexture2D(width, height, 1, bgfx::TextureFormat::D24,   samplerFlags);
		gbufferTex[RT_ALBEDO] = bgfx::createTexture2D(width, height, 1, bgfx::TextureFormat::BGRA8, samplerFlags);
		gbufferTex[RT_NORMALS] = bgfx::createTexture2D(width, height, 1, bgfx::TextureFormat::BGRA8, samplerFlags);
		gbuffer = bgfx::createFrameBuffer(BX_COUNTOF(gbufferTex), gbufferTex, true);

		if (bgfx::isValid(lightBuffer) )
		{
			bgfx::destroyFrameBuffer(lightBuffer);
		}

		lightBuffer = bgfx::createFrameBuffer(width, height, bgfx::TextureFormat::BGRA8, samplerFlags);
	}

	// Setup views
	float vp[16];
	float invMvp[16];
	float invView[16];
	bx::mtxInverse(invView, view);

#if 1

	{
		bgfx::setViewRect(RENDER_PASS_GEOMETRY_ID,      0, 0, width, height);
		bgfx::setViewRect(RENDER_PASS_LIGHT_ID,         0, 0, width, height);
		bgfx::setViewRect(RENDER_PASS_GLOBAL_LIGHT,     0, 0, width, height);
		bgfx::setViewRect(RENDER_PASS_COMBINE_ID,       0, 0, width, height);
		bgfx::setViewRect(RENDER_PASS_DEBUG_LIGHTS_ID,  0, 0, width, height);
		bgfx::setViewRect(RENDER_PASS_DEBUG_GBUFFER_ID, 0, 0, width, height);

		bgfx::setViewClear(RENDER_PASS_GEOMETRY_ID, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH|BGFX_CLEAR_STENCIL, 0, 1.0f, 0);
		bgfx::setViewClear(RENDER_PASS_LIGHT_ID, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH|BGFX_CLEAR_STENCIL, 0, 1.0f, 0);
		bgfx::setViewClear(RENDER_PASS_GLOBAL_LIGHT, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH|BGFX_CLEAR_STENCIL, 0, 1.0f, 0);


		bgfx::setViewFrameBuffer(RENDER_PASS_LIGHT_ID, lightBuffer);
		bgfx::setViewFrameBuffer(RENDER_PASS_GLOBAL_LIGHT, lightBuffer);

		float persproj[16];
		mtxProj(persproj, 60.0f, float(width)/float(height), 0.5f, 1000.0f);

		bgfx::setViewName(RENDER_PASS_GEOMETRY_ID,"FillGBuffer");
		bgfx::setViewFrameBuffer(RENDER_PASS_GEOMETRY_ID, gbuffer);
		bgfx::setViewTransform(RENDER_PASS_GEOMETRY_ID, view, persproj);

		bx::mtxMul(vp, view, persproj);
		bx::mtxInverse(invMvp, vp);

		float proj[16];
		bx::mtxOrtho(proj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1000.0f);
		bgfx::setViewName(RENDER_PASS_LIGHT_ID,"LightID");
		bgfx::setViewTransform(RENDER_PASS_LIGHT_ID,   NULL, proj);
		bgfx::setViewName(RENDER_PASS_GLOBAL_LIGHT,"GlobalLight");
		bgfx::setViewTransform(RENDER_PASS_GLOBAL_LIGHT, view, proj);
		bgfx::setViewName(RENDER_PASS_COMBINE_ID,"CombineID");
		bgfx::setViewTransform(RENDER_PASS_COMBINE_ID, NULL, proj);

		const float aspectRatio = float(height)/float(width);
		const float size = 10.0f;
		bx::mtxOrtho(proj, -size, size, size*aspectRatio, -size*aspectRatio, 0.0f, 1000.0f);
		bgfx::setViewTransform(RENDER_PASS_DEBUG_GBUFFER_ID, NULL, proj);

		bx::mtxOrtho(proj, 0.0f, (float)width, 0.0f, (float)height, 0.0f, 1000.0f);
		bgfx::setViewTransform(RENDER_PASS_DEBUG_LIGHTS_ID, NULL, proj);


		bgfx::setViewName(RENDER_PASS_DEBUG_LINES_3D,"DebugLines");
		bgfx::setViewRect(RENDER_PASS_DEBUG_LINES_3D, 0, 0, width, height);
		//bgfx::setViewClear(RENDER_PASS_DEBUG_LINES_3D, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH|BGFX_CLEAR_STENCIL, 0, 1.0f, 0);
		bgfx::setViewTransform(RENDER_PASS_DEBUG_LINES_3D, view, persproj);
	}

	const uint32_t dim = 11;
	const float offset = (float(dim-1) * 3.0f) * 0.5f;

#if 0
	// Draw into geometry pass.
	for (uint32_t yy = 0; yy < dim; ++yy)
	{
		for (uint32_t xx = 0; xx < dim; ++xx)
		{
			float mtx[16];
			if (animateMesh)
			{
				bx::mtxRotateXY(mtx, time*1.023f + xx*0.21f, time*0.03f + yy*0.37f);
			}
			else
			{
				bx::mtxIdentity(mtx);
			}
			mtx[12] = -offset + float(xx)*3.0f;
			mtx[13] = -offset + float(yy)*3.0f;
			mtx[14] = 0.0f;

			// Set transform for draw call.
			bgfx::setTransform(mtx);

			// Set vertex and fragment shaders.
			bgfx::setUniform(u_inverseViewMat, invView);
			bgfx::setProgram(geomProgram);

			// Set vertex and index buffer.
			bgfx::setVertexBuffer(cubeVB);
			bgfx::setIndexBuffer(cubeIB);

			// Bind textures.
			bgfx::setTexture(0, s_texColor,  textureColor);
			bgfx::setTexture(1, s_texNormal, textureNormal);

			// Set render states.
			bgfx::setState(0
				| BGFX_STATE_RGB_WRITE
				| BGFX_STATE_ALPHA_WRITE
				| BGFX_STATE_DEPTH_WRITE
				| BGFX_STATE_DEPTH_TEST_LESS
				| BGFX_STATE_MSAA
				);

			bgfx::setStencil(0
				| BGFX_STENCIL_TEST_ALWAYS         // pass always
				| BGFX_STENCIL_FUNC_REF(1)         // value = 1
				| BGFX_STENCIL_FUNC_RMASK(0xff)
				| BGFX_STENCIL_OP_FAIL_S_REPLACE
				| BGFX_STENCIL_OP_FAIL_Z_REPLACE
				| BGFX_STENCIL_OP_PASS_Z_REPLACE   // store the value
				);

			// Submit primitive for rendering to view 0.
			bgfx::submit(RENDER_PASS_GEOMETRY_ID);
		}
	}
#endif

	// Draw lights into light buffer.
	if(0)
	{
		for (int32_t light = 0; light < numLights; ++light)
		{
			Sphere lightPosRadius;

			float lightTime = time * lightAnimationSpeed * (sinf(light/float(numLights) * bx::piHalf ) * 0.5f + 0.5f);
			lightPosRadius.m_center[0] = sinf( ( (lightTime + light*0.47f) + bx::piHalf*1.37f ) )*offset;
			lightPosRadius.m_center[1] = cosf( ( (lightTime + light*0.69f) + bx::piHalf*1.49f ) )*offset;
			lightPosRadius.m_center[2] = sinf( ( (lightTime + light*0.37f) + bx::piHalf*1.57f ) )*2.0f;
			lightPosRadius.m_radius = 2.0f;

			Aabb aabb;
			sphereToAabb(aabb, lightPosRadius);

			float box[8][3] =
			{
				{ aabb.m_min[0], aabb.m_min[1], aabb.m_min[2] },
				{ aabb.m_min[0], aabb.m_min[1], aabb.m_max[2] },
				{ aabb.m_min[0], aabb.m_max[1], aabb.m_min[2] },
				{ aabb.m_min[0], aabb.m_max[1], aabb.m_max[2] },
				{ aabb.m_max[0], aabb.m_min[1], aabb.m_min[2] },
				{ aabb.m_max[0], aabb.m_min[1], aabb.m_max[2] },
				{ aabb.m_max[0], aabb.m_max[1], aabb.m_min[2] },
				{ aabb.m_max[0], aabb.m_max[1], aabb.m_max[2] },
			};

			float xyz[3];
			bx::vec3MulMtxH(xyz, box[0], vp);
			float minx = xyz[0];
			float miny = xyz[1];
			float maxx = xyz[0];
			float maxy = xyz[1];
			float maxz = xyz[2];

			for (uint32_t ii = 1; ii < 8; ++ii)
			{
				bx::vec3MulMtxH(xyz, box[ii], vp);
				minx = bx::fmin(minx, xyz[0]);
				miny = bx::fmin(miny, xyz[1]);
				maxx = bx::fmax(maxx, xyz[0]);
				maxy = bx::fmax(maxy, xyz[1]);
				maxz = bx::fmax(maxz, xyz[2]);
			}

			// Cull light if it's fully behind camera.
			if (maxz >= 0.0f)
			{
				float x0 = bx::fclamp( (minx * 0.5f + 0.5f) * width,  0.0f, (float)width);
				float y0 = bx::fclamp( (miny * 0.5f + 0.5f) * height, 0.0f, (float)height);
				float x1 = bx::fclamp( (maxx * 0.5f + 0.5f) * width,  0.0f, (float)width);
				float y1 = bx::fclamp( (maxy * 0.5f + 0.5f) * height, 0.0f, (float)height);

				if (showScissorRects)
				{
					bgfx::TransientVertexBuffer tvb;
					bgfx::TransientIndexBuffer tib;
					if (bgfx::allocTransientBuffers(&tvb, DebugVertex::ms_decl, 4, &tib, 8) )
					{
						uint32_t abgr = 0x8000ff00;

						DebugVertex* vertex = (DebugVertex*)tvb.data;
						vertex->m_x = x0;
						vertex->m_y = y0;
						vertex->m_z = 0.0f;
						vertex->m_abgr = abgr;
						++vertex;

						vertex->m_x = x1;
						vertex->m_y = y0;
						vertex->m_z = 0.0f;
						vertex->m_abgr = abgr;
						++vertex;

						vertex->m_x = x1;
						vertex->m_y = y1;
						vertex->m_z = 0.0f;
						vertex->m_abgr = abgr;
						++vertex;

						vertex->m_x = x0;
						vertex->m_y = y1;
						vertex->m_z = 0.0f;
						vertex->m_abgr = abgr;

						uint16_t* indices = (uint16_t*)tib.data;
						*indices++ = 0;
						*indices++ = 1;
						*indices++ = 1;
						*indices++ = 2;
						*indices++ = 2;
						*indices++ = 3;
						*indices++ = 3;
						*indices++ = 0;

						bgfx::setVertexBuffer(&tvb);
						bgfx::setIndexBuffer(&tib);
						bgfx::setState(0
							| BGFX_STATE_RGB_WRITE
							| BGFX_STATE_PT_LINES
							| BGFX_STATE_BLEND_ALPHA
							);
						bgfx::submit(RENDER_PASS_DEBUG_LIGHTS_ID, lineProgram);
					}
				}

				uint8_t val = light&7;
				float lightRgbInnerR[4] =
				{
					val & 0x1 ? 1.0f : 0.25f,
					val & 0x2 ? 1.0f : 0.25f,
					val & 0x4 ? 1.0f : 0.25f,
					0.8f,
				};

				// Draw light.
				bgfx::setUniform(u_lightPosRadius, &lightPosRadius);
				bgfx::setUniform(u_lightRgbInnerR, lightRgbInnerR);
				bgfx::setUniform(u_mtx, invMvp);
				const uint16_t scissorHeight = uint16_t(y1-y0);
				bgfx::setScissor(uint16_t(x0), height-scissorHeight-uint16_t(y0), uint16_t(x1-x0), scissorHeight);
				bgfx::setTexture(0, s_normal, gbuffer, 1);
				bgfx::setTexture(1, s_depth,  gbuffer, 2);
				bgfx::setState(0
					| BGFX_STATE_RGB_WRITE
					| BGFX_STATE_ALPHA_WRITE
					| BGFX_STATE_BLEND_ADD
					);
				screenSpaceTriangle( (float)width, (float)height, g_texelHalf, g_originBottomLeft);
				bgfx::submit(RENDER_PASS_LIGHT_ID, lightProgram);
			}
		}
	}

	if(1)
	{
		//bgfx::setMarker("dir light");
		float lightDirection[4] = { -1, -1, -1, 0 };//float lightDirection[4] = { 0, -1, 0, 0 };//Vector4_Normalized(Vector4_Set( -1, -1, -1, 0 ));
		float viewSpaceLightDirection[4] = { 0 };
		bx::vec3MulMtx(viewSpaceLightDirection, lightDirection, view);

		float lightVectorWS[4] = { 0, 1, 0, 0 };
		float lightVector[4] = { 0, 1, 0, 0 };
		bx::vec3MulMtx(lightVector, lightVectorWS, view);
		//bx::vec3Neg(lightVector, lightDirection);
		//bx::vec3Norm(lightVector, lightVector);

		//float lightColor[4] = { 0, 1, 0, 1 };
		float lightColor[4] = { 0.4f, 0.6f, 0.5f, 1 };

		bgfx::setTransform(invView);
		bgfx::setUniform(u_lightVector, lightVector);
		bgfx::setUniform(u_lightColor, &lightColor);
		bgfx::setUniform(u_inverseViewMat, invView);
		bgfx::setTexture(0, s_normal, gbuffer, RT_NORMALS);
		bgfx::setTexture(1, s_depth,  gbuffer, RT_DEPTH);
		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			| BGFX_STATE_BLEND_ADD
			);
		//bgfx::setStencil(0
		//	| BGFX_STENCIL_TEST_NOTEQUAL
		//	| BGFX_STENCIL_FUNC_REF(1)
		//	| BGFX_STENCIL_FUNC_RMASK(0xFF)
		//	| BGFX_STENCIL_OP_FAIL_S_KEEP
		//	| BGFX_STENCIL_OP_FAIL_Z_KEEP
		//	| BGFX_STENCIL_OP_PASS_Z_KEEP
		//	);
		screenSpaceTriangle( (float)width, (float)height, g_texelHalf, g_originBottomLeft, 1.0f);
		bgfx::submit(RENDER_PASS_GLOBAL_LIGHT, deferred_directional_light_program);
	}

	// Combine color and light buffers.
	bgfx::setTexture(0, s_albedo, gbuffer,     RT_ALBEDO);
	bgfx::setTexture(1, s_light,  lightBuffer, 0);
	bgfx::setState(0
		| BGFX_STATE_RGB_WRITE
		| BGFX_STATE_ALPHA_WRITE
		);
	screenSpaceTriangle( (float)width, (float)height, g_texelHalf, g_originBottomLeft);
	bgfx::submit(RENDER_PASS_COMBINE_ID, combineProgram);

	if (showGBuffer)
	{
		const float aspectRatio = float(width)/float(height);

		// Draw debug GBuffer.
		for (uint32_t ii = 0; ii < BX_COUNTOF(gbufferTex); ++ii)
		{
			float mtx[16];
			bx::mtxSRT(mtx
				, aspectRatio, 1.0f, 1.0f
				, 0.0f, 0.0f, 0.0f
				, -7.9f - BX_COUNTOF(gbufferTex)*0.1f*0.5f + ii*2.1f*aspectRatio, 4.0f, 0.0f
				);
			bgfx::setTransform(mtx);
			bgfx::setVertexBuffer(cubeVB);
			bgfx::setIndexBuffer(cubeIB, 0, 6);
			bgfx::setTexture(0, s_texColor, gbufferTex[ii]);
			bgfx::setState(BGFX_STATE_RGB_WRITE);
			bgfx::submit(RENDER_PASS_DEBUG_GBUFFER_ID, debugProgram);
		}
	}

#else

		float lightDirection[4] = { 0, -1, 0, 0 };//Vector4_Normalized(Vector4_Set( -1, -1, -1, 0 ));
		float viewSpaceLightDirection[4] = { 0 };
		bx::vec3MulMtx(viewSpaceLightDirection, lightDirection, view);


	{
		bgfx::setViewRect(RENDER_PASS_FORWARD, 0, 0, width, height);
		bgfx::setViewClear(RENDER_PASS_FORWARD, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH|BGFX_CLEAR_STENCIL, 0, 1.0f, 0);
		bgfx::FrameBufferHandle defaultFrameBuffer = BGFX_INVALID_HANDLE;
		bgfx::setViewFrameBuffer(RENDER_PASS_FORWARD, defaultFrameBuffer);

		float proj[16];
		mtxProj(proj, 60.0f, float(width)/float(height), 0.5f, 1000.0f);

		bgfx::setViewTransform(RENDER_PASS_FORWARD, view, proj);


		const uint32_t dim = 11;
		const float offset = (float(dim-1) * 3.0f) * 0.5f;

		for (uint32_t yy = 0; yy < dim; ++yy)
		{
			for (uint32_t xx = 0; xx < dim; ++xx)
			{
				float mtx[16];
				if (animateMesh)
				{
					bx::mtxRotateXY(mtx, time*1.023f + xx*0.21f, time*0.03f + yy*0.37f);
				}
				else
				{
					bx::mtxIdentity(mtx);
				}
				mtx[12] = -offset + float(xx)*3.0f;
				mtx[13] = -offset + float(yy)*3.0f;
				mtx[14] = 0.0f;

				// Set transform for draw call.
				bgfx::setTransform(mtx);

				// Set vertex and fragment shaders.
				bgfx::setUniform(u_inverseViewMat, invView);
				bgfx::setUniform(u_lightVector, viewSpaceLightDirection);
				bgfx::setProgram(forwardProgram);

				// Set vertex and index buffer.
				bgfx::setVertexBuffer(cubeVB);
				bgfx::setIndexBuffer(cubeIB);

				// Bind textures.
				bgfx::setTexture(0, s_texColor,  colorMap);
				bgfx::setTexture(1, s_texNormal, normalMap);

				// Set render states.
				bgfx::setState(0
					| BGFX_STATE_RGB_WRITE
					| BGFX_STATE_ALPHA_WRITE
					| BGFX_STATE_DEPTH_WRITE
					| BGFX_STATE_DEPTH_TEST_LESS
					| BGFX_STATE_MSAA
					);
				bgfx::submit(RENDER_PASS_FORWARD);
			}
		}
	}
#endif

	return ALL_OK;
}

ERet Renderer::EndFrame()
{
	// Advance to next frame. Rendering thread will be kicked to
	// process submitted rendering primitives.
	bgfx::frame();

	return ALL_OK;
}

ERet Renderer::DrawWireframe( const TArray< BSP::Vertex >& vertices, const TArray< UINT16 >& indices )
{
#if 0
	const int numVertices = vertices.Num();
	const int numTriangles = indices.Num() / 3;
	const int numIndices = numTriangles * 3 * 2;

	bgfx::TransientVertexBuffer vb;
	bgfx::TransientIndexBuffer ib;

	if( bgfx::allocTransientBuffers( &vb, BSP::Vertex::ms_decl, numVertices, &ib, numIndices ) )
	{
		BSP::Vertex* vertexData = (BSP::Vertex*)vb.data;
		UINT16* indexData = (UINT16*)ib.data;

		for( int iVertex = 0; iVertex < numVertices; iVertex++ )
		{
			vertexData[ iVertex ] = vertices[ iVertex ];
		}
		for( int iTriangle = 0; iTriangle < numTriangles; iTriangle++ )
		{
			const int base = iTriangle * 3;
			const UINT16* tri = indices.ToPtr() + base;
			indexData[ base*2 + 0 ] = tri[0];
			indexData[ base*2 + 1 ] = tri[1];
			indexData[ base*2 + 2 ] = tri[1];
			indexData[ base*2 + 3 ] = tri[2];
			indexData[ base*2 + 4 ] = tri[2];
			indexData[ base*2 + 5 ] = tri[0];
		}

		bgfx::setVertexBuffer(&vb);
		bgfx::setIndexBuffer(&ib);

		bgfx::setTexture(0, s_texColor,  textureColor);
		bgfx::setTexture(1, s_texNormal, textureNormal);

		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			| BGFX_STATE_PT_LINES
			//| BGFX_STATE_CULL_CCW
			//| BGFX_STATE_DEPTH_TEST_LEQUAL
			| BGFX_STATE_DEPTH_TEST_ALWAYS
			| BGFX_STATE_MSAA
			);

		bgfx::submit(RENDER_PASS_GEOMETRY_ID, lineProgram);

		return ALL_OK;
	}
#endif

	const int numVertices = vertices.Num() * 2;
	const int numTriangles = indices.Num() / 3;

	bgfx::TransientVertexBuffer vb;
	if( bgfx::checkAvailTransientVertexBuffer( numVertices, BSP::Vertex::ms_decl ) )
	{
		bgfx::allocTransientVertexBuffer( &vb, numVertices, BSP::Vertex::ms_decl );
		BSP::Vertex* vertexData = (BSP::Vertex*)vb.data;

		for( int iTriangle = 0; iTriangle < numTriangles; iTriangle++ )
		{
			const int base = iTriangle * 3;
			const UINT16* tri = indices.ToPtr() + base;

			*vertexData++ = vertices[tri[0]];
			*vertexData++ = vertices[tri[1]];
			*vertexData++ = vertices[tri[1]];
			*vertexData++ = vertices[tri[2]];
			*vertexData++ = vertices[tri[2]];
			*vertexData++ = vertices[tri[0]];
		}

		bgfx::setVertexBuffer(&vb);

		bgfx::setTexture(0, s_texColor,  colorMap);
		bgfx::setTexture(1, s_texNormal, normalMap);

		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			| BGFX_STATE_PT_LINES
			//| BGFX_STATE_CULL_CCW
			//| BGFX_STATE_DEPTH_TEST_LEQUAL
			| BGFX_STATE_DEPTH_TEST_ALWAYS
			| BGFX_STATE_MSAA
			);

		bgfx::submit(RENDER_PASS_GEOMETRY_ID, lineProgram);

		return ALL_OK;
	}

	return ERR_OUT_OF_MEMORY;
}

void Renderer::Draw(
	const AuxVertex* _vertices,
	const UINT32 _numVertices,
	const UINT16* _indices,
	const UINT32 _numIndices,
	const Topology::Enum topology,
	const UINT64 shaderID
)
{
#if 1
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;
	if( bgfx::allocTransientBuffers( &tvb, g_AuxVertexDecl, _numVertices, &tib, _numIndices ) )
	{
		memcpy( tvb.data, _vertices, _numVertices * sizeof(_vertices[0]) );
		memcpy( tib.data, _indices, _numIndices * sizeof(_indices[0]) );

		bgfx::setVertexBuffer( &tvb, 0,_numVertices );
		bgfx::setIndexBuffer( &tib, 0,_numIndices );
//DBG_PrintArray(_indices,_numIndices, LogStream(LL_Debug));
		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			
			//| BGFX_STATE_DEPTH_TEST_LEQUAL
			//| BGFX_STATE_DEPTH_TEST_ALWAYS

			| ToBgfxTopology(topology)
		);

		bgfx::submit( RENDER_PASS_DEBUG_LINES_3D, lineProgram );
	}
#else

	//{
	//	LogStream log(LL_Debug);
	//	log << "Topology: " << GetTypeOf_TopologyT().GetStringByValue(topology) << "\n";
	//	DBG_PrintArray(_indices,_numIndices, log);
	//	for( int i = 0; i < _numIndices; i++ ) {
	//		log << "Vertex ["<<i<<"]: " << _vertices[i].xyz << "\n";
	//	}
	//}

	const bgfx::Memory* vertexMemory = bgfx::copy( _vertices, sizeof(_vertices[0])*_numVertices );
	const bgfx::Memory* indexMemory = bgfx::copy( _indices, sizeof(_indices[0])*_numIndices );
	bgfx::updateDynamicVertexBuffer( dynamicVB, 0, vertexMemory );
	bgfx::updateDynamicIndexBuffer( dynamicIB, 0, indexMemory );

		bgfx::setVertexBuffer( dynamicVB );
		bgfx::setIndexBuffer( dynamicIB );

		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			| BGFX_STATE_DEPTH_TEST_LEQUAL
			|BGFX_STATE_PT_LINES
			//| ToBgfxTopology(topology)
		);

		bgfx::submit( RENDER_PASS_DEBUG_LINES_3D, lineProgram );
#endif
}

#if 0
void Renderer::Draw( const DynamicMesh& mesh )
{
	bgfx::setVertexBuffer(mesh.hVB);
	bgfx::setIndexBuffer(mesh.hIB);

	bgfx::setTexture(0, s_texColor,  textureColor);
	bgfx::setTexture(1, s_texNormal, textureNormal);

	bgfx::setState(0
		| BGFX_STATE_RGB_WRITE
		| BGFX_STATE_ALPHA_WRITE
		| BGFX_STATE_CULL_CCW
		| BGFX_STATE_DEPTH_WRITE
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_MSAA
		);

	bgfx::submit(RENDER_PASS_GEOMETRY_ID, geomProgram);
}
#endif

bool animateMesh = false;
bool showScissorRects = false;
bool showGBuffer = true;
int32_t numLights = 512;
float lightAnimationSpeed = 0.0f;
