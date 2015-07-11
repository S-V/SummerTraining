#include "Graphics/Graphics_PCH.h"
#pragma hdrstop
#include <algorithm>
#include <Core/VectorMath.h>
#include <Core/Util/Tweakable.h>
#include <Graphics/Utils.h>

void AuxVertex::BuildVertexDescription( VertexDescription & _description )
{
	_description.Begin();
	_description.Add(AttributeType::Float, 3, VertexAttribute::Position, false);
	_description.Add(AttributeType::Float, 2, VertexAttribute::TexCoord0, false);
	//_description.Add(AttributeType::Float, 2, VertexAttribute::TexCoord1, false);
	_description.Add(AttributeType::UByte, 4, VertexAttribute::Normal, false);
	_description.Add(AttributeType::UByte, 4, VertexAttribute::Tangent, false);
	_description.Add(AttributeType::UByte, 4, VertexAttribute::Color0, true);
	_description.End();
}

#if LLGL_Driver_Is_Direct3D
	#define NDC_NEAR_CLIP	(0.0f)
#endif

#if LLGL_Driver_Is_OpenGL
	#define NDC_NEAR_CLIP	(-1.0f)
#endif

// Corners of the unit cube
// in Direct3D's or OpenGL's Normalized Device Coordinates (NDC)
//
static const Float3 gs_NDC_Cube[8] =
{
	// near plane
	{ -1.0f, -1.0f, NDC_NEAR_CLIP },	// lower left
	{ -1.0f, +1.0f, NDC_NEAR_CLIP },	// upper left
	{ +1.0f, +1.0f, NDC_NEAR_CLIP },	// upper right
	{ +1.0f, -1.0f, NDC_NEAR_CLIP },	// lower right		

	// far plane
	{ -1.0f, -1.0f, 1.0f },	// lower left
	{ -1.0f, +1.0f, 1.0f },	// upper left
	{ +1.0f, +1.0f, 1.0f },	// upper right
	{ +1.0f, -1.0f, 1.0f },	// lower right		
};

void AuxRenderer::DrawLine(
	const Float3& start,
	const Float3& end,
	const RGBAf& startColor,
	const RGBAf& endColor
)
{
	AuxVertex	startVertex;
	AuxVertex	endVertex;

	startVertex.xyz = start;
	startVertex.rgba.v = startColor.ToInt32();
	endVertex.xyz = end;
	endVertex.rgba.v = endColor.ToInt32();

	DrawLine3D( startVertex, endVertex );
}

void AuxRenderer::DrawWireTriangle(
		const Float3& a, const Float3& b, const Float3& c,
		const RGBAf& color
	)
{
	this->DrawLine( a, b, color, color );
	this->DrawLine( b, c, color, color );
	this->DrawLine( c, a, color, color );
}

void AuxRenderer::DrawAABB(
						 const Float3& aabbMin, const Float3& aabbMax,
						 const RGBAf& color
						 )
{
	const UINT8* edgeIndices = AABB_GetEdgeIndices();
	Float3 corners[8];
	AABB_GetCorners(aabbMin, aabbMax, corners);
	for( UINT32 iEdge = 0; iEdge < 12; iEdge++ )
	{
		AuxVertex	start;
		AuxVertex	end;

		start.xyz = corners[ edgeIndices[iEdge*2] ];
		end.xyz = corners[ edgeIndices[iEdge*2 + 1] ];

		start.rgba.v = color.ToInt32();
		end.rgba.v = color.ToInt32();

		DrawLine3D( start, end );
	}
}

BatchRenderer::BatchRenderer()
{
	m_VBSize = 0;
	m_IBSize = 0;
	m_indexStride = 0;

	m_dynamicVB.SetNil();
	m_dynamicIB.SetNil();
	m_layout.SetNil();

	m_topology = Topology::Undefined;

	//m_transform = Matrix_Identity();
}

BatchRenderer::~BatchRenderer()
{
}

ERet BatchRenderer::Initialize( UINT32 vertexBufferSize, UINT32 indexBufferSize )
{
	VertexDescription	vertexDescription;
	AuxVertex::BuildVertexDescription( vertexDescription );
	m_layout = llgl::CreateInputLayout( vertexDescription, "AuxVertex" );

	m_dynamicVB = llgl::CreateBuffer( Buffer_Vertex, vertexBufferSize );
	m_dynamicIB = llgl::CreateBuffer( Buffer_Index, indexBufferSize );
	m_VBSize = vertexBufferSize;
	m_IBSize = indexBufferSize;
	m_vertexStride = sizeof(AuxVertex);
	m_indexStride = sizeof(UINT16);

	m_topology = Topology::Undefined;

	//m_transform = Matrix_Identity();

	//SetShader(NULL);
	//SetTransform(Float4x4(1));
	//SetColor(RGBAf::WHITE.ToPtr());

	return ALL_OK;
}

void BatchRenderer::Shutdown()
{
	m_technique = NULL;

	llgl::DeleteBuffer( m_dynamicVB );
	m_dynamicVB.SetNil();

	llgl::DeleteBuffer( m_dynamicIB );
	m_dynamicIB.SetNil();

	llgl::DeleteInputLayout(m_layout);
	m_layout.SetNil();

	m_batchedVertices.Empty();
	m_batchedIndices.Empty();

	m_topology = Topology::Undefined;
}

//void BatchRenderer::SetShader( FxShader* technique )
//{
//	mxASSERT_PTR(technique);
//	if( m_technique != technique )
//	{
//		Flush();
//		m_technique = technique;
//	}
//}
//
//void BatchRenderer::SetTransform( const Float4x4& mWVP )
//{
//	chkRET_IF_NIL(m_technique);
//	Flush();
//	XConstantBuffer& cbuffer = m_technique->uniforms[0];
//	const XUniform* u_WVP = cbuffer.FindUniformByHash(mxHASH_ID(transform));
//	if( u_WVP ) {
//		cbuffer.UpdateUniform(&mWVP, u_WVP->offset, sizeof(mWVP));
//	}
//}
//
//void BatchRenderer::SetColor( const float* rgba )
//{
//	chkRET_IF_NIL(m_technique);
//	Flush();
//	XConstantBuffer& cbuffer = m_technique->uniforms[0];
//	const XUniform* u_Color = cbuffer.FindUniformByHash(mxHASH_ID(baseColor));
//	if( u_Color ) {
//		cbuffer.UpdateUniform(&rgba, u_Color->offset, sizeof(rgba));
//	}
//}

void BatchRenderer::SetShader( FxShader* technique )
{
	mxASSERT_PTR(technique);
	if( m_technique != technique && technique )
	{
		Flush();
		m_technique = technique;
	}
}

void BatchRenderer::DrawLine3D(
							 const AuxVertex& start,
							 const AuxVertex& end
							 )
{
	AuxVertex *	vertices;
	UINT16 *	indices;
	const UINT32 iBaseVertex = BeginBatch( Topology::LineList, 2, vertices, 2, indices );
	vertices[0] = start;
	vertices[1] = end;
	indices[0] = iBaseVertex + 0;
	indices[1] = iBaseVertex + 1;
}

