#include "stdafx.h"
#pragma hdrstop
#if 0
#include <External/assimp/Importer.hpp>
#include <External/assimp/Postprocess.h>
#include <External/assimp/Scene.h>
#include <External/assimp/DefaultLogger.hpp>
#include <External/assimp/LogStream.hpp>
#if MX_AUTOLINK
#pragma comment( lib, "assimp.lib" )
#endif //MX_AUTOLINK

#include <Meshok/MeshImporter.h>

namespace Meshok
{

static inline Float3 aiVector3D_To_Float3( const aiVector3D& assVec )
{
	return Float3_Set( assVec.x, assVec.y, assVec.z );
}
static inline Vector4 aiQuaternion_To_Vector4( const aiQuaternion& assQuat )
{
	return Vector4_Set( assQuat.x, assQuat.y, assQuat.z, assQuat.w );
}

static void CalculateVertexIndexCount( const aiScene* scene, UINT *numVertices, UINT *numIndices )
{
	*numVertices = 0;
	*numIndices = 0;
	for( UINT iMesh = 0; iMesh < scene->mNumMeshes; iMesh++ )
	{
		const aiMesh* mesh = scene->mMeshes[ iMesh ];
		*numVertices += mesh->mNumVertices;
		*numIndices += mesh->mNumFaces * 3;
	}
}

ERet ImportMesh( AStreamReader& _source, TcMeshData &_output, const char* _hint )
{
	class MyLogger : public Assimp::Logger {
	public:
		virtual bool attachStream(Assimp::LogStream *pStream, unsigned int severity = Debugging | Err | Warn | Info) override
		{
			return true;
		}
		virtual bool detatchStream(Assimp::LogStream *pStream, unsigned int severity = Debugging | Err | Warn | Info) override
		{
			return true;
		}
		virtual void OnDebug(const char* message) override
		{
			//mxGetLog().Logf(LL_Info, "[ASSIMP]: %s\n", message);
		}
		virtual void OnInfo(const char* message) override
		{
			//mxGetLog().Logf(LL_Info, "[ASSIMP]: %s\n", message);
		}
		virtual void OnWarn(const char* message) override
		{
			mxGetLog().PrintF(LL_Warn, "[ASSIMP]: %s\n", message);
		}
		virtual void OnError(const char* message) override
		{
			mxGetLog().PrintF(LL_Info, "[ASSIMP ERROR]: %s\n", message);
		}
	};

	Assimp::DefaultLogger::set( new MyLogger() );

	Assimp::Importer importer;


	UINT32 assimpFlags = 0;


	// DirectX space
	//assimpFlags |= aiProcess_ConvertToLeftHanded;

	// Polygons only
//	assimpFlags |= aiProcess_FindDegenerates | aiProcess_SortByPType;
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

	/** @brief Configures the #aiProcess_PretransformVertices step to normalize
	*  all vertex components into the [-1,1] range. That is, a bounding box
	*  for the whole scene is computed, the maximum component is taken and all
	*  sets are scaled appropriately (uniformly of course!).
	*  This might be useful if you don't know the spatial dimension of the input 
	*  data*/
	//importer.SetPropertyInteger(AI_CONFIG_PP_PTV_NORMALIZE, 1);

	// Triangles only
	assimpFlags |= aiProcess_Triangulate;

	// Generate UVs
	assimpFlags |= aiProcess_GenUVCoords | aiProcess_TransformUVCoords;
	// DirectX tex coords
	assimpFlags |= aiProcess_FlipUVs;


	//assimpFlags |= aiProcess_FlipWindingOrder;


	// Generate mesh with normals and tangents
	assimpFlags |= aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace;

	// Set the maximum number of bones affecting a single vertex
	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, MAX_INFLUENCES);


	// aiProcess_FindInvalidData removes redundant animation key frames,
	// meaning you will get a different number of scaling, rotation and translation keys,
	// forcing you to even them out yourself.
	//assimpFlags |= ~aiProcess_FindInvalidData;	// so that we get the same number of key frames for all channels

