//based on
/*
 * Copyright 2011-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */
#include <Base/Base.h>
#include <Base/Util/LogUtil.h>
#include "render.h"
#include "bsp.h"

static bgfx::DynamicVertexBufferHandle g_dynamicVB = BGFX_INVALID_HANDLE;
static bgfx::DynamicIndexBufferHandle g_dynamicIB = BGFX_INVALID_HANDLE;

static BSP::Vertex g_planeVertices[4] =
{
	//          XYZ              |            N              | T |    U  |  V
	{ { -100.0f, 0.0f, -100.0f }, packF4u( 0.0f, 1.0f, 0.0f ), 0,  {    0, 1.0f }, 0, },
	{ { -100.0f, 0.0f,  100.0f }, packF4u( 0.0f, 1.0f, 0.0f ), 0,  {    0,    0 }, 0, },
	{ {  100.0f, 0.0f,  100.0f }, packF4u( 0.0f, 1.0f, 0.0f ), 0,  { 1.0f,    0 }, 0, },
	{ {  100.0f, 0.0f, -100.0f }, packF4u( 0.0f, 1.0f, 0.0f ), 0,  { 1.0f, 1.0f }, 0, },
};

const UINT16 g_planeIndices[6] = {
	0, 1, 2, 0, 2, 3,
};

