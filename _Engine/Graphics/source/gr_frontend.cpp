// low-level platform-agnostic graphics layer
#include "Graphics/Graphics_PCH.h"
#pragma hdrstop
#include <algorithm>	// std::stable_sort
#include <Core/ObjectModel.h>
#include "Frontend.h"
#include "Backend.h"

#if (LLGL_Driver == LLGL_Driver_Direct3D_11)
	#include "Driver_D3D11.h"
#endif

#if (LLGL_Driver == LLGL_Driver_OpenGL_4plus)
	#include "Driver_OpenGL4.h"
#endif

//#define DBG_CODE(...)	(__VA_ARGS__)

namespace llgl
{
	struct FrontEndData
	{
		//ViewState		currentView;    // the view being submitted
		//BatchData		currentBatch;	// the batch being submitted

		UINT			numBatches;

		CommandBuffer	commands;	// per-frame command data ("batch-breakers" + associated data)

		UINT32			frameCount;	// total number of rendered frames

		UINT32			videoMode;	// VideoModeFlags
		Resolution		resolution;	// back buffer size

		//mxPREALIGN(16) BatchData	batches[LLGL_MAX_DRAW_CALLS] mxPOSTALIGN(16);
	};

	FrontEndData tr;

	ClientInterface *	g_client = NULL;

	struct DefaultCallbacks : ClientInterface
	{
		virtual void* Alloc( UINT32 size ) override
		{
			return mxAlloc(size);
		}
		virtual void Free( void* memory ) override
		{
			mxFree(memory);
		}
		virtual UINT32 cacheReadSize(UINT64 _id) override
		{
			return 0;
		}
		virtual bool cacheRead(UINT64 _id, void* _data, UINT32 _size) override
		{
			return false;
		}
		virtual void cacheWrite(UINT64 _id, const void* _data, UINT32 _size) override
		{
		}
	};

	Settings::Settings()
	{
		window = NULL;
		client = NULL;
	}

	ERet Initialize( const Settings& options )
	{
		chkRET_X_IF_NIL(options.window, ERR_NULL_POINTER_PASSED);

		if( options.client != NULL ) {
			g_client = options.client;
		} else {
			static DefaultCallbacks dummy;
			g_client = &dummy;
		}

		mxDO(driverInitialize(options.window));

		tr.numBatches = 0;

		tr.commands.Initialize(LLGL_COMMAND_BUFFER_SIZE);

		tr.frameCount = 0;

		//mxZERO_OUT(tr.batches);

		tr.videoMode = 0;


#if (mxPLATFORM == mxPLATFORM_WINDOWS)
		HWND hWnd = (HWND) options.window;
		RECT rect;
		::GetClientRect(hWnd, &rect);
		tr.resolution.width = rect.right - rect.left;
		tr.resolution.height = rect.bottom - rect.top;
#else
#		error Unsupported platform!
#endif

DBGOUT("sizeof(ViewState) = %u\n", sizeof(ViewState));
DBGOUT("sizeof(DrawCall) = %u\n", sizeof(DrawCall));

		return ALL_OK;
	}

	void Shutdown()
	{
		driverShutdown();
		tr.commands.Shutdown();
	}

	HContext GetMainContext()
	{
		return driverGetMainContext();
	}

	ERet SetVideoMode( UINT16 width, UINT16 height, UINT32 flags )
	{
		//tr.resolution.width = width;
		//tr.resolution.height = height;
		//tr.resetFlags = flags;
		if(flags != 0)
			UNDONE;
		return ALL_OK;
	}

	HInputLayout CreateInputLayout( const VertexDescription& desc, const char* name )
	{
		mxASSERT_MAIN_THREAD;
		return driverCreateInputLayout(desc, name);
	}

	void DeleteInputLayout( HInputLayout layout )
	{
		mxASSERT_MAIN_THREAD;
		driverDeleteInputLayout(layout);
	}

	HTexture CreateTexture( const void* data, UINT size )
	{
		DBGOUT("CreateTexture(): size=%u\n", size);
		return driverCreateTexture(data, size);
	}
	HTexture CreateTexture2D( const Texture2DDescription& txInfo, const void* imageData )
	{
#if MX_DEBUG
		{LogStream log(LL_Debug);
		log << "CreateTexture2D(): ";
		DBG_DumpFields(&txInfo,txInfo.MetaClass(),log);}
#endif
		return driverCreateTexture2D(txInfo, imageData);
	}
	HTexture CreateTexture3D( const Texture3DDescription& txInfo, const Memory* initialData )
	{
		return driverCreateTexture3D(txInfo, initialData);
	}
	void DeleteTexture( HTexture tx )
	{
		driverDeleteTexture(tx);
	}

