/*
stolen from:
https://github.com/299299/NagaGame/blob/master/Source/Game/Engine/Graphics/DebugDraw.cpp
*/
#include <Base/Base.h>
#include <Base/Util/LogUtil.h>
#include <Base/Util/Color.h>
#include "render.h"
#include "bsp.h"
#include "csg.h"
#include "debug_draw.h"

static void DbgRemoveFirstPoly( BSP::Tree& worldTree, BSP::NodeID nodeIndex )
{
	BSP::Node& node = worldTree.m_nodes[ nodeIndex ];
	BSP::Face& face = worldTree.m_faces[ node.faces ];
	node.faces = face.next;
}

ERet MyEntryPoint()
{
	SetupBaseUtil	setupBase;
	FileLogUtil		fileLog;


	DBGOUT("sizeof(Node)=%d, sizeof(Face)=%d", sizeof(BSP::Node), sizeof(BSP::Face));


	BSP::Vertex::init();

	Renderer	renderer;
	mxDO(renderer.Initialize());


	//DynamicMesh	debugWireframe;
	//debugWireframe.hVB = bgfx::createDynamicVertexBuffer( 1024, BSP::Vertex::ms_decl, BGFX_BUFFER_ALLOW_RESIZE );
	//debugWireframe.hIB = bgfx::createDynamicIndexBuffer( 1024, BGFX_BUFFER_NONE );


	CSG	csg;
	mxDO(csg.Initialize());



	int fireRate = 10; // shots per second
	int64_t lastTimeShot = 0;


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
		const int64_t now = bx::getHPCounter();
		static int64_t last = now;
		const int64_t frameTime = now - last;
		last = now;
		const double freq = double(bx::getHPFrequency() );	// clock rate, in Hertz
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
			csg.Draw( renderer );
		}

#if 0
		if( lastHit.hitAnything )
		{
			float mtx[16];
			bx::mtxTranslate(mtx,lastHit.position.x, lastHit.position.y, lastHit.position.z);
			bx::mtxScale(mtx,0.3,0.3,0.3);

			bgfx::setTransform(mtx);

			bgfx::setVertexBuffer(renderer.cubeVB);
			bgfx::setIndexBuffer(renderer.cubeIB);

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
#endif


#if 0
		{
			float invView[16];
			bx::mtxInverse(invView, view);
			bgfx::setUniform(renderer.u_inverseViewMat, invView);

			bgfx::setVertexBuffer(g_dynamicVB);
			bgfx::setIndexBuffer(g_dynamicIB);

			bgfx::setTexture(0, renderer.s_texColor,  renderer.textureColor);
			bgfx::setTexture(1, renderer.s_texNormal, renderer.textureNormal);

			bgfx::setState(0
				| BGFX_STATE_RGB_WRITE
				| BGFX_STATE_ALPHA_WRITE
				| BGFX_STATE_PT_LINESTRIP
				| BGFX_STATE_CULL_CCW
				| BGFX_STATE_DEPTH_WRITE
				| BGFX_STATE_DEPTH_TEST_LEQUAL
				| BGFX_STATE_MSAA
				);
			bgfx::submit(RENDER_PASS_GEOMETRY_ID, renderer.geomProgram);
		}
#endif



		//renderer.DrawWireframe(rawVertices,rawIndices);


		renderer.EndFrame();

		if( !!mouseState.m_buttons[entry::MouseButton::Left] )
		{
			const int64_t timeElapsed = now - lastTimeShot;
			double secondsElapsed = double(timeElapsed)/freq;

			if( secondsElapsed > (1.0f/fireRate) )
			{
				DBGOUT("Shooting");
				lastTimeShot = now;

#if 1
				Float3 rayPos, lookAt, rayDir;
				cameraGetPosition((float*)&rayPos);
				cameraGetAt((float*)&lookAt);
				rayDir = Float3_Normalized(lookAt - rayPos);

				csg.Shoot( rayPos, rayDir, lastHit );

//				worldTree.CastRay( rayPos, rayDir, lastHit );
//				if( lastHit.hitAnything )
//				{
////					LogStream(LL_Info) << "Hit pos: " << lastHit.position;
//					Subtract(
//						lastHit.position,
//						worldTree,
//						operand,
//						temporary
//						
//					);
//					Generate_CPU_Mesh(
//						worldTree, 0,
//						rawVertices, rawIndices
//					);
//				}
#else
				Float3 pos = Float3_Set(0,-10,0);

				Subtract(
					pos,
					worldTree,
					operand,
					temporary
					);

				Generate_CPU_Mesh(
					worldTree, 0,
					rawVertices, rawIndices
				);
#endif
			}		
		}
	}

	// Cleanup.
	cameraDestroy();
	imguiDestroy();

	//bgfx::destroyDynamicVertexBuffer( debugWireframe.hVB  );
	//bgfx::destroyDynamicIndexBuffer( debugWireframe.hIB  );

	csg.Shutdown();

	renderer.Shutdown();

	return ALL_OK;
}

int _main_(int /*_argc*/, char** /*_argv*/)
{
	MyEntryPoint();

	return 0;
}
