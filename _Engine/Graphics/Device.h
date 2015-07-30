/*
=============================================================================
	File:	Device.h
	Desc:	Abstract low-level graphics API, header file.
	Note:	based on Branimir Karadzic's bgfx library and Humus's Framework3
	ToDo:	merge all booleans into bit masks
			cmd - command buffer API? LibCMD?
	Docs:

	Stateless, layered, multi-threaded rendering
	http://molecularmusings.wordpress.com/2014/11/06/stateless-layered-multi-threaded-rendering-part-1/
	http://molecularmusings.wordpress.com/2014/11/13/stateless-layered-multi-threaded-rendering-part-2-stateless-api-design/
	https://molecularmusings.wordpress.com/2014/12/16/stateless-layered-multi-threaded-rendering-part-3-api-design-details/

	Implementing a Graphic Driver Abstraction
	http://entland.homelinux.com/blog/2008/03/03/implementing-a-graphic-driver-abstraction/
	Order your graphics draw calls around!	
	http://realtimecollisiondetection.net/blog/?p=86
	Input latency
	http://realtimecollisiondetection.net/blog/?p=30
	Streamlined 3D API for current/upcoming GPU
	http://forum.beyond3d.com/showthread.php?t=63565
	How to make a rendering engine
	http://c0de517e.blogspot.ru/2014/04/how-to-make-rendering-engine.html
	My little rendering engine
	http://c0de517e.blogspot.ru/2009/03/my-little-rendering-engine.html

	http://www.gamedev.ru/code/forum/?id=195323
	http://www.gamedev.ru/code/forum/?id=194691
=============================================================================
*/
#pragma once

#include <Graphics/graphics_types.h>

/*
=====================================================================
	LOW LEVEL GRAPHICS LIBRARY (or LAYER)
=====================================================================
*/
namespace llgl
{
	// LLGL => client communication
	struct ClientInterface {
		//NOTE: these functions must be thread-safe
		virtual void* Alloc( UINT32 size ) = 0;
		virtual void Free( void* memory ) = 0;
		// OpenGL shader cache management
		mxSWIPED("bgfx");
		/// Return size of for cached item. Return 0 if no cached item was
		/// found.
		virtual UINT32 cacheReadSize(UINT64 _id) = 0;
		/// Read cached item.
		virtual bool cacheRead(UINT64 _id, void *_dest, UINT32 _size) = 0;
		/// Write cached item.
		virtual void cacheWrite(UINT64 _id, const void* _data, UINT32 _size) = 0;
	protected:
		virtual ~ClientInterface() {}
	};

	struct Settings
	{
		const void *		window;	// window handle (HWND on Windows)
		ClientInterface *	client;	// client interface
	public:
		Settings();
	};

	ERet Initialize( const Settings& options );
	void Shutdown();

	// returns the immediate device window
	HContext GetMainContext();

	HContext CreateContext();
	void DeleteContext( HContext handle );

	enum VideoModeFlags
	{
		FullScreen	= BIT(1),
		EnableVSync	= BIT(2),
	};
	ERet SetVideoMode( UINT16 width, UINT16 height, UINT32 flags = 0 );

	// resource management
	// Geometry
	HInputLayout CreateInputLayout( const VertexDescription& desc, const char* name = NULL );
	void DeleteInputLayout( HInputLayout layout );

	// Textures
	// creates a texture stored in a native GPU texture format
	HTexture CreateTexture( const void* data, UINT size );
	HTexture CreateTexture2D( const Texture2DDescription& txInfo, const void* imageData = NULL );
	HTexture CreateTexture3D( const Texture3DDescription& txInfo, const Memory* initialData = NULL );
	void DeleteTexture( HTexture tx );

	// render targets
	HColorTarget CreateColorTarget( const ColorTargetDescription& rtInfo );
	void DeleteColorTarget( HColorTarget rt );
	HDepthTarget CreateDepthTarget( const DepthTargetDescription& dtInfo );
	void DeleteDepthTarget( HDepthTarget dt );

	// render states
	HDepthStencilState CreateDepthStencilState( const DepthStencilDescription& dsInfo );
	HRasterizerState CreateRasterizerState( const RasterizerDescription& rsInfo );
	HSamplerState CreateSamplerState( const SamplerDescription& ssInfo );
	HBlendState CreateBlendState( const BlendDescription& bsInfo );

	void DeleteDepthStencilState( HDepthStencilState ds );
	void DeleteRasterizerState( HRasterizerState rs );
	void DeleteSamplerState( HSamplerState ss );
	void DeleteBlendState( HBlendState bs );

	HBuffer CreateBuffer( EBufferType type, UINT32 size, const void* data = NULL );
	void DeleteBuffer( HBuffer handle );

	HShader CreateShader( EShaderType shaderType, const void* compiledBytecode, UINT32 bytecodeLength );
	void DeleteShader( HShader handle );

	// Programmable (Shader) Stages
	HProgram CreateProgram( const ProgramDescription& pd );
	void DeleteProgram( HProgram handle );

	// Shader resources (e.g. for binding textures as shader inputs) (@todo: move creation to initialization time?)
	HResource AsResource( HBuffer br );
	HResource AsResource( HTexture tx );
	HResource AsResource( HColorTarget rt );
	HResource AsResource( HDepthTarget rt );

