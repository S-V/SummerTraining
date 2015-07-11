//NOTE: 'Tc' stands for 'Toolchain'
#pragma once

#if MX_AUTOLINK
//#pragma comment( lib, "Meshok.lib" )
#endif //MX_AUTOLINK

#include <Base/Math/Math.h>
#include <Core/Asset.h>
#include <Graphics/Geometry.h>
#include <Renderer/Vertex.h>

class BitArray;

// most models have less than 80 bones, so even a 8-bit integer will suffice
typedef UINT16 BoneIndex;

enum EAnimType
{
	ANIMATION_NODE,
	ANIMATION_MORPHTARGET,
	ANIMATION_SKELETAL,
	ANIMATION_FACIAL,
};

enum EAnimMethod
{
	ANIM_ONESHOT,
	ANIM_ONELOOP,
	ANIM_LOOP,
	ANIM_PINGPONG,
	ANIM_PINGPONG_LOOP,
};

enum EAnimMode
{
	ANIM_FORWARDS   = 1,
	ANIM_STOP       = 0,
	ANIM_BACKWARDS  = -1
};

enum EAnimBehaviour
{
	/** The value from the default node transformation is taken*/
	AnimBehaviour_DEFAULT  = 0x0,  

	/** The nearest key value is used without interpolation */
	AnimBehaviour_CONSTANT = 0x1,

	/** The value of the nearest two keys is linearly
	*  extrapolated for the current time value.*/
	AnimBehaviour_LINEAR   = 0x2,

	/** The animation is repeated.
	*
	*  If the animation key go from n to m and the current
	*  time is t, use the value at (t-n) % (|m-n|).*/
	AnimBehaviour_REPEAT   = 0x3	
};


/*
=======================================================================
	GENERIC MESH FORMAT (FAT & SLOW - FOR TOOLS ONLY)
=======================================================================
*/

/*
-----------------------------------------------------------------------------
	BoneWeight
-----------------------------------------------------------------------------
*/
struct TcWeight : public CStruct
{
	// Index of bone influencing the vertex.
	int		boneIndex;
	// The strength of the influence in the range (0...1).
	// The influence from all bones at one vertex amounts to 1.
	float	boneWeight;
public:
	mxDECLARE_CLASS( TcWeight, CStruct );
	mxDECLARE_REFLECTION;
	TcWeight();
};

typedef TStaticList< TcWeight, MAX_INFLUENCES >	TcWeights;

/*
-----------------------------------------------------------------------------
	MeshBone (aka Joint) - The bone structure as seen by the toolchain.
	Represents one bone in the skeleton of a mesh.
	All bones (with the exception of the root) are oriented with respect to their parent.
		You multiply each bone's matrix by it's parent's matrix
	(which has been multiplied by it's parent, etc., all the way back to the root)
	to calculate the bone's absolute transform (in mesh-object space).
		Skin offset matrices are applied to the bone's combined transform
	to (eventually) transform the mesh vertices from mesh space to bone space.
-----------------------------------------------------------------------------
*/
struct TcBone : public CStruct
{
	String		name;			// The name of the bone from the toolchain
	int			parent;			// array index of parent bone; -1 means there is no parent
	Float4		rotation;		// The joint's rotation relative to the parent joint
	Float3		translation;	// The joint's position relative to the parent joint
public:
	mxDECLARE_CLASS( TcBone, CStruct );
	mxDECLARE_REFLECTION;
	TcBone();
};

// Skeleton/Armature/JointTree
// which can be deformed by an animation clip
struct TcSkeleton : public CStruct
{
	TArray< TcBone >	bones;	// the first bone is always the root
public:
	mxDECLARE_CLASS( TcSkeleton, CStruct );
	mxDECLARE_REFLECTION;
	int FindBoneIndexByName( const char* boneName ) const;
};

/*
-----------------------------------------------------------------------------
	Submesh (aka Mesh Subset, Mesh Part, Vertex-Index Range)
	represents a part of the mesh
	which is typically associated with a single material
-----------------------------------------------------------------------------
*/
struct TcTriMesh : public CStruct
{
	String			name;	// optional

	// vertex data in SoA layout
	TArray< Float3 >		positions;	// always present
	TArray< Float2 >		texCoords;
	TArray< Float3 >		tangents;
	TArray< Float3 >		binormals;	// 'bitangents'
	TArray< Float3 >		normals;
	TArray< Float4 >		colors;		// vertex colors (RGBA)
	TArray< TcWeights >		weights;

	// index data - triangle list
	TArray< UINT32 >	indices;	// always 32-bit indices

	AABB24		aabb;	// local-space bounding box
	//Sphere	sphere;	// local-space bounding sphere

	AssetID		material;	// default material

public:
	mxDECLARE_CLASS( TcTriMesh, CStruct );
	mxDECLARE_REFLECTION;
	TcTriMesh();
	UINT32 NumVertices() const { return positions.Num(); }
	UINT32 NumIndices() const { return indices.Num(); }
};