	EOptLevel _level = OL_Fast;
	switch( _level ) {
		case OL_None :
			break;
		case OL_Fast :
			assimpFlags = aiProcessPreset_TargetRealtime_Fast;
			break;
		case OL_High :
			// Reduce mesh & material count
			assimpFlags = aiProcessPreset_TargetRealtime_Quality;
			break;
		case OL_Max :
			assimpFlags = aiProcessPreset_TargetRealtime_MaxQuality;
			// optimize for rendering
			assimpFlags |= aiProcess_OptimizeGraph;
			break;
	}

	// don't use aiProcess_PreTransformVertices, as this will remove the bones data.
	//assimpFlags &= ~aiProcess_PreTransformVertices;
	// 
	assimpFlags |= aiProcess_PreTransformVertices;
	assimpFlags |= aiProcess_OptimizeMeshes;


	ByteBuffer	blob;
	mxDO(Util_LoadStreamToBlob( _source, blob ));


	ptPRINT("Compiling mesh: '%s'...\n", _hint );

	const ULONG startTimeMSec = mxGetTimeInMilliseconds();

	//const aiScene* scene = importer.ReadFile( filePath, assimpFlags );
	const aiScene* pScene = importer.ReadFileFromMemory( blob.ToPtr(), blob.GetDataSize(), assimpFlags, _hint );

	if( !pScene )
	{
		ptERROR("Failed to import mesh from file '%s'\n", _hint );
		return ERR_UNKNOWN_ERROR;
	}

	//*****************************************************************
	//*** Extract mesh data
	//*****************************************************************

#if 0
	// collect all nodes and store them in a map (for quick searching by name)
	NodeMapT	nodesByName;	// node name => node pointer
	GatherNodesRecursively( pScene->mRootNode, nodesByName );
	//BoneMapT	bonesByName;
	//GatherOriginalBones( pScene, bonesByName );
#endif

	const UINT nMeshCount = pScene->mNumMeshes;

	_output.sets.Reserve( nMeshCount );

	AABB24_Clear( &_output.bounds );

#if 0
	StringSetT namesOfAllBones;
	//GatherBoneNames( pScene, nodesByName, namesOfAllBones );
	GatherNodeNames( pScene->mRootNode, namesOfAllBones );


	const aiMatrix4x4 rootNodeTransform = pScene->mRootNode->mTransformation;

	TArray< BoneDesc* >	bonesArray;
	BuildSkeleton(pScene->mRootNode, namesOfAllBones, rootNodeTransform, -1, bonesArray);

	const UINT nBoneCount = bonesArray.Num();

	DBGOUT("Count: aiNode = %lu, aiBone = %lu, aiMesh = %u, TcBone = %u\n",
		nodesByName.size(), Calculate_aiBone_Count(pScene), nMeshCount, nBoneCount);

	// for performance reasons (to avoid branches, etc.)
	// the skeleton should have only one root joint at index 0
	if( nBoneCount )
	{
		BoneDesc* rootBone = bonesArray[0];
		mxASSERT(rootBone->parentIndex == -1);

		//NodeMapT::const_iterator nodeIt = nodesByName.find(rootBone->name.c_str());
		//mxASSERT( nodeIt != nodesByName.end() );
		//if( nodeIt->second->mParent )
		//{
		//	const aiMatrix4x4 rootTransform = CalculateGlobalTransform(nodeIt->second->mParent);
		//	if( rootTransform != identityMatrix )
		//	{
		//		aiMatrix4x4 inverseRootTransform(rootTransform);
		//		inverseRootTransform.Inverse();

		//		for( UINT iBone = 0; iBone < nBoneCount; iBone++ )
		//		{
		//			BoneDesc* boneDesc = bonesArray[ iBone ];
		//			boneDesc->globalTransform = boneDesc->globalTransform * inverseRootTransform;
		//		}
		//	}
		//}

		aiMatrix4x4 inverseRootTransform(pScene->mRootNode->mTransformation);
		inverseRootTransform.Inverse();

		//rootBone->globalTransform = rootBone->globalTransform * inverseRootTransform;
		for( UINT iBone = 0; iBone < nBoneCount; iBone++ )
		{
			BoneDesc* boneDesc = bonesArray[ iBone ];
		//	boneDesc->globalTransform = boneDesc->globalTransform * inverseRootTransform;
		}

		//NodeMapT::const_iterator nodeIt = nodesByName.find(rootBone->name.c_str());
		//mxASSERT( nodeIt != nodesByName.end() );
		//if( nodeIt->second->mParent )
		//{
		//	const aiMatrix4x4 rootTransform = CalculateGlobalTransform(nodeIt->second->mParent);
		//	if( rootTransform != identityMatrix )
		//	{
		//		aiMatrix4x4 inverseRootTransform(rootTransform);
		//		inverseRootTransform.Inverse();

		//		for( UINT iBone = 0; iBone < nBoneCount; iBone++ )
		//		{
		//			BoneDesc* boneDesc = bonesArray[ iBone ];
		//			boneDesc->globalTransform = boneDesc->globalTransform * inverseRootTransform;
		//		}
		//	}
		//}
	}