//void BatchRenderer::DrawTriangle3D(
//	const AuxVertex& a, const AuxVertex& b, const AuxVertex& c,
//	const RGBAf& color
//)
//{
//	BeginBatch( Topology::TriangleList, 3, 3 );
//	const UINT32 iBaseVertex = m_batchedVertices.Num();
//	m_batchedVertices.Add(a);
//	m_batchedVertices.Add(b);
//	m_batchedVertices.Add(c);
//	m_batchedIndices.Add( iBaseVertex + 0 );
//	m_batchedIndices.Add( iBaseVertex + 1 );
//	m_batchedIndices.Add( iBaseVertex + 2 );
//}

void BatchRenderer::DrawWireQuad3D(
		const AuxVertex& a, const AuxVertex& b, const AuxVertex& c, const AuxVertex& d
)
{
	AuxVertex *	vertices;
	UINT16 *	indices;
	BeginBatch( Topology::LineStrip, 4, vertices, 0, indices );
	vertices[0] = a;
	vertices[1] = b;
	vertices[2] = c;
	vertices[3] = d;
}

void BatchRenderer::DrawDashedLine(
	const Float3& start,
	const Float3& end,
	const float dashSize,
	const RGBAf& startColor,
	const RGBAf& endColor
)
{
	mxSWIPED("Unreal Engine 3");
	Float3 lineDir = end - start;
	float lineLeft = Float3_Length(end - start);
	lineDir /= lineLeft;

	while( lineLeft > 0.f )
	{
		Float3 lineStart = end - ( lineDir * lineLeft );
		Float3 lineEnd = lineStart + ( lineDir * minf(dashSize, lineLeft) );

		DrawLine( lineStart, lineEnd, startColor, endColor );

		lineLeft -= 2*dashSize;
	}
}

void BatchRenderer::DrawAxes( float l )
{
#if 0
	DrawLine3D(
		0.0f,0.0f,0.0f,
		l*1.0f,0.0f,0.0f,
		RGBAf::WHITE,
		RGBAf::RED
	);
	DrawLine3D(
		0.0f,0.0f,0.0f,
		0.0f,l*1.0f,0.0f,
		RGBAf::WHITE,
		RGBAf::GREEN
	);
	DrawLine3D(
		0.0f,0.0f,0.0f,
		0.0f,0.0f,l*1.0f,
		RGBAf::WHITE,
		RGBAf::BLUE
	);
#elif 0
	// Draw axes.

	// Draw the x-axis in red
	DrawLine3D(
		0.0f,0.0f,0.0f,
		l*1.0f,0.0f,0.0f,
		RGBAf::RED,
		RGBAf::RED
	);
	// Draw the y-axis in green
	DrawLine3D(
		0.0f,0.0f,0.0f,
		0.0f,l*1.0f,0.0f,
		RGBAf::GREEN,
		RGBAf::GREEN
	);
	// Draw the z-axis in blue
	DrawLine3D(
		0.0f,0.0f,0.0f,
		0.0f,0.0f,l*1.0f,
		RGBAf::BLUE,
		RGBAf::BLUE
	);
#else
	DrawArrow(
		Matrix_FromAxes(
			Float4_As_Float3(g_Float4_UnitX),
			Float4_As_Float3(g_Float4_UnitY),
			Float4_As_Float3(g_Float4_UnitZ)
		),
		RGBAf::RED, l*1.0f, l*0.1f
	);
	DrawArrow(
		Matrix_FromAxes(
			Float4_As_Float3(g_Float4_UnitY),
			Float4_As_Float3(g_Float4_UnitZ),
			Float4_As_Float3(g_Float4_UnitX)
		),
		RGBAf::GREEN, l*1.0f, l*0.1f
	);
	DrawArrow(
		Matrix_FromAxes(
			Float4_As_Float3(g_Float4_UnitZ),
			Float4_As_Float3(g_Float4_UnitX),
			Float4_As_Float3(g_Float4_UnitY)
		),
		RGBAf::BLUE, l*1.0f, l*0.1f
	);
#endif
}

void BatchRenderer::DrawArrow(
							const Float4x4& arrowTransform,
							const RGBAf& color,
							float arrowLength,
							float headSize
							)
{
	mxSWIPED("Unreal Engine 3");
	const Float3 origin = Matrix_GetTranslation(arrowTransform);
	const Float3 endPoint = Matrix_TransformPoint(arrowTransform, Float3_Set( arrowLength, 0.0f, 0.0f ));
	const float headScale = 0.5f;

	const RGBAf& startColor = RGBAf::WHITE;
	const RGBAf& endColor = color;

	DrawLine( origin, endPoint, startColor, endColor );
	DrawLine( endPoint, Matrix_TransformPoint(arrowTransform, Float3_Set( arrowLength-headSize, +headSize*headScale, +headSize*headScale )), startColor, endColor );
	DrawLine( endPoint, Matrix_TransformPoint(arrowTransform, Float3_Set( arrowLength-headSize, +headSize*headScale, -headSize*headScale )), startColor, endColor );
	DrawLine( endPoint, Matrix_TransformPoint(arrowTransform, Float3_Set( arrowLength-headSize, -headSize*headScale, +headSize*headScale )), startColor, endColor );
	DrawLine( endPoint, Matrix_TransformPoint(arrowTransform, Float3_Set( arrowLength-headSize, -headSize*headScale, -headSize*headScale )), startColor, endColor );
}

// based on DXSDK, June 2010, XNA collision detection sample
void BatchRenderer::DrawGrid(
						   const Float3& origin,
						   const Float3& axisX,
						   const Float3& axisY,
						   int numXDivisions,
						   int numYDivisions,
						   const RGBAf& color
						   )
{
	numXDivisions = Max( 1, numXDivisions );
	numYDivisions = Max( 1, numYDivisions );

	float invXDivisions2 = 2.0f / numXDivisions;
	float invYDivisions2 = 2.0f / numYDivisions;

	for( int i = 0; i <= numXDivisions; i++ )
	{
		float fPercent = (i * invXDivisions2) - 1.0f;	// [-1..+1]
		Float3 offsetX = origin + axisX * fPercent;

		Float3 pointA = offsetX - axisY;
		Float3 pointB = offsetX + axisY;

		DrawLine( pointA, pointB, color, color );
	}

	for( int i = 0; i <= numYDivisions; i++ )
	{
		float fPercent = (i * invYDivisions2) - 1.0f;	// [-1..+1]
		Float3 offsetY = origin + axisY * fPercent;

		Float3 pointA = offsetY - axisX;
		Float3 pointB = offsetY + axisX;

		DrawLine( pointA, pointB, color, color );
	}
}

static AuxVertex MakeAuxVertex(
							   const Float3& _xyz,
							   const Float2& _uv,
							   const Float3& _N,
							   const Float3& _T,
							   UINT32 _rgba = ~0 )
{
	AuxVertex result = { _xyz, _uv, PackNormal(_N.x,_N.y,_N.z), PackNormal(_N.x,_N.y,_N.z), _rgba };
	return result;
}

