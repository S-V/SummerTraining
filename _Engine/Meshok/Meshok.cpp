#include "stdafx.h"
#pragma hdrstop
#include <Base/Template/Containers/BitSet/BitArray.h>
#include <Meshok/Meshok.h>

/*
-----------------------------------------------------------------------------
	TcWeight
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS( TcWeight );
mxBEGIN_REFLECTION( TcWeight )
	mxMEMBER_FIELD( boneIndex ),
	mxMEMBER_FIELD( boneWeight ),
mxEND_REFLECTION
TcWeight::TcWeight()
{
	boneIndex = 0;
	boneWeight = 0.0f;
}

/*
-----------------------------------------------------------------------------
	MeshBone
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS( TcBone );
mxBEGIN_REFLECTION( TcBone )
	mxMEMBER_FIELD( name ),
	mxMEMBER_FIELD( parent ),
	mxMEMBER_FIELD( rotation ),
	mxMEMBER_FIELD( translation ),
mxEND_REFLECTION
TcBone::TcBone()
{
	parent = -1;
	rotation = Quaternion_Identity();
	translation = Float3_Zero();
}

mxDEFINE_CLASS( TcSkeleton );
mxBEGIN_REFLECTION( TcSkeleton )
	mxMEMBER_FIELD( bones ),
mxEND_REFLECTION
int TcSkeleton::FindBoneIndexByName( const char* boneName ) const
{
	for( UINT i = 0; i < bones.Num(); i++ ) {
		if( Str::EqualS(bones[i].name, boneName) ) {
			return i;
		}
	}
	return -1;
}

/*
-----------------------------------------------------------------------------
	MeshPart
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS( TcTriMesh );
mxBEGIN_REFLECTION( TcTriMesh )
	mxMEMBER_FIELD( name ),
	mxMEMBER_FIELD( positions ),
	mxMEMBER_FIELD( texCoords ),
	mxMEMBER_FIELD( tangents ),
	mxMEMBER_FIELD( binormals ),
	mxMEMBER_FIELD( normals ),
	mxMEMBER_FIELD( colors ),
	mxMEMBER_FIELD( weights ),
	mxMEMBER_FIELD( indices ),
	mxMEMBER_FIELD( aabb ),
	//mxMEMBER_FIELD( sphere ),
	mxMEMBER_FIELD( material ),
mxEND_REFLECTION
TcTriMesh::TcTriMesh()
{
}

mxDEFINE_CLASS( TcVecKey );
mxBEGIN_REFLECTION( TcVecKey )
	mxMEMBER_FIELD( data ),
	mxMEMBER_FIELD( time ),
mxEND_REFLECTION

mxDEFINE_CLASS( TcQuatKey );
mxBEGIN_REFLECTION( TcQuatKey )
	mxMEMBER_FIELD( data ),
	mxMEMBER_FIELD( time ),
mxEND_REFLECTION

mxDEFINE_CLASS( TcAnimChannel );
mxBEGIN_REFLECTION( TcAnimChannel )
	mxMEMBER_FIELD( target ),
	mxMEMBER_FIELD( positionKeys ),
	mxMEMBER_FIELD( rotationKeys ),
	mxMEMBER_FIELD( scalingKeys ),
mxEND_REFLECTION

mxDEFINE_CLASS( TcAnimation );
mxBEGIN_REFLECTION( TcAnimation )
	mxMEMBER_FIELD( name ),
	mxMEMBER_FIELD( channels ),
	mxMEMBER_FIELD( duration ),
	mxMEMBER_FIELD( numFrames ),
mxEND_REFLECTION
TcAnimation::TcAnimation()
{
	duration = 0.0f;
	numFrames = 0;
}
const TcAnimChannel* TcAnimation::FindChannelByName( const char* name ) const
{
	for( UINT i = 0; i < channels.Num(); i++ ) {
		if(Str::EqualS(channels[i].target, name)) {
			return &channels[i];
		}
	}
	return nil;
}

/*
-----------------------------------------------------------------------------
	MeshData
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS( TcMeshData );
mxBEGIN_REFLECTION( TcMeshData )
	mxMEMBER_FIELD( sets ),
	mxMEMBER_FIELD( bounds ),
	mxMEMBER_FIELD( skeleton ),
	mxMEMBER_FIELD( animations ),
mxEND_REFLECTION

TcMeshData::TcMeshData()
{
	AABB24_Clear( &bounds );
}

void CalculateTotalVertexIndexCount( const TcMeshData& data, UINT &vertexCount, UINT &indexCount )
{
	vertexCount = 0;
	indexCount = 0;
	for( UINT iSubMesh = 0; iSubMesh < data.sets.Num(); iSubMesh++ )
	{
		const TcTriMesh& submesh = data.sets[ iSubMesh ];
		vertexCount += submesh.NumVertices();
		indexCount += submesh.NumIndices();
	}
}

namespace Meshok
{

// keeps attribute indices into VertexDescription::attribsArray
struct VertexStream
{
	TStaticList< UINT8, LLGL_MAX_VERTEX_ATTRIBS >	components;
};

static UINT GatherStreams( const VertexDescription& vertex, TArray< VertexStream > &streams )
{
	UINT numVertexStreams = 0;

	for( UINT attribIndex = 0; attribIndex < vertex.attribCount; attribIndex++ )
	{
		const VertexElement& attrib = vertex.attribsArray[ attribIndex ];

		numVertexStreams = largest( numVertexStreams, attrib.inputSlot + 1 );

		streams.SetNum(numVertexStreams);
		VertexStream &stream = streams[ attrib.inputSlot ];

		stream.components.Add( attribIndex );
	}

	return numVertexStreams;
}

// Converts the given floating-point data into the specified _dstType format.
// _srcDimension - dimension of the source data (size of vector: 1,2,3,4).
// _dstDimension is assumed to be less than or equal to _srcDimension.
// if _inputNormalized is true then the source data is assumed to be normalized
// to the range of [-1..+1] if _inputPositive is false,
// and to the range of [0..1] if _inputPositive is true.
// if _inputNormalized is false then _inputPositive is ignored.
//
static void PackVertexAttribF( const float* _srcPtr, UINT _srcDimension,
					   void *_dstPtr, AttributeTypeT _dstType, UINT _dstDimension,
					   bool _inputNormalized, bool _inputPositive )
{
	const float* srcPtrF = _srcPtr;
	const UINT minCommonDim = smallest(_srcDimension, _dstDimension);
	mxASSERT(minCommonDim <= 4);
	//const UINT leftOver = _dstDimension - minCommonDim;
	switch( _dstType )
	{
	case AttributeType::Byte :
	case AttributeType::UByte :
		{
			UINT8* dstPtr8 = (UINT8*)_dstPtr;
			if( _inputNormalized )
			{
				if( _inputPositive )
				{
					// [0..1] float => [0..255] integer, scale: f * 255.0f
					switch(minCommonDim)
					{
					default:	*dstPtr8++ = (UINT8)( *srcPtrF++ * 255.0f );
					case 3:		*dstPtr8++ = (UINT8)( *srcPtrF++ * 255.0f );
					case 2:		*dstPtr8++ = (UINT8)( *srcPtrF++ * 255.0f );
					case 1:		*dstPtr8++ = (UINT8)( *srcPtrF++ * 255.0f );
					}
				}
				else
				{
					// [-1..+1] float => [0..255] integer, scale and bias: ((f + 1.0f) * 0.5f) * 255.0f
					switch(minCommonDim)
					{
					default:	*dstPtr8++ = (UINT8)( *srcPtrF++ * 127.5f + 127.5f );
					case 3:		*dstPtr8++ = (UINT8)( *srcPtrF++ * 127.5f + 127.5f );
					case 2:		*dstPtr8++ = (UINT8)( *srcPtrF++ * 127.5f + 127.5f );
					case 1:		*dstPtr8++ = (UINT8)( *srcPtrF++ * 127.5f + 127.5f );
					}
				}
			}
			else
			{
				switch(minCommonDim)
				{
				default:	*dstPtr8++ = (UINT8)( *srcPtrF++ );
				case 3:		*dstPtr8++ = (UINT8)( *srcPtrF++ );
				case 2:		*dstPtr8++ = (UINT8)( *srcPtrF++ );
				case 1:		*dstPtr8++ = (UINT8)( *srcPtrF++ );
				}
			}
		}
		break;

	case AttributeType::Short :
		UNDONE;
	case AttributeType::UShort :
		{
			UINT16* dstPtr16 = (UINT16*)_dstPtr;
			if( _inputNormalized )
			{
				// [-1..+1] float => [0..65535] int
				// scale and bias: ((f + 1.0f) * 0.5f) * 65535.0f
				switch(minCommonDim)
				{
				default:	*dstPtr16++ = (UINT16)( *srcPtrF++ * 32767.5f + 32767.5f );
				case 3:		*dstPtr16++ = (UINT16)( *srcPtrF++ * 32767.5f + 32767.5f );
				case 2:		*dstPtr16++ = (UINT16)( *srcPtrF++ * 32767.5f + 32767.5f );
				case 1:		*dstPtr16++ = (UINT16)( *srcPtrF++ * 32767.5f + 32767.5f );
				}
			}
			else
			{
				switch(minCommonDim)
				{
				default:	*dstPtr16++ = (UINT16)( *srcPtrF++ );
				case 3:		*dstPtr16++ = (UINT16)( *srcPtrF++ );
				case 2:		*dstPtr16++ = (UINT16)( *srcPtrF++ );
				case 1:		*dstPtr16++ = (UINT16)( *srcPtrF++ );
				}
			}
		}
		break;

	case AttributeType::Half :
		{
			Half* dstPtr16 = (Half*)_dstPtr;
			switch(minCommonDim)
			{
			default:	*dstPtr16++ = Float_To_Half( *srcPtrF++ );
			case 3:		*dstPtr16++ = Float_To_Half( *srcPtrF++ );
			case 2:		*dstPtr16++ = Float_To_Half( *srcPtrF++ );
			case 1:		*dstPtr16++ = Float_To_Half( *srcPtrF++ );
			}
		}
		break;
	case AttributeType::Float :
		{
			float* dstPtrF = (float*)_dstPtr;
			switch(minCommonDim)
			{
			default:	*dstPtrF++ = *srcPtrF++;
			case 3:		*dstPtrF++ = *srcPtrF++;
			case 2:		*dstPtrF++ = *srcPtrF++;
			case 1:		*dstPtrF++ = *srcPtrF++;
			}
		}
		break;
	mxNO_SWITCH_DEFAULT;
	}
}

static void PackVertexAttribI( const int* _srcPtr, UINT _srcDimension,
					   void *_dstPtr, AttributeTypeT _dstType, UINT _dstDimension )
{
	const UINT minCommonDim = smallest(_srcDimension, _dstDimension);
	//const UINT leftOver = _dstDimension - minCommonDim;
	switch( _dstType )
	{
	case AttributeType::Byte :
	case AttributeType::UByte :
		{
			UINT8* dstPtr8 = (UINT8*)_dstPtr;
			switch(minCommonDim)
			{
			default:	*dstPtr8++ = (UINT8)( *_srcPtr++ );
			case 3:		*dstPtr8++ = (UINT8)( *_srcPtr++ );
			case 2:		*dstPtr8++ = (UINT8)( *_srcPtr++ );
			case 1:		*dstPtr8++ = (UINT8)( *_srcPtr++ );
			}
		}
		break;

	case AttributeType::Short :
		UNDONE;
	case AttributeType::UShort :
		UNDONE;
	case AttributeType::Half :
		UNDONE;
	case AttributeType::Float :
		{
			float* dstPtrF = (float*)_dstPtr;
			switch(minCommonDim)
			{
			default:	*dstPtrF++ = (float)( *_srcPtr++ );
			case 3:		*dstPtrF++ = (float)( *_srcPtr++ );
			case 2:		*dstPtrF++ = (float)( *_srcPtr++ );
			case 1:		*dstPtrF++ = (float)( *_srcPtr++ );
			}
		}
		break;
	mxNO_SWITCH_DEFAULT;
	}
}

// Merges data into a single vertex and index buffer referenced by primitive groups (submeshes).
//
ERet CompileMesh( const TcMeshData& src, const VertexDescription& vertex, RawMeshData &dst )
{
	UINT totalVertexCount = 0;
	UINT totalIndexCount = 0;
	CalculateTotalVertexIndexCount( src, totalVertexCount, totalIndexCount );

	const bool use32indices = (totalVertexCount >= MAX_UINT16);
	const UINT indexStride = use32indices ? 4 : 2;

	VertexStream	streamStorage[8];

	TArray< VertexStream >	streams;
	streams.SetExternalStorage( streamStorage, mxCOUNT_OF(streamStorage) );

	GatherStreams(vertex, streams);

	const UINT numStreams = streams.Num();

	// reserve space in vertex data
	RawVertexData& dstVD = dst.vertexData;
	dstVD.streams.SetNum(numStreams);
	dstVD.count = totalVertexCount;
	dstVD.type = VertexType::Generic;
	for( UINT streamIndex = 0; streamIndex < numStreams; streamIndex++ )
	{
		const UINT32 stride = vertex.streamStrides[ streamIndex ];	// stride of the vertex buffer
		RawVertexStream &dstVB = dstVD.streams[ streamIndex ];
		dstVB.data.SetNum( stride * totalVertexCount );
	}

	// reserve space in index data
	RawIndexData& dstID = dst.indexData;
	dstID.data.SetNum(indexStride * totalIndexCount);
	dstID.stride = indexStride;

	dst.topology = Topology::TriangleList;
	dst.bounds = src.bounds;

	// iterate over all submeshes (primitive groups) and merge vertex/index data
	const UINT numMeshes = src.sets.Num();
	dst.parts.SetNum(numMeshes);

	UINT32  currentVertexIndex = 0;
	UINT32  currentIndexNumber = 0;
	for( UINT meshIndex = 0; meshIndex < numMeshes; meshIndex++ )
	{
		const TcTriMesh& submesh = src.sets[ meshIndex ];
		const UINT32 numVertices = submesh.NumVertices();	// number of vertices in this submesh
		const UINT32 numIndices = submesh.NumIndices();	// number of indices in this submesh

		RawMeshPart &meshPart = dst.parts[ meshIndex ];
		meshPart.baseVertex = currentVertexIndex;
		meshPart.startIndex = currentIndexNumber;
		meshPart.indexCount = numIndices;
		meshPart.vertexCount = numVertices;

		const int maxBoneInfluences = numVertices * MAX_INFLUENCES;

		ScopedStackAlloc	stackAlloc(gCore.frameAlloc);
		int *	boneIndices = stackAlloc.AllocMany< int >( maxBoneInfluences );
		float *	boneWeights = stackAlloc.AllocMany< float >( maxBoneInfluences );

		memset(boneIndices, 0, maxBoneInfluences * sizeof(boneIndices[0]));
		memset(boneWeights, 0, maxBoneInfluences * sizeof(boneWeights[0]));

		if( submesh.weights.NonEmpty() )
		{
			mxASSERT(submesh.weights.Num() == numVertices);
			for( UINT32 vertexIndex = 0; vertexIndex < numVertices; vertexIndex++ )
			{
				const TcWeights& weights = submesh.weights[ vertexIndex ];
				const int numWeights = smallest(weights.Num(), MAX_INFLUENCES);
				for( int weightIndex = 0; weightIndex < numWeights; weightIndex++ )
				{
					const int offset = vertexIndex * MAX_INFLUENCES + weightIndex;
					boneIndices[ offset ] = weights[ weightIndex ].boneIndex;
					boneWeights[ offset ] = weights[ weightIndex ].boneWeight;
				}
			}//for each vertex
		}

		// process each vertex stream
		for( UINT streamIndex = 0; streamIndex < numStreams; streamIndex++ )
		{
			const UINT32 streamStride = vertex.streamStrides[ streamIndex ];	// stride of the vertex buffer
			const VertexStream& stream = streams[ streamIndex ];				
			RawVertexStream &dstVB = dstVD.streams[ streamIndex ];
			dstVB.data.SetNum( streamStride * totalVertexCount );

			const UINT32 currentVertexDataOffset = currentVertexIndex * streamStride;

			// first fill the vertex data with zeros
			void* dstStreamData = mxAddByteOffset( dstVB.data.ToPtr(), currentVertexDataOffset );
			memset(dstStreamData, 0, streamStride * numVertices);

			// process each vertex stream component
			for( UINT elementIndex = 0; elementIndex < stream.components.Num(); elementIndex++ )
			{
				const UINT32 attribIndex = stream.components[ elementIndex ];
				const VertexElement& attrib = vertex.attribsArray[ attribIndex ];
				const UINT32 attribOffset = vertex.attribOffsets[ attribIndex ];	// offset within stream
				const AttributeType::Enum attribType = (AttributeType::Enum) attrib.type;
				const VertexAttribute::Enum semantic = (VertexAttribute::Enum) attrib.semantic;				
				const UINT attribDimension = attrib.dimension + 1;

				// process each vertex
				const void* srcPtr = NULL;
				UINT srcDimension = 0;	// vector dimension: [1,2,3,4]
				bool srcDataIsFloat = true;
				bool srcIsPositive = false;
				bool normalize = attrib.normalized;
				int srcStride = 4; // sizeof(float) == sizeof(int)

				switch( semantic )
				{
				case VertexAttribute::Position :
					srcPtr = (float*) submesh.positions.SafeGetFirstItemPtr();
					srcDimension = 3;
					break;

				case VertexAttribute::Color0 :
					srcPtr = (float*) submesh.colors.SafeGetFirstItemPtr();
					srcDimension = 4;
					srcIsPositive = true;	// RGBA colors are in range [0..1]
					break;
				case VertexAttribute::Color1 :
					ptERROR("Vertex colors > 1 are not supported!\n");
					break;

				case VertexAttribute::Normal :
					srcPtr = (float*) submesh.normals.SafeGetFirstItemPtr();
					srcDimension = 3;
					normalize = true;	// we pack normals and tangents into Ubyte4
					break;
				case VertexAttribute::Tangent :
					srcPtr = (float*) submesh.tangents.SafeGetFirstItemPtr();
					srcDimension = 3;
					normalize = true;	// we pack normals and tangents into Ubyte4
					break;

				case VertexAttribute::TexCoord0 :
					srcPtr = (float*) submesh.texCoords.SafeGetFirstItemPtr();
					srcDimension = 2;
					break;
				case VertexAttribute::TexCoord1 :
				case VertexAttribute::TexCoord2 :
				case VertexAttribute::TexCoord3 :
				case VertexAttribute::TexCoord4 :
				case VertexAttribute::TexCoord5 :
				case VertexAttribute::TexCoord6 :
				case VertexAttribute::TexCoord7 :
					ptWARN("TexCoord > 1 are not supported!\n");
					break;

				case VertexAttribute::BoneIndices :
					srcPtr = boneIndices;
					srcDimension = 4;
					srcDataIsFloat = false;
					srcIsPositive = true;
					break;
				case VertexAttribute::BoneWeights :
					srcPtr = boneWeights;
					srcDimension = 4;
					srcIsPositive = true;	// bone weights are always in range [0..1]
					break;

				default:
					ptERROR("Unknown vertex attrib semantic: %d\n", semantic);
				}

				if( srcPtr && srcDimension > 0 )
				{
					for( UINT32 vertexIndex = 0; vertexIndex < numVertices; vertexIndex++ )
					{
						UINT vertexOffset = currentVertexDataOffset + vertexIndex*streamStride + attribOffset;
						void* dstPtr = mxAddByteOffset( dstVB.data.ToPtr(), vertexOffset );
						if( srcDataIsFloat ) {
 							PackVertexAttribF( (float*)srcPtr, srcDimension, dstPtr, attribType, attribDimension, normalize, srcIsPositive );
						} else {
							mxASSERT2( !attrib.normalized, "Integer data cannot be normalized!" );
							PackVertexAttribI( (int*)srcPtr, srcDimension, dstPtr, attribType, attribDimension );
						}
						srcPtr = mxAddByteOffset( srcPtr, srcDimension * srcStride );
					}//for each vertex
				}
			}//for each component
		}//for each stream

		void* indices = mxAddByteOffset(dstID.data.ToPtr(), currentIndexNumber * indexStride);
		for( UINT32 i = 0; i < numIndices; i++ )
		{
			const UINT32 index = submesh.indices[i];
			if( use32indices ) {
				UINT32* indices32 = (UINT32*) indices;
				indices32[i] = currentVertexIndex + index;
			} else {
				UINT16* indices16 = (UINT16*) indices;
				indices16[i] = currentVertexIndex + index;
			}
		}

		currentVertexIndex += submesh.NumVertices();
		currentIndexNumber += submesh.NumIndices();
	}

	const UINT numBones = src.skeleton.bones.Num();

	Skeleton& skeleton = dst.skeleton;

	skeleton.bones.SetNum(numBones);
	skeleton.boneNames.SetNum(numBones);
	skeleton.invBindPoses.SetNum(numBones);

	for( UINT boneIndex = 0; boneIndex < numBones; boneIndex++ )
	{
		const TcBone& bone = src.skeleton.bones[boneIndex];

		Bone& joint = skeleton.bones[boneIndex];

		joint.orientation = bone.rotation;
		joint.position = bone.translation;
		joint.parent = bone.parent;
#if 0
		if( bone.parent >= 0 )
		{
			const Joint& parent = skeleton.joints[bone.parent];
			UNDONE;			
			//ConcatenateJointTransforms(parent.position, parent.orientation, joint.position, joint.orientation);

			//Float4x4 parentMatrix = CalculateJointMatrix( parent.position, parent.orientation );
			//Float4x4 boneMatrix = CalculateJointMatrix( bone.translation, bone.rotation );
			//Float4x4 localMatrix = boneMatrix * glm::inverse(parentMatrix);

			//glm::vec3 localT(localMatrix[3]);
			//glm::quat localQ(localMatrix);
			//joint.orientation = localQ;
			//joint.position = localT;

			//Float4x4 parentMatrix = CalculateJointMatrix( parent.position, parent.orientation );
			//Float4x4 boneTransform = CalculateJointMatrix( bone.translation, bone.rotation );
			//Float4x4 globalMatrix = parentMatrix * boneTransform;
			//joint.orientation = glm::quat(globalMatrix);
			//joint.position = glm::vec3(globalMatrix[3]);
		}

		//joint.orientation = glm::quat(bone.absolute);
		//joint.position = glm::vec3(bone.absolute[3]);

		joint.orientation = glm::normalize(joint.orientation);

		//ptPRINT("Joint[%u]: '%s', parent = %d  (P = %.3f, %.3f, %.3f, Q = %.3f, %.3f, %.3f, %.3f)\n",
		//	boneIndex, bone.name.ToPtr(), bone.parent,
		//	joint.position.iX, joint.position.iY, joint.position.iZ,
		//	joint.orientation.iX, joint.orientation.iY, joint.orientation.iZ, joint.orientation.w);


		skeleton.jointNames[boneIndex] = bone.name;

		Float4x4 jointMatrix = CalculateJointMatrix( joint.position, joint.orientation );

		skeleton.invBindPoses[boneIndex] = glm::inverse(jointMatrix);


		//ptPRINT("Bone[%u]: '%s', absolute = %s,\ncomputed = %s\n",
		//	boneIndex, bone.name.ToPtr(),
		//	MatrixToString(bone.absolute).ToPtr(),
		//	MatrixToString(jointMatrix).ToPtr());


		//ptPRINT("Bone[%u]: '%s', parent = %d (P = %.3f, %.3f, %.3f, Q = %.3f, %.3f, %.3f, %.3f)\n",
		//	boneIndex, bone.name.ToPtr(),
		//	bone.parent, bone.translation.iX, bone.translation.iY, bone.translation.iZ,
		//	bone.rotation.iX, bone.rotation.iY, bone.rotation.iZ, bone.rotation.w);

#endif

	}

	return ALL_OK;
}

// Euler's formula for solids bounded by 2-dimensional manifolds:

// V - Number of vertices
// E - Number of edges
// F - Number of faces
// H - Number of holes in faces
// 
UINT32 EulerNumber( UINT32 V, UINT32 E, UINT32 F, UINT32 H )
{
	return V - E + F - H;
}
// C - Number of connected components
// G - Number of holes in the solid (genus)
// For example, for a cube, V = 8, E = 12, F = 6, H = 0, C = 1, G = 0.
// Therefore, V - E + F - H = 2 = 2*(C - G).
// If Euler's formula is not satisfied, then there is an error in the model.
// 
bool EulerTest( UINT32 V, UINT32 E, UINT32 F, UINT32 H, UINT32 C, UINT32 G )
{
	return EulerNumber( V, E, F, H ) == 2 * (C - G);
}

// Creating simple volumes with different shapes, like spheres, ellipsoids, other shapes:
// https://github.com/marwan-abdellah/VolumeTools

ERet VoxelizeSphere( const float _radius, const UINT32 _gridSize, void *_voxels )
{
	UINT32 *	bits = (UINT32*) _voxels;

	const UINT32 gridSizeX = _gridSize;
	const UINT32 gridSizeY = _gridSize;
	const UINT32 gridSizeZ = _gridSize;
	const UINT32 halfSizeX = gridSizeX / 2;
	const UINT32 halfSizeY = gridSizeY / 2;
	const UINT32 halfSizeZ = gridSizeZ / 2;

	for( UINT32 iX = 0; iX < gridSizeX; iX++ )
	{
		for( UINT32 iY = 0; iY < gridSizeY; iY++ )
		{
			for( UINT32 iZ = 0; iZ < gridSizeZ; iZ++ )
			{
				const UINT32 index = iX + iY * gridSizeX + iZ * (gridSizeX * gridSizeY);

				const INT32 x = iX - halfSizeX;
				const INT32 y = iY - halfSizeY;
				const INT32 z = iZ - halfSizeZ;

				const float length = Float_Sqrt( x*x + y*y + z*z );
				const bool isSolid = (length < _radius);

				// bits[ i / 32 ] |= (isSolid << (i % 32));
				bits[ index >> 5 ] |= (isSolid << (index & 31));
			}
		}
	}

	return ALL_OK;
}

// Dimensions of the model's bounding box in voxels.

}//namespace Meshok

mxSWIPED("http://richardssoftware.net/Home/Post/60");
//https://github.com/ericrrichards/dx11/blob/b640855654aba2b30a3794f30c079812ee829aa2/DX11/Core/GeometryGenerator.cs

static FatVertex1 NewVertex(
							float Px, float Py, float Pz,
							float Nx, float Ny, float Nz,
							float Tx, float Ty, float Tz,
							float U, float V
							)
{
	FatVertex1 result;
	result.position = Float3_Set( Px, Py, Pz );
	result.normal = Float3_Set( Nx, Ny, Nz );
	result.tangent = Float3_Set( Tx, Ty, Tz );
	result.texCoord = Float2_Set( U, V );
	return result;
}

ERet CreateBox(
			   float width, float height, float depth,
			   TArray<FatVertex1> &_vertices, TArray<int> &_indices
			   )
{
	_vertices.Empty();
	_indices.Empty();

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	const FatVertex1 vertices[] = {
		// front
		NewVertex(-w2, -h2, -d2, 0, 0, -1, 1, 0, 0, 0, 1),
		NewVertex(-w2, +h2, -d2, 0, 0, -1, 1, 0, 0, 0, 0),
		NewVertex(+w2, +h2, -d2, 0, 0, -1, 1, 0, 0, 1, 0),
		NewVertex(+w2, -h2, -d2, 0, 0, -1, 1, 0, 0, 1, 1),
		// back
		NewVertex(-w2, -h2, +d2, 0, 0, 1, -1, 0, 0, 1, 1),
		NewVertex(+w2, -h2, +d2, 0, 0, 1, -1, 0, 0, 0, 1),
		NewVertex(+w2, +h2, +d2, 0, 0, 1, -1, 0, 0, 0, 0),
		NewVertex(-w2, +h2, +d2, 0, 0, 1, -1, 0, 0, 1, 0),
		// top
		NewVertex(-w2, +h2, -d2, 0, 1, 0, 1, 0, 0, 0, 1),
		NewVertex(-w2, +h2, +d2, 0, 1, 0, 1, 0, 0, 0, 0),
		NewVertex(+w2, +h2, +d2, 0, 1, 0, 1, 0, 0, 1, 0),
		NewVertex(+w2, +h2, -d2, 0, 1, 0, 1, 0, 0, 1, 1),
		// bottom
		NewVertex(-w2, -h2, -d2, 0, -1, 0, -1, 0, 0, 1, 1),
		NewVertex(+w2, -h2, -d2, 0, -1, 0, -1, 0, 0, 0, 1),
		NewVertex(+w2, -h2, +d2, 0, -1, 0, -1, 0, 0, 0, 0),
		NewVertex(-w2, -h2, +d2, 0, -1, 0, -1, 0, 0, 1, 0),
		// left
		NewVertex(-w2, -h2, +d2, -1, 0, 0, 0, 0, -1, 0, 1),
		NewVertex(-w2, +h2, +d2, -1, 0, 0, 0, 0, -1, 0, 0),
		NewVertex(-w2, +h2, -d2, -1, 0, 0, 0, 0, -1, 1, 0),
		NewVertex(-w2, -h2, -d2, -1, 0, 0, 0, 0, -1, 1, 1),
		// right
		NewVertex(+w2, -h2, -d2, 1, 0, 0, 0, 0, 1, 0, 1),
		NewVertex(+w2, +h2, -d2, 1, 0, 0, 0, 0, 1, 0, 0),
		NewVertex(+w2, +h2, +d2, 1, 0, 0, 0, 0, 1, 1, 0),
		NewVertex(+w2, -h2, +d2, 1, 0, 0, 0, 0, 1, 1, 1),
	};
	mxDO(_vertices.Add( vertices, mxCOUNT_OF(vertices) ));

	const int indices[] = {
		0,1,2,0,2,3,
		4,5,6,4,6,7,
		8,9,10,8,10,11,
		12,13,14,12,14,15,
		16,17,18,16,18,19,
		20,21,22,20,22,23
	};
	mxDO(_indices.Add( indices, mxCOUNT_OF(indices) ));

	return ALL_OK;
}

ERet CreateSphere(
				  float radius, int sliceCount, int stackCount,
				  TArray<FatVertex1> &_vertices, TArray<int> &_indices
				  )
{
	_vertices.Empty();
	_indices.Empty();

	{
		FatVertex1& northPole = _vertices.Add();
		northPole.position	= Float3_Set( 0, radius, 0 );
		northPole.normal	= Float3_Set( 0, 1, 0 );
		northPole.tangent	= Float3_Set( 1, 0, 0 );
		northPole.texCoord	= Float2_Set( 0, 0 );
	}

	float phiStep = mxPI / stackCount;
	float thetaStep = 2.0f * mxPI / sliceCount;

	for( int i = 1; i <= stackCount - 1; i++ )
	{
		float phi = i * phiStep;

		float sinPhi, cosPhi;
		Float_SinCos( phi, sinPhi, cosPhi );

		for( int j = 0; j <= sliceCount; j++ )
		{
			float theta = j * thetaStep;

			float sinTheta, cosTheta;			
			Float_SinCos( theta, sinTheta, cosTheta );

			FatVertex1& vertex = _vertices.Add();

			vertex.position = Float3_Set(
				(radius * sinPhi * cosTheta),
				(radius * cosPhi),
				(radius * sinPhi * sinTheta)
			);

			vertex.normal = Float3_Normalized( vertex.position );

			vertex.tangent = Float3_Set(
				-radius * sinPhi * sinTheta,
				0,
				radius * sinPhi * cosTheta
			);
			vertex.tangent = Float3_Normalized( vertex.tangent );

			vertex.texCoord = Float2_Set(
				theta / (mxPI * 2),
				phi / mxPI
			);
		}
	}

	{
		FatVertex1& southPole = _vertices.Add();
		southPole.position	= Float3_Set( 0, -radius, 0 );
		southPole.normal	= Float3_Set( 0, -1, 0 );
		southPole.tangent	= Float3_Set( 1, 0, 0 );
		southPole.texCoord	= Float2_Set( 0, 1 );
	}

	for( int i = 1; i <= sliceCount; i++ ) {
		_indices.Add(0);
		_indices.Add(i + 1);
		_indices.Add(i);
	}
	int baseIndex = 1;
	int ringVertexCount = sliceCount + 1;
	for( int i = 0; i < stackCount - 2; i++ ) {
		for( int j = 0; j < sliceCount; j++ ) {
			_indices.Add(baseIndex + i * ringVertexCount + j);
			_indices.Add(baseIndex + i * ringVertexCount + j + 1);
			_indices.Add(baseIndex + (i + 1) * ringVertexCount + j);

			_indices.Add(baseIndex + (i + 1) * ringVertexCount + j);
			_indices.Add(baseIndex + i * ringVertexCount + j + 1);
			_indices.Add(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}
	int southPoleIndex = _vertices.Num() - 1;
	baseIndex = southPoleIndex - ringVertexCount;
	for( int i = 0; i < sliceCount; i++ ) {
		_indices.Add(southPoleIndex);
		_indices.Add(baseIndex + i);
		_indices.Add(baseIndex + i + 1);
	}

	return ALL_OK;
}

static Float3 IcosahedronVertices[] = {
	{-0.525731f, 0, 0.850651f}, {0.525731f, 0, 0.850651f},
	{-0.525731f, 0, -0.850651f}, {0.525731f, 0, -0.850651f},
	{0, 0.850651f, 0.525731f}, {0, 0.850651f, -0.525731f},
	{0, -0.850651f, 0.525731f}, {0, -0.850651f, -0.525731f},
	{0.850651f, 0.525731f, 0}, {-0.850651f, 0.525731f, 0},
	{0.850651f, -0.525731f, 0}, {-0.850651f, -0.525731f, 0}
};

static int IcosahedronIndices[] = {
	1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
	1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
	3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
	10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
};

#if 0
class Subdivider {
	List<Vertex> _vertices;
	List<int> _indices;
	Dictionary<Tuple<int, int>, int> _newVertices;

	public void Subdivide4(MeshData mesh) {
		_newVertices = new Dictionary<Tuple<int, int>, int>();
		_vertices = mesh.Vertices;
		_indices = new List<int>();
		var numTris = mesh.Indices.Count / 3;

		for (var i = 0; i < numTris; i++) {
			//       i2
			//       *
			//      / \
			//     /   \
			//   a*-----*b
			//   / \   / \
			//  /   \ /   \
			// *-----*-----*
			// i1    c      i3

			var i1 = mesh.Indices[i * 3];
			var i2 = mesh.Indices[i * 3 + 1];
			var i3 = mesh.Indices[i * 3 + 2];

			var a = GetNewVertex(i1, i2);
			var b = GetNewVertex(i2, i3);
			var c = GetNewVertex(i3, i1);

			_indices.AddRange(new[] {
				i1, a, c,
					i2, b, a,
					i3, c, b,
					a, b, c
			});
		}
#if DEBUG
		Console.WriteLine(mesh.Vertices.Count);
#endif
		mesh.Indices = _indices;
	}

	int GetNewVertex(int i1, int i2) {
		var t1 = new Tuple<int, int>(i1, i2);
		var t2 = new Tuple<int, int>(i2, i1);

		if (_newVertices.ContainsKey(t2)) {
			return _newVertices[t2];
		}
		if (_newVertices.ContainsKey(t1)) {
			return _newVertices[t1];
		}
		var newIndex = _vertices.Count;
		_newVertices.Add(t1, newIndex);

		_vertices.Add(new Vertex() { Position = (_vertices[i1].Position + _vertices[i2].Position) * 0.5f });

		return newIndex;
	}
}

ERet CreateGeodesicSphere(
						  float radius, int numSubdivisions, //[0..8]
						  TArray<FatVertex1> &_vertices, TArray<int> &_indices
						  )
{
	tempMesh.Vertices = IcosahedronVertices.Select(p => new Vertex { Position = p }).ToList();
	tempMesh.Indices = IcosahedronIndices;

	var mh = new Subdivider();

	for (var i = 0; i < (int)numSubdivisions; i++) {
		mh.Subdivide4(tempMesh);
	}

	// Project vertices onto sphere and scale.
	for (var i = 0; i < tempMesh.Vertices.Count; i++) {
		// Project onto unit sphere.
		var n = Float3.Normalize(tempMesh.Vertices[i].Position);
		// Project onto sphere.
		var p = radius * n;

		// Derive texture coordinates from spherical coordinates.
		var theta = MathF.AngleFromXY(tempMesh.Vertices[i].Position.X, tempMesh.Vertices[i].Position.Z);
		var phi = MathF.Acos(tempMesh.Vertices[i].Position.Y / radius);
		var texC = new Vector2(theta / (2 * MathF.PI), phi / MathF.PI);

		// Partial derivative of P with respect to theta
		var tangent = new Float3(
			-radius * MathF.Sin(phi) * MathF.Sin(theta),
			0,
			radius * MathF.Sin(phi) * MathF.Cos(theta)
			);
		tangent.Normalize();

		tempMesh.Vertices[i] = new Vertex(p, n, tangent, texC);
	}

	return ALL_OK;
}

#elif 0

void Subdivide( Float3 *&dest, const Float3 &v0, const Float3 &v1, const Float3 &v2, int level )
{
	if (level){
		level--;
		Float3 v3 = Float3_Normalized(v0 + v1);
		Float3 v4 = Float3_Normalized(v1 + v2);
		Float3 v5 = Float3_Normalized(v2 + v0);

		Subdivide(dest, v0, v3, v5, level);
		Subdivide(dest, v3, v4, v5, level);
		Subdivide(dest, v3, v1, v4, level);
		Subdivide(dest, v5, v4, v2, level);
	} else {
		*dest++ = v0;
		*dest++ = v1;
		*dest++ = v2;
	}
}

ERet CreateGeodesicSphere(
						  float radius, int numSubdivisions, //[0..8]
						  TArray<FatVertex1> &_vertices, TArray<int> &_indices
						  )
{
	const int nVertices = 8 * 3 * (1 << (2 * numSubdivisions));
	mxDO(_vertices.SetNum( nVertices ));
	Float3* vertices = _vertices.ToPtr();

	// Tessellate a octahedron
	Float3 px0(-1,  0,  0);
	Float3 px1( 1,  0,  0);
	Float3 py0( 0, -1,  0);
	Float3 py1( 0,  1,  0);
	Float3 pz0( 0,  0, -1);
	Float3 pz1( 0,  0,  1);

	Float3 *dest = vertices;
	Subdivide(dest, py0, px0, pz0, numSubdivisions);
	Subdivide(dest, py0, pz0, px1, numSubdivisions);
	Subdivide(dest, py0, px1, pz1, numSubdivisions);
	Subdivide(dest, py0, pz1, px0, numSubdivisions);
	Subdivide(dest, py1, pz0, px0, numSubdivisions);
	Subdivide(dest, py1, px0, pz1, numSubdivisions);
	Subdivide(dest, py1, pz1, px1, numSubdivisions);
	Subdivide(dest, py1, px1, pz0, numSubdivisions);

	mxASSERT(dest - vertices == nVertices);

	nIndices = nVertices;
	addBatch(0, nVertices);

	return ALL_OK;
}

public static MeshData CreateCylinder(float bottomRadius, float topRadius, float height, int sliceCount, int stackCount) {
    var ret = new MeshData();

    var stackHeight = height / stackCount;
    var radiusStep = (topRadius - bottomRadius) / stackCount;
    var ringCount = stackCount + 1;

    for (int i = 0; i < ringCount; i++) {
        var y = -0.5f * height + i * stackHeight;
        var r = bottomRadius + i * radiusStep;
        var dTheta = 2.0f * MathF.PI / sliceCount;
        for (int j = 0; j <= sliceCount; j++) {

            var c = MathF.Cos(j * dTheta);
            var s = MathF.Sin(j * dTheta);

            var v = new Vector3(r * c, y, r * s);
            var uv = new Vector2((float)j / sliceCount, 1.0f - (float)i / stackCount);
            var t = new Vector3(-s, 0.0f, c);

            var dr = bottomRadius - topRadius;
            var bitangent = new Vector3(dr * c, -height, dr * s);

            var n = Vector3.Normalize(Vector3.Cross(t, bitangent));

            ret.Vertices.Add(new Vertex(v, n, t, uv));

        }
    }
    var ringVertexCount = sliceCount + 1;
    for (int i = 0; i < stackCount; i++) {
        for (int j = 0; j < sliceCount; j++) {
            ret.Indices.Add(i * ringVertexCount + j);
            ret.Indices.Add((i + 1) * ringVertexCount + j);
            ret.Indices.Add((i + 1) * ringVertexCount + j + 1);

            ret.Indices.Add(i * ringVertexCount + j);
            ret.Indices.Add((i + 1) * ringVertexCount + j + 1);
            ret.Indices.Add(i * ringVertexCount + j + 1);
        }
    }
    BuildCylinderTopCap(topRadius, height, sliceCount, ref ret);
    BuildCylinderBottomCap(bottomRadius, height, sliceCount, ref ret);
    return ret;
}

private static void BuildCylinderTopCap(float topRadius, float height, int sliceCount, ref MeshData ret) {
    var baseIndex = ret.Vertices.Count;

    var y = 0.5f * height;
    var dTheta = 2.0f * MathF.PI / sliceCount;

    for (int i = 0; i <= sliceCount; i++) {
        var x = topRadius * MathF.Cos(i * dTheta);
        var z = topRadius * MathF.Sin(i * dTheta);

        var u = x / height + 0.5f;
        var v = z / height + 0.5f;
        ret.Vertices.Add(new Vertex(x, y, z, 0, 1, 0, 1, 0, 0, u, v));
    }
    ret.Vertices.Add(new Vertex(0, y, 0, 0, 1, 0, 1, 0, 0, 0.5f, 0.5f));
    var centerIndex = ret.Vertices.Count - 1;
    for (int i = 0; i < sliceCount; i++) {
        ret.Indices.Add(centerIndex);
        ret.Indices.Add(baseIndex + i + 1);
        ret.Indices.Add(baseIndex + i);
    }
}

private static void BuildCylinderBottomCap(float bottomRadius, float height, int sliceCount, ref MeshData ret) {
    var baseIndex = ret.Vertices.Count;

    var y = -0.5f * height;
    var dTheta = 2.0f * MathF.PI / sliceCount;

    for (int i = 0; i <= sliceCount; i++) {
        var x = bottomRadius * MathF.Cos(i * dTheta);
        var z = bottomRadius * MathF.Sin(i * dTheta);

        var u = x / height + 0.5f;
        var v = z / height + 0.5f;
        ret.Vertices.Add(new Vertex(x, y, z, 0, -1, 0, 1, 0, 0, u, v));
    }
    ret.Vertices.Add(new Vertex(0, y, 0, 0, -1, 0, 1, 0, 0, 0.5f, 0.5f));
    var centerIndex = ret.Vertices.Count - 1;
    for (int i = 0; i < sliceCount; i++) {
        ret.Indices.Add(centerIndex);
        ret.Indices.Add(baseIndex + i);
        ret.Indices.Add(baseIndex + i + 1);
    }
}

public static MeshData CreateGrid(float width, float depth, int m, int n) {
    var ret = new MeshData();

    var halfWidth = width * 0.5f;
    var halfDepth = depth * 0.5f;

    var dx = width / (n - 1);
    var dz = depth / (m - 1);

    var du = 1.0f / (n - 1);
    var dv = 1.0f / (m - 1);

    for (var i = 0; i < m; i++) {
        var z = halfDepth - i * dz;
        for (var j = 0; j < n; j++) {
            var x = -halfWidth + j * dx;
            ret.Vertices.Add(new Vertex(new Vector3(x, 0, z), new Vector3(0, 1, 0), new Vector3(1, 0, 0), new Vector2(j * du, i * dv)));
        }
    }
    for (var i = 0; i < m - 1; i++) {
        for (var j = 0; j < n - 1; j++) {
            ret.Indices.Add(i * n + j);
            ret.Indices.Add(i * n + j + 1);
            ret.Indices.Add((i + 1) * n + j);

            ret.Indices.Add((i + 1) * n + j);
            ret.Indices.Add(i * n + j + 1);
            ret.Indices.Add((i + 1) * n + j + 1);
        }
    }
    return ret;
}

public static MeshData CreateFullScreenQuad() {
    var ret = new MeshData();

    ret.Vertices.Add(new Vertex(-1, -1, 0, 0, 0, -1, 1, 0, 0, 0, 1));
    ret.Vertices.Add(new Vertex(-1, 1, 0, 0, 0, -1, 1, 0, 0, 0, 0));
    ret.Vertices.Add(new Vertex(1, 1, 0, 0, 0, -1, 1, 0, 0, 1, 0));
    ret.Vertices.Add(new Vertex(1, -1, 0, 0, 0, -1, 1, 0, 0, 1, 1));

    ret.Indices.AddRange(new[] { 0, 1, 2, 0, 2, 3 });


    return ret;
}

#endif


ERet MeshBuilder::Begin()
{
	vertices.Empty();
	indices.Empty();
	return ALL_OK;
}
int MeshBuilder::AddVertex( const DrawVertex& vertex )
{
	int index = vertices.Num();
	vertices.Add(vertex);
	return index;
}
int MeshBuilder::AddTriangle( int v1, int v2, int v3 )
{
	indices.Add(v1);
	indices.Add(v2);
	indices.Add(v3);
	return indices.Num() / 3;
}
ERet MeshBuilder::End( int &verts, int &tris )
{
	verts = vertices.Num();
	tris = indices.Num();
	return ALL_OK;
}


//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
