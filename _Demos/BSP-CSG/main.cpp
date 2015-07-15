/*
 * Copyright 2011-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */
#include <Base/Base.h>
#include <Base/Util/LogUtil.h>
#include "render.h"

ERet MyEntryPoint()
{
	SetupBaseUtil	setupBase;
	FileLogUtil		fileLog;

	Renderer	renderer;
	mxDO(renderer.Initialize());

	// Imgui.
	imguiCreate();

	const int64_t timeOffset = bx::getHPCounter();

	int32_t scrollArea = 0;

	float view[16];
	float initialPos[3] = { 0.0f, 0.0f, -15.0f };
	cameraCreate();
	cameraSetPosition(initialPos);
	cameraSetVerticalAngle(0.0f);
	cameraGetViewMtx(view);

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

		// Use debug font to print information about this example.
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 1, 0x4f, "bgfx/examples/21-deferred");
		bgfx::dbgTextPrintf(0, 2, 0x6f, "Description: MRT rendering and deferred shading.");
		bgfx::dbgTextPrintf(0, 3, 0x0f, "Frame: % 7.3f[ms]", double(frameTime)*toMs);

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
		cameraGetViewMtx(view);

		renderer.BeginFrame( width, height, reset, view, time );
		renderer.EndFrame();
	}

	// Cleanup.
	cameraDestroy();
	imguiDestroy();

	renderer.Shutdown();

	return ALL_OK;
}

int _main_(int /*_argc*/, char** /*_argv*/)
{
	MyEntryPoint();

	return 0;
}