ERet MyEntryPoint()
{
	SetupBaseUtil	setupBase;
	FileLogUtil		fileLog;


	Renderer	renderer;
	mxDO(renderer.Initialize());

	BSP::Vertex::init();

	calcTangents( g_planeVertices, BX_COUNTOF(g_planeVertices), BSP::Vertex::ms_decl,
		g_planeIndices, BX_COUNTOF(g_planeIndices) );

	BSP::Tree	tree;
	{
		using namespace BSP;
		struct EnumerateMeshVertices : ATriangleMeshInterface
		{
			virtual void ProcessAllTriangles( ATriangleIndexCallback* callback ) override
			{
				callback->ProcessTriangle(
					g_planeVertices[ g_planeIndices[0] ],
					g_planeVertices[ g_planeIndices[1] ],
					g_planeVertices[ g_planeIndices[2] ]
					);
				callback->ProcessTriangle(
					g_planeVertices[ g_planeIndices[3] ],
					g_planeVertices[ g_planeIndices[4] ],
					g_planeVertices[ g_planeIndices[5] ]
					);
			}
		} enumerateMeshVertices;

		tree.Build( &enumerateMeshVertices );
	}

	{

	}


	g_dynamicVB = bgfx::createDynamicVertexBuffer( 1024, BSP::Vertex::ms_decl, BGFX_BUFFER_ALLOW_RESIZE );
	g_dynamicIB = bgfx::createDynamicIndexBuffer( 1024, BGFX_BUFFER_NONE );

	const bgfx::Memory* vertexMemory = bgfx::makeRef( g_planeVertices, sizeof(g_planeVertices) );
	const bgfx::Memory* indexMemory = bgfx::makeRef( g_planeIndices, sizeof(g_planeIndices) );

	bgfx::updateDynamicVertexBuffer( g_dynamicVB, 0, vertexMemory );
	bgfx::updateDynamicIndexBuffer( g_dynamicIB, 0, indexMemory );


	// Imgui.
	imguiCreate();

	const int64_t timeOffset = bx::getHPCounter();

	int32_t scrollArea = 0;

	float initialPos[3] = { 0.0f, 10.0f, -15.0f };
	cameraCreate();
	cameraSetPosition(initialPos);
	cameraSetVerticalAngle(0.0f);

	BSP::RayCastResult lastHit;

	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t debug = 0;
	uint32_t reset = 0;
	entry::MouseState mouseState;
	while (!entry::processEvents(width, height, debug, reset, &mouseState) )
	{
		int64_t now = bx::getHPCounter();
		static int64_t last = now;
		const int64_t frameTime = now - last;
		last = now;
		const double freq = double(bx::getHPFrequency() );
		const double toMs = 1000.0/freq;
		const float deltaTime = float(frameTime/freq);

		float time = (float)( (now-timeOffset)/freq);

		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 0, 0x0f, "Frame: % 7.3f[ms]", double(frameTime)*toMs);

		imguiBeginFrame(mouseState.m_mx
			, mouseState.m_my
			, (mouseState.m_buttons[entry::MouseButton::Left  ] ? IMGUI_MBUT_LEFT  : 0)
			| (mouseState.m_buttons[entry::MouseButton::Right ] ? IMGUI_MBUT_RIGHT : 0)
			, mouseState.m_mz
			, width
			, height
			);

		imguiBeginScrollArea("Settings", width - width / 5 - 10, 10, width / 5, height / 3, &scrollArea);
		imguiSeparatorLine();

		imguiSlider("Num lights", numLights, 1, 2048);

		if (imguiCheck("Show G-Buffer.", showGBuffer) )
		{
			showGBuffer = !showGBuffer;
		}

		if (imguiCheck("Show light scissor.", showScissorRects) )
		{
			showScissorRects = !showScissorRects;
		}

		if (imguiCheck("Animate mesh.", animateMesh) )
		{
			animateMesh = !animateMesh;
		}

		imguiSlider("Lights animation speed", lightAnimationSpeed, 0.0f, 0.4f, 0.01f);

		imguiEndScrollArea();
		imguiEndFrame();

		// Update camera.
		cameraUpdate(deltaTime, mouseState);

		float view[16];
		cameraGetViewMtx(view);

		renderer.BeginFrame( width, height, reset, view, time );

		if(1)
		{
			float invView[16];
			bx::mtxInverse(invView, view);
			bgfx::setUniform(renderer.u_inverseViewMat, invView);
			bgfx::setProgram(renderer.geomProgram);

			bgfx::setVertexBuffer(g_dynamicVB);
			bgfx::setIndexBuffer(g_dynamicIB);

			bgfx::setTexture(0, renderer.s_texColor,  renderer.textureColor);
			bgfx::setTexture(1, renderer.s_texNormal, renderer.textureNormal);

			bgfx::setState(0
				| BGFX_STATE_RGB_WRITE
				| BGFX_STATE_ALPHA_WRITE
				| BGFX_STATE_DEPTH_WRITE
				| BGFX_STATE_DEPTH_TEST_LESS
				| BGFX_STATE_MSAA
				);

			bgfx::submit(RENDER_PASS_GEOMETRY_ID);
		}

		if( lastHit.hitAnything )
		{
			float mtx[16];
			bx::mtxTranslate(mtx,lastHit.position.x,lastHit.position.y,lastHit.position.z);
			bx::mtxScale(mtx,0.3,0.3,0.3);


			bgfx::setTransform(mtx);

			bgfx::setVertexBuffer(renderer.vbh);
			bgfx::setIndexBuffer(renderer.ibh);

			bgfx::setTexture(0, renderer.s_texColor,  renderer.textureColor);
			bgfx::setTexture(1, renderer.s_texNormal, renderer.textureNormal);

			bgfx::setState(0
				| BGFX_STATE_RGB_WRITE
				| BGFX_STATE_ALPHA_WRITE
				| BGFX_STATE_DEPTH_WRITE
				| BGFX_STATE_DEPTH_TEST_LESS
				| BGFX_STATE_MSAA
				);
			bgfx::submit(RENDER_PASS_DEBUG_LINES_3D);
		}

		renderer.EndFrame();

		if( !!mouseState.m_buttons[entry::MouseButton::Left] )
		{
			DBGOUT("LMB down!");
		}
	}

	// Cleanup.
	cameraDestroy();
	imguiDestroy();


	bgfx::destroyDynamicIndexBuffer( g_dynamicIB );
	bgfx::destroyDynamicVertexBuffer( g_dynamicVB );

	//bgfx::release( indexMemory );
	//bgfx::release( vertexMemory );

	renderer.Shutdown();

	return ALL_OK;
}

int _main_(int /*_argc*/, char** /*_argv*/)
{
	MyEntryPoint();

	return 0;
}
