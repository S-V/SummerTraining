/*
=============================================================================
	Graphics mesh used for rendering.
=============================================================================
*/
#include "Renderer/Renderer_PCH.h"
#pragma hdrstop
#include <Core/Serialization.h>
#include <Graphics/Geometry.h>
#include <Renderer/Mesh.h>
#include <Renderer/Vertex.h>

/*
-----------------------------------------------------------------------------
	rxSubmesh
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS( rxSubmesh );
mxBEGIN_REFLECTION( rxSubmesh )
	mxMEMBER_FIELD( startIndex ),	
	mxMEMBER_FIELD( indexCount ),
	mxMEMBER_FIELD( baseVertex ),
	mxMEMBER_FIELD( vertexCount ),
mxEND_REFLECTION

/*
-----------------------------------------------------------------------------
	rxMesh
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS( rxMesh );
mxBEGIN_REFLECTION( rxMesh )
	//mxMEMBER_FIELD( m_vertexFormat ),
	mxMEMBER_FIELD( m_indexStride ),
	mxMEMBER_FIELD( m_topology ),
	mxMEMBER_FIELD( m_parts ),
	mxMEMBER_FIELD( m_numVertices ),
	mxMEMBER_FIELD( m_numIndices ),
	mxMEMBER_FIELD( m_bounds ),
mxEND_REFLECTION

rxMesh::rxMesh()
{
	m_vertexBuffer.SetNil();
	m_indexBuffer.SetNil();
	//m_vertexFormat = VertexType::Static;
	//m_vertexLayout.SetNil();
	m_indexStride = 0;
	m_topology = Topology::Undefined;
	m_numVertices = 0;
	m_numIndices = 0;
	AABB24_Clear(&m_bounds);
}

rxMesh::~rxMesh()
{
}

static ERet CreateVertexBuffer( rxMesh &mesh, const RawVertexData& source )
{
	const UINT numStreams = source.streams.Num();
	//mxASSERT(numStreams <= mxCOUNT_OF(mesh.m_vertexBuffers));
	//for( UINT iVertexStream = 0; iVertexStream < numStreams; iVertexStream++ )
	//{
	//	const RawVertexStream& streamData = source.streams[ iVertexStream ];
	//	const void* sourceVertexData = streamData.ToVoidPtr();
	//	const UINT vertexBufferSize = streamData.SizeInBytes();
	//	mesh.m_vertexBuffers[ iVertexStream ] = llgl::CreateBuffer( Buffer_Vertex, vertexBufferSize, sourceVertexData );
	//}
	mxASSERT(numStreams == 1);
	const RawVertexStream& streamData = source.streams[ 0 ];
	const void* sourceVertexData = streamData.ToVoidPtr();
	const UINT vertexBufferSize = streamData.SizeInBytes();
	mesh.m_vertexBuffer = llgl::CreateBuffer( Buffer_Vertex, vertexBufferSize, sourceVertexData );
	//mesh.m_vertexFormat = source.type;
	//mesh.m_vertexLayout = gs_inputLayouts[ source.type ];
	mesh.m_numVertices = source.count;
	return ALL_OK;
}

static ERet CreateIndexBuffer( rxMesh &mesh, const RawIndexData& source )
{
	const void* indexData = source.ToVoidPtr();
	const UINT indexDataSize = source.SizeInBytes();
	mesh.m_indexBuffer = llgl::CreateBuffer( Buffer_Index, indexDataSize, indexData );
	mesh.m_indexStride = source.stride;
	mesh.m_numIndices = source.NumIndices();
	return ALL_OK;
}

ERet rxMesh::Create( const RawMeshData& source )
{
	rxMesh &mesh = *this;

	mxDO(CreateVertexBuffer( mesh, source.vertexData ));
	mxDO(CreateIndexBuffer( mesh, source.indexData ));

	mesh.m_topology = Topology::TriangleList;

	mesh.m_parts.SetNum( source.parts.Num() );
	for( UINT iSubmesh = 0; iSubmesh < source.parts.Num(); iSubmesh++ )
	{
		const RawMeshPart & srcSubmesh = source.parts[ iSubmesh ];
		mxASSERT(srcSubmesh.indexCount > 0);
		mxASSERT(srcSubmesh.vertexCount > 0);

		rxSubmesh & dstSubmesh = mesh.m_parts[ iSubmesh ];
		dstSubmesh.startIndex = srcSubmesh.startIndex;
		dstSubmesh.indexCount = srcSubmesh.indexCount;
		dstSubmesh.baseVertex = srcSubmesh.baseVertex;
		dstSubmesh.vertexCount = srcSubmesh.vertexCount;
	}

	return ALL_OK;
}

ERet rxMesh::Load( Assets::LoadContext2 & context )
{
	Clump* clump = context.clump;
	rxMesh* mesh = static_cast< rxMesh* >( context.o );

	MeshHeader_d	header;
	mxDO(context.Get(header));

	mesh->m_indexStride = (header.flags & MeshHeader_d::USE_32BIT_INDICES) ? sizeof(UINT32) : sizeof(UINT16);
	mesh->m_topology = (Topology::Enum) header.topology;

	mesh->m_parts.SetNum(header.submeshes);
	for( UINT32 iSubMesh = 0; iSubMesh < header.submeshes; iSubMesh++ )
	{
		rxSubmesh & subMesh = mesh->m_parts[ iSubMesh ];
		context.Get(subMesh);
	}
	mesh->m_numVertices = header.numVertices;
	mesh->m_numIndices = header.numIndices;
	mesh->m_bounds = header.bounds;

	return ALL_OK;
}
ERet rxMesh::Online( Assets::LoadContext2 & context )
{
	rxMesh* mesh = static_cast< rxMesh* >( context.o );

	const UINT32 vertexBufferSize = mesh->m_numVertices * sizeof(DrawVertex);
	const UINT32 indexBufferSize = mesh->m_numIndices * mesh->m_indexStride;

	{
		ScopedStackAlloc	tempAlloc( gCore.frameAlloc );
		void* vertexData = tempAlloc.AllocA( vertexBufferSize );
		mxDO(context.Read( vertexData, vertexBufferSize ));

		mesh->m_vertexBuffer = llgl::CreateBuffer( Buffer_Vertex, vertexBufferSize, vertexData );
	}
	{
		ScopedStackAlloc	tempAlloc( gCore.frameAlloc );
		void* indexData = tempAlloc.AllocA( indexBufferSize );
		mxDO(context.Read( indexData, indexBufferSize ));

		mesh->m_indexBuffer = llgl::CreateBuffer( Buffer_Index, indexBufferSize, indexData );
	}
	return ALL_OK;
}
void rxMesh::Offline( Assets::LoadContext2 & context )
{
	rxMesh* mesh = static_cast< rxMesh* >( context.o );

	if( mesh->m_vertexBuffer.IsValid() ) {
		llgl::DeleteBuffer(mesh->m_vertexBuffer);
		mesh->m_vertexBuffer.SetNil();
	}
	if( mesh->m_indexBuffer.IsValid() ) {
		llgl::DeleteBuffer(mesh->m_indexBuffer);
		mesh->m_indexBuffer.SetNil();
	}
}
void rxMesh::Destruct( Assets::LoadContext2 & context )
{

}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