void BatchRenderer::DrawSolidBox(
	const Float3& origin, const Float3& halfSize,
	const RGBAf& color
)
{
	enum { NUM_VERTICES = 8 };
	enum { NUM_TRIANGLES = 12 };
	enum { NUM_INDICES = NUM_TRIANGLES * 3 };

	Float3 corners[ NUM_VERTICES ];
	for( int i = 0; i < NUM_VERTICES; i++ )
	{
		corners[i].x = (i & 1) ? halfSize.x : -halfSize.x;
		corners[i].y = (i & 2) ? halfSize.y : -halfSize.y;
		corners[i].z = (i & 4) ? halfSize.z : -halfSize.z;
		corners[i] += origin;
	}

	static const Float3 vertexNormals[ NUM_VERTICES ] = 
	{
		{ -mxINV_SQRT_3, -mxINV_SQRT_3, -mxINV_SQRT_3	},
		{  mxINV_SQRT_3, -mxINV_SQRT_3, -mxINV_SQRT_3	},
		{ -mxINV_SQRT_3,  mxINV_SQRT_3, -mxINV_SQRT_3	},
		{  mxINV_SQRT_3,  mxINV_SQRT_3, -mxINV_SQRT_3	},

		{ -mxINV_SQRT_3, -mxINV_SQRT_3,  mxINV_SQRT_3	},
		{  mxINV_SQRT_3, -mxINV_SQRT_3,  mxINV_SQRT_3	},
		{ -mxINV_SQRT_3,  mxINV_SQRT_3,  mxINV_SQRT_3	},
		{  mxINV_SQRT_3,  mxINV_SQRT_3,  mxINV_SQRT_3	},
	};

	static const UINT8 triangleIndices[ NUM_INDICES ] =
	{
		// front
		0, 1, 5,
		0, 5, 4,
		// back
		2, 6, 7,
		2, 7, 3,

		// top
		4, 5, 7,
		4, 7, 6,
		// bottom
		0, 2, 3,
		0, 3, 1,

		// left
		0, 4, 6,
		0, 6, 2,
		// right
		1, 3, 7,
		1, 7, 5,
	};
#if 0
	// if you're standing in the center of a box
	// the planes' normals are directed outwards...
	enum EBoxPlaneSide
	{
		SIDE_IN_FRONT	= 0,
		SIDE_BEHIND		= 1,

		SIDE_ABOVE		= 2,
		SIDE_BENEATH	= 3,

		SIDE_LEFT		= 4,
		SIDE_RIGHT		= 5,

		NUM_SIDES		= 6
	};

	const Float3 planeNormals[6] = {
		{  0.0f, -1.0f,  0.0f },	// front
		{  0.0f, +1.0f,  0.0f },	// back
		{  0.0f,  0.0f, +1.0f },	// top
		{  0.0f,  0.0f, -1.0f },	// bottom
		{ -1.0f, -1.0f,  0.0f },	// left
		{ +1.0f, -1.0f,  0.0f },	// right
	};
#endif

	AuxVertex *	vertices;
	UINT16 *	indices;
	const UINT32 iBaseVertex = BeginBatch( Topology::TriangleList, NUM_VERTICES, vertices, NUM_INDICES, indices );

	for( int i = 0; i < NUM_VERTICES; i++ )
	{
		AuxVertex& v = vertices[i];
		v.xyz = corners[i];
		v.N = PackNormal( vertexNormals[i] );
		v.rgba.v = color.ToInt32();
	}

	for( int i = 0; i < NUM_INDICES; i++ )
	{
		indices[i] = triangleIndices[i] + iBaseVertex;
	}
}

//void DrawAABB( const rxAABB& box, const RGBAf& color )
//{
//	XMMATRIX matWorld = XMMatrixScaling( box.Extents.x, box.Extents.y, box.Extents.z );
//	XMVECTOR position = XMLoadFloat3( &box.Center );
//	matWorld.r[3] = XMVectorSelect( matWorld.r[3], position, XMVectorSelectControl( 1, 1, 1, 0 ) );
//
//	DrawCube( matWorld, color );
//}
//
//void DrawOBB( const rxOOBB& box, const RGBAf& color )
//{
//	XMMATRIX matWorld = XMMatrixRotationQuaternion( XMLoadFloat4( &box.Orientation ) );
//	XMMATRIX matScale = XMMatrixScaling( box.Extents.x, box.Extents.y, box.Extents.z );
//	matWorld = XMMatrixMultiply( matScale, matWorld );
//	XMVECTOR position = XMLoadFloat3( &box.Center );
//	matWorld.r[3] = XMVectorSelect( matWorld.r[3], position, XMVectorSelectControl( 1, 1, 1, 0 ) );
//
//	DrawCube( matWorld, color );
//}

void BatchRenderer::DrawCircle(
							 const Float3& origin,
							 const Float3& right,
							 const Float3& up,
							 const RGBAf& color,
							 float radius,
							 int numSides
							 )
{
	float	angleDelta = mxTWO_PI / (float)numSides;
	Float3	prevVertex = origin + right * radius;

	for( int iSide = 0; iSide < numSides; iSide++ )
	{
		float	fSin, fCos;
		Float_SinCos( angleDelta * (iSide + 1), fSin, fCos );

		Float3	currVertex = origin + (right * fCos + up * fSin) * radius;

		DrawLine( prevVertex, currVertex, color, color );

		prevVertex = currVertex;
	}
}

void BatchRenderer::DrawWireFrustum(
					 const Float4x4& _viewProjection,
					 const RGBAf& _color
					 )
{
	const Float4x4 inverseViewProjection = Matrix_Inverse(_viewProjection);

	AuxVertex frustumCorners[8];
	for( int i = 0; i < 8; i++ )
	{
		const Float4 pointH = Float4_Set( gs_NDC_Cube[i], 1.0f );
		const Float4 point = Matrix_Transform( inverseViewProjection, pointH );
		const float invW = 1.0f / point.w;
		frustumCorners[i].xyz.x = point.x * invW;
		frustumCorners[i].xyz.y = point.y * invW;
		frustumCorners[i].xyz.z = point.z * invW;

		frustumCorners[i].rgba.v = _color.ToInt32();
	}

	// near plane
	DrawLine3D( frustumCorners[0], frustumCorners[1] );
	DrawLine3D( frustumCorners[1], frustumCorners[2] );
	DrawLine3D( frustumCorners[2], frustumCorners[3] );
	DrawLine3D( frustumCorners[3], frustumCorners[0] );
	// far plane
	DrawLine3D( frustumCorners[4], frustumCorners[5] );
	DrawLine3D( frustumCorners[5], frustumCorners[6] );
	DrawLine3D( frustumCorners[6], frustumCorners[7] );
	DrawLine3D( frustumCorners[7], frustumCorners[4] );
	// middle edges
	DrawLine3D( frustumCorners[0], frustumCorners[4] );
	DrawLine3D( frustumCorners[1], frustumCorners[5] );
	DrawLine3D( frustumCorners[2], frustumCorners[6] );
	DrawLine3D( frustumCorners[3], frustumCorners[7] );
}