	// View/Scene pass/layer/stage management
	enum ViewFlags
	{
		ClearColor0	= BIT(0),	// should we clear the color target 0?
		ClearColor1	= BIT(1),	// should we clear the color target 1?
		ClearColor2	= BIT(2),	// should we clear the color target 2?
		ClearColor3	= BIT(3),	// should we clear the color target 3?
		ClearColor4	= BIT(4),	// should we clear the color target 4?
		ClearColor5	= BIT(5),	// should we clear the color target 5?
		ClearColor6	= BIT(6),	// should we clear the color target 6?
		ClearColor7	= BIT(7),	// should we clear the color target 7?
		ClearDepth	= BIT(8),	// should we clear the depth target?
		ClearStencil= BIT(9),	// should we clear the stencil buffer?
		ClearColor	= ClearColor0|ClearColor1|ClearColor2|ClearColor3|ClearColor4|ClearColor5|ClearColor6|ClearColor7,
		ClearAll	= BITS_ALL,
	};
	struct ViewState
	{
		HColorTarget	colorTargets[LLGL_MAX_BOUND_TARGETS];// render targets to bind
		float			clearColors[LLGL_MAX_BOUND_TARGETS][4];// RGBA colors for clearing each RT
		UINT8			targetCount;	// number of color targets to bind
		HDepthTarget	depthTarget;	// optional depth-stencil surface
		Viewport		viewport;	// rectangular destination area
		float			depth;		// value to clear depth buffer with (default=1.)
		UINT16			flags;		// ViewFlags
		UINT8			stencil;	// value to clear stencil buffer with (default=0)
	public:
		ViewState();
		void Reset();	// initializes with default values
	};
	void SubmitView( HContext _context, const ViewState& _view );

	// Update* commands
	void UpdateBuffer( HContext _context, HBuffer _handle, UINT32 _size, const void* _data, UINT32 _start = 0 );

void UpdateTexture2( HContext _context, HTexture _handle, UINT32 _size, const void* _data );

	//void CopyResource( source, destination );
	void GenerateMips( HContext _context, HColorTarget target );
	void ResolveTexture( HContext _context, HColorTarget target );
	void ReadPixels( HContext _context, HColorTarget source, void *destination, UINT32 bufferSize );
	void* MapBuffer( HContext _context, HBuffer _handle, UINT32 _size, EMapMode _mode, UINT32 _start = 0 );
	void UnmapBuffer( HContext _context, HBuffer _handle );

	// configurable states
	void SetRasterizerState( HContext _context, HRasterizerState rasterizerState );
	void SetDepthStencilState( HContext _context, HDepthStencilState depthStencilState, UINT8 stencilReference = 0 );
	void SetBlendState( HContext _context, HBlendState blendState, const float* blendFactor = NULL, UINT32 sampleMask = ~0 );

	// shader inputs binding
	//void SetTexture( HContext _context, UINT8 _stage, HResource _texture, HSamplerState _sampler );
	//void SetCBuffer( HContext _context, UINT8 _slot, HBuffer _buffer );

	// Batch submission (defers the sending of modified state to the GPU until a Draw command is called).
	struct DrawCall
	{
		// shader state
		HBuffer			CBs[LLGL_MAX_BOUND_UNIFORM_BUFFERS];	// constant buffers to bind
		HSamplerState	SSs[LLGL_MAX_TEXTURE_UNITS];	// shader samplers to bind
		HResource		SRs[LLGL_MAX_TEXTURE_UNITS];	// shader resources to bind

		HProgram		program;

		// geometry
		HInputLayout	inputLayout;
		TopologyT		topology;	// Topology::Enum
		UINT8			_unused0;

		HBuffer			VB[LLGL_MAX_VERTEX_STREAMS];
		HBuffer			IB;
		UINT8			b32bit;	// 16 or 32-bit indices?
		UINT8			_unused1;

		UINT32		baseVertex;	//4 index of the first vertex
		UINT32		vertexCount;//4
		UINT32		startIndex;	//4 offset of the first index
		UINT32		indexCount;	//4 number of indices

		// rasterizer
		Rectangle64	scissor;	//8
	public:
		void Clear();
	};
	void Submit( HContext _context, const DrawCall& batch );


	// Frame submission
	ERet NextFrame();

	// performance monitoring tools
	void SetMarker( HContext _context, const wchar_t* markerName, const UINT32 colorRGBA = ~0 );
	void PushMarker( HContext _context, const wchar_t* markerName, const UINT32 colorRGBA = ~0 );
	void PopMarker( HContext _context );

	void SaveScreenshot( const char* _where );

}//namespace llgl

/*
=======================================================================
	UTILITIES
=======================================================================
*/
class ScopedPerfMarker
{
	HContext m_context;
public:
	inline ScopedPerfMarker( HContext _context, const wchar_t* name, const UINT32 rgba = ~0 )
		: m_context( _context )
	{
		llgl::PushMarker( _context, name, rgba );
	}
	inline ~ScopedPerfMarker()
	{
		llgl::PopMarker( m_context );
	}
	PREVENT_COPY(ScopedPerfMarker);
};

#if LLGL_ENABLE_PERF_HUD
	#define gfxMARKER( NAME )	ScopedPerfMarker	perfMarker_##NAME( llgl::GetMainContext(), L#NAME )
#else
	#define gfxMARKER( NAME )
#endif

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
