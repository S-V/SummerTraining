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

void MakeBoxMesh(
				 float length, float height, float depth,
				 TArray< BSP::Vertex > &vertices, TArray< UINT16 > &indices
				 )
{
	enum { NUM_VERTICES = 24 };
	enum { NUM_INDICES = 36 };

	// Create vertex buffer.

	vertices.SetNum( NUM_VERTICES );

	const float HL = 0.5f * length;
	const float HH = 0.5f * height;
	const float HD = 0.5f * depth;

	// Fill in the front face vertex data.
	vertices[0]  = BSP::Vertex( Float3_Set( -HL, -HH, -HD ),	packF4u( 0.0f, 0.0f, -1.0f ), 	packF4u( 1.0f, 0.0f, 0.0f ), 	Float2_Set( 0.0f, 1.0f )	);
	vertices[1]  = BSP::Vertex( Float3_Set( -HL,  HH, -HD ),	packF4u( 0.0f, 0.0f, -1.0f ), 	packF4u( 1.0f, 0.0f, 0.0f ), 	Float2_Set( 0.0f, 0.0f )	);
	vertices[2]  = BSP::Vertex( Float3_Set(  HL,  HH, -HD ),	packF4u( 0.0f, 0.0f, -1.0f ), 	packF4u( 1.0f, 0.0f, 0.0f ), 	Float2_Set( 1.0f, 0.0f )	);
	vertices[3]  = BSP::Vertex( Float3_Set(  HL, -HH, -HD ),	packF4u( 0.0f, 0.0f, -1.0f ), 	packF4u( 1.0f, 0.0f, 0.0f ), 	Float2_Set( 1.0f, 1.0f )	);

	// Fill in the back face vertex data.
	vertices[4]  = BSP::Vertex( Float3_Set( -HL, -HH, HD ), 	packF4u( 0.0f, 0.0f, 1.0f ), 	packF4u( -1.0f, 0.0f, 0.0f ),	Float2_Set( 1.0f, 1.0f )	);
	vertices[5]  = BSP::Vertex( Float3_Set(  HL, -HH, HD ), 	packF4u( 0.0f, 0.0f, 1.0f ), 	packF4u( -1.0f, 0.0f, 0.0f ),	Float2_Set( 0.0f, 1.0f )	);
	vertices[6]  = BSP::Vertex( Float3_Set(  HL,  HH, HD ), 	packF4u( 0.0f, 0.0f, 1.0f ), 	packF4u( -1.0f, 0.0f, 0.0f ),	Float2_Set( 0.0f, 0.0f )	);
	vertices[7]  = BSP::Vertex( Float3_Set( -HL,  HH, HD ), 	packF4u( 0.0f, 0.0f, 1.0f ), 	packF4u( -1.0f, 0.0f, 0.0f ),	Float2_Set( 1.0f, 0.0f )	);

	// Fill in the top face vertex data.
	vertices[8]  = BSP::Vertex( Float3_Set( -HL, HH, -HD ),		packF4u( 0.0f, 1.0f, 0.0f ), 	packF4u( 1.0f, 0.0f, 0.0f ), 	Float2_Set( 0.0f, 1.0f )	);
	vertices[9]  = BSP::Vertex( Float3_Set( -HL, HH,  HD ),		packF4u( 0.0f, 1.0f, 0.0f ), 	packF4u( 1.0f, 0.0f, 0.0f ), 	Float2_Set( 0.0f, 0.0f )	);
	vertices[10] = BSP::Vertex( Float3_Set(  HL, HH,  HD ),		packF4u( 0.0f, 1.0f, 0.0f ), 	packF4u( 1.0f, 0.0f, 0.0f ), 	Float2_Set( 1.0f, 0.0f )	);
	vertices[11] = BSP::Vertex( Float3_Set(  HL, HH, -HD ),		packF4u( 0.0f, 1.0f, 0.0f ), 	packF4u( 1.0f, 0.0f, 0.0f ), 	Float2_Set( 1.0f, 1.0f )	);

	// Fill in the bottom face vertex data.
	vertices[12] = BSP::Vertex( Float3_Set( -HL, -HH, -HD ),	packF4u( 0.0f, -1.0f, 0.0f ), 	packF4u( -1.0f, 0.0f, 0.0f ),	Float2_Set( 1.0f, 1.0f )	);
	vertices[13] = BSP::Vertex( Float3_Set(  HL, -HH, -HD ),	packF4u( 0.0f, -1.0f, 0.0f ), 	packF4u( -1.0f, 0.0f, 0.0f ),	Float2_Set( 0.0f, 1.0f )	);
	vertices[14] = BSP::Vertex( Float3_Set(  HL, -HH,  HD ),	packF4u( 0.0f, -1.0f, 0.0f ), 	packF4u( -1.0f, 0.0f, 0.0f ),	Float2_Set( 0.0f, 0.0f )	);
	vertices[15] = BSP::Vertex( Float3_Set( -HL, -HH,  HD ),	packF4u( 0.0f, -1.0f, 0.0f ), 	packF4u( -1.0f, 0.0f, 0.0f ),	Float2_Set( 1.0f, 0.0f )	);

	// Fill in the left face vertex data.
	vertices[16] = BSP::Vertex( Float3_Set( -HL, -HH,  HD ),	packF4u( -1.0f, 0.0f, 0.0f ), 	packF4u( 0.0f, 0.0f, -1.0f ),	Float2_Set( 0.0f, 1.0f )	);
	vertices[17] = BSP::Vertex( Float3_Set( -HL,  HH,  HD ),	packF4u( -1.0f, 0.0f, 0.0f ), 	packF4u( 0.0f, 0.0f, -1.0f ),	Float2_Set( 0.0f, 0.0f )	);
	vertices[18] = BSP::Vertex( Float3_Set( -HL,  HH, -HD ),	packF4u( -1.0f, 0.0f, 0.0f ), 	packF4u( 0.0f, 0.0f, -1.0f ),	Float2_Set( 1.0f, 0.0f )	);
	vertices[19] = BSP::Vertex( Float3_Set( -HL, -HH, -HD ),	packF4u( -1.0f, 0.0f, 0.0f ), 	packF4u( 0.0f, 0.0f, -1.0f ),	Float2_Set( 1.0f, 1.0f )	);

	// Fill in the right face vertex data.
	vertices[20] = BSP::Vertex( Float3_Set(  HL, -HH, -HD ),	packF4u( 1.0f, 0.0f, 0.0f ), 	packF4u( 0.0f, 0.0f, 1.0f ), 	Float2_Set( 0.0f, 1.0f )	);
	vertices[21] = BSP::Vertex( Float3_Set(  HL,  HH, -HD ),	packF4u( 1.0f, 0.0f, 0.0f ), 	packF4u( 0.0f, 0.0f, 1.0f ), 	Float2_Set( 0.0f, 0.0f )	);
	vertices[22] = BSP::Vertex( Float3_Set(  HL,  HH,  HD ),	packF4u( 1.0f, 0.0f, 0.0f ), 	packF4u( 0.0f, 0.0f, 1.0f ), 	Float2_Set( 1.0f, 0.0f )	);
	vertices[23] = BSP::Vertex( Float3_Set(  HL, -HH,  HD ),	packF4u( 1.0f, 0.0f, 0.0f ), 	packF4u( 0.0f, 0.0f, 1.0f ), 	Float2_Set( 1.0f, 1.0f )	);

	//// Scale the box.
	//{
	//	Float4x4 scaleMatrix = Matrix_Scaling( length, height, depth );
	//	Float4x4 scaleMatrixIT = Matrix_Transpose( Matrix_Inverse( scaleMatrix ) );

	//	for( UINT i = 0; i < NUM_VERTICES; ++i )
	//	{
	//		scaleMatrixIT.TransformNormal( vertices[i].Normal );
	//		scaleMatrixIT.TransformNormal( vertices[i].Tangent );
	//	}
	//}

	// Create the index buffer.

	indices.SetNum( NUM_INDICES );

	// Fill in the front face index data
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 0; indices[4] = 2; indices[5] = 3;

	// Fill in the back face index data
	indices[6] = 4; indices[7]  = 5; indices[8]  = 6;
	indices[9] = 4; indices[10] = 6; indices[11] = 7;

	// Fill in the top face index data
	indices[12] = 8; indices[13] =  9; indices[14] = 10;
	indices[15] = 8; indices[16] = 10; indices[17] = 11;

	// Fill in the bottom face index data
	indices[18] = 12; indices[19] = 13; indices[20] = 14;
	indices[21] = 12; indices[22] = 14; indices[23] = 15;

	// Fill in the left face index data
	indices[24] = 16; indices[25] = 17; indices[26] = 18;
	indices[27] = 16; indices[28] = 18; indices[29] = 19;

	// Fill in the right face index data
	indices[30] = 20; indices[31] = 21; indices[32] = 22;
	indices[33] = 20; indices[34] = 22; indices[35] = 23;
}

