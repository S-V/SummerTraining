#pragma once

#if LLGL_Driver == LLGL_Driver_Direct3D_11

// Include headers for DirectX 11 programming.
#include "d3d_common.h"

#include <D3Dcommon.h>
#include <D3Dcompiler.h>
#include <D3D11.h>
#include <D3DX11.h>

#if MX_AUTOLINK
	#pragma comment( lib, "d3d11.lib" )
	#if MX_DEBUG
		#pragma comment( lib, "d3dx11d.lib" )
	#else
		#pragma comment( lib, "d3dx11.lib" )
	#endif
#endif // MX_AUTOLINK


#include <DXGI.h>

#if MX_AUTOLINK
	#pragma comment (lib, "dxgi.lib")
	#pragma comment( lib, "dxguid.lib" )
	#pragma comment( lib, "d3dcompiler.lib" )
	#pragma comment( lib, "winmm.lib" )
	#pragma comment( lib, "comctl32.lib" )
#endif // MX_AUTOLINK

#include "Backend.h"

/*
=====================================================================
	CORE RUN-TIME
=====================================================================
*/
namespace llgl
{
	struct ColorTargetD3D11
	{
		ID3D11Texture2D *		m_colorTexture;
		ID3D11RenderTargetView *	m_RTV;
		ID3D11ShaderResourceView *	m_SRV;
		UINT16		m_width;
		UINT16		m_height;
		UINT8		m_format;	// DXGI_FORMAT
		UINT8		m_flags;
	public:
		ColorTargetD3D11();
		void Create( const ColorTargetDescription& rtInfo );
		void Destroy();
	};
	struct DepthTargetD3D11
	{
		ID3D11Texture2D *		m_depthTexture;
		ID3D11DepthStencilView *	m_DSV;
		ID3D11ShaderResourceView *	m_SRV;
		UINT16		m_width;
		UINT16		m_height;
		UINT32		m_flags;
	public:
		DepthTargetD3D11();
		void Create( const DepthTargetDescription& dtInfo );
		void Destroy();
	};

	struct TextureD3D11
	{
		union {
			ID3D11Resource *		m_resource;
			ID3D11Texture2D *		m_texture2D;
			ID3D11Texture3D *		m_texture3D;
		};
		ID3D11ShaderResourceView *	m_SRV;
		//TextureTypeT	m_type;
		UINT8			m_numMips;
	public:
		TextureD3D11();
		void Create( const void* _data, UINT _size );
		void Create( const Texture2DDescription& txInfo, const void* imageData = NULL );
		void Create( const Texture3DDescription& txInfo, const Memory* data = NULL );
		void Destroy();
	private:
		void CreateInternal( const TextureImage& _image, bool _dynamic = false );
	};

	enum EShaderResourceType
	{
		SRT_Buffer,
		SRT_Texture,
		SRT_ColorSurface,
		SRT_DepthSurface,
		SRT_UnorderedAccess,

		SRT_NumBits = 3
	};

	struct DepthStencilStateD3D11
	{
		ID3D11DepthStencilState *	m_ptr;
		//UINT32						m_checksum;
	public:
		DepthStencilStateD3D11();
		void Create( const DepthStencilDescription& dsInfo );
		void Destroy();
	};
	struct RasterizerStateD3D11
	{
		ID3D11RasterizerState *		m_ptr;
		//UINT32						m_checksum;
	public:
		RasterizerStateD3D11();
		void Create( const RasterizerDescription& rsInfo );
		void Destroy();
	};
	struct SamplerStateD3D11
	{
		ID3D11SamplerState *	m_ptr;
		//UINT32					m_checksum;
	public:
		SamplerStateD3D11();
		void Create( const SamplerDescription& ssInfo );
		void Destroy();
	};
	struct BlendStateD3D11
	{
		ID3D11BlendState *	m_blendState;
		//UINT32				m_checksum;
	public:
		BlendStateD3D11();
		void Create( const BlendDescription& bsInfo );
		void Destroy();
	};

	struct InputLayoutD3D11
	{
		ID3D11InputLayout *	m_layout;
		UINT32	m_checksum;	// for detecting duplicate input layouts
		UINT8	m_numSlots;	// number of input slots (vertex streams)
		UINT8	m_strides[LLGL_MAX_VERTEX_STREAMS];	// strides of each vertex buffer stream
	public:
		InputLayoutD3D11();
		void Create( const VertexDescription& desc, UINT32 hash );
		void Destroy();
	};

	struct BufferD3D11
	{
		ID3D11Buffer *		m_ptr;
	public:
		BufferD3D11();
		void Destroy();
	};

	struct StructuredBufferD3D11
	{
		ID3D11Buffer *				m_ptr;
		ID3D11ShaderResourceView *	m_SRV;
		ID3D11UnorderedAccessView *	m_UAV;
	public:
		StructuredBufferD3D11();
		void Destroy();
	};

