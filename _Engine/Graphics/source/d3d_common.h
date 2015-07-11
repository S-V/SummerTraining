#pragma once

// Use DirectX utility library
#define USE_D3DX	(1)

// Enable extra D3D debugging in debug builds if using the debug DirectX runtime.  
// This makes D3D objects work well in the debugger watch window, but slows down 
// performance slightly.

#if MX_DEBUG && (!defined( D3D_DEBUG_INFO ))
#	define D3D_DEBUG_INFO
#endif

// DirectX error codes.
#include <DxErr.h>
#if MX_AUTOLINK
	#pragma comment( lib, "DxErr.lib" )
#endif

#include <D3Dcommon.h>
#include <D3Dcompiler.h>

#include <DXGI.h>

// DirectX 9 headers are needed for PIX support and some D3DERR_* codes
#include <d3d9.h>
#pragma comment( lib, "d3d9.lib" )

#include <D3D11.h>

#include <Graphics/Device.h>

//------------------------------------------------------------------------
//	Useful macros
//------------------------------------------------------------------------

#ifndef SAFE_RELEASE
#define SAFE_RELEASE( p )		{ if( (p) != NULL ) { (p)->Release(); (p) = NULL; } }
#endif

#define SAFE_ADDREF( p )		{ if( (p) != NULL ) { (p)->AddRef(); } }
#define SAFE_ACQUIRE( dst, p )	{ if( (dst) != NULL ) { SAFE_RELEASE(dst); } if( p != NULL ) { (p)->AddRef(); } dst = (p); }

#define RELEASE( p )			(p)->Release()


//------------------------------------------------------------------------
//	Debugging and run-time checks - they slow down a lot!
//------------------------------------------------------------------------

#if MX_DEBUG

	#define dxCHK( expr )		for( HRESULT hr = (expr); ; )\
								{\
									if( FAILED( hr ) )\
									{\
										dxERROR( hr );\
										ptBREAK;\
									}\
									break;\
								}

	#define dxTRY( expr )		for( HRESULT hr = (expr); ; )\
								{\
									if( FAILED( hr ) )\
									{\
										dxERROR( hr );\
										ptBREAK;\
										return D3D_HRESULT_To_EResult( hr );\
									}\
									break;\
								}

#else

	#define dxCHK( expr )		(expr)

	#define dxTRY( expr )		(expr)

#endif // RX_DEBUG_RENDERER



//------------------------------------------------------------------------
//	Declarations
//------------------------------------------------------------------------

template< class T >
mxFORCEINLINE
void SafeRelease( T *&p )
{
	if( p != NULL ) {
		p->Release();
		p = NULL;
	}
}

mxFORCEINLINE
bool dxCheckedRelease( IUnknown* pInterface )
{
	const ULONG refCount = pInterface->Release();
	mxASSERT( refCount == 0 );
	return( refCount == 0 );
}

template< class TYPE, UINT COUNT >
inline
void dxSafeReleaseArray( TYPE* (&elements) [COUNT] )
{
	for( UINT iElement = 0; iElement < COUNT; ++iElement )
	{
		if( elements[ iElement ] != NULL )
		{
			elements[ iElement ]->Release();
		}
	}
}

// Verify that a pointer to a COM object is still valid
//
// Usage:
//   VERIFY_COM_INTERFACE(pFoo);
//
template< class Q >
void VERIFY_COM_INTERFACE( Q* p )
{
#if MX_DEBUG
	p->AddRef();
	p->Release();
#endif // MX_DEBUG
}

template< class Q >
void FORCE_RELEASE_COM_INTERFACE( Q*& p )
{
	mxPLATFORM_PROBLEM("used to get around the live device warning - this is possibly a bug in SDKLayer");
	while( p->Release() )
		;
	p = NULL;
}


template< class Q >
void DbgPutNumRefs( Q* p, const char* msg = NULL )
{
#if MX_DEBUG
	// the new reference count
	const UINT numRefs = p->AddRef();
	DBGOUT( "%sNumRefs = %u\n", (msg != NULL) ? msg : "", numRefs );
	p->Release();
#endif // MX_DEBUG
}

//
//	dxPtr< T > - smart pointer for safe and automatic handling of DirectX resources.
//
template< class T >
struct dxPtr 
{
	T *	Ptr;

public:
	mxFORCEINLINE dxPtr()
		: Ptr( NULL )
	{}

	mxFORCEINLINE dxPtr( T * pointer )
		: Ptr( pointer )
	{
		if( this->Ptr ) {
			this->AcquirePointer();
		}
	}

	mxFORCEINLINE dxPtr( const dxPtr<T> & other )
	{
		this->Ptr = other.Ptr;
		if( this->Ptr ) {
			this->AcquirePointer();
		}
	}