	TcSkeleton& skeleton = _output.skeleton;
	skeleton.bones.SetNum(nBoneCount);

	for( UINT iBone = 0; iBone < nBoneCount; iBone++ )
	{
		const BoneDesc* boneDesc = bonesArray[ iBone ];
		const aiNode* node = boneDesc->node;
		mxASSERT_PTR(node);

		TcBone& bone = skeleton.bones[ iBone ];

		Str::CopyS(bone.name, node->mName.C_Str());

		bone.parent = boneDesc->parentIndex;

		// The bone's transformation in the skeleton space,
		// aka the bind matrix - the bone's parent's local matrices concatenated with the bone's local matrix.
		const aiMatrix4x4 nodeGlobalTransform = boneDesc->globalTransform;
		//mxASSERT(nodeGlobalTransform == CalculateGlobalTransform(node));

		//aiMatrix4x4 inverseGlobalTransform(globalTransform);
		//inverseGlobalTransform.Inverse();

		// The transformation relative to the bone's parent (parent => bone space).
		aiMatrix4x4 nodeLocalTransform;
		if( boneDesc->parentIndex != -1 ) {
			const BoneDesc* parentBone = bonesArray[ boneDesc->parentIndex ];
			aiMatrix4x4 parentGlobalTransform = parentBone->globalTransform;
			aiMatrix4x4 inverseParentGlobalTransform(parentGlobalTransform);
			inverseParentGlobalTransform.Inverse();
			nodeLocalTransform = nodeGlobalTransform * inverseParentGlobalTransform;	// N = P^-1 * B
		} else {
			nodeLocalTransform = node->mTransformation;
		}

		aiVector3D		scaling;
		aiQuaternion	rotation;
		aiVector3D		translation;
		nodeLocalTransform.Decompose(scaling, rotation, translation);

		bone.translation = aiVector3D_To_Float3(translation);
		bone.rotation = aiQuaternion_To_Vector4(rotation);

		ptPRINT("Bone[%u]: '%s', parent = %d (P = %.3f, %.3f, %.3f, Q = %.3f, %.3f, %.3f, %.3f)\n",
			iBone, bone.name.ToPtr(), bone.parent, translation.x, translation.y, translation.z, rotation.x, rotation.y, rotation.z, rotation.w);
	}


	TArray< TArray< VertexDesc > >	skinningInfo;
	skinningInfo.SetNum(nMeshCount);
#endif