void BatchRenderer::DrawSolidTriangle3D(
	const AuxVertex& a, const AuxVertex& b, const AuxVertex& c
	)
{
	AuxVertex *	vertices;
	UINT16 *	indices;
	const UINT32 iBaseVertex = BeginBatch( Topology::TriangleList, 3, vertices, 3, indices );

	vertices[0] = a;
	vertices[1] = b;
	vertices[2] = c;

	indices[0] = iBaseVertex + 0;
	indices[1] = iBaseVertex + 1;
	indices[2] = iBaseVertex + 2;
}

void BatchRenderer::DrawSolidQuad3D(
	const AuxVertex& a, const AuxVertex& b, const AuxVertex& c, const AuxVertex& d
)
{
	AuxVertex *	vertices;
	UINT16 *	indices;
	const UINT32 iBaseVertex = BeginBatch( Topology::TriangleList, 4, vertices, 6, indices );

	vertices[0] = a;
	vertices[1] = b;
	vertices[2] = c;
	vertices[3] = d;

	indices[0] = iBaseVertex + 0;
	indices[1] = iBaseVertex + 1;
	indices[2] = iBaseVertex + 2;
	indices[3] = iBaseVertex + 0;
	indices[4] = iBaseVertex + 2;
	indices[5] = iBaseVertex + 3;
}

void BatchRenderer::DrawSolidSphere(
	const Float3& center, float radius,
	const RGBAf& color,
	int numStacks, int numSlices
)
{
	UByte4 rgbaColor;
	rgbaColor.v = color.ToInt32();

	// This approach is much like a globe in that there certain number of vertical lines of latitude
	// and horizontal lines of longitude which break the sphere up into many rectangular (4 sided) parts.
	// Note however there will be a point at the top and bottom (north and south pole)
	// and polygons attached to these will be triangular (3 sided).
	// These are easiest to generate
	// and the number of polygons will be equal to: LONGITUDE_LINES * (LATITUDE_LINES + 1).
	// Of these (LATITUDE_LINES*2) will be triangular and connected to a pole, and the rest rectangular.

	// do not count the poles as rings
	const UINT32 numRings = numStacks-1;
	const UINT32 numRingVertices = numSlices + 1;
	const UINT32 numVertices = numRings * numRingVertices + 2;
	const UINT32 numIndices = (numStacks-2) * numSlices * 6 + numSlices * 6;

	AuxVertex *	vertices;
	UINT16 *	indices;
	const UINT32 iBaseVertex = BeginBatch( Topology::TriangleList, numVertices, vertices, numIndices, indices );

	UINT32	vertexIndex = 0;

	// use polar coordinates to generate slices of the sphere

	float phiStep = mxPI / (float) numStacks;

	// Compute vertices for each stack ring.
	for ( UINT32 i = 1; i <= numRings; ++i )
	{
		float phi = i * phiStep;	// polar angle (vertical) or inclination, latitude, [0..PI]

		float sinPhi, cosPhi;
		Float_SinCos( phi, sinPhi, cosPhi );

		// vertices of ring
		float thetaStep = 2.0f * mxPI / numSlices;
		for ( UINT32 j = 0; j <= numSlices; ++j )
		{
			float theta = j * thetaStep;	// azimuthal angle (horizontal) or azimuth, longitude, [0..2*PI]

			float sinTheta, cosTheta;			
			Float_SinCos( theta, sinTheta, cosTheta );

			AuxVertex & vertex = vertices[ vertexIndex++ ];

			// spherical to cartesian
			vertex.xyz.x = radius * sinPhi * cosTheta;
			vertex.xyz.y = radius * sinPhi * sinTheta;
			vertex.xyz.z = radius * cosPhi;
			vertex.xyz += center;
			vertex.rgba = rgbaColor;

			Float3 normal = Float3_Normalized(vertex.xyz);
			vertex.N = PackNormal(normal.x, normal.y, normal.z);

			Float3 tangent;
			// partial derivative of P with respect to theta
			tangent.x = -radius * sinPhi * sinTheta;
			tangent.y = radius * sinPhi * cosTheta;
			tangent.z = 0.0f;
			vertex.T = PackNormal(tangent.x, tangent.y, tangent.z);

			vertex.uv.x = theta / (2.0f*mxPI );
			vertex.uv.y = phi / mxPI;
		}
	}

	const UINT32 northPoleIndex = m_batchedVertices.Num();
	const UINT32 southPoleIndex = northPoleIndex + 1;

	// poles: note that there will be texture coordinate distortion
	AuxVertex & northPole = vertices[ vertexIndex++ ];
	northPole.xyz = center;
	northPole.xyz.z += radius;
	northPole.uv = Float2_Set(0.0f, 1.0f);
	northPole.N = PackNormal( 0.0f, 1.0f, 0.0f );
	northPole.T = PackNormal( 1.0f, 0.0f, 0.0f );
	northPole.rgba = rgbaColor;

	AuxVertex & southPole = vertices[ vertexIndex++ ];
	southPole.xyz = center;
	southPole.xyz.z -= radius;
	southPole.uv = Float2_Set(0.0f, 1.0f);
	southPole.N = PackNormal( 0.0f, -1.0f, 0.0f );
	southPole.T = PackNormal( 1.0f, 0.0f, 0.0f );
	southPole.rgba = rgbaColor;

	// square faces between intermediate points:

	// Compute indices for inner stacks (not connected to poles).
	for ( UINT32 i = 0; i < numStacks-2; ++i )
	{
		for( UINT32 j = 0; j < numSlices; ++j )
		{
			m_batchedIndices.Add( iBaseVertex+i*numRingVertices + j );
			m_batchedIndices.Add( iBaseVertex+i*numRingVertices + j+1 );
			m_batchedIndices.Add( iBaseVertex+(i+1)*numRingVertices + j );

			m_batchedIndices.Add( iBaseVertex+( i+1)*numRingVertices + j );
			m_batchedIndices.Add( iBaseVertex+i*numRingVertices + j+1 );
			m_batchedIndices.Add( iBaseVertex+(i+1)*numRingVertices + j+1 );
		}
	}

	// triangle faces connecting to top and bottom vertex:

	// Compute indices for top stack.  The top stack was written 
	// first to the vertex buffer.
	for(UINT32 i = 0; i < numSlices; ++i)
	{
		m_batchedIndices.Add(northPoleIndex );
		m_batchedIndices.Add(iBaseVertex+i+1 );
		m_batchedIndices.Add(iBaseVertex+i );
	}

	// Compute indices for bottom stack.  The bottom stack was written
	// last to the vertex buffer, so we need to offset to the index
	// of first vertex in the last ring.
	UINT32 baseIndex = ( numRings - 1 ) * numRingVertices;
	for( UINT32 i = 0; i < numSlices; ++i )
	{
		m_batchedIndices.Add(southPoleIndex );
		m_batchedIndices.Add(baseIndex+i );
		m_batchedIndices.Add(baseIndex+i+1 );
	}
}

