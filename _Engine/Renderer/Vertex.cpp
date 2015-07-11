#include "Renderer/Renderer_PCH.h"
#pragma hdrstop
#include <Renderer/Vertex.h>

void DrawVertex::BuildVertexDescription( VertexDescription & _description )
{
	_description.Begin();
	_description.Add(AttributeType::Float, 3, VertexAttribute::Position, false, 0);
	_description.Add(AttributeType::Half,  2, VertexAttribute::TexCoord0, false, 0);
	_description.Add(AttributeType::UByte, 4, VertexAttribute::Normal, false, 0);
	_description.Add(AttributeType::UByte, 4, VertexAttribute::Tangent, false, 0);
	_description.Add(AttributeType::UByte, 4, VertexAttribute::BoneIndices, false, 0);
	_description.Add(AttributeType::UByte, 4, VertexAttribute::BoneWeights, true, 0);
	_description.End();
}
#if 0

HInputLayout gs_inputLayouts[VertexType::Count];

void CreateVertexFormats()
{
	memset(gs_inputLayouts, LLGL_NULL_HANDLE, sizeof(gs_inputLayouts));

	// P3f_TEX2f_N4Ub_T4Ub
	{
		VertexDescription	vertexDesc;
		{
			vertexDesc.Begin();
			vertexDesc.Add(AttributeType::Float, 3, VertexAttribute::Position, false, 0);
			vertexDesc.Add(AttributeType::Float, 2, VertexAttribute::TexCoord0, false, 1);
			vertexDesc.Add(AttributeType::UByte, 4, VertexAttribute::Normal, false, 1);
			vertexDesc.Add(AttributeType::UByte, 4, VertexAttribute::Tangent, false, 1);
			vertexDesc.End();
		}
		gs_inputLayouts[VertexType::Static] = llgl::CreateInputLayout( vertexDesc, "Static" );
	}
}

void DeleteVertexFormats()
{
	for( UINT i = 0; i < mxCOUNT_OF(gs_inputLayouts); i++ )
	{
		if( gs_inputLayouts[i].IsValid() )
		{
			llgl::DeleteInputLayout(gs_inputLayouts[i]);
			gs_inputLayouts[i].SetNil();
		}
	}
}
#endif

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
