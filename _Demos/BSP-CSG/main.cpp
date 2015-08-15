/*
stolen from:
https://github.com/299299/NagaGame/blob/master/Source/Game/Engine/Graphics/DebugDraw.cpp
*/
#include <Base/Base.h>
#include <Base/Util/LogUtil.h>
#include <Base/Util/Color.h>
#include <Driver/Windows/ConsoleWindow.h>
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

template< typename TYPE, class ARRAY >
ERet TSetMany( ARRAY &_array, const TYPE* _items, UINT32 _count )
{
	mxDO(_array.SetNum(_count));
	TCopyArray( _array.ToPtr(), _items, _count );
	return ALL_OK;
}
template< class ARRAY >
ERet TAppend( ARRAY &_destination, const ARRAY& _source )
{
	const UINT32 oldNum = _destination.Num();
	const UINT32 newNum = oldNum + _source.Num();
	mxDO(_destination.SetNum(newNum));
	TCopyArray( _destination.ToPtr() + oldNum, _source.ToPtr(), _source.Num() );
	return ALL_OK;
}

const Float3 Plane_Project( const Float4& plane, const Float3& point )
{
	const Float3& N = Plane_GetNormal( plane );
	mxASSERT(Float3_IsNormalized( N ));
	return point - N * Plane_PointDistance( plane, point );
}

static void CreatePolygon( const Vector4& plane, const Float3& _eyePosition, BSP::Vertex vertices[4] )
{
	const Float3 normal = Plane_GetNormal(plane);
	const Float3 axis2 = Float3_Normalized(Float3_FindOrthogonalTo(normal));
	const Float3 axis1 = Float3_Negate(Float3_Normalized(Float3_Cross(axis2, normal)));
	const float size = 100.0f;

	const Float3 center = Plane_Project(plane, _eyePosition);

	const Float3 right = axis1 * size;
	const Float3 up = axis2 * size;
	const Float3 left = right * -1.0f;
	const Float3 down = up * -1.0f;

	BSP::Vertex &lowerLeft = vertices[0];
	BSP::Vertex &upperLeft = vertices[1];
	BSP::Vertex &upperRight = vertices[2];
	BSP::Vertex &lowerRight = vertices[3];

	lowerLeft.xyz = left + down;
	upperLeft.xyz = left + up;
	upperRight.xyz = right + up;
	lowerRight.xyz = right + down;

	lowerLeft.N = PackNormal(normal).v;
	upperLeft.N = PackNormal(normal).v;
	upperRight.N = PackNormal(normal).v;
	lowerRight.N = PackNormal(normal).v;

	lowerLeft.T = PackNormal(axis1).v;
	upperLeft.T = PackNormal(axis1).v;
	upperRight.T = PackNormal(axis1).v;
	lowerRight.T = PackNormal(axis1).v;

	lowerLeft.UV	= Float2_Set( 0.0f, 1.0f );
	upperLeft.UV	= Float2_Set( 0.0f, 0.0f );
	upperRight.UV	= Float2_Set( 1.0f, 0.0f );
	lowerRight.UV	= Float2_Set( 1.0f, 1.0f );
}

static void GeneratePolygonsR(
							 const BSP::Tree& _tree,
							 const BSP::NodeID _node,
							 const Float3& _eyePosition,
							 const TArray< BSP::Face >& _inFaces,
							 TArray< BSP::Face >& _outFaces
							 )
{
	using namespace BSP;
	if( IS_INTERNAL(_node) )
	{
		const Node& node = _tree.m_nodes[ _node ];
		const Vector4& plane = _tree.m_planes[ node.plane ];

		Vertex	buffer1[64];
		Vertex	buffer2[64];

		TArray< Vertex > frontPoly;
		TArray< Vertex > backPoly;
		frontPoly.SetExternalStorage( buffer1, mxCOUNT_OF(buffer1) );
		backPoly.SetExternalStorage( buffer2, mxCOUNT_OF(buffer2) );

		TArray< BSP::Face > frontFaces, backFaces;

		for( int i = 0; i < _inFaces.Num(); i++ )
		{
			const BSP::Face& face = _inFaces[i];

			const BSP::EPlaneSide side = SplitConvexPolygonByPlane(
				face.vertices.ToPtr(),
				face.vertices.Num(),
				frontPoly,	// valid only if the polygon was split
				backPoly,	// valid only if the polygon was split
				plane,
				0.13f
			);
			if( side == BSP::PLANESIDE_CROSS )
			{
				Face& newFrontPoly = frontFaces.Add();
				TSetMany(newFrontPoly.vertices, frontPoly.ToPtr(), frontPoly.Num());
				Face& newBackPoly = backFaces.Add();
				TSetMany(newBackPoly.vertices, backPoly.ToPtr(), backPoly.Num());
			}
			else if( side == BSP::PLANESIDE_FRONT )
			{
				Face& newFrontPoly = frontFaces.Add();
				TSetMany(newFrontPoly.vertices, frontPoly.ToPtr(), frontPoly.Num());
			}
			else if( side == BSP::PLANESIDE_BACK )
			{
				Face& newBackPoly = backFaces.Add();
				TSetMany(newBackPoly.vertices, backPoly.ToPtr(), backPoly.Num());
			}
			else
			{
				//_outFaces.Add(face);
				//frontFaces.Add(face);
				//backFaces.Add(face);
			}
		}

		GeneratePolygonsR(_tree, node.front, _eyePosition, frontFaces, _outFaces);
		GeneratePolygonsR(_tree, node.back, _eyePosition, backFaces, _outFaces);
	}
	else if( IS_SOLID_LEAF(_node) )
	{
		TAppend(_outFaces, _inFaces );
	}
}

