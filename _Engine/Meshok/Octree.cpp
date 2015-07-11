#include "stdafx.h"
#pragma hdrstop
#include <Meshok/Meshok.h>
#include <Meshok/Octree.h>

const UINT8* CUBE_GetEdgeIndices()
{
	static const UINT8 edgeIndices[ 12*2 ] =
	{
		// bottom
		0, 2,
		2, 3,
		3, 1,
		1, 0,

		// middle - bottom to top
		0, 4,
		1, 5,
		3, 7,
		2, 6,

		// top
		4, 5,
		5, 7,
		7, 6,
		6, 4
	};
	return edgeIndices;
}

const UINT32 g_bitCounts8[256] =
{
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

mxDEFINE_CLASS(FeatureVertex);
mxBEGIN_REFLECTION(FeatureVertex)
	mxMEMBER_FIELD(xyz),
	mxMEMBER_FIELD(N),
mxEND_REFLECTION
FeatureVertex::FeatureVertex()
{
}

mxDEFINE_CLASS(MeshOctreeNode);
mxBEGIN_REFLECTION(MeshOctreeNode)
	mxMEMBER_FIELD(features),
	mxMEMBER_FIELD(kids),
mxEND_REFLECTION
MeshOctreeNode::MeshOctreeNode()
{
}

mxDEFINE_CLASS(MeshOctree);
mxBEGIN_REFLECTION(MeshOctree)
	mxMEMBER_FIELD(m_nodes),
mxEND_REFLECTION
MeshOctree::MeshOctree()
{
}

ERet MeshOctree::Build( ATriangleMeshInterface* triangleMesh )
{
	UNDONE;
#if 0
const UINT32 startTimeMSec = mxGetTimeInMilliseconds();

	struct CollectTriangles : ATriangleIndexCallback
	{
		Tree &	m_tree;
		CollectTriangles( Tree & tree ) : m_tree( tree )
		{}
		virtual void ProcessTriangle( const Vertex& a, const Vertex& b, const Vertex& c ) override
		{
			BspPoly & triangle = m_tree.m_polys.Add();
			triangle.vertices.SetNum(3);
			BspVertex & v1 = triangle.vertices[0];
			BspVertex & v2 = triangle.vertices[1];
			BspVertex & v3 = triangle.vertices[2];
			v1.xyz = a.xyz;
			v2.xyz = b.xyz;
			v3.xyz = c.xyz;
			v1.st = Float2_To_Half2(a.st);
			v2.st = Float2_To_Half2(b.st);
			v3.st = Float2_To_Half2(c.st);
			triangle.next = NIL_ID;
		}
	};

	CollectTriangles	callback( *this );
	triangleMesh->ProcessAllTriangles( &callback );

	chkRET_X_IF_NOT(m_polys.Num() > 0, ERR_INVALID_PARAMETER);

	// setup a linked list of polygons
	for( int iPoly = 0; iPoly < m_polys.Num()-1; iPoly++ )
	{
		m_polys[iPoly].next = iPoly+1;
	}

	BspStats	stats;
	stats.m_polysBefore = m_polys.Num();

	BuildTree_R( *this, 0, stats );

	stats.m_polysAfter = m_polys.Num();

	m_nodes.Shrink();
	m_planes.Shrink();
	m_polys.Shrink();

	stats.m_numInternalNodes = m_nodes.Num();
	stats.m_numPlanes = m_planes.Num();
	stats.m_bytesAllocated = this->BytesAllocated();

	const UINT32 currentTimeMSec = mxGetTimeInMilliseconds();
	stats.Print(currentTimeMSec - startTimeMSec);
#endif
	return ALL_OK;
}

// copied from
// http://www.gamedev.net/topic/552906-closest-point-on-triangle/
// which is based on
// http://www.geometrictools.com/Documentation/DistancePoint3Triangle3.pdf
Float3 ClosestPointOnTriangle( const Float3 triangle[3], const Float3& sourcePosition )
{
    Float3 edge0 = triangle[1] - triangle[0];
    Float3 edge1 = triangle[2] - triangle[0];
    Float3 v0 = triangle[0] - sourcePosition;

    float a = Float3_Dot( edge0, edge0 );
    float b = Float3_Dot( edge0, edge1 );
    float c = Float3_Dot( edge1, edge1 );
    float d = Float3_Dot( edge0, v0 );
    float e = Float3_Dot( edge1, v0 );

    float det = a*c - b*b;
    float s = b*e - c*d;
    float t = b*d - a*e;

    if ( s + t < det )
    {
        if ( s < 0.f )
        {
            if ( t < 0.f )
            {
                if ( d < 0.f )
                {
                    s = clampf( -d/a, 0.f, 1.f );
                    t = 0.f;
                }
                else
                {
                    s = 0.f;
                    t = clampf( -e/c, 0.f, 1.f );
                }
            }
            else
            {
                s = 0.f;
                t = clampf( -e/c, 0.f, 1.f );
            }
        }
        else if ( t < 0.f )
        {
            s = clampf( -d/a, 0.f, 1.f );
            t = 0.f;
        }
        else
        {
            float invDet = 1.f / det;
            s *= invDet;
            t *= invDet;
        }
    }
    else
    {
        if ( s < 0.f )
        {
            float tmp0 = b+d;
            float tmp1 = c+e;
            if ( tmp1 > tmp0 )
            {
                float numer = tmp1 - tmp0;
                float denom = a-2*b+c;
                s = clampf( numer/denom, 0.f, 1.f );
                t = 1-s;
            }
            else
            {
                t = clampf( -e/c, 0.f, 1.f );
                s = 0.f;
            }
        }
        else if ( t < 0.f )
        {
            if ( a+d > b+e )
            {
                float numer = c+e-b-d;
                float denom = a-2*b+c;
                s = clampf( numer/denom, 0.f, 1.f );
                t = 1-s;
            }
            else
            {
                s = clampf( -e/c, 0.f, 1.f );
                t = 0.f;
            }
        }
        else
        {
            float numer = c+e-b-d;
            float denom = a-2*b+c;
            s = clampf( numer/denom, 0.f, 1.f );
            t = 1.f - s;
        }
    }

    return triangle[0] + edge0 * s + edge1 * t;
}

float MeshSDF::GetDistanceAt( const Float3& _position ) const
{
UNDONE;
return 0;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