	for( UINT iMesh = 0; iMesh < nMeshCount; iMesh++ )
	{
		const aiMesh* pMesh = pScene->mMeshes[ iMesh ];

		const UINT nTexCoordsCount = pMesh->GetNumUVChannels();

		if( !pMesh->HasFaces() ) {
			ptPRINT("[ASSIMP]: mesh doesn't have faces\n");
			continue;
		}
		if( pMesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE ) {
			ptPRINT("[ASSIMP]: mesh doesn't consist of triangles\n");
			continue;
		}
		if( !pMesh->HasPositions() ) {
			ptPRINT("[ASSIMP]: mesh doesn't have vertex positions\n");
			continue;
		}
		if( !pMesh->HasNormals() ) {
			ptPRINT("[ASSIMP]: mesh doesn't have vertex normals\n");
			continue;
		}
		if( !pMesh->HasTangentsAndBitangents() ) {
			ptPRINT("[ASSIMP]: mesh doesn't have tangents and bitangents\n");
			continue;
		}
		if( !nTexCoordsCount || !pMesh->HasTextureCoords(0) ) {
			ptPRINT("[ASSIMP]: mesh doesn't have texture coordinates\n");
			continue;
		}
		//if( !pMesh->HasVertexColors(0) ) {
		//	ptPRINT("[ASSIMP]: mesh doesn't have vertex colors\n");
		//	return ERR_OBJECT_NOT_FOUND;
		//}

		const std::string sMeshName( pMesh->mName.C_Str() );

		ptPRINT("Mesh[%u]: '%s' (%u verts, %u tris, %u bones\n",
			iMesh, sMeshName.c_str(), pMesh->mNumVertices, pMesh->mNumFaces, pMesh->mNumBones);


		const UINT nVertexCount = pMesh->mNumVertices;
		const UINT nIndexCount = pMesh->mNumFaces * 3;

		TcTriMesh & newSubmesh = _output.sets.Add();

		AABB24_Clear( &newSubmesh.aabb );

		Str::CopyS(newSubmesh.name, sMeshName.c_str());

		//*****************************************************************
		//*** Collect vertices
		//*****************************************************************

		newSubmesh.positions.SetNum( nVertexCount );
		newSubmesh.texCoords.SetNum( nVertexCount );
//		newSubmesh.tangents.SetNum( nVertexCount );
//		newSubmesh.binormals.SetNum( nVertexCount );
		newSubmesh.normals.SetNum( nVertexCount );
		//newSubmesh.colors.SetNum( nVertexCount );
		//newSubmesh.weights.SetNum( nVertexCount );

		for( UINT iVertex = 0; iVertex < nVertexCount; iVertex++ )
		{
			const aiVector3D& UVW = pMesh->mTextureCoords[0][ iVertex ];

			newSubmesh.positions[ iVertex ] = aiVector3D_To_Float3( pMesh->mVertices[ iVertex ] );
			newSubmesh.texCoords[ iVertex ] = Float2_Set( UVW.x, UVW.y );			
//			newSubmesh.tangents	[ iVertex ] = aiVector3D_To_Float3( pMesh->mTangents[ iVertex ] );
//			newSubmesh.binormals[ iVertex ] = aiVector3D_To_Float3( pMesh->mBitangents[ iVertex ] );
			newSubmesh.normals	[ iVertex ] = aiVector3D_To_Float3( pMesh->mNormals[ iVertex ] );
			//newSubmesh.colors	[ iVertex ] = ColorToVec4D( pMesh->mColors[0][ iVertex ] );

			AABB24_AddPoint( &newSubmesh.aabb, newSubmesh.positions[iVertex] );
		}

		//*****************************************************************
		//*** Collect indices
		//*****************************************************************

		newSubmesh.indices.SetNum( nIndexCount );

		for( UINT iFace = 0; iFace < pMesh->mNumFaces; iFace++ )
		{
			const aiFace& face = pMesh->mFaces[ iFace ];
			mxASSERT2(face.mNumIndices == 3, "Expected a triangle list!");
			newSubmesh.indices[ iFace*3 + 0 ] = face.mIndices[ 0 ];
			newSubmesh.indices[ iFace*3 + 1 ] = face.mIndices[ 1 ];
			newSubmesh.indices[ iFace*3 + 2 ] = face.mIndices[ 2 ];
		}

		AABB24_AddAABB( &_output.bounds, newSubmesh.aabb );

		//*****************************************************************
		//*** Collect skinning info
		//*****************************************************************

#if 0
		TArray< VertexDesc >& verts = skinningInfo[iMesh];
		verts.SetNum(nVertexCount);

		const unsigned int nBoneCount = pMesh->mNumBones;
		for( unsigned int iBone = 0; iBone < nBoneCount; iBone++ )
		{
			const aiBone* pBone = pMesh->mBones[ iBone ];

			const int boneIndex = skeleton.FindBoneIndexByName(pBone->mName.C_Str());
			mxASSERT(boneIndex != -1);

			for( unsigned int iWeight = 0; iWeight < pBone->mNumWeights; iWeight++ )
			{
				const aiVertexWeight& weight = pBone->mWeights[ iWeight ];
				VertexDesc& vertDesc = verts[ weight.mVertexId ];
				Weight& newWeight = vertDesc.weights.Add();
				newWeight.boneIndex = (UINT) boneIndex;
				newWeight.boneWeight = weight.mWeight;
			}
		}//for each bone

		for( UINT i = 0; i < verts.Num(); i++ )
		{
			VertexDesc& vertDesc = verts[i];
			//mxASSERT(vertDesc.weights.Num() <= MAX_INFLUENCES);
			if( vertDesc.weights.Num() > MAX_INFLUENCES ) {
				ptWARN("Vertex[%u] (mesh:%u) has too many (%u) bone influences!\n", i, iMesh, vertDesc.weights.Num());
				continue;
			}

			float sum = 0.0f;
			for( UINT iWeight = 0; iWeight < vertDesc.weights.Num(); iWeight++ )
			{
				Weight& newWeight = vertDesc.weights[ iWeight ];
				sum += newWeight.boneWeight;
			}
			const float epsilon = 1e-4f;
			if( sum > 1.0f+epsilon ) {
				ptWARN("Sum of bone influences (%.3f) exceeds 1.0f!\n", sum);
			}

			for( UINT iWeight = 0; iWeight < vertDesc.weights.Num(); iWeight++ )
			{
				Weight& newWeight = vertDesc.weights[ iWeight ];
				newWeight.boneWeight /= sum;
			}

			TcWeights& dstWeights = newSubmesh.weights[i];
			dstWeights.SetNum(vertDesc.weights.Num());
			for( UINT iWeight = 0; iWeight < dstWeights.Num(); iWeight++ )
			{
				dstWeights[iWeight].boneIndex = vertDesc.weights[iWeight].boneIndex;
				dstWeights[iWeight].boneWeight = vertDesc.weights[iWeight].boneWeight;
			}
		}//for each vertex
#endif
	}// for each mesh

#if 0
	const UINT nAnimCount = pScene->mNumAnimations;
	_output.animations.SetNum( nAnimCount );
	for( UINT iAnimIndex = 0; iAnimIndex < nAnimCount; iAnimIndex++ )
	{
		const aiAnimation* pAnim = pScene->mAnimations[iAnimIndex];
		TcAnimation& newAnim = _output.animations[iAnimIndex];

		Str::CopyS(newAnim.name, pAnim->mName.C_Str());

		newAnim.duration = pAnim->mDuration * pAnim->mTicksPerSecond;
UNDONE;
		newAnim.numFrames = 0;

		//mxASSERT(newAnim.framesPerSecond > 0.0f);
		if( pAnim->mTicksPerSecond <= 0.0f ) {
			ptWARN("Anim[%u] '%s': has 0 ticks per second!\n", iAnimIndex, pAnim->mName.C_Str());
		}

		ptPRINT("Anim[%u] '%s': %u channels\n", iAnimIndex, pAnim->mName.C_Str(), pAnim->mNumChannels);

		newAnim.channels.SetNum(pAnim->mNumChannels);
		for( UINT iAnimChannel = 0; iAnimChannel < pAnim->mNumChannels; iAnimChannel++ )
		{
			const aiNodeAnim* pAnimChannel = pAnim->mChannels[ iAnimChannel ];
			const std::string sNodeName( pAnimChannel->mNodeName.C_Str() );

			const NodeMapT::iterator nodeIt = nodesByName.find( sNodeName );
			mxASSERT2( nodeIt != nodesByName.end(), "Failed to find node controlled by anim channel!" );

			const StringSetT::iterator boneIt = namesOfAllBones.find( sNodeName );
			if( boneIt == namesOfAllBones.end() ) {
				ptWARN("Failed to find bone '%s' controlled by anim channel [%u]!\n", sNodeName.c_str(), iAnimChannel);
			}

			TcAnimChannel& newAnimChannel = newAnim.channels[ iAnimChannel ];

			Str::CopyS(newAnimChannel.target, sNodeName.c_str());
			newAnimChannel.positionKeys.SetNum(pAnimChannel->mNumPositionKeys);
			newAnimChannel.rotationKeys.SetNum(pAnimChannel->mNumRotationKeys);
			newAnimChannel.scalingKeys.SetNum(pAnimChannel->mNumScalingKeys);

			for( UINT i = 0; i < pAnimChannel->mNumPositionKeys; i++ )
			{
				newAnimChannel.positionKeys[i].data = aiVector3D_To_Float3(pAnimChannel->mPositionKeys[i].mValue);
				newAnimChannel.positionKeys[i].time = (float)pAnimChannel->mPositionKeys[i].mTime;
			}
			for( UINT i = 0; i < pAnimChannel->mNumRotationKeys; i++ )
			{
				newAnimChannel.rotationKeys[i].data = aiQuaternion_To_Vector4(pAnimChannel->mRotationKeys[i].mValue);
				newAnimChannel.rotationKeys[i].time = (float)pAnimChannel->mRotationKeys[i].mTime;
			}
			//for( UINT i = 0; i < pAnimChannel->mNumScalingKeys; i++ )
			//{
			//	newAnimChannel.scalingKeys[i].data = aiVector3D_To_Float3(pAnimChannel->mScalingKeys[i].mValue);
			//	newAnimChannel.scalingKeys[i].time = (float)pAnimChannel->mScalingKeys[i].mTime;
			//}

			//ptPRINT("Channel[%u]: '%s', %u position keys, %u rotation keys, %u scaling keys\n",
			//	iAnimChannel, sNodeName.c_str(), pAnimChannel->mNumPositionKeys, pAnimChannel->mNumRotationKeys, pAnimChannel->mNumScalingKeys);
		}
	}