static void UpdateRenderMesh( const BSP::Vertex* vertices, int numVertices, const UINT16* indices, int numIndices )
{
	const bgfx::Memory* vertexMemory = bgfx::makeRef( vertices, sizeof(vertices[0])*numVertices );
	const bgfx::Memory* indexMemory = bgfx::makeRef( indices, sizeof(indices[0])*numIndices );

	bgfx::updateDynamicVertexBuffer( g_dynamicVB, 0, vertexMemory );
	bgfx::updateDynamicIndexBuffer( g_dynamicIB, 0, indexMemory );
}

static void Subtract(
				  const Float3& position,
				  BSP::Tree & worldTree,
				  const BSP::Tree& mesh,	// subtractive brush
				  BSP::Tree & temporary
				  )
{
	temporary.CopyFrom( mesh );
	temporary.Translate( position );
	worldTree.Subtract( temporary );

	DBGOUT("\nBSP tree after CSG:\n");
	BSP::Debug::PrintTree(worldTree);
}

static void GenerateMesh(
						 const BSP::Tree& worldTree,
						 TArray< BSP::Vertex > &vertices, TArray< UINT16 > &indices
						 )
{
	worldTree.GenerateMesh( vertices, indices );
	DBGOUT("GenerateMesh: %d vertices, %d indices", vertices.Num(), indices.Num());
	UpdateRenderMesh( vertices.ToPtr(), vertices.Num(), indices.ToPtr(), indices.Num() );
}

