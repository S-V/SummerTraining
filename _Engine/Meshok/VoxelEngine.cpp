//
#include "stdafx.h"
#pragma hdrstop
#include <Meshok/Meshok.h>
#include <Meshok/VoxelEngine.h>

// Dual contouring of Hermite Data

inline int SampleGrid()
{
	return 0;
}

//void Polygonize( const UINT32 _gridSize, void *_voxels, Mesh &_mesh )
//{
//
//}

// Implementation copied from:
// https://github.com/tunabrain/sparse-voxel-octrees/blob/master/src/VoxelOctree.cpp

static const int s_BitCount[256] = {
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
#if 0

VoxelOctree::VoxelOctree()
{

}

// recursively builds octree from top to bottom
void VoxelOctree::Build( const AVolume* _volume, int radius )
{
	m_nodes.Empty();
	m_nodes.Reserve(256);
	// add the root node
	m_nodes.Add();
	this->BuildOctreeRecursive( 0,0,0, radius, _volume, 0 );
	DBGOUT("Built octree: %u nodes (%u reserved) (%u bytes)\n", m_nodes.Num(), m_nodes.Capacity(), m_nodes.Num()*sizeof(m_nodes[0]));
}

bool NeedsSubdivision( int parentX, int parentY, int parentZ, int size, const AVolume* _volume )
{
	float error = 0.0f;
	float centerDensity = _volume->SampleAt( parentX, parentY, parentZ ).density;
	int halfSize = size / 2;
	int signMask = 0;
	// estimate density in all 8 corners of the parent cube
	for( int i = 0; i < 8; i++ ) {
		int cornerX = parentX + ((i & CHILD_MASK_X) ? halfSize : -halfSize);
		int cornerY = parentY + ((i & CHILD_MASK_Y) ? halfSize : -halfSize);
		int cornerZ = parentZ + ((i & CHILD_MASK_Z) ? halfSize : -halfSize);
		float cornerDensity = _volume->SampleAt( cornerX, cornerY, cornerZ ).density;
		if( cornerDensity <= 0.0f ) {
			signMask |= (1<<i);
		}
		error += Square( cornerDensity - centerDensity );
	}
	if( !signMask ) {
		return false;
	}
	error *= 1.0f / 8.0f;
	float threshhold = 0.1f;
	return error > threshhold;
}

int GetSignMask( int nodeX, int nodeY, int nodeZ, int nodeRadius, const AVolume* _volume )
{
	int signMask = 0;
	for( int i = 0; i < 8; i++ ) {
		int cornerX = nodeX + ((i & CHILD_MASK_X) ? nodeRadius : -nodeRadius);
		int cornerY = nodeY + ((i & CHILD_MASK_Y) ? nodeRadius : -nodeRadius);
		int cornerZ = nodeZ + ((i & CHILD_MASK_Z) ? nodeRadius : -nodeRadius);
		float cornerDensity = _volume->SampleAt( cornerX, cornerY, cornerZ ).density;
		if( cornerDensity <= 0.0f ) {
			signMask |= (1<<i);
		}
	}
	return signMask;
}

// parentX, parentY, parentZ - center of the parent node
// parentSize - size of the parent node (side length of the cube)
// parentIndex - absolute index of the parent node in the octree nodes array
void VoxelOctree::BuildOctreeRecursive( int parentX, int parentY, int parentZ, int parentRadius, const AVolume* _volume, int parentIndex )
{
	if(m_nodes.Num() > 64*64*64)return;

	// determine the center and extent of each child octant
	const OctCubeI parent = { parentX, parentY, parentZ, parentRadius };
	OctCubeI childOctants[8];
	for( int i = 0; i < 8; i++ ) {
		childOctants[i] = GetChildOctant( parent, i );
	}

	// relative offset of the array of children of this node
	int childrenOffset = m_nodes.Num() - parentIndex;

	// figure out which octants need further subdividing
	int childMask = 0;
	for( int i = 0; i < 8; i++ ) {
		const OctCubeI child = childOctants[i];
		if( NeedsSubdivision(child.x, child.y, child.z, child.radius, _volume)) {
			childMask |= (1 << i);
		}
	}

	int signMask = GetSignMask( parentX, parentY, parentZ, parentRadius, _volume );

	if(!childMask)
	{
		//DBGOUT("Node[%u]: childMask==0 (%d,%d,%d,%d)\n", parentIndex, parentX, parentY, parentZ, parentSize);

		//float density = _volume->SampleAt( parentX, parentY, parentZ );
		//bool isSolid = (density < 0.0f);
		//if( isSolid )
		//{
		//	//const Float3 N = _volume->GetNormal(parentX, parentY, parentZ);
		//	//const UByte4 packed = { _NormalToUInt8(N.x), _NormalToUInt8(N.y), _NormalToUInt8(N.z), 0 };
		//	//m_nodes[ parentIndex ] = (packed.v << 8);
		//	m_nodes[ parentIndex ] = 0;
		//}

		bool isBlackLeaf = (signMask == 0xFF);
		if(isBlackLeaf){
			m_nodes[ parentIndex ] = 0;
			return;
		}

		bool isWhiteLeaf = (signMask == 0x00);
		if(isWhiteLeaf){
			// empty nodes are not stored
			return;
		}

		// this is a gray leaf
		const Float3 N = _volume->SampleAt(parentX, parentY, parentZ).normal;
		const UByte4 packed = { _NormalToUInt8(N.x), _NormalToUInt8(N.y), _NormalToUInt8(N.z), 0 };
		m_nodes[ parentIndex ] = (packed.v << 8);
		return;
	}

	const int numKids = s_BitCount[ childMask ];

	if( parentRadius == 1 ) {

		for( int i = 0; i < 8; i++ ) {
			if( childMask & (1<<i) ) {
				const OctCubeI child = childOctants[i];
				const Float3 N = _volume->SampleAt(child.x, child.y, child.z).normal;
				const UByte4 packed = { _NormalToUInt8(N.x), _NormalToUInt8(N.y), _NormalToUInt8(N.z), 0 };
				m_nodes.Add(packed.v);
			}
		}

	} else {
		UINT32 childIndex = m_nodes.Num();
		m_nodes.AddZeroed(numKids);

		for( int i = 0; i < 8; i++ ) {
			if( childMask & (1<<i) ) {
				const OctCubeI child = childOctants[i];
				BuildOctreeRecursive(child.x, child.y, child.z, child.radius, _volume, childIndex++);
			}
		}
	}

	mxASSERT(childMask);

	m_nodes[ parentIndex ] = (childrenOffset << 8) | childMask;

	//DBGOUT("Node[%u]: %u kids at %u (%d,%d,%d,%d)\n", parentIndex, numKids, childrenOffset, parentX, parentY, parentZ, parentSize);
}

Octree2::Octree2()
{

}

// recursively builds octree from top to bottom
void Octree2::Build( const AVolume* _volume, int radius )
{
	m_nodes.Empty();
	m_nodes.Reserve(256);
	// add the root node
	m_nodes.Add();
	this->BuildOctreeRecursive( 0,0,0, radius, _volume, 0 );
	DBGOUT("Built octree: %u nodes (%u reserved) (%u bytes)\n", m_nodes.Num(), m_nodes.Capacity(), m_nodes.Num()*sizeof(m_nodes[0]));
}

void Octree2::BuildOctreeRecursive( int parentX, int parentY, int parentZ, int parentRadius, const AVolume* _volume, int parentIndex )
{
	// determine the center and extent of each child octant
	const OctCubeI parent = { parentX, parentY, parentZ, parentRadius };
	OctCubeI childOctants[8];
	for( int i = 0; i < 8; i++ ) {
		childOctants[i] = GetChildOctant( parent, i );
	}

#if 0
	// relative offset of the array of children of this node
	int childrenOffset = m_nodes.Num() - parentIndex;

	// figure out which octants need further subdividing
	int childMask = 0;
	for( int i = 0; i < 8; i++ ) {
		const OctreeNode child = childOctants[i];
		if( NeedsSubdivision(child.x, child.y, child.z, child.w, _volume)) {
			childMask |= (1 << i);
		}
	}

	int signMask = GetSignMask( parentX, parentY, parentZ, parentRadius, _volume );

	if(!childMask)
	{
		//DBGOUT("Node[%u]: childMask==0 (%d,%d,%d,%d)\n", parentIndex, parentX, parentY, parentZ, parentSize);

		//float density = _volume->SampleAt( parentX, parentY, parentZ );
		//bool isSolid = (density < 0.0f);
		//if( isSolid )
		//{
		//	//const Float3 N = _volume->GetNormal(parentX, parentY, parentZ);
		//	//const UByte4 packed = { _NormalToUInt8(N.x), _NormalToUInt8(N.y), _NormalToUInt8(N.z), 0 };
		//	//m_nodes[ parentIndex ] = (packed.v << 8);
		//	m_nodes[ parentIndex ] = 0;
		//}

		bool isBlackLeaf = (signMask == 0xFF);
		if(isBlackLeaf){
			m_nodes[ parentIndex ] = 0;
			return;
		}

		bool isWhiteLeaf = (signMask == 0x00);
		if(isWhiteLeaf){
			// empty nodes are not stored
			UINT32 vv = m_nodes[ parentIndex ];
			return;
		}

		// this is a gray leaf
		const Float3 N = _volume->GetNormal(parentX, parentY, parentZ);
		const UByte4 packed = { _NormalToUInt8(N.x), _NormalToUInt8(N.y), _NormalToUInt8(N.z), 0 };
		m_nodes[ parentIndex ] = (packed.v << 8);
		return;
	}

	const int numKids = s_BitCount[ childMask ];

	if( parentRadius == 1 ) {

		for( int i = 0; i < 8; i++ ) {
			if( childMask & (1<<i) ) {
				const OctreeNode child = childOctants[i];
				const Float3 N = _volume->GetNormal(child.x, child.y, child.z);
				const UByte4 packed = { _NormalToUInt8(N.x), _NormalToUInt8(N.y), _NormalToUInt8(N.z), 0 };
				m_nodes.Add(packed.v);
			}
		}

	} else {
		UINT32 childIndex = m_nodes.Num();
		m_nodes.AddZeroed(numKids);

		for( int i = 0; i < 8; i++ ) {
			if( childMask & (1<<i) ) {
				const OctreeNode child = childOctants[i];
				BuildOctreeRecursive(child.x, child.y, child.z, child.w, _volume, childIndex++);
			}
		}
	}

	mxASSERT(childMask);

	m_nodes[ parentIndex ] = (childrenOffset << 8) | childMask;

	//DBGOUT("Node[%u]: %u kids at %u (%d,%d,%d,%d)\n", parentIndex, numKids, childrenOffset, parentX, parentY, parentZ, parentSize);
#endif
}

Octree3::Octree3()
{

}

// recursively builds octree from top to bottom
void Octree3::Build( const AVolume* _volume, int radius )
{
	m_nodes.Empty();
	m_nodes.Reserve(256);
	// add the root node
	m_nodes.Add();
	this->BuildOctreeRecursive( 0,0,0, radius, _volume, 0 );
	DBGOUT("Built octree: %u nodes (%u bytes) (%u reserved)\n",
		m_nodes.Num(), m_nodes.Num()*sizeof(m_nodes[0]), m_nodes.Capacity());
}

bool NeedsSubdivision3( int nodeX, int nodeY, int nodeZ, int nodeRadius, const AVolume* _volume )
{
	// determine the center and extent of each child octant
	const OctCubeI nodeBounds = { nodeX, nodeY, nodeZ, nodeRadius };

	OctCubeI childOctants[8];
	for( int i = 0; i < 8; i++ ) {
		childOctants[i] = GetChildOctant( nodeBounds, i );
	}

	float error = 0.0f;
	float centerDensity = _volume->SampleAt( nodeX, nodeY, nodeZ ).distance;
	int signMask = 0;
	// estimate density in all 8 corners of the parent cube
	for( int i = 0; i < 8; i++ ) {
		const OctCubeI& child = childOctants[i];
		float cornerDensity = _volume->SampleAt( child.x, child.y, child.z ).distance;
		if( cornerDensity <= 0.0f ) {
			signMask |= (1<<i);
		}
		error += Square( cornerDensity - centerDensity );
	}
	if( (signMask == 0) || (signMask == 0xFF) ) {
		return false;
	}
	error *= 1.0f / 8.0f;
	float threshhold = 0.1f;
	return error > threshhold;
}
// parentX, parentY, parentZ - center of the parent node
// parentSize - radius of the parent node (half size of the cube)
// parentIndex - absolute index of the parent node in the octree nodes array
void Octree3::BuildOctreeRecursive( int nodeX, int nodeY, int nodeZ, int nodeRadius, const AVolume* _volume, int parentIndex )
{
	mxASSERT(nodeRadius >= 1);
	#if 0
/*
In each octree level:
	tag nodes that need further division.
	Allocate buffer spaces for next level.
	Initialize the allocated buffer space.
*/

	const ASurface::Point nodeCenter = _volume->Sample( nodeX, nodeY, nodeZ ).density;
	const Float3 centerNormal = nodeCenter.normal;
	const float centerDensity = nodeCenter.density;

	if( nodeRadius == 1 )
	{
		//this is a leaf node - cannot recurse down any more
		bool isSolidLeaf = (centerDensity <= 0);
		if ( isSolidLeaf ) {
			UINT32 nodeValue = 0;
			m_nodes.Add(nodeValue);
			//m_nodes[ parentIndex ] = 
		}
		// empty space nodes are not stored
		return;
	}

	// determine the center and extent of each child octant
	const OctreeNode nodeBounds = { nodeX, nodeY, nodeZ, nodeRadius };

	OctreeNode childOctants[8];
	for( int i = 0; i < 8; i++ ) {
		childOctants[i] = GetChildOctant( nodeBounds, i );
	}

	int signMask = 0;

	float error = 0.0f;
	for( int i = 0; i < 8; i++ ) {
		const OctreeNode& child = childOctants[i];
		float cornerDensity = _volume->Sample( child.x, child.y, child.z ).density;
		if( cornerDensity <= 0.0f ) {
			signMask |= (1<<i);
		}
		error += Square( cornerDensity - centerDensity );
	}

	const float threshhold = 0.1f;

	bool needToSubdivide = (error >= threshhold);

	// check if this is an empty space
	if( signMask == 0 && !needToSubdivide )
	{
		// empty nodes are not stored
		return;
	}

	// check if this is a solid space
	if( signMask == 0xFF && !needToSubdivide )
	{
		// no need to subdivide any further
		UINT32 nodeValue = 0;
		m_nodes.Add(nodeValue);
		return;
	}

	if( needToSubdivide )
	{
		// figure out which child octants need further subdividing
		int childMask = 0;
		for( int i = 0; i < 8; i++ ) {
			const OctreeNode child = childOctants[i];
			if( NeedsSubdivision( child.x, child.y, child.z, child.w, _volume ) ) {
				childMask |= (1 << i);
			}
		}

		const int numChildren = s_BitCount[ childMask ];

		// Allocate buffer spaces for next level.
		m_nodes.AddZeroed( numChildren );

		// The base address of the children of this node.
		UINT32 childIndex = m_nodes.Num();

		for( int i = 0; i < 8; i++ ) {
			if( childMask & (1<<i) ) {
				const OctreeNode child = childOctants[i];
				BuildOctreeRecursive(child.x, child.y, child.z, child.w, _volume, childIndex++);
			}
		}

		m_nodes[ parentIndex ] = (childrenOffset << 8) | childMask;
	}

//	bool isLeaf = NeedsSubdivision( parentX, parentY, parentZ, child.w, _volume );
#endif
}
#endif

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