static void GeneratePolygonsR2(
							 const BSP::Tree& _tree,
							 const BSP::NodeID _node,
							 const Float3& _eyePosition,
							 const TArray< BSP::Face >& _inFaces,
							 TArray< BSP::Face >& _outFaces
							 )
{
	using namespace BSP;
	if( IS_INTERNAL(_node) )
	{
		const Node& node = _tree.m_nodes[ _node ];
		const Vector4& plane = _tree.m_planes[ node.plane ];

		Vertex	buffer1[64];
		Vertex	buffer2[64];

		TArray< Vertex > frontPoly;
		TArray< Vertex > backPoly;
		frontPoly.SetExternalStorage( buffer1, mxCOUNT_OF(buffer1) );
		backPoly.SetExternalStorage( buffer2, mxCOUNT_OF(buffer2) );

		TArray< BSP::Face > frontFaces, backFaces;

		for( int i = 0; i < _inFaces.Num(); i++ )
		{
			const BSP::Face& face = _inFaces[i];

			const BSP::EPlaneSide side = SplitConvexPolygonByPlane(
				face.vertices.ToPtr(),
				face.vertices.Num(),
				frontPoly,	// valid only if the polygon was split
				backPoly,	// valid only if the polygon was split
				plane,
				0.13f
			);
			if( side == BSP::PLANESIDE_CROSS )
			{
				Face& newFrontPoly = frontFaces.Add();
				TSetMany(newFrontPoly.vertices, frontPoly.ToPtr(), frontPoly.Num());
				Face& newBackPoly = backFaces.Add();
				TSetMany(newBackPoly.vertices, backPoly.ToPtr(), backPoly.Num());
			}
			else if( side == BSP::PLANESIDE_FRONT )
			{
				Face& newFrontPoly = frontFaces.Add();
				TSetMany(newFrontPoly.vertices, frontPoly.ToPtr(), frontPoly.Num());
			}
			else if( side == BSP::PLANESIDE_BACK )
			{
				Face& newBackPoly = backFaces.Add();
				TSetMany(newBackPoly.vertices, backPoly.ToPtr(), backPoly.Num());
			}
			else
			{
				//_outFaces.Add(face);
				//frontFaces.Add(face);
				backFaces.Add(face);
			}
		}

		GeneratePolygonsR2(_tree, node.front, _eyePosition, frontFaces, _outFaces);
		GeneratePolygonsR2(_tree, node.back, _eyePosition, backFaces, _outFaces);
	}
	else if( IS_SOLID_LEAF(_node) )
	{
		TAppend(_outFaces, _inFaces );
	}
}

static void GeneratePolygons(
							 const BSP::Tree& _tree,
							 const BSP::NodeID _start,
							 const Float3& _cameraPosition,
							 TArray< BSP::Vertex > &_vertices,
							 TArray< UINT16 > &_indices
							 )
{
	_vertices.Empty();
	_indices.Empty();

	TArray< BSP::Face > inFaces, outFaces;

	for( UINT32 iPlane = 0; iPlane < _tree.m_planes.Num(); iPlane++ )
	{
		const Vector4& plane = _tree.m_planes[ iPlane ];

		//Plane_PointDistance()

		BSP::Vertex vertices[4];
		CreatePolygon( plane, _cameraPosition, vertices );

		inFaces.Empty();

		BSP::Face &newFace = inFaces.Add();
		newFace.vertices.Add(vertices[0]);
		newFace.vertices.Add(vertices[1]);
		newFace.vertices.Add(vertices[2]);
		newFace.vertices.Add(vertices[3]);

//		GeneratePolygonsR2(_tree,_start,_cameraPosition, inFaces,outFaces);
	}

	GeneratePolygonsR2(_tree,_start,_cameraPosition, inFaces,outFaces);

	for( int i = 0; i < outFaces.Num(); i++ )
	{
		const BSP::Face& face = outFaces[i];
		if(face.vertices.Num())
			TriangulateFace( face, _vertices, _indices );
	}
}