// Key - A set of data points defining the position and rotation of an individual bone at a specific time.

// A time-value pair specifying a certain 3D vector for the given time.
struct TcVecKey : public CStruct {
	Float3	data;	// holds the data for this key frame
	float	time;	// time stamp
public:
	mxDECLARE_CLASS( TcVecKey, CStruct );
	mxDECLARE_REFLECTION;
};
// A time-value pair specifying a rotation for the given time. 
struct TcQuatKey : public CStruct {
	Float4	data;	// holds the data for this key frame
	float	time;
public:
	mxDECLARE_CLASS( TcQuatKey, CStruct );
	mxDECLARE_REFLECTION;
};

/*
-----------------------------------------------------------------------------
	AnimationChannel/AnimTrack/BoneTrack/AnimCurve
	It is a set of keys describing the motion an individual bone over time.
	Stores frame time stamp and value for each component of the transformation.
-----------------------------------------------------------------------------
*/
struct TcAnimChannel : public CStruct
{
	String					target;			// e.g. name of the mesh bone
	TArray< TcVecKey >		positionKeys;	// translation key frames
	TArray< TcQuatKey >		rotationKeys;	// rotation key frames
	TArray< TcVecKey >		scalingKeys;	// scaling key frames
public:
	mxDECLARE_CLASS( TcAnimChannel, CStruct );
	mxDECLARE_REFLECTION;
};

// Animation Clip/Track/Stack(in FBX terminology)
// AnimSequence in Unreal Engine parlance:
// A set of Tracks defining the motion of all the bones making up an entire skeleton
// (e.g. a run cycle, attack)
struct TcAnimation : public CStruct
{
	String	name;
	TArray< TcAnimChannel >	channels;
	float	duration;	// Duration of the animation in seconds.
	int		numFrames;	// Number of frames.
public:
	mxDECLARE_CLASS( TcAnimation, CStruct );
	mxDECLARE_REFLECTION;
	TcAnimation();
	const TcAnimChannel* FindChannelByName( const char* name ) const;
};

/*
-----------------------------------------------------------------------------
	MeshData
	MeshData is a useful interchange format,
	which is usually created by an asset importer.

	It's mainly used for storing asset data in intermediate format,
	it cannot be directly loaded by the engine.

	The intermediate format is either a standard format like COLLADA
	or a custom defined format that is easy for our tools to read.
	This file is exported from the content creation tool
	and stores all of the information we could possibly desire
	about the asset, both now and in the future.

	Since the intermediate format is designed to be general and easy to read,
	this intermediate format isn't optimized for space or fast loading in-game.
	To create the game-ready format, we compile the intermediate format
	into a game format.
-----------------------------------------------------------------------------
*/
struct TcMeshData : public CStruct
{
	// parts of sets typically associated with a single material
	TArray< TcTriMesh >	sets;

	// local-space bounding box of the mesh (in bind pose)
	AABB24				bounds;

	// skinning data for skeletal animation
	TcSkeleton			skeleton;

	// A set of related animations which all play on the this skeleton.
	TArray< TcAnimation >	animations;

public:
	mxDECLARE_CLASS( TcMeshData, CStruct );
	mxDECLARE_REFLECTION;
	TcMeshData();
};

void CalculateTotalVertexIndexCount( const TcMeshData& data, UINT &vertexCount, UINT &indexCount );

#if 0
struct Tc2Mesh : public CStruct
{
	// parts of sets typically associated with a single material
	TArray< TcTriMesh >	meshes;

	// local-space bounding box of the mesh (in bind pose)
	Float3				aabbMin;
	Float3				aabbMax;

	// skinning data for skeletal animation
	TcSkeleton			skeleton;

	// A set of related animations which all play on the this skeleton.
	TArray< TcAnimation >	animations;

public:
	mxDECLARE_CLASS(TcMeshData,CStruct);
	mxDECLARE_REFLECTION;
	Tc2Mesh();
};

struct Tc2SkinBinding : public CStruct
{
	int			meshIndex;

public:
	mxDECLARE_CLASS(Tc2SkinBinding,CStruct);
	mxDECLARE_REFLECTION;
	Tc2SkinBinding();
};


struct Tc2Scene : public CStruct
{
	// parts of meshes typically associated with a single material
	TArray< Tc2Mesh >	meshes;

	// local-space bounding box of the mesh (in bind pose)
	Float3			aabbMin;
	Float3			aabbMax;

	// A set of related animations which all play on the this skeleton.
	TArray< TcAnimation >	animations;

public:
	mxDECLARE_CLASS(Tc2Scene,CStruct);
	mxDECLARE_REFLECTION;
	Tc2Scene();
};

#endif



