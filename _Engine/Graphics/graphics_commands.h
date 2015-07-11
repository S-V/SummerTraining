/*
=============================================================================

=============================================================================
*/
#pragma once

namespace llgl2
{
	// fixed-pipeline states to set before issuing any particular drawcall
	struct RenderState2
	{
		UINT32				blendFactor;	//4 blend factor (rgba)
		HDepthStencilState	depthStencil;	//1
		UINT8				stencilRef;		//1
		HRasterizerState	rasterizer;		//1
		HBlendState			blendState;		//1
	};
	// this is the collection of rendering-data that is necessary for a draw-call.
	struct DrawCall2
	{
		// shader state
		HBuffer			CBs[LLGL_MAX_BOUND_UNIFORM_BUFFERS];	// constant buffers to bind
		HSamplerState	SSs[LLGL_MAX_TEXTURE_UNITS];	// shader samplers to bind
		HResource		SRs[LLGL_MAX_TEXTURE_UNITS];	// shader resources to bind

		HProgram		program;
		// geometry
		HInputLayout	inputLayout;
		TopologyT		topology;	// Topology::Enum

		HBuffer			vertexBuffer;
		HBuffer			indexBuffer;

		// draw call parameters
		UINT32		baseVertex;	//4 index of the first vertex
		UINT32		vertexCount;//4
		UINT32		startIndex;	//4 offset of the first index
		UINT32		indexCount;	//4 number of indices

		// rasterizer
		UINT16		scissor;	//2 index of the scissor rectangle
	public:
		void Reset();	// initializes with default values
	};

	// allocates a new drawcall
	DrawCall2* NewBatch2();
	void Submit2( DrawCall2* batch );
}//namespace llgl2

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
