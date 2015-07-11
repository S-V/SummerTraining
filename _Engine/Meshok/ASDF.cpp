// Adaptively Sampled Distance Field.
#include "stdafx.h"
#pragma hdrstop
#include <Meshok/Meshok.h>
#include <Meshok/ASDF.h>
#include <Meshok/Octree.h>

namespace ASDF
{

Options::Options()
{
	//AABB24_Clear(&bounds);
	radius = 1.0f;
	max_depth = ~0;
	min_subdiv = 0;
	error_threshold = 0;
}

static NodeID CreateLeaf(
						 Octree & tree,
						 const OctCubeF& _bounds,
						 const UINT32 _treeLevel,
						 const AVolume* _volume,
						 PerfStats &stats
						 )
{
	const NodeID leafIndex = tree.m_leaves.Alloc();
	Leaf& leaf = tree.m_leaves[ leafIndex ];

	Float3 corners[8];
	OCT_GetCorners( _bounds, corners );

	for( int i = 0; i < 8; i++ )
	{
		leaf.d[i] = _volume->GetDistanceAt(corners[i]);
	}

	return MAKE_LEAF_ID(leafIndex);
}

static NodeID BuildTreeRecursive(
								 Octree & tree,
								 const OctCubeF& _bounds,
								 const UINT32 _treeLevel,
								 const AVolume* _volume,
								 const Options& options,
								 PerfStats &stats
								 )
{
	const bool isLeaf = (_treeLevel >= options.max_depth)
				|| (_bounds.radius <= options.min_subdiv);

	if( isLeaf )
	{
		return CreateLeaf( tree, _bounds, _treeLevel, _volume, stats );
	}
	else
	{
		OctCubeF octants[8];
		GetChildOctants(_bounds,octants);
		const UINT32 nextLevel = _treeLevel + 1;

		NodeID nodeID = tree.m_nodes.Alloc();
		for( int i = 0; i < 8; i++ )
		{
			NodeID childID = BuildTreeRecursive( tree, octants[i], nextLevel, _volume, options, stats );
			Node& node = tree.m_nodes[ nodeID ];
			node.kids[i] = childID;
		}

		//// collapse empty nodes
		//if( IsBadNode( m_nodes[ nodeID ] ) )
		//{
		//	stats.numBadNodes++;
		//	m_nodes.Free( nodeID );
		//	nodeID = NIL_NODE;
		//}
		//else
		//{
		//	stats.numInternalNodes++;
		//}		

		return nodeID;
	}
}
Octree::Octree()
{
}
Octree::~Octree()
{
	m_nodes.Empty();
	m_leaves.Empty();
}
ERet Octree::Build( AVolume* _volume, const Options& _options )
{
	OctCubeF worldBounds;
	worldBounds.x = 0;
	worldBounds.y = 0;
	worldBounds.z = 0;
	worldBounds.radius = _options.radius;

	PerfStats	stats;

	NodeID rootNodeID = BuildTreeRecursive( *this, worldBounds, 0, _volume, _options, stats );
	
	return ALL_OK;
}

Volume::Volume()
{
	radius = 1.0f;
}

//float InterpolateLinear( const float value, const float min, const float max )
//{
//	return value * min + (1.0 - value) * max;
//}
//float InterpolateBilinear( const Float2& value, const Float2& min, const Float2& max )
//{
//	float range_x = max.x - min.x;
//	float range_y = max.y - min.y;
//
//	float a = value.x - min.x;
//	float b = value.x - min.x;
//
//	float x1 = (value.x - min.x) / range_x;
//
//	return value * min + (1.0 - value) * max;
//}

// http://en.wikipedia.org/wiki/Bilinear_interpolation
float InterpolateBilinear(
						  const Float2& xy,
						  const Float2& min,
						  const Float2& max,
						  const float Q12, const float Q22,	// upper values
						  const float Q11, const float Q21	// lower values
						  )
{
	float range_x = max.x - min.x;
	float range_y = max.y - min.y;
	float max_minus_x = max.x - xy.x;
	float x_minus_min = xy.x - min.x;
	// We first do linear interpolation in the x-direction.
	float lower = max_minus_x * Q11 + x_minus_min * Q21;
	float upper = max_minus_x * Q12 + x_minus_min * Q22;
	// We proceed by interpolating in the y-direction.
	return (max.y - xy.y) * lower + (xy.y - min.y) * upper;
}

float GetLerpParam( float x, float min, float max )
{
	mxASSERT( x >= min && x <= max );
	return (x - min) / (max - min);
}
float Float_Lerp01( float t, float min, float max )
{
	mxASSERT( t >= 0.0f && t <= 1.0f );
	return min + (max - min) * t;
}

float InterpolateBilinear2(
						  const Float2& xy,
						  const Float2& min,
						  const Float2& max,
						  const float Q12, const float Q22,	// upper values
						  const float Q11, const float Q21	// lower values
						  )
{
	float tx = GetLerpParam( xy.x, min.x, max.x );
	float xl = Float_Lerp01( Q11, Q21, tx );
	float xu = Float_Lerp01( Q12, Q22, tx );

	float ty = GetLerpParam( xy.y, min.y, max.y );
	float result = Float_Lerp01( xl, xu, ty );
	return result;


	float range_x = max.x - min.x;
	float range_y = max.y - min.y;
	float max_minus_x = max.x - xy.x;
	float x_minus_min = xy.x - min.x;
	// We first do linear interpolation in the x-direction.
	float lower = max_minus_x * Q11 + x_minus_min * Q21;
	float upper = max_minus_x * Q12 + x_minus_min * Q22;
	// We proceed by interpolating in the y-direction.
	return (max.y - xy.y) * lower + (xy.y - min.y) * upper;
}

float InterpolateTrilinear(
						   const Float3& point,
						   const Float3& min, const Float3& max,
						   const float values[8]
)
{
	const Float2& xy = Float3_As_Float2( point );
	const Float2& min_xy = Float3_As_Float2( min );
	const Float2& max_xy = Float3_As_Float2( max );

	float lower = InterpolateBilinear( xy, min_xy, max_xy, values[2], values[3], values[0], values[1] );
	float upper = InterpolateBilinear( xy, min_xy, max_xy, values[6], values[7], values[4], values[5] );
	return lower * ( max.z - point.z ) + upper * ( point.z - min.z );


	float range_x = max.x - min.x;
	float range_y = max.y - min.y;
	float range_z = max.z - min.z;

	float dx = (point.x - min.x) / range_x;
	float dy = (point.y - min.y) / range_y;
	float dz = (point.z - min.z) / range_z;

	float one_minus_dx = 1.0f - dx;
	float one_minus_dy = 1.0f - dy;
	float one_minus_dz = 1.0f - dz;

	//@todo: optimize, fold constant subexpr
	return
		values[0] * dx * dy * dz +
		values[1] * one_minus_dx * dy * dz +
		values[2] * dx * one_minus_dy * dz +
		values[3] * one_minus_dx * one_minus_dy * dz +
		values[4] * dx * dy * one_minus_dz +
		values[5] * one_minus_dx * dy * one_minus_dz +
		values[6] * dx * one_minus_dy * one_minus_dz +
		values[7] * one_minus_dx * one_minus_dy * one_minus_dz
		;

#if 0
    const float c00 = values[0]*(1-dx) + values[4]*dx;
    const float c01 = values[1]*(1-dx) + values[5]*dx;
    const float c10 = values[2]*(1-dx) + values[6]*dx;
    const float c11 = values[3]*(1-dx) + values[7]*dx;

    const float  c0 = c00*(1-dy) + c10*dy;
    const float  c1 = c01*(1-dy) + c11*dy;

    const float   c = c0*(1-dz) + c1*dz;

    return c;
#endif

}

float Volume::GetDistanceAt( const Float3& _position ) const
{
	OctCubeF bounds;
	bounds.x = 0;
	bounds.y = 0;
	bounds.z = 0;
	bounds.radius = radius;

	NodeID nodeID = 0;

	while( !IS_LEAF_ID( nodeID ) )
	{
		// get the index of the child octant containing the point
		int childIndex =
			((_position.x >= bounds.x) ? CHILD_MASK_X : 0)|
			((_position.y >= bounds.y) ? CHILD_MASK_Y : 0)|
			((_position.z >= bounds.z) ? CHILD_MASK_Z : 0);

		bounds = GetChildOctant( bounds, childIndex );

		const Node& node = tree->m_nodes[ nodeID ];
		nodeID = node.kids[ childIndex ];
	}
	if( nodeID != NIL_NODE )
	{
		mxASSERT(IS_LEAF_ID(nodeID));
		const Leaf& leaf = tree->m_leaves[ GET_ID(nodeID) ];

		AABB24 aabb;
		OctBoundsToAABB( bounds, aabb );

		float dminx = Float_Abs( _position.x - aabb.min_point.x );
		float dminy = Float_Abs( _position.y - aabb.min_point.y );
		float dminz = Float_Abs( _position.z - aabb.min_point.z );
		float dmaxx = Float_Abs( _position.x - aabb.max_point.x );
		float dmaxy = Float_Abs( _position.z - aabb.max_point.z );
		float dmaxz = Float_Abs( _position.y - aabb.max_point.y );

		mxASSERT(AABB_ContainsPoint( aabb, _position, 1e-3f ));

		float result = InterpolateTrilinear(_position, aabb.min_point, aabb.max_point, leaf.d);
		return result;
	}

	return 1.0f;
}

}//namespace ASDF

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