namespace Meshok
{

ERet CompileMesh( const TcMeshData& src, const VertexDescription& vertex, RawMeshData &dst );

UINT32 EulerNumber( UINT32 V, UINT32 E, UINT32 F, UINT32 H = 0 );
bool EulerTest( UINT32 V, UINT32 E, UINT32 F, UINT32 H, UINT32 C, UINT32 G );

// _voxels - bit array (_gridSize^3 bits)
ERet VoxelizeSphere( const float _radius, const UINT32 _gridSize, void *_voxels );

ERet VoxelizeMesh( const TcMeshData& _source,
				  const UINT32 _sizeX, const UINT32 _sizeY, const UINT32 _sizeZ,
				  void *_volume
				  );

}//namespace Meshok

struct ATriangleIndexCallback
{
	struct Vertex {
		Float3 xyz;
		Float2 st;
	};
	virtual void ProcessTriangle( const Vertex& a, const Vertex& b, const Vertex& c ) = 0;
	virtual ~ATriangleIndexCallback() {}
};

struct ATriangleMeshInterface : DbgNamedObject<>
{
	virtual void ProcessAllTriangles( ATriangleIndexCallback* callback ) = 0;
	virtual ~ATriangleMeshInterface() {}
};

template< typename INDEX_TYPE >
void ProcessAllTriangles(
						   const INDEX_TYPE* indices, const UINT numTriangles,
						   const Float3* positions, const UINT numVertices,
						   ATriangleIndexCallback* callback
						   )
{
	mxSTATIC_ASSERT( sizeof(indices[0]) == 2 || sizeof(indices[0]) == 4 );

	for( UINT iTriangle = 0; iTriangle < numTriangles; iTriangle++ )
	{
		const UINT	idx0 = indices[ iTriangle*3 + 0 ];
		const UINT	idx1 = indices[ iTriangle*3 + 1 ];
		const UINT	idx2 = indices[ iTriangle*3 + 2 ];

		callback->ProcessTriangle(
			positions[idx0],
			positions[idx1],
			positions[idx2]
		);
	}
}

struct ProcessMeshDataTriangles : public ATriangleMeshInterface
{
	const TcMeshData &	m_meshData;
public:
	ProcessMeshDataTriangles( const TcMeshData& meshData )
		: m_meshData( meshData )
	{}
	virtual void ProcessAllTriangles( ATriangleIndexCallback* callback ) override
	{
		for( int meshIndex = 0; meshIndex < m_meshData.sets.Num(); meshIndex++ )
		{
			const TcTriMesh& mesh = m_meshData.sets[ meshIndex ];
			const UINT32* indices = mesh.indices.ToPtr();
			const UINT32 numIndices = mesh.indices.Num();
			const UINT32 numTriangles = numIndices / 3;
			for( UINT32 i = 0; i < numTriangles; i++ )
			{
				const UINT32* tri = indices + i*3;
				ATriangleIndexCallback::Vertex a;
				ATriangleIndexCallback::Vertex b;
				ATriangleIndexCallback::Vertex c;
				a.xyz = mesh.positions[tri[0]];
				b.xyz = mesh.positions[tri[1]];
				c.xyz = mesh.positions[tri[2]];
				a.st = mesh.texCoords[tri[0]];
				b.st = mesh.texCoords[tri[1]];
				c.st = mesh.texCoords[tri[2]];
				callback->ProcessTriangle( a, b, c );
			}
		}
	}
};


struct AMeshBuilder
{
	virtual ERet Begin() = 0;
	virtual int AddVertex( const DrawVertex& vertex ) = 0;
	virtual int AddTriangle( int v1, int v2, int v3 ) = 0;
	virtual ERet End( int &verts, int &tris ) = 0;

protected:
	virtual ~AMeshBuilder() {}
};

struct MeshBuilder : AMeshBuilder
{
	TArray< DrawVertex >	vertices;
	TArray< UINT16 >		indices;

	virtual ERet Begin() override;
	virtual int AddVertex( const DrawVertex& vertex ) override;
	virtual int AddTriangle( int v1, int v2, int v3 ) override;
	virtual ERet End( int &verts, int &tris ) override;
};



struct FatVertex1
{
	Float3	position;
	Float2	texCoord;
	Float3	normal;
	Float3	tangent;
};
struct FatVertex2
{
	Float3	position;
	Float2	texCoord;
	Float3	normal;
	Float3	tangent;
	Float3	binormal;	// 'bitangent' - orthogonal to both the normal and the tangent, B = T x N
};



ERet CreateBox(
			   float width, float height, float depth,
			   TArray<FatVertex1> &_vertices, TArray<int> &_indices
			   );

ERet CreateSphere(
				  float radius, int sliceCount, int stackCount,
				  TArray<FatVertex1> &_vertices, TArray<int> &_indices
				  );

ERet CreateGeodesicSphere(
						  float radius, int numSubdivisions, //[0..8]
						  TArray<FatVertex1> &_vertices, TArray<int> &_indices
						  );




//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
