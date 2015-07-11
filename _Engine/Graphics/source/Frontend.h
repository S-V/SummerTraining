// renderer front-end
#pragma once

#include <Graphics/Device.h>

namespace llgl
{

// At the start of the frame, the simulation thread begins to fill a render buffer with commands.
// At the same time, the render thread is processing the render buffer from the previous simulation frame.
// At the end of the frame, both threads synchronize and the buffers switch
// (i.e. one buffer can be filled with render commands, while the other is being executed, and vice versa).
// See:
// http://flohofwoe.blogspot.com/2012/08/twiggys-low-level-render-pipeline.html
// http://devmaster.net/forums/topic/9067-multi-threaded-rendering/
//
enum { NUM_COMMAND_BUFFERS = 2 };

// NOTE: order is important for sorting!
enum RENDER_COMMAND_ID
{
	// Create* commands
	RC2_CreateInputLayout,
	RC2_CreateBuffer,
	RC2_CreateTexture,
	RC2_CreateRenderTarget,

	// Update* commands
	CMD_UPDATE_BUFFER,
	RC2_UpdateTexture,
	RC2_CopyResource,
	RC2_GenerateMips,

	// Set* commands
	RC_SET_VIEW,
	RC_SET_RASTERIZER_STATE,
	RC_SET_DEPTH_STENCIL_STATE,
	RC_SET_BLEND_STATE,
	//RC_SET_CBUFFER,		// e.g. Set Constant Buffer
	//RC_SET_SAMPLER,		// e.g. Set Shader Sampler
	//RC_SET_RESOURCE,	// e.g. Set Shader Resource
	//RC_BIND_STREAM,		// SET_VERTEX_BUFFER

	// Draw* commands
	RC_DRAW_BATCH,

	// Retrieve* commands
	RC_RESOLVE_TEXTURE,
	RC_READ_PIXELS,

	// Debugging / Performance
	RC_SET_MARKER,
	RC_PUSH_MARKER,
	RC_POP_MARKER,

	// Delete* commands

	// Special commands
	//RC_END_OF_LIST
};

#if 0
enum ERenderCommand
{
	RC_BeginFrame,	// begin rendering a new frame, reset internal states/counters

	RC_UpdateConstantBuffer,
	RC_UpdateMeshBuffer,

	RC_SetView,	// clear and set render targets/depth stencil
	RC_SetState,	// set rasterizer, depth-stencil and blend states
	RC_DrawBatch,
	//RC_DrawInstanced,

	RC_ReadPixels,	// e.g. read hit test render target

#if LLGL_ENABLE_PERF_HUD
	RC_SetMarker,
	RC_PushMarker,
	RC_PopMarker,
#endif // LLGL_ENABLE_PERF_HUD


	RC_EndOfList,	//<=  finish recording commands
};

#endif

class CommandBuffer
{
	char *	m_data;	// command buffer memory
	UINT32	m_used;	// current write or read offset
	UINT32	m_size;	// size of allocated command buffer memory
	UINT32	_pad16;

public:
	CommandBuffer();

	void Initialize( UINT32 size );
	void Shutdown();

	inline UINT32 GetOffset() const { return m_used; }

	inline const char* At( UINT32 offset ) const
	{
		return mxAddByteOffset(m_data, offset);
	}

	inline void* Alloc( UINT32 size )
	{
		mxASSERT(m_used + size < m_size);
		void* mem = &m_data[m_used];
		m_used += size;
		return mem;
	}
	template< typename TYPE >
	inline TYPE& Alloc()
	{
		void* dest = this->Alloc(sizeof(TYPE));
		return *(TYPE*) dest;
	}

	inline void Put( const void* data, UINT32 size )
	{
		void* dest = this->Alloc(size);
		memcpy( dest, data, size );
	}
	template< typename TYPE >
	inline void Put( const TYPE& value )
	{
		mxASSERT(m_used + sizeof(value) < m_size);
		*(TYPE*)&m_data[m_used] = value;
		m_used += sizeof(value);
	}

	inline void Get( void* data, UINT32 size )
	{
		mxASSERT(m_used + size < m_size);
		memcpy( data, &m_data[m_used], size );
		m_used += size;
	}
	template< typename TYPE >
	inline TYPE Get()
	{
		mxASSERT(m_used + sizeof(TYPE) < m_size);
        char* result = &m_data[m_used];
        m_used += sizeof(TYPE);
        return *(TYPE*)result;
	}

	inline void* GetRef( UINT32 size )
	{
		mxASSERT(m_used + size < m_size);
		void* result = &m_data[m_used];
		m_used += size;
		return result;
	}
	template< typename TYPE >
	inline TYPE* GetRef()
	{
		mxASSERT(m_used + sizeof(TYPE) < m_size);
		char* result = &m_data[m_used];
        m_used += sizeof(TYPE);
        return (TYPE*)result;
	}

	// adjust current pointer to N-byte aligned boundary
	template< UINT ALIGNMENT >
	inline void AlignPointer()
	{
		const UINT32 unalignedOffset = m_used;
		const UINT32 alignedOffset = (unalignedOffset + (ALIGNMENT-1)) & ~(ALIGNMENT-1);
		m_used = alignedOffset;
	}

	inline void WriteToken( UINT32 commandId )
	{
		this->Put( commandId );
	}
	inline UINT32 ReadToken()
	{
		return this->Get< UINT32 >();
	}

	inline void Reset()
	{
		m_used = 0;
	}
};




struct PerfMarkerData
{
	char	text[48];
	float	rgba[4];	// 16 bytes
	// 64 bytes
public:
	PerfMarkerData( const char* name, const float* color = NULL )
	{
		strcpy_s(text, name);
		if( color ) {
			rgba[0] = color[0];
			rgba[1] = color[1];
			rgba[2] = color[2];
			rgba[3] = color[3];
		} else {
			rgba[0] = 1.0f;
			rgba[1] = 1.0f;
			rgba[2] = 1.0f;
			rgba[3] = 1.0f;
		}
	}
	PerfMarkerData( const char* name, const UINT32 color )
	{
		strcpy_s(text, name);
		float f = 1.0f / 255.0f;
		rgba[0] = f * (float) UINT8( color >> 24 );
		rgba[1] = f * (float) UINT8( color >> 16 );
		rgba[2] = f * (float) UINT8( color >> 8 );
		rgba[3] = f * (float) UINT8( color >> 0 );
	}
};

extern ClientInterface *	g_client;

}//namespace llgl

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