enum {
	// The width in pixels of the image
	RT_SCREEN_SIZE_X = 512,
	// The height in pixels of the image.
	RT_SCREEN_SIZE_Y = 256
};

ERet MyEntryPoint()
{
	SetupBaseUtil	setupBase;
	FileLogUtil		fileLog;
	SetupCoreUtil	setupCore;
	CConsole		consoleWindow;

	DBGOUT("sizeof(Node)=%d, sizeof(Face)=%d", sizeof(BSP::Node), sizeof(BSP::Face));


	BSP::Vertex::init();

	Renderer	renderer;
	mxDO(renderer.Initialize());

	BatchRenderer	debugDraw;
	mxDO(debugDraw.Initialize(&renderer));

	bgfx::TextureHandle renderTarget = bgfx::createTexture2D(
		RT_SCREEN_SIZE_X, RT_SCREEN_SIZE_Y, 1,
		bgfx::TextureFormat::R32F,//bgfx::TextureFormat::RGBA8,
		BGFX_TEXTURE_RT
		);
	bgfx::FrameBufferHandle frameBuffer = bgfx::createFrameBuffer( 1, &renderTarget, true );

	bgfx::ProgramHandle screenspaceQuad = loadProgram(
		"vs_screenspace_quad",
		"fs_screenspace_quad"
	);

	bgfx::TextureHandle testTexture  = loadTexture("tree01S.dds");

	//DynamicMesh	debugWireframe;
	//debugWireframe.hVB = bgfx::createDynamicVertexBuffer( 1024, BSP::Vertex::ms_decl, BGFX_BUFFER_ALLOW_RESIZE );
	//debugWireframe.hIB = bgfx::createDynamicIndexBuffer( 1024, BGFX_BUFFER_NONE );


	CSG	csg;
	mxDO(csg.Initialize());


	//csg.RunTestCode();
//csg.UpdateRenderMesh(csg.worldTree);
	BSP::Debug::PrintTree(csg.worldTree);
GeneratePolygons(csg.worldTree, 0, Float3_Zero(), csg.vertices, csg.indices);
csg.UpdateRenderMesh(csg.vertices, csg.indices);

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

		{
			int y = 0;
			{
				imguiBeginScrollArea("Settings", width - width / 5 - 10, 10, width / 5, height / 3, &scrollArea);
				imguiSeparatorLine();

				imguiLabel("Nodes: %d", csg.worldTree.m_nodes.Num());

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
	

				//y = imguiGetWidgetY();
			}

			//imguiLabel("fsfsfs");
			//{
			//	imguiBeginArea("Stats", width - width / 5 - 10, y, width / 5, height / 3);
			//	imguiValue("fsfsfaf");
			//	imguiEndArea();
			//}
		}
		imguiEndFrame();

		// Update camera.
		cameraUpdate(deltaTime, mouseState);

		float view[16];
		cameraGetViewMtx(view);

		float invView[16];
		bx::mtxInverse(invView, view);

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

			#if 0
					{
						ScopedStackAlloc	tempAlloc( gCore.frameAlloc );
						const uint32_t dataSize = RT_SCREEN_SIZE_X*RT_SCREEN_SIZE_Y*sizeof(UINT32);
						//uint32_t* imageData = (uint32_t*)tempAlloc.Alloc( dataSize );
						float* imageData = (float*)tempAlloc.Alloc( dataSize );

			#if 0
						//int val = 0x000000FF;
						int val = 0xFF000000;
						memset(temp,val,dataSize);

						for( uint32_t yy = 0; yy < RT_SCREEN_SIZE_Y; ++yy )
						{
							for (uint32_t xx = 0; xx < RT_SCREEN_SIZE_X; ++xx)
							{
								uint32_t& cc = imageData[ yy*RT_SCREEN_SIZE_X + xx ];
								if(yy==xx)
								cc = 0x0000FF00;	// RGBA (R - lowest bits, A - highest)
								if( yy > 100 )
									cc = 0x0000FFFF;
							}
						}
			#endif


						float horizFoV = DEG2RAD(90);

						// image plane in world space
						float XL = -RT_SCREEN_SIZE_X/2;	// left
						float XR = +RT_SCREEN_SIZE_X/2;	// right
						float ZB = -RT_SCREEN_SIZE_Y/2;	// bottom
						float ZT = +RT_SCREEN_SIZE_Y/2;	// top			
						//float dN = (RT_SCREEN_SIZE_Y/2) / tanFoVy;	// distance to the image plane
						float dN = (RT_SCREEN_SIZE_X/2) / tanf(horizFoV*0.5f);	// distance to the image plane

						// calculate deltas for interpolation
						float DX = (XR - XL) / RT_SCREEN_SIZE_X;
						float DZ = (ZB - ZT) / RT_SCREEN_SIZE_Y;

						//Float4x4* mat1 = (Float4x4*) invView;
						//Float4x4 mat = Matrix_Transpose( *mat1 );
						//Camera* s_camera
						Float4x4 mat = *(Float4x4*) invView;
						Float3 eyePosition = Vector4_As_Float3( mat.r3 );
						Float3 rightDirection = Vector4_As_Float3( mat.r0 );
						Float3 upDirection = Vector4_As_Float3( mat.r1 );
						Float3 lookDirection = Vector4_As_Float3( mat.r2 );

						float camPos[3], camLookAt[3];
						cameraGetPosition(camPos);
						cameraGetAt(camLookAt);
						Float3 lookAtTarget = eyePosition + lookDirection;

						{
							float thit;
							int hit = csg.worldTree.CastRay(
									eyePosition, lookDirection,
									0.0f, 1000.0f, &thit
								);
							if(hit) {
								DBGOUT("HIT!!!: %f",thit);
							} else {
								DBGOUT("NO HIT.");
							}
						}

						// for each pixel...
						float RZ = ZT;
						for( int iY = 0; iY < RT_SCREEN_SIZE_Y; iY++ )
						{
							float RX = XL;
							for( int iX = 0; iX < RT_SCREEN_SIZE_X; iX++ )
							{
								int offset = iX+iY*RT_SCREEN_SIZE_X;

								// ...compute the viewing ray:
			#if 1
								//Float3 rayTarget = {
								//	RX,
								//	dN,
								//	RZ
								//};
								Float3 rayTarget = eyePosition
									+ rightDirection * RX
									+ upDirection * RZ
									+ lookDirection * dN;

								Float3 rayOrigin = eyePosition;
								Float3 rayDirection = rayTarget - rayOrigin;
								rayDirection = Float3_Normalized(rayDirection);

								float thit = 0.0f;
								int hit = csg.worldTree.CastRay(
									rayOrigin, rayDirection,
									0.0f, 1000.0f, &thit
								);
								//hit *= 1e-3f;
								//imageData[offset] = thit;
								imageData[offset] = hit ? 1.0f : 0.0f;
			#endif

								//csg.worldTree.CastRay();

								imageData[offset] = 0.5f;

								//imageData[offset] = 0x0000FF00;	// RGBA (R - lowest bits, A - highest)

								RX += DX;
							}
							RZ += DZ;
						}

						updateTexture2D(
							renderTarget,
							0,	// uint8_t _mip
							0,	// uint16_t _x
							0,	// uint16_t _y
							RT_SCREEN_SIZE_X,// uint16_t _width
							RT_SCREEN_SIZE_Y,// uint16_t _height
							bgfx::copy(imageData,dataSize)// const Memory* _mem
						);
					}

					bgfx::setTexture(0, renderer.s_texColor, renderTarget);
					//bgfx::setTexture(0, renderer.s_texColor, testTexture);

					bgfx::setState(BGFX_STATE_RGB_WRITE);
					screenSpaceQuad(RT_SCREEN_SIZE_X,RT_SCREEN_SIZE_Y,g_texelHalf,g_originBottomLeft, 2*3,1*3);
					bgfx::submit(RENDER_PASS_DEBUG_GBUFFER_ID, screenspaceQuad);

			#endif


		debugDraw.DrawAxes(100);

		debugDraw.Flush();

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

				GeneratePolygons(csg.worldTree, 0, Float3_Zero(), csg.vertices, csg.indices);
csg.UpdateRenderMesh(csg.vertices, csg.indices);

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

	bgfx::destroyFrameBuffer( frameBuffer );

	debugDraw.Shutdown();

	renderer.Shutdown();

	return ALL_OK;
}

int _main_(int /*_argc*/, char** /*_argv*/)
{
	MyEntryPoint();

	return 0;
}