mxUNDONE
#if 0
void BatchRenderer::DrawRing( const XMFLOAT3& Origin, const XMFLOAT3& MajorAxis, const XMFLOAT3& MinorAxis, const RGBAf& Color )
{
   static const DWORD dwRingSegments = 32;

    XMFLOAT3 verts[ dwRingSegments + 1 ];

    XMVECTOR vOrigin = XMLoadFloat3( &Origin );
    XMVECTOR vMajor = XMLoadFloat3( &MajorAxis );
    XMVECTOR vMinor = XMLoadFloat3( &MinorAxis );

    float fAngleDelta = XM_2PI / ( float )dwRingSegments;
    // Instead of calling cos/sin for each segment we calculate
    // the sign of the angle delta and then incrementally calculate sin
    // and cosine from then on.
    XMVECTOR cosDelta = XMVectorReplicate( cosf( fAngleDelta ) );
    XMVECTOR sinDelta = XMVectorReplicate( sinf( fAngleDelta ) );
    XMVECTOR incrementalSin = XMVectorZero();
    static const XMVECTOR initialCos =
    {
        1.0f, 1.0f, 1.0f, 1.0f
    };
    XMVECTOR incrementalCos = initialCos;
    for( DWORD i = 0; i < dwRingSegments; i++ )
    {
        XMVECTOR Pos;
        Pos = XMVectorMultiplyAdd( vMajor, incrementalCos, vOrigin );
        Pos = XMVectorMultiplyAdd( vMinor, incrementalSin, Pos );
        XMStoreFloat3( ( XMFLOAT3* )&verts[i], Pos );
        // Standard formula to rotate a vector.
        XMVECTOR newCos = incrementalCos * cosDelta - incrementalSin * sinDelta;
        XMVECTOR newSin = incrementalCos * sinDelta + incrementalSin * cosDelta;
        incrementalCos = newCos;
        incrementalSin = newSin;
    }
    verts[ dwRingSegments ] = verts[0];

    // Copy to vertex buffer
    assert( (dwRingSegments+1) <= MAX_VERTS );

    XMFLOAT3* pVerts = NULL;
    HRESULT hr;
    V( g_pVB->Lock( 0, 0, (void**)&pVerts, D3DLOCK_DISCARD ) )
    memcpy( pVerts, verts, sizeof(verts) );
    V( g_pVB->Unlock() )

    // Draw ring
    D3DXCOLOR clr = Color;
    g_pEffect9->SetFloatArray( g_Color, clr, 4 );
    g_pEffect9->CommitChanges();
    pd3dDevice->DrawPrimitive( D3DPT_LINESTRIP, 0, dwRingSegments );
}
void BatchRenderer::DrawSphere( const XNA::Sphere& sphere, const RGBAf& Color )
{
	const XMFLOAT3 Origin = sphere.Center;
	const float fRadius = sphere.Radius;

	DrawRing( pd3dDevice, Origin, XMFLOAT3( fRadius, 0, 0 ), XMFLOAT3( 0, 0, fRadius ), Color );
	DrawRing( pd3dDevice, Origin, XMFLOAT3( fRadius, 0, 0 ), XMFLOAT3( 0, fRadius, 0 ), Color );
	DrawRing( pd3dDevice, Origin, XMFLOAT3( 0, fRadius, 0 ), XMFLOAT3( 0, 0, fRadius ), Color );
}
void BatchRenderer::DrawRay( const XMFLOAT3& Origin, const XMFLOAT3& Direction, BOOL bNormalize, const RGBAf& Color )
{
    XMFLOAT3 verts[3];
    memcpy( &verts[0], &Origin, 3 * sizeof( float ) );

    XMVECTOR RayOrigin = XMLoadFloat3( &Origin );
    XMVECTOR RayDirection = XMLoadFloat3( &Direction );
    XMVECTOR NormDirection = XMVector3Normalize( RayDirection );
    if( bNormalize )
        RayDirection = NormDirection;

    XMVECTOR PerpVector;
    XMVECTOR CrossVector = XMVectorSet( 0, 1, 0, 0 );
    PerpVector = XMVector3Cross( NormDirection, CrossVector );

    if( XMVector3Equal( XMVector3LengthSq( PerpVector ), XMVectorSet( 0, 0, 0, 0 ) ) )
    {
        CrossVector = XMVectorSet( 0, 0, 1, 0 );
        PerpVector = XMVector3Cross( NormDirection, CrossVector );
    }
    PerpVector = XMVector3Normalize( PerpVector );

    XMStoreFloat3( ( XMFLOAT3* )&verts[1], XMVectorAdd( RayDirection, RayOrigin ) );
    PerpVector = XMVectorScale( PerpVector, 0.0625f );
    NormDirection = XMVectorScale( NormDirection, -0.25f );
    RayDirection = XMVectorAdd( PerpVector, RayDirection );
    RayDirection = XMVectorAdd( NormDirection, RayDirection );
    XMStoreFloat3( ( XMFLOAT3* )&verts[2], XMVectorAdd( RayDirection, RayOrigin ) );
    
    // Copy to vertex buffer
    assert( 3 <= MAX_VERTS );
    XMFLOAT3* pVerts = NULL;
    HRESULT hr;
    V( g_pVB->Lock( 0, 0, (void**)&pVerts, D3DLOCK_DISCARD ) )
    memcpy( pVerts, verts, sizeof(verts) );
    V( g_pVB->Unlock() )

    // Draw ray
    D3DXCOLOR clr = Color;
    g_pEffect9->SetFloatArray( g_Color, clr, 4 );
    g_pEffect9->CommitChanges();
    pd3dDevice->DrawPrimitive( D3DPT_LINESTRIP, 0, 2 );
}
#endif

#if 0
void BatchRenderer::DrawSprite(
	const Float4x4& cameraWorldMatrix,
	const Float3& spriteOrigin,
	const float spriteSizeX, const float spriteSizeY,
	const RGBAf& color
)
{
	const Float3 spriteX = cameraWorldMatrix[0].ToVec3() * spriteSizeX;
	const Float3 spriteY = cameraWorldMatrix[1].ToVec3() * spriteSizeY;

	const UINT32 rgbaColor = color.ToRGBA32();

	BeginBatch( Topology::TriangleList, 4, 6 );

	const UINT32 iBaseVertex = m_batchedVertices.Num();

	AuxVertex & v0 = m_batchedVertices.Add();
	AuxVertex & v1 = m_batchedVertices.Add();
	AuxVertex & v2 = m_batchedVertices.Add();
	AuxVertex & v3 = m_batchedVertices.Add();

	v0.xyz			= spriteOrigin - spriteX - spriteY;	// bottom left
	v0.uv0.x 		= 0.0f;
	v0.uv0.y 		= 1.0f;
	v0.rgba.v 	= rgbaColor;

	v1.xyz			= spriteOrigin - spriteX + spriteY;	// top left
	v1.uv0.x 		= 0.0f;
	v1.uv0.y 		= 0.0f;
	v1.rgba.v 	= rgbaColor;

	v2.xyz			= spriteOrigin + spriteX + spriteY;	// top right
	v2.uv0.x 		= 1.0f;
	v2.uv0.y 		= 0.0f;
	v2.rgba.v 	= rgbaColor;

	v3.xyz			= spriteOrigin + spriteX - spriteY;	// bottom right
	v3.uv0.x 		= 1.0f;
	v3.uv0.y 		= 1.0f;
	v3.rgba.v 	= rgbaColor;

	// indices:
	// 0,	1,	2,
	// 0,	2,	3,

	m_batchedIndices.Add( iBaseVertex + 0 );
	m_batchedIndices.Add( iBaseVertex + 1 );
	m_batchedIndices.Add( iBaseVertex + 2 );

	m_batchedIndices.Add( iBaseVertex + 0 );
	m_batchedIndices.Add( iBaseVertex + 2 );
	m_batchedIndices.Add( iBaseVertex + 3 );
}
#endif

