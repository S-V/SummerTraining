/*
=============================================================================
	Graphics mesh used for rendering.
=============================================================================
*/
#pragma once

#include <Core/Asset.h>
#include <Graphics/Device.h>
#include <Graphics/Geometry.h>
#include <Renderer/Renderer.h>
#include <Renderer/Vertex.h>

// Submesh/MeshPart/MeshSection/VertexIndexRange/ModelSurface
// It is a part of mesh typically associated with a single material.
struct rxSubmesh : public CStruct
{
	UINT32	startIndex;	// offset of the first index
	UINT32	indexCount;	// number of indices
	UINT32	baseVertex;	// index of the first vertex
	UINT32	vertexCount;// number of vertices
public:
	mxDECLARE_CLASS( rxSubmesh, CStruct );
	mxDECLARE_REFLECTION;
};

struct MeshHeader_d
{
	UINT32	magic;
	UINT32	version;
	UINT32	flags;
	UINT32	topology;
	UINT32	submeshes;
	UINT32	numVertices;
	UINT32	numIndices;
	AABB24	bounds;	// local-space bounding box

	enum Flags
	{
		USE_32BIT_INDICES = BIT(0)
	};
};

/*
-----------------------------------------------------------------------------
	rxMesh
	represents a renderable mesh; doesn't keep shadow copy in system memory.
	it's basically a collection of hardware mesh buffers used for rendering.
	VBs and IB are created from raw mesh data which is loaded in-place.
-----------------------------------------------------------------------------
*/
struct rxMesh : public CStruct
{
	HBuffer				m_vertexBuffer;
	HBuffer				m_indexBuffer;

	// currently, we use DrawVertex for all kinds of geometry
	//HInputLayout		m_vertexLayout;
	//VertexTypeT			m_vertexFormat;

	UINT8				m_indexStride;	// index buffer format
	TopologyT			m_topology;	// primitive type

	TBuffer< rxSubmesh >	m_parts;

	UINT32					m_numVertices;
	UINT32					m_numIndices;

// this is used only for software skinning
TBuffer< DrawVertex >	vertexData;

	//TArray< float4x4 >	m_bindPoseMatrices;
	//TArray< UINT32 >		m_matrixIndices;
	//TArray< UINT32 >		m_boneParents;	// -1 if root index

	AABB24		m_bounds;	// local-space bounding box

public:
	mxDECLARE_CLASS( rxMesh, CStruct );
	mxDECLARE_REFLECTION;

	rxMesh();
	~rxMesh();

	ERet Create( const RawMeshData& source );

public:
	static AssetTypeT GetAssetType() { return AssetTypes::MESH; }

	static ERet Load( Assets::LoadContext2 & context );
	static ERet Online( Assets::LoadContext2 & context );
	static void Offline( Assets::LoadContext2 & context );
	static void Destruct( Assets::LoadContext2 & context );
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