	if( nBoneCount )
	{
		// for performance reasons (to avoid branches, etc.)
		// the skeleton should have only one root joint at index 0

		const TcBone& rootBone = skeleton.bones[0];

		mxASSERT(rootBone.parent == -1);

		for( UINT i = 1; i < nBoneCount; i++ )
		{
			const TcBone& bone = skeleton.bones[i];
			if( bone.parent < 0 )
			{
				ptWARN("Bone '%s' at index [%u] shouldn't be a root joint!\n", bone.name.ToPtr(), i);
			}
		}
		for( UINT iAnimIndex = 0; iAnimIndex < nAnimCount; iAnimIndex++ )
		{
			const TcAnimation& anim = _output.animations[iAnimIndex];
			//mxASSERT(anim.channels.Num() == nBoneCount);
			//for( UINT iAnimChannel = 0; iAnimChannel < anim.channels.Num(); iAnimChannel++ )
			//{
			//	const AnimationChannel& animChannel = anim.channels[iAnimChannel];
			//	mxASSERT(Str::Equal(animChannel.nodeName, skeleton.bones[iAnimChannel].name));
			//}
		}
	}

	const UINT numOutputMeshes = _output.meshes.Num();

	ptPRINT("%u meshes, %u bones, %u anims\n", numOutputMeshes, nBoneCount, nAnimCount);
#endif

	ptPRINT( "Taken %u msec to import mesh '%s'.\n", mxGetTimeInMilliseconds() - startTimeMSec, _hint );

	Assimp::DefaultLogger::kill();

	return ALL_OK;
}

ERet ImportMeshFromFile( const char* _path, TcMeshData &_mesh )
{
	FileReader	stream;
	mxDO(stream.Open(_path));
	return ImportMesh(stream, _mesh, _path);
}

}//namespace Meshok
#endif

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