	mxFORCEINLINE ~dxPtr()
	{
		if( this->Ptr ) {
			this->ReleasePointer();
		}
	}

	mxFORCEINLINE void Release()
	{
		if( this->Ptr ) {
			this->ReleasePointer();
			this->Ptr = NULL;
		}
	}

	mxFORCEINLINE void operator = ( T * pointer )
	{
		if( this->Ptr ) {
			this->ReleasePointer();
		}
		
		this->Ptr = pointer;
		
		if( this->Ptr ) {
			this->AcquirePointer();
		}
	}

	mxFORCEINLINE void operator = ( const dxPtr<T> & other )
	{
		if( this->Ptr ) {
			this->ReleasePointer();
		}

		this->Ptr = other.Ptr;

		if( this->Ptr ) {
			this->AcquirePointer();
		}
	}

	mxFORCEINLINE operator T* () const
	{
		return this->Ptr;
	}

	mxFORCEINLINE T * operator -> () const
	{
		mxASSERT_PTR( this->Ptr );
		return this->Ptr;
	}

	mxFORCEINLINE bool operator == ( const T * pointer )
	{
		return this->Ptr == pointer;
	}
	mxFORCEINLINE bool operator == ( const dxPtr<T> & other )
	{
		return this->Ptr == other.Ptr;
	}

	mxFORCEINLINE bool IsNull() const
	{
		return ( NULL == this->Ptr );
	}
	mxFORCEINLINE bool IsValid() const
	{
		return ( NULL != this->Ptr );
	}

	mxFORCEINLINE T** ToArrayPtr()
	{
		return & this->Ptr;
	}
	//@todo: assignGet() ?

private:
	// assumes that the pointer is not null
	mxFORCEINLINE void AcquirePointer()
	{
		mxASSERT_PTR( this->Ptr );
		this->Ptr->AddRef();
	}

	// assumes that the pointer is not null
	mxFORCEINLINE void ReleasePointer()
	{
		mxASSERT_PTR( this->Ptr );
		this->Ptr->Release();
	}
};


#define mxDECLARE_COM_PTR( className )\
	typedef dxPtr< className > className ## Ptr



//------------------------------------------------------------------------
//	Logging and error reporting / error handling
//------------------------------------------------------------------------

HRESULT dxWARN( HRESULT errorCode, const char* message, ... );
HRESULT dxERROR( HRESULT errorCode, const char* message, ... );
HRESULT dxERROR( HRESULT errorCode );

// Returns a string corresponding to the given error code.
const char* D3D_GetErrorCodeString( HRESULT hErrorCode );

// Converts an HRESULT to a meaningfull message
#define dxLAST_ERROR_STRING		::DXGetErrorStringA(::GetLastError())

ERet D3D_HRESULT_To_EResult( HRESULT errorCode );

//--------------------------------------------------------------------------------------
// Profiling/instrumentation support
//--------------------------------------------------------------------------------------

// Stolen from: http://svn.alleg.net/svn/Allegiance/trunk/src/zlib/olddxdefns.h
/*
 * Format of D3DCOLOR RGBA colors is
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    alpha      |      red      |     green     |     blue      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define RGBA_GETALPHA(rgb)      ((rgb) >> 24)
#define RGBA_GETRED(rgb)        (((rgb) >> 16) & 0xff)
#define RGBA_GETGREEN(rgb)      (((rgb) >> 8) & 0xff)
#define RGBA_GETBLUE(rgb)       ((rgb) & 0xff)
#define RGBA_MAKE(r, g, b, a)   ((D3DCOLOR) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))

/* D3DRGB and D3DRGBA may be used as initialisers for D3DCOLORs
 * The float values must be in the range 0..1
 */
#define D3DRGB(r, g, b) \
    (0xff000000L | ( ((long)((r) * 255)) << 16) | (((long)((g) * 255)) << 8) | (long)((b) * 255))
#define D3DRGBA(r, g, b, a) \
    (   (((long)((a) * 255)) << 24) | (((long)((r) * 255)) << 16) \
    |   (((long)((g) * 255)) << 8) | (long)((b) * 255) \
    )

inline D3DCOLOR D3DCOLOR_From_Floats( const float* rgba )
{
	return(((long)(rgba[3] * 255.0f)) << 24)
		| (((long)(rgba[0] * 255.0f)) << 16)
		| (((long)(rgba[1] * 255.0f)) << 8)
		| ((long)(rgba[2] * 255.0f));
}

struct WideName48
{
	wchar_t	data[48];
public:
	WideName48( const char* source ) {
		int length = smallest(strlen(source), mxCOUNT_OF(data)-1);
		mbstowcs( data, source, length );
		data[length] = 0;
	}
};