ERet MyEntryPoint()
{
	SetupBaseUtil	setupBase;
	FileLogUtil		fileLog;


	Renderer	renderer;
	mxDO(renderer.Initialize());

	BSP::Vertex::init();



	// Build a BSP tree for the destructible environment.

	BSP::Tree	worldTree;
	{
		BSP::Vertex planeVertices[4] =
		{
			//          XYZ              |            N              | T |    U  |  V
			BSP::Vertex( Float3_Set( -100.0f, 0.0f, -100.0f ), packF4u( 0.0f, 1.0f, 0.0f ), 0,  Float2_Set(    0, 1.0f ) ),
			BSP::Vertex( Float3_Set( -100.0f, 0.0f,  100.0f ), packF4u( 0.0f, 1.0f, 0.0f ), 0,  Float2_Set(    0,    0 ) ),
			BSP::Vertex( Float3_Set(  100.0f, 0.0f,  100.0f ), packF4u( 0.0f, 1.0f, 0.0f ), 0,  Float2_Set( 1.0f,    0 ) ),
			BSP::Vertex( Float3_Set(  100.0f, 0.0f, -100.0f ), packF4u( 0.0f, 1.0f, 0.0f ), 0,  Float2_Set( 1.0f, 1.0f ) ),
		};

		const UINT16 planeIndices[6] = {
			0, 1, 2, 0, 2, 3,
		};

		// Calculate tangent frames (needed for normal mapping).
		calcTangents(
			planeVertices, BX_COUNTOF(planeVertices), BSP::Vertex::ms_decl,
			planeIndices, BX_COUNTOF(planeIndices) );

		BSP::TProcessTriangles< BSP::Vertex, UINT16 > enumerateMeshVertices(
			planeVertices, BX_COUNTOF(planeVertices), planeIndices, BX_COUNTOF(planeIndices)
		);
		worldTree.Build( &enumerateMeshVertices );

		DBGOUT("\nWorld BSP tree:\n");
		BSP::Debug::PrintTree(worldTree);
	}


	// Temporary storage for storing intermediate results of CSG calculations to reduce memory allocations.

	BSP::Tree	temporary;

	TArray< BSP::Vertex >	rawVertices;
	TArray< UINT16 >		rawIndices;



	// Build a BSP tree for the subtractive mesh.

	BSP::Tree	operand;
	{
		MakeBoxMesh( 1.0f, 1.0f, 1.0f, rawVertices, rawIndices );

		{
			Float4x4 scaleMatrix = Matrix_Scaling( 20, 20, 20 );
			Float4x4 scaleMatrixIT = Matrix_Transpose( Matrix_Inverse( scaleMatrix ) );

			for( UINT i = 0; i < rawVertices.Num(); ++i )
			{
				rawVertices[i].xyz = Matrix_TransformPoint( scaleMatrix, rawVertices[i].xyz );
				//scaleMatrixIT.TransformNormal( vertices[i].Tangent );
			}
		}

		{
			const int numTriangles = rawIndices.Num() / 3;
			for( int i = 0; i < numTriangles; i++ )
			{
				UINT16 * tri = rawIndices.ToPtr() + i*3;
				TSwap( tri[0], tri[2] );
			}
		}

		BSP::TProcessTriangles< BSP::Vertex, UINT16 > enumerateMeshVertices(
			rawVertices.ToPtr(), rawVertices.Num(), rawIndices.ToPtr(), rawIndices.Num()
		);
		operand.Build( &enumerateMeshVertices );

		rawVertices.Empty();
		rawIndices.Empty();

		DBGOUT("\nModel BSP tree:\n");
		BSP::Debug::PrintTree(operand);
	}





	g_dynamicVB = bgfx::createDynamicVertexBuffer( 1024, BSP::Vertex::ms_decl, BGFX_BUFFER_ALLOW_RESIZE );
	g_dynamicIB = bgfx::createDynamicIndexBuffer( 1024, BGFX_BUFFER_NONE );

	


	int fireRate = 10; // shots per second
	int64_t lastTimeShot = 0;





#if 1
	{
		Float3 pos = Float3_Set(0,-5,0);
#if 0
		{
			Subtract(
				pos,
				worldTree,
				operand,
				temporary
				);
			DBGOUT("\nWorld tree after CSG:\n");
			BSP::Debug::PrintTree(worldTree);
		}
#else
		temporary.CopyFrom( operand );
		temporary.Translate( pos );
		//worldTree.CopyFrom( temporary );
		worldTree.m_nodes[0].back = BSP::CopySubTree(worldTree, temporary, 0);
		BSP::Debug::PrintTree(worldTree);
#endif
	}
#endif

	GenerateMesh(
		worldTree,
		rawVertices, rawIndices
	);






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
				| BGFX_STATE_CULL_CCW
				| BGFX_STATE_DEPTH_WRITE
				| BGFX_STATE_DEPTH_TEST_LESS
				| BGFX_STATE_MSAA
				);

			bgfx::submit(RENDER_PASS_GEOMETRY_ID);
		}

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

		renderer.EndFrame();

		if( !!mouseState.m_buttons[entry::MouseButton::Left] )
		{
			const int64_t timeElapsed = now - lastTimeShot;
			double secondsElapsed = double(timeElapsed)/freq;

			if( secondsElapsed > (1.0f/fireRate) )
			{
				DBGOUT("Shooting");
				lastTimeShot = now;

				Float3 rayPos, lookAt, rayDir;
				cameraGetPosition((float*)&rayPos);
				cameraGetAt((float*)&lookAt);
				rayDir = Float3_Normalized(lookAt - rayPos);

				worldTree.CastRay( rayPos, rayDir, lastHit );
				if( lastHit.hitAnything )
				{
//					LogStream(LL_Info) << "Hit pos: " << lastHit.position;
					Subtract(
						lastHit.position,
						worldTree,
						operand,
						temporary
						
					);
					GenerateMesh(
						worldTree,
						rawVertices, rawIndices
					);
				}
			}		
		}
	}

	// Cleanup.
	cameraDestroy();
	imguiDestroy();


	bgfx::destroyDynamicIndexBuffer( g_dynamicIB );
	bgfx::destroyDynamicVertexBuffer( g_dynamicVB );

	renderer.Shutdown();

	return ALL_OK;
}

int _main_(int /*_argc*/, char** /*_argv*/)
{
	MyEntryPoint();

	return 0;
}
