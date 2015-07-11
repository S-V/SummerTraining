//@todo: rename this file to "grAssets.h"?
#pragma once

//@todo: remove this (but mesh structs still need math types)
#include <Base/Math/Math.h>
#include <Core/VectorMath.h>
#include <Graphics/Device.h>

//	Vertex format tells us how to interpret raw bytes of vertex data
//	when loading models and matching shader inputs before binding.
//	these enum values will be mapped to platform-specific vertex formats
struct VertexType {
	enum Enum {
		Static,	// vertex format used for rendering most of static meshes
		Skinned,// for drawing skeletally-animated meshes
		Generic,// universal vertex type

		Count,	//<= Marker. Don't use!
		NumBits = 2	// should be > log2(Count)
	};
};
mxDECLARE_ENUM( VertexType::Enum, UINT8, VertexTypeT );


// enumeration of scene passes (hardcoded into the engine)
// NOTE: the order is important (used for batch sorting)
struct ScenePass {
	enum Enum {
		Unknown,
		FillBuffers,
		Translucent,
		Shadow,
		Fallback,

		Count	//<= Marker. Don't use!
	};
};
mxDECLARE_ENUM( ScenePass::Enum, UINT8, ScenePassT );

enum EAnimationConstants
{
	// Maximum number of bones in a mesh.
	MAX_MESH_BONES = 256,
	// Each skinned vertex has a maximum of 4 bone indices and 4 weights for each bone.
	MAX_INFLUENCES = 4
};

struct BoneWeight
{
	int	index;
	float weight;
};

/*
=======================================================================
	RUN-TIME MESH FORMAT (LEAN & SLIM)
=======================================================================
*/
struct RawVertexStream : public CStruct
{
	TBuffer< BYTE, UINT32 >	data;	// this is how raw vertex data is stored on disk
public:
	mxDECLARE_CLASS( RawVertexStream, CStruct );
	mxDECLARE_REFLECTION;
	RawVertexStream() {}
	inline UINT SizeInBytes() const { return data.GetDataSize(); }
	inline const BYTE* ToVoidPtr() const { return this->data.ToPtr(); }
};
struct RawVertexData : public CStruct
{
	TBuffer< RawVertexStream >	streams;// vertex streams
	UINT32						count;	// number of vertices
	VertexTypeT					type;	// VertexTypeT - tells us how to interpret raw bytes of vertex data
public:
	mxDECLARE_CLASS( RawVertexData, CStruct );
	mxDECLARE_REFLECTION;
	RawVertexData()
		: count(0), type(VertexType::Static)
	{}
};
// used for storing index data on disk
struct RawIndexData : public CStruct
{
	TBuffer< BYTE, UINT32 >	data;
	UINT16					stride;	// 2 or 4 bytes
public:
	mxDECLARE_CLASS( RawIndexData, CStruct );
	mxDECLARE_REFLECTION;
	RawIndexData()
		: stride(0)
	{}
	// Returns the number of indices in the buffer.
	inline UINT NumIndices() const {return data.GetDataSize() / stride;}
	inline UINT SizeInBytes() const	{return data.GetDataSize();}
	inline void* ToVoidPtr() {return data.ToPtr();}
	inline const void* ToVoidPtr() const {return data.ToPtr();}
};
// part of mesh typically associated with a single material
struct RawMeshPart : public CStruct
{
	UINT32		baseVertex;		// index of the first vertex
	UINT32		startIndex;		// offset of the first index
	UINT32		indexCount;		// number of indices
	UINT32		vertexCount;	// number of vertices
public:
	mxDECLARE_CLASS( RawMeshPart, CStruct );
	mxDECLARE_REFLECTION;
};
// bind-pose skeleton's joint
struct Bone : public CStruct
{
	Float4		orientation;	//16 joint's orientation quaternion
	Float3		position;		//12 joint's position in object space
	INT32		parent;			//4 index of the parent joint; (-1) if this is the root joint
public:
	mxDECLARE_CLASS( Bone, CStruct );
	mxDECLARE_REFLECTION;
	Bone();
};
// Skeleton/Armature/JointTree
// Describes the hierarchy of the bones and the skeleton's binding pose.
// and can be deformed by an animation clip.
struct Skeleton : public CStruct
{
	TBuffer< Bone >			bones;			// bind-pose bones data (in object space)
	TBuffer< String >		boneNames;
	TBuffer< Float3x4 >		invBindPoses;	// inverse bind pose matrices
public:
	mxDECLARE_CLASS( Skeleton, CStruct );
	mxDECLARE_REFLECTION;
	Skeleton();
};
/*
-----------------------------------------------------------------------------
Raw mesh data as it is usually stored on disk
and is used for loading/filling hardware mesh buffers
-----------------------------------------------------------------------------
*/
struct RawMeshData : public CStruct
{
	RawVertexData			vertexData;	// raw vertex data
	RawIndexData			indexData;	// raw index data
	TopologyT				topology;	// primitive type
	Skeleton				skeleton;
	AABB24					bounds;		// local-space bounding box
	TBuffer< RawMeshPart >	parts;
public:
	mxDECLARE_CLASS( RawMeshData, CStruct );
	mxDECLARE_REFLECTION;
	RawMeshData();
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