void BatchRenderer::DrawPoint( const AuxVertex& _p )
{
	AuxVertex *	vertices;
	UINT16 *	indices;
	BeginBatch( Topology::PointList, 1, vertices, 0, indices );
	// point lists are rendered as isolated points
	vertices[0] = _p;
}

void BatchRenderer::Flush()
{
	const HContext mainContext = llgl::GetMainContext();

	const UINT32 vertexDataSize = m_batchedVertices.Num();
	const UINT32 indexDataSize = m_batchedIndices.Num();
	const UINT32 numVertices = vertexDataSize / m_vertexStride;
	const UINT32 numIndices = indexDataSize / m_indexStride;

	if( numVertices )
	{
		mxASSERT_PTR(m_technique);

		llgl::DrawCall	batch;
		batch.Clear();

		FxApplyShaderState(batch, *m_technique);

		llgl::UpdateBuffer( mainContext, m_dynamicVB, m_batchedVertices.Num(), m_batchedVertices.ToPtr() );

		if( numIndices > 0 ) {
			llgl::UpdateBuffer( mainContext, m_dynamicIB, m_batchedIndices.Num(), m_batchedIndices.ToPtr() );
		}

		batch.inputLayout = m_layout;
		batch.topology = m_topology;

		batch.VB[0] = m_dynamicVB;
		batch.IB = m_dynamicIB;
		batch.b32bit = (m_indexStride==sizeof(UINT32));

		batch.baseVertex = 0;
		batch.vertexCount = numVertices;
		batch.startIndex = 0;
		batch.indexCount = numIndices;

		llgl::Submit(mainContext, batch);
	}

	m_batchedVertices.Empty();
	m_batchedIndices.Empty();
}

void BatchRenderer::Draw(
					  const AuxVertex* vertices, UINT32 numVertices,
					  const UINT16* indices, UINT32 numIndices,
					  const Topology::Enum topology
					  )
{
	mxASSERT_PTR(m_technique);
	const UINT32 vertexDataSize = numVertices * sizeof(vertices[0]);
	const UINT32 indexDataSize = numIndices * sizeof(indices[0]);
	mxASSERT(vertexDataSize <= m_VBSize);
	mxASSERT(indexDataSize <= m_IBSize);

	Flush();

	const HContext mainContext = llgl::GetMainContext();

	llgl::UpdateBuffer( mainContext, m_dynamicVB, numVertices*sizeof(vertices[0]), vertices );

	if( indices && numIndices ) {
		llgl::UpdateBuffer( mainContext, m_dynamicIB, numIndices*sizeof(indices[0]), indices );
	}

	llgl::DrawCall	batch;
	batch.Clear();

	FxApplyShaderState(batch, *m_technique);

	batch.inputLayout = m_layout;
	batch.topology = topology;

	batch.VB[0] = m_dynamicVB;
	batch.IB = m_dynamicIB;
	batch.b32bit = (sizeof(indices[0])==sizeof(UINT32));

	batch.baseVertex = 0;
	batch.vertexCount = numVertices;
	batch.startIndex = 0;
	batch.indexCount = numIndices;

	llgl::Submit(mainContext, batch);
}

namespace Fonts
{
	static const UINT32 SPRITE_FONT_MAGIC = 'TNOF';    // 'FONT'

	// Comparison operators make our sorted glyph vector work with std::binary_search and lower_bound.
	static inline bool operator < ( const Glyph& left, const Glyph& right)
	{
		return left.Character < right.Character;
	}
	static inline bool operator < ( wchar_t left, const Glyph& right )
	{
		return left < right.Character;
	}
	static inline bool operator < ( const Glyph& left, wchar_t right )
	{
		return left.Character < right;
	}

	static PixelFormat::Enum GetEngineFormat( UINT32 nativeFormat )
	{
		enum {
			__DXGI_FORMAT_R8G8B8A8_UNORM = 28,
			__DXGI_FORMAT_B4G4R4A4_UNORM = 115,
			__DXGI_FORMAT_B8G8R8A8_UNORM = 87,
			__DXGI_FORMAT_BC2_UNORM = 74
		};

		if( nativeFormat == __DXGI_FORMAT_R8G8B8A8_UNORM ) {
			return PixelFormat::RGBA8;
		}
		if( nativeFormat == __DXGI_FORMAT_B8G8R8A8_UNORM ) {
			return PixelFormat::BGRA8;
		}
		if( nativeFormat == __DXGI_FORMAT_BC2_UNORM ) {
			return PixelFormat::BC2;
		}

		return PixelFormat::Unknown;
	}

	SpriteFont::SpriteFont()
	{
		m_default = NULL;
		m_lineSpacing = 0.0f;
		m_textureAtlas.SetNil();
	}
	SpriteFont::~SpriteFont()
	{
	}
	ERet SpriteFont::Load( AStreamReader& stream )
	{
		FileHeader	header;
		stream.Get( header );

		// Validate the header.
		if( header.magic != SPRITE_FONT_MAGIC ) {
			ptERROR("Invalid font library!\n");
			return ERR_INVALID_PARAMETER;
		}

		// Read the (sorted) glyph data.
		m_glyphs.SetNum( header.numGlyphs );
		stream.Read( m_glyphs.ToPtr(), m_glyphs.GetDataSize() );
		//mxASSERT(std::is_sorted( m_glyphs.ToPtr(), m_glyphs.ToPtr() + m_glyphs.Num() ), "Glyphs must be in ascending codepoint order");

		// Read font properties.
		m_lineSpacing = header.lineSpacing;
		
		SetDefaultCharacter( (UNICODECHAR)header.defaultChar );

		// Read the texture data.
		BitmapHeader	bitmap;
		stream.Get( bitmap );

		const UINT32 imageSize = bitmap.pitch * bitmap.rows;

		ScopedStackAlloc    tempAlloc( gCore.frameAlloc );
		void* imageData = tempAlloc.AllocA( imageSize );
		stream.Read( imageData, imageSize );

		const PixelFormat::Enum textureFormat = GetEngineFormat(bitmap.format);
		chkRET_X_IF_NOT(textureFormat != PixelFormat::Unknown, ERR_INVALID_PARAMETER);

		Texture2DDescription	texDesc;
		texDesc.format = textureFormat;
		texDesc.width = bitmap.width;
		texDesc.height = bitmap.height;
		texDesc.numMips = 1;
		texDesc.name.SetReference("SpriteFont");

		m_textureAtlas = llgl::CreateTexture2D(texDesc, imageData);
		m_resourceHandle = llgl::AsResource( m_textureAtlas );

		m_invTextureWidth = 1.0f / bitmap.width;
		m_invTextureHeight = 1.0f / bitmap.height;

		return ALL_OK;
	}

	void SpriteFont::Shutdown()
	{
		m_resourceHandle.SetNil();

		if( m_textureAtlas.IsValid() ) {
			llgl::DeleteTexture( m_textureAtlas );
			m_textureAtlas.SetNil();
		}
	}