	struct ShaderD3D11
	{
		// pointer to created shader object (e.g.: ID3D11VertexShader, ID3D11PixelShader, etc)
		union {
			ID3D11VertexShader *	m_VS;
			ID3D11HullShader *		m_HS;
			ID3D11DomainShader *	m_DS;
			ID3D11GeometryShader *	m_GS;
			ID3D11PixelShader *		m_PS;
			ID3D11ComputeShader *	m_CS;
			ID3D11DeviceChild *		m_ptr;
		};
	public:
		ShaderD3D11();
		void Create( EShaderType shaderType, const void* compiledBytecode, UINT bytecodeLength );
		void Destroy();
	};
	struct ProgramD3D11 : CStruct
	{
		HShader	VS;
		HShader	HS;
		HShader	DS;
		HShader	GS;
		HShader	PS;
		UINT16	_pad12;
		UINT32	_pad16;	// pad to 16 bytes
#if LL_DEBUG_LEVEL >= 3
		String name;
		ProgramBindingsOGL bindings;	// for debugging
#endif
	public:
		mxDECLARE_CLASS(ProgramD3D11, CStruct);
		mxDECLARE_REFLECTION;
		ProgramD3D11();
		void Destroy();
	};

	class ScopedLock_D3D11
	{
		D3D11_MAPPED_SUBRESOURCE		m_mappedData;
		dxPtr< ID3D11DeviceContext >	m_pD3DContext;
		dxPtr< ID3D11Resource >			m_pDstResource;
		const UINT						m_iSubResource;
	public:
		inline ScopedLock_D3D11( ID3D11DeviceContext* pDeviceContext, ID3D11Resource* pDstResource, D3D11_MAP eMapType, UINT iSubResource = 0, UINT nMapFlags = 0 )
			: m_iSubResource( iSubResource )
		{
			m_pD3DContext = pDeviceContext;
			m_pDstResource = pDstResource;
			dxCHK(pDeviceContext->Map( pDstResource, iSubResource, eMapType, nMapFlags, &m_mappedData ));
		}
		inline ~ScopedLock_D3D11()
		{
			m_pD3DContext->Unmap( m_pDstResource, m_iSubResource );
		}
		template< typename TYPE >
		inline TYPE As()
		{
			return static_cast< TYPE >( m_mappedData.pData );
		}
		inline void* ToVoidPtr()
		{
			return m_mappedData.pData;
		}
		inline D3D11_MAPPED_SUBRESOURCE& GetMappedData()
		{
			return m_mappedData;
		}
	};

	// ideally, these should be
	// D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT
	// D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT
	// D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT
	// D3D11_PS_CS_UAV_REGISTER_COUNT
	// and so on...
	enum {
		MAX_CREATED_RENDER_STATES = 256,
		MAX_CREATED_RENDER_TARGETS = 64,
		MAX_CREATED_TEXTURES = 4096,
		MAX_CREATED_INPUT_LAYOUTS = 16,
		MAX_CREATED_MESHES = 4096,
		MAX_CREATED_SHADERS = 8192,
	};

	struct PerfCounters
	{
		// time taken by the render thread to wait for the submitter thread, in milliseconds
		UINT64	waitForSubmitMsec;

		UINT64	waitForRenderMsec;
	};

	struct DeviceContext
	{
		ID3D11DeviceContext *	m_deviceContext;

		//UINT32					m_renderTargets;	// RT mask

		Viewport				m_viewport;

		// currently set render states
		HRasterizerState		m_rasterizerState;
		HDepthStencilState		m_depthStencilState;
		UINT8					m_stencilReference;
		HBlendState				m_blendState;
		UINT32					m_sampleMask;
		float					m_blendFactor[4];

		// currently bound shader programs
		HProgram				m_currentProgram;
		HShader					m_currentShaders[ShaderTypeCount];

		// currently bound input layout
		HInputLayout	m_currentInputLayout;

		// Array of offset values; one offset value for each buffer in the vertex-buffer array. Each offset is the number of bytes between the first element of a vertex buffer and the first element that will be used.
		// NOTE: currently, we always fetch vertices starting from zero.
		UINT    m_streamOffsets[LLGL_MAX_VERTEX_STREAMS];
		// Array of stride values; one stride value for each buffer in the vertex-buffer array. Each stride is the size (in bytes) of the elements that are to be used from that vertex buffer.
		UINT    m_streamStrides[LLGL_MAX_VERTEX_STREAMS];
		// The number of vertex buffers in the array.
		UINT    m_numInputSlots;

		// currently bound index buffer
		HBuffer	m_currentIndexBuffer;

		UINT	m_primitiveTopology;	// Topology::Enum

	public:
		DeviceContext();
		~DeviceContext();

		void Initialize( ID3D11DeviceContext* context11 );
		void Shutdown();

		void ResetState();

		void SubmitView( const ViewState& view );

		void UpdateBuffer( HBuffer handle, UINT32 start, const void* data, UINT32 size );
		void UpdateTexture2( HTexture handle, const void* data, UINT32 size );
		void* MapBuffer( HBuffer _handle, UINT32 _start, EMapMode _mode, UINT32 _size );
		void UnmapBuffer( HBuffer _handle );
		//void CopyResource( source, destination );
		void GenerateMips( HColorTarget target );
		void ResolveTexture( HColorTarget target );
		void ReadPixels( HColorTarget source, void *destination, UINT32 bufferSize );

		void SetRasterizerState( HRasterizerState rasterizerState );
		void SetDepthStencilState( HDepthStencilState depthStencilState, UINT8 stencilReference );
		void SetBlendState( HBlendState blendState, const float* blendFactor = NULL, UINT32 sampleMask = ~0 );

		void SubmitBatch( const DrawCall& batch );

		void EndFrame();
	};

}//namespace llgl

#endif // LLGL_Driver == LLGL_Driver_Direct3D_11

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