	HColorTarget CreateColorTarget( const ColorTargetDescription& rtInfo )
	{
		DBGOUT("CreateColorTarget(): size=%ux%u, format='%s' ('%s')",
			rtInfo.width, rtInfo.height, mxGET_ENUM_TYPE(PixelFormatT).GetStringByValue(rtInfo.format), rtInfo.name.c_str());
		return driverCreateColorTarget(rtInfo);
	}
	void DeleteColorTarget( HColorTarget rt )
	{
		driverDeleteColorTarget(rt);
	}
	HDepthTarget CreateDepthTarget( const DepthTargetDescription& dtInfo )
	{
#if MX_DEBUG
		{LogStream log(LL_Debug);
		log << "CreateDepthTarget(): ";
		DBG_DumpFields(&dtInfo,dtInfo.MetaClass(),log);}
#endif
		return driverCreateDepthTarget(dtInfo);
	}
	void DeleteDepthTarget( HDepthTarget dt )
	{
		driverDeleteDepthTarget(dt);
	}

	HDepthStencilState CreateDepthStencilState( const DepthStencilDescription& dsInfo )
	{
		return driverCreateDepthStencilState(dsInfo);
	}
	HRasterizerState CreateRasterizerState( const RasterizerDescription& rsInfo )
	{
		return driverCreateRasterizerState(rsInfo);
	}
	HSamplerState CreateSamplerState( const SamplerDescription& ssInfo )
	{
		return driverCreateSamplerState(ssInfo);
	}
	HBlendState CreateBlendState( const BlendDescription& bsInfo )
	{
		return driverCreateBlendState(bsInfo);
	}
	void DeleteDepthStencilState( HDepthStencilState ds )
	{
		driverDeleteDepthStencilState(ds);
	}
	void DeleteRasterizerState( HRasterizerState rs )
	{	
		driverDeleteRasterizerState(rs);
	}
	void DeleteSamplerState( HSamplerState ss )
	{
		driverDeleteSamplerState(ss);
	}
	void DeleteBlendState( HBlendState bs )
	{
		driverDeleteBlendState(bs);
	}

	HBuffer CreateBuffer( EBufferType type, UINT32 size, const void* data )
	{
		//mxASSERT2(size >= 16,"buffer size too small");
		DBGOUT("CreateBuffer(): type=%s, size=%u\n", mxGET_ENUM_TYPE(BufferTypeT).GetStringByValue(type), size);
		return driverCreateBuffer(type, data, size);
	}
	void DeleteBuffer( HBuffer handle )
	{
		driverDeleteBuffer(handle);
	}

	HShader CreateShader( EShaderType shaderType, const void* compiledBytecode, UINT32 bytecodeLength )
	{
		return driverCreateShader(shaderType, compiledBytecode, bytecodeLength);
	}
	void DeleteShader( HShader handle )
	{
		driverDeleteShader(handle);
	}
	HProgram CreateProgram( const ProgramDescription& pd )
	{
		mxTODO("Prevent creation of duplicate shader programs?");
		const HProgram handle = driverCreateProgram(pd);
		DBGOUT("CreateProgram(%s) -> %u\n", pd.name.c_str(), handle.id);
		return handle;
	}
	void DeleteProgram( HProgram handle )
	{
		driverDeleteProgram(handle);
	}

	HResource AsResource( HBuffer br )
	{
		return driverGetShaderResource(br);
	}
	HResource AsResource( HTexture tx )
	{
		return driverGetShaderResource(tx);
	}
	HResource AsResource( HColorTarget rt )
	{
		return driverGetShaderResource(rt);
	}
	HResource AsResource( HDepthTarget rt )
	{
		return driverGetShaderResource(rt);
	}

	CommandBuffer::CommandBuffer()
	{
		m_data = NULL;
		m_used = 0;
		m_size = 0;
	}
	void CommandBuffer::Initialize( UINT32 size )
	{
		m_data = (char*) g_client->Alloc(size);
		m_size = size;
	}
	void CommandBuffer::Shutdown()
	{
		g_client->Free(m_data);
		m_data = NULL;
		m_size = 0;
	}

