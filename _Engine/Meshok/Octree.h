#pragma once

#include <Meshok/Meshok.h>
#include <Meshok/Morton.h>
#include <Meshok/Cube.h>
#include <Meshok/SDF.h>

/*
	The Coordinate System Used for Voxel Space:

	   Z
	   |  /Y
	   | /
	   |/_____X
	 (0,0,0)

 Octree node enumeration:
        6___________7
       /|           /
      / |          /|
     /  |         / |
    4------------5  |
    |   2________|__3
    |   /        |  /
    |  /         | /
    | /          |/
    0/___________1

or using locational (Morton) codes (X - lowest bit, Y - middle, Z - highest):

       110__________111
       /|           /
      / |          /|
     /  |         / |
  100-----------101 |
    |  010_______|__011
    |   /        |  /
    |  /         | /
    | /          |/
  000/___________001

(see Gray code, Hamming distance, De Bruijn sequence)

	Each octant gets a 3-bit number between 0 and 7 assigned,
	depending on the node's relative position to its parent's center.
	The possible relative positions are:
	bottom-left-front (000), bottom-right-front (001), bottom-left-back (010), bottom-right-back (011),
	top-left-front (100), top-right-front (101), top-left-back (110), top-right-back (111).
	The locational code of any child node in the tree can be computed recursively
	by concatenating the octant numbers of all the nodes from the root down to the node in question. 

	Numbering of Octree children.
	Children follow a predictable pattern to make accesses simple.

	Cells are numbered such that:
	bit 0 determines the X axis where left   is 0 and right is 1;
	bit 1 determines the Y axis where front  is 0 and back  is 2;
	bit 2 determines the Z axis where bottom is 0 and top   is 4;
	Cells can be referenced by ORing the three axis bits together.
	(Z-Space Filling Curve)	This pattern defines a 3-bit value, where "-" is a zero-bit and "+" is a one-bit.
	Here, '-' means less than 'origin' in that dimension, '+' means greater than:

	0 = --- (000b)
	1 = +-- (001b)
	2 = -+- (010b)
	3 = ++- (011b)
	4 = --+ (100b)
	5 = +-+ (101b)
	6 = -++ (110b)
	7 = +++ (111b)
*/

enum {
	CHILD_MASK_X = BIT(0),
	CHILD_MASK_Y = BIT(1),
	CHILD_MASK_Z = BIT(2)
};

template< typename N >
struct TOctBounds
{
	N x, y, z;	// center or corner (depends on chosen conventions)
	N radius;	// extent (cube's half size)

public:
	const Float3 XYZ() const { return Float3_Set(x,y,z); }
};

typedef TOctBounds< int > OctCubeI;
typedef TOctBounds< float > OctCubeF;
typedef TOctBounds< double > OctCubeD;

template< class BOUNDS >
inline ATextStream & operator << ( ATextStream & log, const BOUNDS& bounds ) {
	log << "{x=" << bounds.x << ", y=" << bounds.y << ", z=" << bounds.z << ", r=" << bounds.radius << "}";
	return log;
}


template< class BOUNDS >
inline void OctBoundsToAABB( const BOUNDS& _node, AABB24 &_aabb )
{
	const Float3 center = { _node.x, _node.y, _node.z };
	const Float3 extent = Float3_Replicate( _node.radius );
	_aabb.min_point = center - extent;
	_aabb.max_point = center + extent;
}

// calculates the center and radius of the child octant
// returns:
// x,y,z - cube's center
// radius - cube's half size
// NOTE: this is not recommended to use with floating-point bounds - precision is lost!
//
template< typename N >
inline TOctBounds<N> GetChildOctant( const TOctBounds<N>& _parent, int _octant )
{
	const N	childRadius = _parent.radius / 2;
	const TOctBounds<N>	childBounds = {
		_parent.x + ((_octant & CHILD_MASK_X) ? childRadius : -childRadius),
		_parent.y + ((_octant & CHILD_MASK_Y) ? childRadius : -childRadius),
		_parent.z + ((_octant & CHILD_MASK_Z) ? childRadius : -childRadius),
		childRadius
	};
	return childBounds;
}

// determines the center and extent of each child octant
template< typename N >
inline void GetChildOctants( const TOctBounds<N>& _parent, TOctBounds<N> (&_children)[8] )
{
	for( int i = 0; i < 8; i++ ) {
		_children[i] = GetChildOctant( _parent, i );
	}
}