	const Glyph* SpriteFont::FindGlyph( UNICODECHAR character ) const
	{
		const Glyph* glyph = std::lower_bound( m_glyphs.ToPtr(), m_glyphs.ToPtr() + m_glyphs.Num(), character );
		if( glyph != NULL && glyph->Character == character ) {
			return glyph;
		}
		return m_default;
	}

	bool SpriteFont::ContainsCharacter( UNICODECHAR character ) const
	{
		return std::binary_search( m_glyphs.ToPtr(), m_glyphs.ToPtr() + m_glyphs.Num(), character );
	}

	void SpriteFont::SetDefaultCharacter( UNICODECHAR character )
	{
		m_default = FindGlyph( character );
		if( !m_default ) {
			ptWARN("Couldn't find default glyph!\n");
			m_default = m_glyphs.GetItemPtr( 0 );
		}
	}

	float SpriteFont::GetLineSpacing() const
	{
		return m_lineSpacing;
	}

	// The core glyph layout algorithm, shared between DrawString and MeasureString.
	template< typename WHAT >    // where WHAT has operator () ( const Glyph& glyph, float x, float y, UINT32 index, void* userData )
	void SpriteFont::TForEachGlyph( _In_z_ wchar_t const* text, WHAT action, void* userData ) const
	{
		float x = 0;
		float y = 0;
		UINT32 index = 0;

		for( ; *text; text++ )
		{
			wchar_t character = *text;

			switch( character )
			{
			case '\r':
				// Skip carriage returns.
				continue;

			case '\n':
				// New line.
				x = 0;
				y += m_lineSpacing;
				break;

			default:
				{
					// Output this character.
					const Glyph& glyph = *FindGlyph( character );

					x += glyph.XOffset;

					if( x < 0 ) {
						x = 0;
					}

					if( !iswspace( character ) )
					{
						action( glyph, x, y, index++, userData );
					}

					x += glyph.Subrect.right - glyph.Subrect.left + glyph.XAdvance;
				}
				break;
			}
		}
	}

	void SpriteFont::ForEachGlyph( _In_z_ wchar_t const* text, GlyphCallback* callback, void* userData ) const
	{
		struct CallbackWrapper
		{
			GlyphCallback *    m_callback;
			CallbackWrapper( GlyphCallback* _callback )
				: m_callback( _callback )
			{}
			void operator () ( const Glyph& glyph, float x, float y, UINT32 index, void* userData )
			{
				(*m_callback)( glyph, x, y, index, userData );
			}
		};
		TForEachGlyph( text, CallbackWrapper(callback), userData );
	}

	void SpriteFont::MeasureString( _In_z_ wchar_t const* text, float *width, float *height )
	{
		*width = 0;
		*height = 0;

		struct MeasureString
		{
			const float lineSpacing;
			float        maxWidth;
			float        maxHeight;
		public:
			MeasureString( float _lineSpacing )
				: lineSpacing( _lineSpacing ), maxWidth( 0.0f ), maxHeight( 0.0f )
			{}
			void operator () ( const Glyph& glyph, float x, float y, UINT32 index, void* userData )
			{
				float w = (float)(glyph.Subrect.right - glyph.Subrect.left);
				float h = (float)(glyph.Subrect.bottom - glyph.Subrect.top) + glyph.YOffset;

				h = std::max(h, lineSpacing);

				maxWidth = std::max(maxWidth, x + w);
				maxHeight = std::max(maxHeight, y + h);
			}
		};
		MeasureString    measureString( m_lineSpacing );
		TForEachGlyph( text, measureString, NULL );

		*width = measureString.maxWidth;
		*height = measureString.maxHeight;
	}

	// each single character corresponds to exactly one vertex:
	// we do point sprite => quad expansion in geometry shader.
	struct TextVertex
	{
		Float4	xy_wh;
		Float4	tl_br;	// UV texture coords for top left and bottom right corners
	};
	enum { MAX_TEXT_VERTS = 512 };
	static const UINT32 TEXT_VERTEX_BUFFER_SIZE = MAX_TEXT_VERTS * sizeof(TextVertex);

	FontRenderer::FontRenderer()
	{

	}

	ERet FontRenderer::Initialize()
	{
		VertexDescription	vertexDesc;
		{
			vertexDesc.Begin();
			vertexDesc.Add(AttributeType::Float, 4, VertexAttribute::TexCoord0, false);
			vertexDesc.Add(AttributeType::Float, 4, VertexAttribute::TexCoord1, false);
			vertexDesc.End();
		}
		m_vertexLayout = llgl::CreateInputLayout( vertexDesc, "TextVertex" );

		m_vertexBuffer = llgl::CreateBuffer( Buffer_Vertex, TEXT_VERTEX_BUFFER_SIZE );

		return ALL_OK;
	}
	void FontRenderer::Shutdown()
	{
		llgl::DeleteBuffer(m_vertexBuffer);
		m_vertexBuffer.SetNil();
		llgl::DeleteInputLayout(m_vertexLayout);
		m_vertexLayout.SetNil();
	}

	// viewport coordinates => normalized device coordinates (NDC)
	template< typename POINT_SIZE >
	static inline void ViewportToNDC(
		float invScreenWidth, float invScreenHeight,
		POINT_SIZE screenX, POINT_SIZE screenY,
		float &ndcX, float &ndcY)
	{
		ndcX = (screenX * invScreenWidth) * 2.0f - 1.0f;	// [0..ScreenWidth] => [-1..+1]
		ndcY = (screenY * invScreenHeight) * 2.0f - 1.0f;	// [0..ScreenHeight] => [+1..-1] (flip Y axis)
		ndcY *= -1.0f;
	}

	struct ParamData
	{
		const SpriteFont *	font;
		TextVertex *		verts;
		float				startX;
		float				startY;
		float				invScreenWidth;
		float				invScreenHeight;		
	};
	static void AddCharacterVertex( const Glyph& glyph, float x, float y, UINT32 index, void* param )
	{
		const ParamData& data = *(const ParamData*) param;
		const SpriteFont* font = data.font;
		const float invTextureWidth = font->m_invTextureWidth;
		const float invTextureHeight = font->m_invTextureHeight;

		x += data.startX;
		y += data.startY;

		// Calculate the X and Y pixel position on the screen to start drawing to.
		float glyphLeftX, glyphTopY;
		ViewportToNDC( data.invScreenWidth, data.invScreenHeight, x, y, glyphLeftX, glyphTopY );

		const UINT32 glyphWidth = glyph.Subrect.right - glyph.Subrect.left;
		const UINT32 glyphHeight = glyph.Subrect.bottom - glyph.Subrect.top;

		// this controls the size of the character sprite
		float glyphScalingX, glyphScalingY;
		glyphScalingX = data.invScreenWidth;
		glyphScalingY = data.invScreenHeight;
		glyphScalingX *= 2.0f;
		glyphScalingY *= 2.0f;
		glyphLeftX += x * glyphScalingX;
		glyphTopY -= (y + glyph.YOffset) * glyphScalingY;

		float glyphSizeX = (float)glyphWidth * glyphScalingX;
		float glyphSizeY = (float)glyphHeight * glyphScalingY;

		TextVertex& vertex = data.verts[ index ];

		vertex.xy_wh = Float4_Set(
			glyphLeftX,
			glyphTopY,
			glyphSizeX,
			glyphSizeY
			);

		vertex.tl_br = Float4_Set(
			glyph.Subrect.left * invTextureWidth,
			glyph.Subrect.top * invTextureHeight,
			glyph.Subrect.right * invTextureWidth,
			glyph.Subrect.bottom * invTextureHeight
			);
	}