	ViewState::ViewState()
	{
		this->Reset();
	}
	void ViewState::Reset()
	{
		memset(colorTargets, LLGL_NULL_HANDLE, sizeof(colorTargets));
		mxZERO_OUT(clearColors);
		targetCount = 0;
		depthTarget.SetNil();
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = tr.resolution.width;
		viewport.height = tr.resolution.height;
		flags = 0;
		depth = 1.0f;
		stencil = 0;
	}
	void SubmitView( HContext _context, const ViewState& _view )
	{
		mxASSERT(_view.targetCount <= LLGL_MAX_BOUND_TARGETS);
		DeviceContext* deviceContext = (DeviceContext*) _context.ptr;
		deviceContext->SubmitView( _view );
	}

	void UpdateBuffer( HContext _context, HBuffer _handle, UINT32 _size, const void* _data, UINT32 _start )
	{
		mxASSERT(_size > 0);
		mxASSERT_PTR(_data);
		DeviceContext* deviceContext = (DeviceContext*) _context.ptr;
		deviceContext->UpdateBuffer( _handle, _start, _data, _size );
		//tr.commands.WriteToken(CMD_UPDATE_BUFFER);
		//tr.commands.Put(handle);
		//tr.commands.Put(start);
		//tr.commands.Put(size);
		//tr.commands.AlignPointer<16>();
		//tr.commands.Put(data, size);
	}
	void UpdateTexture2( HContext _context, HTexture _handle, UINT32 _size, const void* _data )
	{
		mxASSERT(_size > 0);
		mxASSERT_PTR(_data);
		DeviceContext* deviceContext = (DeviceContext*) _context.ptr;
		deviceContext->UpdateTexture2( _handle, _data, _size );
	}
	void ReadPixels( HContext _context, HColorTarget source, void *destination, UINT32 bufferSize )
	{
		UNDONE;
		//tr.commands.WriteToken(RC_READ_PIXELS);
		//tr.commands.Put(source);
		//tr.commands.Put(destination);
		//tr.commands.Put(bufferSize);
	}
	void* MapBuffer( HContext _context, HBuffer _handle, UINT32 _size, EMapMode _mode, UINT32 _start )
	{
		mxASSERT(_size > 0);
		DeviceContext* deviceContext = (DeviceContext*) _context.ptr;
		return deviceContext->MapBuffer( _handle, _start, _mode, _size );
	}
	void UnmapBuffer( HContext _context, HBuffer _handle )
	{
		DeviceContext* deviceContext = (DeviceContext*) _context.ptr;
		deviceContext->UnmapBuffer( _handle );
	}