#if 0
// returns:
// x,y,z - cube's corner
// radius - cube's side length
//
template< typename N >
inline TOctBounds<N> GetChildCorner( const TOctBounds<N>& _parent, int _octant )
{
	const TOctBounds<N>	childBounds = {
		_parent.x + ((_octant & CHILD_MASK_X) ? _parent.radius : 0),
		_parent.y + ((_octant & CHILD_MASK_Y) ? _parent.radius : 0),
		_parent.z + ((_octant & CHILD_MASK_Z) ? _parent.radius : 0),
		_parent.radius / 2
	};
	return childBounds;
}
#endif

struct OctaCube
{
	UINT32	x,y,z;	// cube's corner
	UINT32	size;	// cube's side length
public:
	const Float3 XYZ() const { return Float3_Set(x,y,z); }
};
inline ATextStream & operator << ( ATextStream & log, const OctaCube& bounds ) {
	log << "{x=" << bounds.x << ", y=" << bounds.y << ", z=" << bounds.z << ", size=" << bounds.size << "}";
	return log;
}

inline OctaCube GetChildCorner( const OctaCube& _parent, int _octant )
{
	const UINT32 childSize = _parent.size / 2;
	const OctaCube	childBounds = {
		_parent.x + ((_octant & CHILD_MASK_X) ? childSize : 0),
		_parent.y + ((_octant & CHILD_MASK_Y) ? childSize : 0),
		_parent.z + ((_octant & CHILD_MASK_Z) ? childSize : 0),
		childSize
	};
	return childBounds;
}
// splits the parent's bounds into 8 uniform cubes
inline void GetChildCorners( const OctaCube& _parent, OctaCube (&_children)[8] )
{
	for( int i = 0; i < 8; i++ ) {
		_children[i] = GetChildCorner( _parent, i );
	}
}

template< typename N >
void OCT_GetCorners( const TOctBounds<N>& _bounds, Float3 points[8] )
{
	for( int i = 0; i < 8; i++ )
	{
		points[i].x = _bounds.x + ((i & CHILD_MASK_X) ? _bounds.radius : -_bounds.radius);
		points[i].y = _bounds.y + ((i & CHILD_MASK_Y) ? _bounds.radius : -_bounds.radius);
		points[i].z = _bounds.z + ((i & CHILD_MASK_Z) ? _bounds.radius : -_bounds.radius);
	}
}

const UINT8* CUBE_GetEdgeIndices();

extern const UINT32 g_bitCounts8[256];

//inline OctBounds GetChildOctant2( const OctBounds& _parent, int _octant ) {
//	int			childRadius = _parent.radius / 2;
//	int			min[3], max[3];
//	min[0] = _parent.x - childRadius;
//	min[1] = _parent.y - childRadius;
//	min[2] = _parent.z - childRadius;
//	max[0] = _parent.x + childRadius;
//	max[1] = _parent.y + childRadius;
//	max[2] = _parent.z + childRadius;
//	OctBounds	childBounds = {
//		(_octant & CHILD_MASK_X) ? max[0] : min[0],
//		(_octant & CHILD_MASK_Y) ? max[1] : min[1],
//		(_octant & CHILD_MASK_Z) ? max[2] : min[2],
//		childRadius
//	};
//	return childBounds;
//}

//enum ENodeType {
//	Empty,		// empty space
//	Solid,		// solid volume
//	Boundary,	// surface, 'grey' leaf
//	Interior,	// internal node with 8 children
//};

struct FeatureVertex : public CStruct
{
	Float3 xyz;
	Float3 N;
public:
	mxDECLARE_CLASS(FeatureVertex, CStruct);
	mxDECLARE_REFLECTION;
	FeatureVertex();
};

struct MeshOctreeNode : public CStruct
{
	TArray< FeatureVertex >	features;
	UINT16	kids[8];	// eight children of this node (empty if leaf)
public:
	mxDECLARE_CLASS(MeshOctreeNode, CStruct);
	mxDECLARE_REFLECTION;
	MeshOctreeNode();
	NO_ASSIGNMENT(MeshOctreeNode);
};
struct MeshOctree : public CStruct
{
	TArray< MeshOctreeNode >	m_nodes;
public:
	mxDECLARE_CLASS(MeshOctree, CStruct);
	mxDECLARE_REFLECTION;
	MeshOctree();
	PREVENT_COPY(MeshOctree);

	ERet Build( ATriangleMeshInterface* triangleMesh );
};

struct MeshSDF : AVolume
{
	TPtr< MeshOctree >	tree;
public:
	virtual float GetDistanceAt( const Float3& _position ) const override;
};

Float3 ClosestPointOnTriangle( const Float3 triangle[3], const Float3& sourcePosition );

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