	// x and y are in viewport coordinates of the text sprite's top left corner
	ERet FontRenderer::RenderText(
		HContext _context, llgl::DrawCall& batch,
		UINT16 screenWidth, UINT16 screenHeight,
		const SpriteFont* font,
		UINT16 x, UINT16 y,
		const char* text, UINT16 length
		)
	{
		if( !length ) {
			length = strlen(text);
		}
		if( !length ) {
			return ALL_OK;
		}
		if( length > MAX_TEXT_VERTS ) {
			return ERR_BUFFER_TOO_SMALL;
		}

		ScopedStackAlloc	tempAlloc( gCore.frameAlloc );
		wchar_t * wideCharData = tempAlloc.AllocMany< wchar_t >( length + 1 );
		mbstowcs( wideCharData, text, length );
		wideCharData[ length ] = 0;

		{
			ParamData	data;
			data.font = font;
			data.verts = (TextVertex*) llgl::MapBuffer(_context, m_vertexBuffer, TEXT_VERTEX_BUFFER_SIZE, Map_Write_Discard);
			data.startX = x;
			data.startY = y;
			data.invScreenWidth = 1.0f / screenWidth;
			data.invScreenHeight = 1.0f / screenHeight;

			font->ForEachGlyph( wideCharData, &AddCharacterVertex, &data );

			llgl::UnmapBuffer(_context, m_vertexBuffer);
		}

		batch.inputLayout = m_vertexLayout;
		batch.topology = Topology::PointList;

		batch.VB[0] = m_vertexBuffer;

		batch.baseVertex = 0;
		batch.vertexCount = length;
		batch.startIndex = 0;
		batch.indexCount = 0;

		return ALL_OK;
	}

}//namespace Fonts


/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/
AxisArrowGeometry::AxisArrowGeometry()
{
	BuildGeometry();
}

void AxisArrowGeometry::BuildGeometry()
{
	// Pre-compute the vertices for drawing axis arrows.
	static float AXIS_ARROW_RADIUS = 0.09f * 1;
	HOT_VAR(AXIS_ARROW_RADIUS);

	static float ARROW_BASE_HEIGHT = 1.8f * 1;
	HOT_VAR(ARROW_BASE_HEIGHT);

	static float ARROW_TOTAL_LENGTH = 2.0f * 1;
	HOT_VAR(ARROW_TOTAL_LENGTH);

	//const float ARROW_CONE_HEIGHT = arrowHeadVertex.z - arrowBaseVertex.z;

	static float ARROW_CONE_HEIGHT = 1.6f * 1;
	HOT_VAR(ARROW_CONE_HEIGHT);

	m_axisRoot = Float3_Set( 0,0,0 );
	m_arrowBase = Float3_Set( 0,0,ARROW_BASE_HEIGHT );
	m_arrowHead = Float3_Set( 0,0,ARROW_TOTAL_LENGTH );

	// Generate the axis arrow cone

	// Generate the vertices for the base of the cone

	for( UINT iSegment = 0 ; iSegment <= AXIS_ARROW_SEGMENTS ; iSegment++ )
	{
		const float theta = iSegment * (mxTWO_PI / AXIS_ARROW_SEGMENTS);	// in radians

		float	s, c;
		Float_SinCos( theta, s, c );

		m_segments[ iSegment ] = Float3_Set( AXIS_ARROW_RADIUS * s, AXIS_ARROW_RADIUS * c, ARROW_CONE_HEIGHT );
	}
}

void AxisArrowGeometry::Draw( BatchRenderer& renderer, const RGBAf& color ) const
{
	// draw the 'stem'
	renderer.DrawLine( Float3_Zero(), m_arrowHead, color );

	// draw the arrow head
	for( UINT iSegment = 0 ; iSegment < AXIS_ARROW_SEGMENTS ; iSegment++ )
	{
		const Float3& p0 = m_segments[ iSegment ];
		const Float3& p1 = m_segments[ iSegment + 1 ];

		AuxVertex	a, b, c;
		a.xyz = p0;
		b.xyz = p1;
		c.xyz = m_arrowHead;

		a.rgba.v = color.ToInt32();
		b.rgba.v = color.ToInt32();
		c.rgba.v = color.ToInt32();

		// Draw the base triangle of the cone.
		// NOTE: no need because we disable backface culling
		//renderer.DrawTriangle3D( a, b, c );

		// Draw the top triangle of the cone.
		renderer.DrawSolidTriangle3D( a, b, c );
	}
}

void AxisArrowGeometry::Draw( BatchRenderer& renderer, const Float4x4& transform, const RGBAf& color ) const
{
	Float3 transformedHead = Matrix_TransformPoint( transform, m_arrowHead );

	// draw the 'stem'
	renderer.DrawLine( Matrix_GetTranslation(transform), transformedHead, color );

	// draw the arrow head
	for( UINT iSegment = 0 ; iSegment < AXIS_ARROW_SEGMENTS ; iSegment++ )
	{
		const Float3& p0 = Matrix_TransformPoint( transform, m_segments[ iSegment ] );
		const Float3& p1 = Matrix_TransformPoint( transform, m_segments[ iSegment + 1 ] );

		AuxVertex	a, b, c;
		a.xyz = p0;
		b.xyz = p1;
		c.xyz = transformedHead;

		a.rgba.v = color.ToInt32();
		b.rgba.v = color.ToInt32();
		c.rgba.v = color.ToInt32();

		// Draw the base triangle of the cone.
		// NOTE: no need because we disable backface culling
		//renderer.DrawTriangle3D( a, b, c );

		// Draw the top triangle of the cone.
		renderer.DrawSolidTriangle3D( a, b, c );
	}
}

void DrawGizmo( const AxisArrowGeometry& gizmo, const Float4x4& localToWorld, const Float3& cameraPosition, BatchRenderer & renderer )
{
	float scale = GetGizmoScale(cameraPosition, Matrix_GetTranslation(localToWorld));
	scale *= 0.1f;
	Float4x4 S = Matrix_Scaling(scale,scale,scale);
	Float3 axisX = Float4_As_Float3(localToWorld[0]);
	Float3 axisY = Float4_As_Float3(localToWorld[1]);
	Float3 axisZ = Float4_As_Float3(localToWorld[2]);
	gizmo.Draw(renderer, S*Matrix_FromAxes(axisY,axisZ,axisX), RGBAf::RED);
	gizmo.Draw(renderer, S*Matrix_FromAxes(axisZ,axisX,axisY), RGBAf::GREEN);
	gizmo.Draw(renderer, S*Matrix_FromAxes(axisX,axisY,axisZ), RGBAf::BLUE);
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