	void SetRasterizerState( HContext _context, HRasterizerState rasterizerState )
	{
		DeviceContext* deviceContext = (DeviceContext*) _context.ptr;
		deviceContext->SetRasterizerState( rasterizerState );
		//tr.commands.WriteToken(RC_SET_RASTERIZER_STATE);
		//tr.commands.Put(rasterizerState);
		//tr.commands.AlignPointer<4>();
	}
	void SetDepthStencilState( HContext _context, HDepthStencilState depthStencilState, UINT8 stencilReference )
	{
		DeviceContext* deviceContext = (DeviceContext*) _context.ptr;
		deviceContext->SetDepthStencilState( depthStencilState, stencilReference );
		//tr.commands.WriteToken(RC_SET_DEPTH_STENCIL_STATE);
		//tr.commands.Put(depthStencilState);
		//tr.commands.Put(stencilReference);
		//tr.commands.AlignPointer<4>();
	}
	void SetBlendState( HContext _context, HBlendState blendState, const float* blendFactor /*= NULL*/, UINT32 sampleMask /*= ~0*/ )
	{
		DeviceContext* deviceContext = (DeviceContext*) _context.ptr;
		deviceContext->SetBlendState( blendState, blendFactor, sampleMask );
		//tr.commands.WriteToken(RC_SET_BLEND_STATE);
		//tr.commands.Put(blendState);
		//tr.commands.AlignPointer<4>();
	}
	//void SetTexture( HContext _context, UINT8 _stage, HResource _texture, HSamplerState _sampler )
	//{
	//	DeviceContext* deviceContext = (DeviceContext*) _context.ptr;
	//	deviceContext->SetTexture( _stage, _texture, _sampler );
	//}
	//void SetCBuffer( HContext _context, UINT8 _slot, HBuffer _buffer )
	//{
	//	DeviceContext* deviceContext = (DeviceContext*) _context.ptr;
	//	deviceContext->SetCBuffer( _slot, _buffer );
	//}
#if 0
	void SetSamplerState( UINT slot, HSamplerState handle )
	{
		tr.commands.WriteToken(RC_SET_SAMPLER);
		tr.commands.Put(handle);
		tr.commands.Put(slot);
	}
	void SetUniformBuffer( UINT slot, HBuffer handle )
	{
		tr.commands.WriteToken(RC_SET_CBUFFER);
		tr.commands.Put(handle);
		tr.commands.Put(slot);
	}
	void SetShaderResource( UINT slot, HResource handle )
	{
		tr.commands.WriteToken(RC_SET_RESOURCE);
		tr.commands.Put(handle);
		tr.commands.Put(slot);
	}
	void SetShaderProgram( HProgram program )
	{
		tr.currentBatch.program = program;
	}
	void SetVertexBuffers( UINT numBuffers, const HBuffer* buffers, UINT baseVertex, UINT numVertices )
	{
		mxASSERT(numBuffers < LLGL_MAX_VERTEX_STREAMS);
		for( UINT iVB = 0; iVB < numBuffers; iVB++ ) {
			tr.currentBatch.VB[iVB] = buffers[iVB];
		}
		tr.currentBatch.baseVertex = baseVertex;
		tr.currentBatch.vertexCount = numVertices;
	}
	void SetInputLayout( HInputLayout layout )
	{
		tr.currentBatch.inputLayout = layout;
	}
	void SetIndexBuffer( UINT stride, HBuffer buffer, UINT startIndex, UINT numIndices )
	{
		mxASSERT( stride == sizeof(UINT16) || stride == sizeof(UINT32) );
		tr.currentBatch.IB = buffer;
		tr.currentBatch.b32bit = (stride == sizeof(UINT32));
		tr.currentBatch.startIndex = startIndex;
		tr.currentBatch.indexCount = numIndices;
	}
	void SetTopology( TopologyT topology )
	{
		tr.currentBatch.primitiveTopology = topology;
	}	
	void NextBatch()
	{
		//@todo: checks for null shader, invalid params, etc.
		mxASSERT(tr.currentBatch.program.IsValid());
		mxASSERT(tr.currentBatch.primitiveTopology != Topology::Undefined);

		tr.commands.WriteToken(RC_DRAW_BATCH);
		tr.commands.Put(tr.currentBatch);

		//tr.currentBatch.Reset();

		tr.numBatches++;
	}
#endif
	void DrawCall::Clear()
	{
		mxZERO_OUT(*this);

		memset(CBs, LLGL_NULL_HANDLE, sizeof(CBs));
		memset(SSs, LLGL_NULL_HANDLE, sizeof(SSs));
		memset(SRs, LLGL_NULL_HANDLE, sizeof(SRs));

		program.SetNil();
		inputLayout.SetNil();
		topology = Topology::Undefined;

		memset(VB, LLGL_NULL_HANDLE, sizeof(VB));
		IB.SetNil();
	}
	void Submit( HContext _context, const DrawCall& batch )
	{
		mxASSERT(batch.program.IsValid());
		//mxASSERT(batch.inputLayout.IsValid());
		mxASSERT(batch.topology != Topology::Undefined);
		mxASSERT(batch.vertexCount > 0);

		DeviceContext* deviceContext = (DeviceContext*) _context.ptr;
		deviceContext->SubmitBatch( batch );
		//tr.commands.WriteToken(RC_DRAW_BATCH);
		//tr.commands.Put(batch);
		//tr.numBatches++;
	}

	ERet NextFrame()
	{
		mxASSERT_MAIN_THREAD;

		const UINT32 size = tr.commands.GetOffset();
		tr.commands.Reset();
		driverSubmitFrame(tr.commands, size);

		++tr.frameCount;

		return ALL_OK;
	}

	void SetMarker( HContext _context, const wchar_t* markerName, const UINT32 colorRGBA )
	{
		::D3DPERF_SetMarker( colorRGBA, markerName );
		//tr.commands.WriteToken(RC_SET_MARKER);
		//PerfMarkerData& markerData = tr.commands.Alloc< PerfMarkerData >();
		//new(&markerData) PerfMarkerData( markerName, colorRGBA );
	}
	void PushMarker( HContext _context, const wchar_t* markerName, const UINT32 colorRGBA )
	{
		::D3DPERF_BeginEvent( colorRGBA, markerName );
		//tr.commands.WriteToken(RC_PUSH_MARKER);
		//PerfMarkerData& markerData = tr.commands.Alloc< PerfMarkerData >();
		//new(&markerData) PerfMarkerData( markerName, colorRGBA );
	}
	void PopMarker( HContext _context )
	{
		::D3DPERF_EndEvent();
		//tr.commands.WriteToken(RC_POP_MARKER);
	}

}//namespace llgl

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
