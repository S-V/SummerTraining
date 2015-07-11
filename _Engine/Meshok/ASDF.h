// Adaptively Sampled Distance Field.
#pragma once

#include <Base/Memory/FreeList/TPool.h>
#include <Meshok/SDF.h>

namespace ASDF
{
	struct Options
	{
		//AABB24	bounds;
		float	radius;
		int		max_depth;
		float	min_subdiv;
		float	error_threshold;
	public:
		Options();
	};
	struct PerfStats
	{

	};

	//NOTE: upper bit = is_leaf flag
	typedef UINT32 NodeID;

	enum { NIL_NODE = (NodeID)~0 };

	inline NodeID MAKE_LEAF_ID( NodeID nodeID ) {
		return nodeID | (1<<31);
	}
	inline bool IS_LEAF_ID( NodeID nodeID ) {
		return (nodeID & (1<<31));
	}
	inline NodeID GET_ID( NodeID nodeID ) {
		return nodeID & ~(1<<31);
	}

	struct Leaf
	{
		float	d[8];
	};
	struct Node
	{
		NodeID	kids[8];
	};

	struct Octree : public CStruct
	{
		TPool< Node >	m_nodes;
		TPool< Leaf >	m_leaves;
	public:
		mxDECLARE_CLASS(Octree, CStruct);
		mxDECLARE_REFLECTION;
		Octree();
		~Octree();
		ERet Build( ATriangleMeshInterface* triangleMesh );
		ERet Build( AVolume* _volume, const Options& _options );
		PREVENT_COPY(Octree);
	};

	struct Volume : AVolume
	{
		TPtr< Octree >	tree;
		float			radius;
	public:
		Volume();
		virtual float GetDistanceAt( const Float3& _position ) const override;
	};

}//namespace ASDF

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