//--------------------------------------------------------------------------------------
// Debugging support
//--------------------------------------------------------------------------------------

// Use dxSetDebugName() to attach names to D3D objects for use by 
// SDKDebugLayer, PIX's object table, etc.

//NOTE: i had to disable this functionality
// because of bugs in MS functions leading to
// D3D11 WARNING: ID3D11DepthStencilState::SetPrivateData:
// Existing private data of same name with different size found!
// [ STATE_SETTING WARNING #55: SETPRIVATEDATA_CHANGINGPARAMS]
//
#if 0//LLGL_ENABLE_PERF_HUD

	mxFORCEINLINE void dxSetDebugName( IDXGIObject* o, const char* name )
	{
		//DBGOUT("dxSetDebugName: %s\n", name);
		if( o != NULL && name != NULL ) {
			o->SetPrivateData( WKPDID_D3DDebugObjectName, lstrlenA(name), name );
		}
	}
	mxFORCEINLINE void dxSetDebugName( ID3D11Device* o, const char* name )
	{
		//DBGOUT("dxSetDebugName: %s\n", name);
		if( o != NULL && name != NULL ) {
			o->SetPrivateData( WKPDID_D3DDebugObjectName, lstrlenA(name), name );
		}
	}
	mxFORCEINLINE void dxSetDebugName( ID3D11DeviceChild* o, const char* name )
	{
		//DBGOUT("dxSetDebugName: %s\n", name);
		if( o != NULL && name != NULL ) {
			o->SetPrivateData( WKPDID_D3DDebugObjectName, lstrlenA(name), name );
		}
	}

#else

	inline void dxSetDebugName( const void* o, const char* name ) {}

#endif // RX_D3D_USE_PERF_HUD

//------------------------------------------------------------------------
//	DXGI formats
//------------------------------------------------------------------------

#ifndef DXGI_FORMAT_DEFINED
	#error DXGI_FORMAT must be defined!
#endif

mxDECLARE_ENUM( DXGI_FORMAT, UINT32, DXGI_FORMAT_ENUM );

mxSWIPED("bgfx");
struct TextureFormatInfoD3D11
{
	DXGI_FORMAT m_fmt;
	DXGI_FORMAT m_fmtSrv;
	DXGI_FORMAT m_fmtDsv;
};
extern const TextureFormatInfoD3D11 gs_textureFormats[PixelFormat::MAX];

const char* DXGI_FORMAT_ToChars( DXGI_FORMAT format );
DXGI_FORMAT String_ToDXGIFormat( const char* _str );

UINT DXGI_FORMAT_BitsPerPixel( DXGI_FORMAT format );
UINT DXGI_FORMAT_GetElementSize( DXGI_FORMAT format );

UINT DXGI_FORMAT_GetElementCount( DXGI_FORMAT format );
bool DXGI_FORMAT_HasAlphaChannel( DXGI_FORMAT format );

// Helper functions to create SRGB formats from typeless formats and vice versa.
DXGI_FORMAT DXGI_FORMAT_MAKE_SRGB( DXGI_FORMAT format );
DXGI_FORMAT DXGI_FORMAT_MAKE_TYPELESS( DXGI_FORMAT format );

bool DXGI_FORMAT_IsBlockCompressed( DXGI_FORMAT format );

void DXGI_FORMAT_GetSurfaceInfo( UINT width, UINT height, DXGI_FORMAT fmt, UINT* pNumBytes, UINT* pRowBytes, UINT* pNumRows );

const char* DXGI_ScanlineOrder_ToStr( DXGI_MODE_SCANLINE_ORDER scanlineOrder );
const char* DXGI_ScalingMode_ToStr( DXGI_MODE_SCALING scaling );

DXGI_FORMAT DXGI_GetDepthStencil_Typeless_Format( DXGI_FORMAT depthStencilFormat );
DXGI_FORMAT DXGI_GetDepthStencilView_Format( DXGI_FORMAT depthStencilFormat );
DXGI_FORMAT DXGI_GetDepthStencil_SRV_Format( DXGI_FORMAT depthStencilFormat );

mxFORCEINLINE
DXGI_FORMAT DXGI_GetIndexBufferFormat( const UINT indexStride )
{
	mxASSERT((indexStride == sizeof UINT16) || (indexStride == sizeof UINT32));
	return (indexStride == sizeof UINT16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
}

//------------------------------------------------------------------------
//	Shader profiles
//------------------------------------------------------------------------

mxDECLARE_ENUM( D3D_FEATURE_LEVEL, UINT32, D3D_FEATURE_LEVEL_ENUM );

const char* D3D_FeatureLevelToStr( D3D_FEATURE_LEVEL featureLevel );

D3D11_MAP ConvertMapModeD3D( EMapMode mapMode );

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
