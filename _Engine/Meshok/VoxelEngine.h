// NOTE: voxel regions must be a power of two in size!

#pragma once

#include <Meshok/BSP.h>
#include <Meshok/Octree.h>

//struct Vertex
//{
//	Float3	P;	// 12
//	Float3	N;	// 12
//	Float2	UV;	// 8	
//};
//
//struct Mesh
//{
//	TArray< Vertex >	vertices;
//	TArray< UINT32 >	indices;	// triangle list
//};

enum ENodeType
{
	Node_Empty = 0,	// represents empty space, air, 'white' node
	Node_Solid = 1,	// represents solid space, rock, 'black' node
	// 'grey' nodes:
	Node_Convex,	// convex polytope, polygonal mesh
	Node_Volume,	// RLE-compressed voxel data
};

// _voxels - bit array (_gridSize^3 bits)
//void Polygonize( const UINT32 _gridSize, void *_voxels, Mesh &_mesh );

struct AVoxelData {
	// returns packed normal and material index
	virtual UByte4 GetVoxel( int x, int y, int z ) = 0;
	virtual bool CubeContainsVoxels( int x, int y, int z, int size ) = 0;
};

/*
Voxel octree:
each node is 4 bytes:
lower 8 bits - child mask.
*/

/*
	Sparse voxel octree.
	Empty nodes are not stored at all.
	Each node contains:
		voxel data (or a reference to an extended data);
		offset of the first child, relative to this node;
		8-bit child bitmask.
*/
class VoxelOctree
{
public:
	VoxelOctree();

	// create an octree from the density source
	void Build( const AVolume* _volume, int radius );

	TArray< UINT32 >	m_nodes;

private:
	void BuildOctreeRecursive( int nodeX, int nodeY, int nodeZ, int size, const AVolume* _volume, int nodeID );
};

class Octree2
{
public:
	Octree2();

	// create an octree from the density source
	void Build( const AVolume* _volume, int radius );
#pragma pack (push,1)
	struct Node
	{
		union {
			UINT32	u0;
			struct {
				INT8	x;
				INT8	y;
				INT8	z;
				UINT8	childmask;
			};
		};

		UINT16	children;
		UINT8	_unused0[2];

		UByte4	N;

		UINT8	_unused1[4];

		//union {
		//	UINT32	u0;
		//	struct {
		//		INT8	x;
		//		INT8	y;
		//		INT8	z;
		//		UINT8	childmask;
		//	};
		//};

		//union {
		//	UINT32	u1;
		//	struct {
		//		UINT16	U;
		//		UINT16	V;
		//	};
		//};

		//union {
		//	UINT64	u2;
		//	struct {
		//		UINT32	N : 24;
		//		UINT32	T : 24;
		//		UINT16	children;
		//	};
		//};

		//UINT32	N : 24;
		//UINT32	T : 24;
		//UINT16	children;

		//UINT16	U;
		//UINT16	V;

		//INT8	x;
		//INT8	y;
		//INT8	z;
		//UINT8	childmask;


		//BITFIELD	children	: 16;
		//BITFIELD	childmask	: 8;
		//BITFIELD	x			: 8;

		//BITFIELD	U	: 16;
		//BITFIELD	V	: 16;

		//BITFIELD	N	: 24;
		//BITFIELD	T	: 24;
		//BITFIELD	y			: 8;
		//BITFIELD	z			: 8;

		//UINT16	children;
		//UINT8	childmask;
		//INT8	x;

		//UINT16	U;
		//UINT16	V;

		//UINT32	N : 24;
		//UINT32	T : 24;
		//INT8	y;
		//INT8	z;
	};
#pragma pack (pop)

	mxSTATIC_ASSERT(sizeof(Node)==16);

	TArray< Node >	m_nodes;

private:
	void BuildOctreeRecursive( int nodeX, int nodeY, int nodeZ, int size, const AVolume* _volume, int nodeID );
};


/*
	Sparse voxel octree.
	Empty nodes are not stored at all.
	Each node contains:
		voxel data (or a reference to an extended data);
		offset of the first child, relative to this node;
		8-bit child bitmask.
*/
class Octree3
{
public:
	Octree3();

	// create an octree from the density source
	void Build( const AVolume* _volume, int radius );

	TArray< UINT32 >	m_nodes;

	// surface control points are defined only for boundary nodes
	struct Vertex {
		UINT8	x, y, z;
		UINT8	_pad0;
		UINT8	Nx, Ny, Nz;
		UINT8	_pad1;
	};
	TArray< Vertex >	m_controlPoints;

private:
	void BuildOctreeRecursive( int nodeX, int nodeY, int nodeZ, int size, const AVolume* _volume, int nodeID );
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
