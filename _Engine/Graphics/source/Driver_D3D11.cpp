/*
=============================================================================
ToDo:
	prevent creation of duplicate render states

References:
http://seanmiddleditch.com/journal/2014/02/direct3d-11-debug-api-tricks/
=============================================================================
*/
#include "Graphics/Graphics_PCH.h"
#pragma hdrstop

#if LLGL_Driver == LLGL_Driver_Direct3D_11

#include <Base/Template/THandleManager.h>
#include <typeinfo.h>	// typeid()
#include <Core/Serialization.h>
#include <Graphics/Device.h>
#include <Graphics/Legacy.h>
#include "Driver_D3D11.h"
#include "DDS_Reader.h"

#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

namespace llgl
{
	struct DriverD3D11
	{
		dxPtr< ID3D11Device >			device;
		dxPtr< IDXGISwapChain >			swapChain;
		dxPtr< IDXGIOutput >			output;

		Resolution    resolution;	// back buffer size

		DeviceContext	immediateContext;

		// all created graphics resources
		THandleManager< DepthStencilStateD3D11 >	depthStencilStates;
		THandleManager< RasterizerStateD3D11 >		rasterizerStates;
		THandleManager< SamplerStateD3D11 >			samplerStates;
		THandleManager< BlendStateD3D11 >			blendStates;

		THandleManager< ColorTargetD3D11 >		colorTargets;
		THandleManager< DepthTargetD3D11 >		depthTargets;

		THandleManager< InputLayoutD3D11 >	inputLayouts;

		THandleManager< BufferD3D11 >	buffers;
		THandleManager< TextureD3D11 >	textures;

		THandleManager< ShaderD3D11 >	shaders;
		THandleManager< ProgramD3D11 >	programs;
	};

	mxDECLARE_PRIVATE_DATA( DriverD3D11, gDriverData );

	//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	#define me	mxGET_PRIVATE_DATA( DriverD3D11, gDriverData )

	//-----------------------------------------------------------------------

	static UINT32 mxPASCAL RenderThreadFunction( void* userData );

// if h == -1 then return 0 (which is usually the default value)
// if h != -1 then return h
static inline UINT32 GetValidIndex( UINT32 h )
{
	return (h != ~0) ? h : 0;
}

#ifndef DXGI_FORMAT_B4G4R4A4_UNORM
// Win8 only BS
// https://blogs.msdn.com/b/chuckw/archive/2012/11/14/directx-11-1-and-windows-7.aspx?Redirected=true
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb173059%28v=vs.85%29.aspx
#	define DXGI_FORMAT_B4G4R4A4_UNORM DXGI_FORMAT(115)
#endif // DXGI_FORMAT_B4G4R4A4_UNORM

static bool SetupDXGI(
	dxPtr< IDXGIFactory > & outDXGIFactory,
	dxPtr< IDXGIAdapter > & outDXGIAdapter,
	dxPtr< IDXGIOutput > & outDXGIOutput,
	DeviceVendor::Enum & outDeviceVendor
	)
{
	HRESULT hr = E_FAIL;

	// Create DXGI factory.

	dxPtr< IDXGIFactory >	pDXGIFactory;
	hr = CreateDXGIFactory(
		__uuidof( IDXGIFactory ),
		(void**) &pDXGIFactory.Ptr
	);
	if( FAILED( hr ) ) {
		dxERROR( hr, "Failed to create DXGIFactory" );
		return false;
	}

	// Enumerate adapters.
	{
		UINT iAdapter = 0;
		IDXGIAdapter * pAdapter = NULL;
		while( pDXGIFactory->EnumAdapters( iAdapter, &pAdapter ) != DXGI_ERROR_NOT_FOUND )
		{
			pAdapter->Release();
			++iAdapter;
		}
		ptPRINT("Detected %d video adapter(s)",iAdapter);
	}

	dxPtr< IDXGIAdapter >	pDXGIAdapter0;
	hr = pDXGIFactory->EnumAdapters( 0, &pDXGIAdapter0.Ptr );
	if( hr == DXGI_ERROR_NOT_FOUND ) {
		ptERROR( "Failed to enumerate video adapter 0" );
		return false;
	}

	DXGI_ADAPTER_DESC  adapterDesc;
	hr = pDXGIAdapter0->GetDesc( &adapterDesc );
	if( FAILED( hr ) ) {
		dxERROR( hr, "Failed to get adapter description" );
		return false;
	}

	{
		ptPRINT("Adapter description: %s",mxTO_ANSI(adapterDesc.Description));
		ptPRINT("VendorId: %u, DeviceId: %u, SubSysId: %u, Revision: %u",
			adapterDesc.VendorId,adapterDesc.DeviceId,adapterDesc.SubSysId,adapterDesc.Revision);

		outDeviceVendor = DeviceVendor::FourCCToVendorEnum( adapterDesc.VendorId );
		if( DeviceVendor::Vendor_Unknown != outDeviceVendor ) {
			ptPRINT("Device vendor: %s", DeviceVendor::GetVendorString( outDeviceVendor ));
		}
		ptPRINT("Dedicated video memory: %u Mb, Dedicated system memory: %u Mb, Shared system memory: %u Mb",
			adapterDesc.DedicatedVideoMemory / (1024*1024),
			adapterDesc.DedicatedSystemMemory / (1024*1024),
			adapterDesc.SharedSystemMemory / (1024*1024)
		);
	}

	dxPtr< IDXGIOutput > pDXGIOutput0;
	hr = pDXGIAdapter0->EnumOutputs( 0 /* The index of the output (monitor) */,
									&pDXGIOutput0.Ptr );
	if( hr == DXGI_ERROR_NOT_FOUND )
	{
		ptERROR( "Failed to enumerate video card output 0" );
		return false;
	}

	DXGI_OUTPUT_DESC  oDesc;
	hr = pDXGIOutput0->GetDesc( &oDesc );
	if( FAILED( hr ) ) {
		dxERROR( hr, "Failed to get video card output description" );
		return false;
	}

	{
		ptPRINT("Detected output device: %s",mxTO_ANSI(oDesc.DeviceName));
	//	ptPRINT("Desktop coordinates: left(%d),top(%d),right(%d),bottom(%d)\n",
	//		oDesc.DesktopCoordinates.left,oDesc.DesktopCoordinates.top,oDesc.DesktopCoordinates.right,oDesc.DesktopCoordinates.bottom);
		UINT deckstopScrWidth, deckstopScrHeight;
		mxGetCurrentDeckstopResolution( deckstopScrWidth, deckstopScrHeight );
		ptPRINT( "Current desktop resolution: %ux%u", deckstopScrWidth, deckstopScrHeight );
	}

	outDXGIFactory = pDXGIFactory;
	outDXGIAdapter = pDXGIAdapter0;
	outDXGIOutput = pDXGIOutput0;

	return true;
}
//-------------------------------------------------------------------------------------------------------------//
static UINT GetCreateDeviceFlags()
{
	UINT  createDeviceFlags = 0;

	// Use this flag if your application will only call methods of Direct3D 11 interfaces from a single thread.
	// By default, the ID3D11Device object is thread-safe. By using this flag, you can increase performance.
	// However, if you use this flag and your application calls methods of Direct3D 11 interfaces from multiple threads,
	// undefined behavior might result.
	createDeviceFlags |= D3D11_CREATE_DEVICE_SINGLETHREADED;

#if MX_DEBUG || MX_DEVELOPER
	bool	bCreateDebugRenderDevice = false;
	gINI->GetBoolean("bCreateDebugRenderDevice",bCreateDebugRenderDevice);
	if(bCreateDebugRenderDevice)
	{
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
#endif // MX_DEBUG || MX_DEVELOPER

	return createDeviceFlags;
}
//-------------------------------------------------------------------------------------------------------------//
static void SetupDebugHelpers( const dxPtr< ID3D11Device >& inD3DDevice )
{
	// NOTE: this incurs a significant overhead (frame render time went up from 2 to 15 ms) but helps to find rare driver bugs...
	if (0 && MX_DEBUG)
	{
		dxPtr< ID3D11Debug >	d3dDebug;
		if( SUCCEEDED( inD3DDevice->QueryInterface( IID_ID3D11Debug, (void**)&d3dDebug.Ptr ) )
			&& (d3dDebug != NULL) )
		{
			dxCHK(d3dDebug->SetFeatureMask( D3D11_DEBUG_FEATURE_FINISH_PER_RENDER_OP ));
		}
	}
}
//-------------------------------------------------------------------------------------------------------------//
static bool CreateDirect3DDevice(
	const dxPtr< IDXGIAdapter >& inDXGIAdapter,
	dxPtr< ID3D11Device > & outDevice,
	dxPtr< ID3D11DeviceContext > & outImmediateContext
	)
{
	const UINT  createDeviceFlags = GetCreateDeviceFlags();

	// Required for Direct2D interoperability with Direct3D resources.
	// Needed for supporting DXGI_FORMAT_B8G8R8A8_UNORM. Only works with 10.1 and above.
	//createDeviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	D3D_FEATURE_LEVEL	featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		//D3D_FEATURE_LEVEL_10_1,
		//D3D_FEATURE_LEVEL_10_0,
		//D3D_FEATURE_LEVEL_9_3,
		//D3D_FEATURE_LEVEL_9_2,
		//D3D_FEATURE_LEVEL_9_1,
	};

	D3D_FEATURE_LEVEL	selectedFeatureLevel;

	HRESULT hr = E_FAIL;

	{
		// If you set the pAdapter parameter to a non-NULL value, you must also set the DriverType parameter to the D3D_DRIVER_TYPE_UNKNOWN value.
		// If you set the pAdapter parameter to a non-NULL value and the DriverType parameter to the D3D_DRIVER_TYPE_HARDWARE value, D3D11CreateDevice returns an HRESULT of E_INVALIDARG.
		// See: "D3D11CreateDevice function"
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476082(v=vs.85).aspx
		// http://www.gamedev.net/topic/561002-d3d11createdevice---invalid_arg/
		//
		hr = D3D11CreateDevice(
			inDXGIAdapter,
			D3D_DRIVER_TYPE_UNKNOWN,
			( HMODULE )NULL,	/* HMODULE Software rasterizer */
			createDeviceFlags,
			featureLevels,	// array of feature levels, null means 'get the greatest feature level available'
			mxCOUNT_OF(featureLevels),
			D3D11_SDK_VERSION,
			&outDevice.Ptr,
			&selectedFeatureLevel,
			&outImmediateContext.Ptr
		);
	}

	if( FAILED( hr ) )
	{
		dxERROR( hr,
			"Failed to create Direct3D 11 device.\n"
			"This application requires a Direct3D 11 class device "
			"running on Windows Vista (or later)" );
		return false;
	}

	SetupDebugHelpers( outDevice );

	return true;
}
//-------------------------------------------------------------------------------------------------------------//
static bool CheckFeatureLevel( const dxPtr< ID3D11Device >& pD3DDevice )
{
	const D3D_FEATURE_LEVEL selectedFeatureLevel = pD3DDevice->GetFeatureLevel();

	ptPRINT("Selected feature level: %s", D3D_FeatureLevelToStr( selectedFeatureLevel ));

	const D3D_FEATURE_LEVEL minimumRequiredFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	if( selectedFeatureLevel < minimumRequiredFeatureLevel )
	{
		ptERROR("This application requires a Direct3D 11 class pD3DDevice with at least %s support",
			D3D_FeatureLevelToStr(minimumRequiredFeatureLevel));
		return false;
	}

	return true;
}
//-------------------------------------------------------------------------------------------------------------//
static void LogDeviceInfo( const dxPtr< ID3D11Device >& inD3DDevice )
{
	// GetGPUThreadPriority() returns E_FAIL when running in PIX
	if(!LLGL_ENABLE_PERF_HUD)
	{
		dxPtr< IDXGIDevice >	pDXGIDevice;
		const HRESULT hr = inD3DDevice->QueryInterface(
			__uuidof( IDXGIDevice ),
			(void**) &pDXGIDevice.Ptr
		);
		if( FAILED( hr ) ) {
			dxERROR( hr, "Failed to get IDXGIDevice interface" );
		}
		if( NULL != pDXGIDevice )
		{
			INT renderThreadPriority;
			if(FAILED( pDXGIDevice->GetGPUThreadPriority( &renderThreadPriority ) ))
			{
				dxERROR( hr, "GetGPUThreadPriority() failed" );
			}
			ptPRINT("GPU thread priority: %d.",renderThreadPriority);
		}
	}

	D3D11_FEATURE_DATA_THREADING  featureSupportData_Threading;
	if(SUCCEEDED(inD3DDevice->CheckFeatureSupport(
		D3D11_FEATURE_THREADING,
		&featureSupportData_Threading,
		sizeof(featureSupportData_Threading)
		)))
	{
		if( TRUE == featureSupportData_Threading.DriverConcurrentCreates ) {
			ptPRINT("Driver supports concurrent resource creation");
		}
		if( TRUE == featureSupportData_Threading.DriverCommandLists ) {
			ptPRINT("Driver supports command lists");
		}
	}

	D3D11_FEATURE_DATA_DOUBLES  featureSupportData_Doubles;
	if(SUCCEEDED(inD3DDevice->CheckFeatureSupport(
		D3D11_FEATURE_DOUBLES,
		&featureSupportData_Doubles,
		sizeof(featureSupportData_Doubles)
		)))
	{
		if( TRUE == featureSupportData_Doubles.DoublePrecisionFloatShaderOps ) {
			ptPRINT("Driver supports double data types");
		}
	}

	D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS		hardwareOptions;
	if(SUCCEEDED(inD3DDevice->CheckFeatureSupport(
		D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS,
		&hardwareOptions,
		sizeof(hardwareOptions)
		)))
	{
		if( TRUE == hardwareOptions.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x ) {
			ptPRINT("Driver supports Compute Shaders Plus Raw And Structured Buffers Via Shader Model 4.x");
		}
	}
}
//-------------------------------------------------------------------------------------------------------------//
static bool FindSuitableRefreshRate(
	const dxPtr< IDXGIOutput >& inDXGIOutput,
	const UINT inBackbufferWidth, const UINT inBackbufferHeight,
	const DXGI_FORMAT inBackBufferFormat,
	DXGI_RATIONAL &outRefreshRate )
{
	// Microsoft best practices advises this step.
	outRefreshRate.Numerator = 0;
	outRefreshRate.Denominator = 0;

	const UINT	flags = DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING;

	UINT	numDisplayModes = 0;

	HRESULT	hr = inDXGIOutput->GetDisplayModeList(
		inBackBufferFormat,
		flags,
		&numDisplayModes,
		0
	);
	if( FAILED( hr ) ) {
		dxERROR( hr, "Failed to get the number of display modes." );
		return false;
	}

	enum { MAX_DISPLAY_MODES = 32 };

	DXGI_MODE_DESC	displayModes[ MAX_DISPLAY_MODES ];

	numDisplayModes = smallest( numDisplayModes, mxCOUNT_OF(displayModes) );

	hr = inDXGIOutput->GetDisplayModeList(
		inBackBufferFormat,
		flags,
		&numDisplayModes,
		displayModes
	);
	if( FAILED( hr ) ) {
		dxERROR( hr, "Failed to get display mode list" );
		return false;
	}

	ptPRINT( "List of supported (%d) display modes matching the color format '%s':",
			numDisplayModes, DXGI_FORMAT_ToChars( inBackBufferFormat ) );

	for(UINT iDisplayMode = 0; iDisplayMode < numDisplayModes; iDisplayMode++)
	{
		const DXGI_MODE_DESC& displayMode = displayModes[ iDisplayMode ];

		ptPRINT( "[%u]: %ux%u, %u Hz",
			iDisplayMode+1, displayMode.Width, displayMode.Height,
			displayMode.RefreshRate.Numerator / displayMode.RefreshRate.Denominator );

		if( displayMode.Width == inBackbufferWidth
			&& displayMode.Height == inBackbufferHeight )
		{
			outRefreshRate.Numerator = displayMode.RefreshRate.Numerator;
			outRefreshRate.Denominator = displayMode.RefreshRate.Denominator;
		}
	}

	return true;
}
//-------------------------------------------------------------------------------------------------------------//
static bool FindSuitableDisplayMode(
	const dxPtr< IDXGIOutput >& inDXGIOutput,
	const UINT inBackbufferWidth, const UINT inBackbufferHeight,
	const DXGI_FORMAT inBackBufferFormat,
	const DXGI_RATIONAL& inRefreshRate,
	DXGI_MODE_DESC &outBufferDesc
	)
{
	DXGI_MODE_DESC	desiredDisplayMode;
	mxZERO_OUT( desiredDisplayMode );

	desiredDisplayMode.Width	= inBackbufferWidth;
	desiredDisplayMode.Height 	= inBackbufferHeight;
	desiredDisplayMode.Format 	= inBackBufferFormat;
	desiredDisplayMode.RefreshRate = inRefreshRate;

	desiredDisplayMode.ScanlineOrdering	= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desiredDisplayMode.Scaling			= DXGI_MODE_SCALING_UNSPECIFIED;

	const HRESULT hr = inDXGIOutput->FindClosestMatchingMode(
		&desiredDisplayMode,
		&outBufferDesc,
		NULL
	);
	if( FAILED( hr ) ) {
		dxERROR( hr, "Failed to find a supported video mode." );
		return false;
	}

	return true;
}
//-------------------------------------------------------------------------------------------------------------//
UINT GetNumMSAASamples(
	const DXGI_FORMAT inBackBufferFormat,
	const dxPtr< ID3D11Device >& inD3DDevice
	)
{
	const bool	bMultisampleAntiAliasing = false;

	if( bMultisampleAntiAliasing )
	{
		UINT	numMsaaSamples = 32;

		// Get the number of quality levels available during multisampling.
		while( numMsaaSamples > 1 )
		{
			UINT	numQualityLevels = 0;
			if(SUCCEEDED( inD3DDevice->CheckMultisampleQualityLevels( inBackBufferFormat, numMsaaSamples, &numQualityLevels ) ))
			{
				if( numQualityLevels > 0 ) {
					// The format and sample count combination are supported for the installed adapter.
					break;
				}
			}
			numMsaaSamples /= 2;
		}

		ptPRINT("Multisample Anti-Aliasing: %d samples.",numMsaaSamples);

		return numMsaaSamples;
	}
	else
	{
		return 1;
	}
}
//-------------------------------------------------------------------------------------------------------------//
static void CreateSwapChainDescription(
	const dxPtr< ID3D11Device >& inD3DDevice,
	const HWND inWindowHandle, const BOOL inWindowedMode,
	const UINT inBackbufferWidth, const UINT inBackbufferHeight,
	const DXGI_FORMAT inBackBufferFormat,
	const DXGI_RATIONAL& inRefreshRate,
	DXGI_SWAP_CHAIN_DESC &outSwapChainDesc
	)
{
	// for switching between windowed/fullscreen modes
	const bool	allowModeSwitch = true;

	const bool	bSampleBackBuffer = false;

	DXGI_USAGE	backBufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//backBufferUsage |= DXGI_USAGE_BACK_BUFFER;	//<- this line is optional

	if( bSampleBackBuffer ) {
		backBufferUsage |= DXGI_USAGE_SHADER_INPUT;
	}


	mxZERO_OUT( outSwapChainDesc );

	outSwapChainDesc.BufferCount	= 1;

	outSwapChainDesc.BufferDesc.Width			= inBackbufferWidth;
	outSwapChainDesc.BufferDesc.Height 			= inBackbufferHeight;
	outSwapChainDesc.BufferDesc.Format 			= inBackBufferFormat;
	outSwapChainDesc.BufferDesc.RefreshRate		= inRefreshRate;
	outSwapChainDesc.BufferDesc.ScanlineOrdering= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	outSwapChainDesc.BufferDesc.Scaling			= DXGI_MODE_SCALING_UNSPECIFIED;

	outSwapChainDesc.BufferUsage	= backBufferUsage;

	outSwapChainDesc.OutputWindow	= inWindowHandle;	// the window that the swap chain will use to present images on the screen

	outSwapChainDesc.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;	// the contents of the back buffer are discarded after calling IDXGISwapChain::Present()

	outSwapChainDesc.Windowed	= inWindowedMode;

	if( allowModeSwitch ) {
		outSwapChainDesc.Flags	= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// flag for switching between windowed/fullscreen modes
	}

#if 0
	FindSuitableDisplayMode(
		inDXGIOutput,
		inBackbufferWidth, inBackbufferHeight, inBackBufferFormat, inRefreshRate,
		sd.BufferDesc );
#endif

	outSwapChainDesc.SampleDesc.Count = GetNumMSAASamples( outSwapChainDesc.BufferDesc.Format, inD3DDevice );
	outSwapChainDesc.SampleDesc.Quality	= 0;
}
//-------------------------------------------------------------------------------------------------------------//
void GetSwapChainDescription( HWND hWnd, ID3D11Device* device, DXGI_SWAP_CHAIN_DESC &desc )
{
	const BOOL	isWindowed = true;

	RECT	rect;
	::GetClientRect( hWnd, &rect );

	const UINT	windowWidth = rect.right - rect.left;
	const UINT	windowHeight = rect.bottom - rect.top;

	//const BOOL	isFullScreen = !isWindowed;
	//if( isFullScreen )
	//{
	//	windowWidth = GetSystemMetrics( SM_CXSCREEN );
	//	windowHeight = GetSystemMetrics( SM_CYSCREEN );
	//}

	const DXGI_FORMAT	backBufferFormat =
		DXGI_FORMAT_R8G8B8A8_UNORM
		//DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
		;

	DXGI_RATIONAL refreshRate;

	// Microsoft best practices advises this step.
	refreshRate.Numerator = 0;
	refreshRate.Denominator = 0;

	CreateSwapChainDescription(
		device,
		hWnd, isWindowed,
		windowWidth, windowHeight, backBufferFormat,
		refreshRate,
		desc
	);

	// Set this flag to enable an application to render using GDI on a swap chain or a surface.
	// This will allow the application to call IDXGISurface1::GetDC on the 0th back buffer or a surface.
	//swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
}
//-------------------------------------------------------------------------------------------------------------//
static bool CreateDirect3DSwapChain(
	DXGI_SWAP_CHAIN_DESC& sd,
	const dxPtr< IDXGIFactory >& inDXGIFactory,
	const dxPtr< ID3D11Device >& inD3DDevice,
	dxPtr< IDXGISwapChain > &outSwapChain
	)
{
	ptPRINT("Creating a swap chain...");
	ptPRINT("Selected display mode: %ux%u",sd.BufferDesc.Width,sd.BufferDesc.Height);
	ptPRINT("Back buffer format: %s",DXGI_FORMAT_ToChars(sd.BufferDesc.Format));
	if( sd.BufferDesc.RefreshRate.Denominator != 0 ) {
		ptPRINT("Selected refresh rate: %u Hertz", sd.BufferDesc.RefreshRate.Numerator/sd.BufferDesc.RefreshRate.Denominator );
	} else {
		ptPRINT("Selected default refresh rate");
	}
	ptPRINT("Scanline ordering: %s",DXGI_ScanlineOrder_ToStr(sd.BufferDesc.ScanlineOrdering));
	ptPRINT("Scaling: %s",DXGI_ScalingMode_ToStr(sd.BufferDesc.Scaling));

	const HRESULT hr = inDXGIFactory->CreateSwapChain( inD3DDevice, &sd, &outSwapChain.Ptr );
	if( FAILED( hr ) ) {
		dxERROR( hr, "Failed to create a swap chain" );
		return false;
	}

/*
	An alternative way to create device and swap chain:

	hr = D3D11CreateDeviceAndSwapChain(
		pDXGIAdapter0,
		driverType,
		( HMODULE )null,	// HMODULE Software rasterizer
		createDeviceFlags,
		featureLevels,	// array of feature levels, null means 'get the greatest feature level available'
		mxCOUNT_OF(featureLevels),	// numFeatureLevels
		D3D11_SDK_VERSION,
		&swapChainInfo,
		&swapChain.Ptr,
		&device.Ptr,
		&selectedFeatureLevel,
		&immediateContext.Ptr
	);
	if( FAILED( hr ) ) {
		dxERROR( hr,
			"Failed to create device and swap chain.\n"
			"This application requires a Direct3D 11 class device "
			"running on Windows Vista (or later)\n" );
		RETURN_ON_ERROR;
	}
*/

	return true;
}
//-------------------------------------------------------------------------------------------------------------//
static bool DisableAltEnter( const HWND inWindowHandle,
							const dxPtr< IDXGIFactory >& inDXGIFactory )
{
	// Prevent DXGI from monitoring message queue for the Alt-Enter key sequence
	// (which causes the application to switch from windowed to fullscreen or vice versa).
	// IDXGIFactory::MakeWindowAssociation is recommended,
	// because a standard control mechanism for the user is strongly desired.

	const HRESULT hr = inDXGIFactory->MakeWindowAssociation(
		inWindowHandle,
		DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER /*| DXGI_MWA_NO_PRINT_SCREEN*/
	);
	if( FAILED( hr ) ) {
		dxWARN( hr, "MakeWindowAssociation() failed" );
		return false;
	}

	return true;
}

static DXGI_FORMAT D3D11_Convert_DepthStencilFormat( DepthStencilFormat::Enum e )
{
	switch( e ) {
		case DepthStencilFormat::D16 :		return DXGI_FORMAT_D16_UNORM;
		case DepthStencilFormat::D24S8 :	return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case DepthStencilFormat::D32 :		return DXGI_FORMAT_D32_FLOAT;
		mxNO_SWITCH_DEFAULT;
	}
	return DXGI_FORMAT_UNKNOWN;
}

static D3D11_COMPARISON_FUNC D3D11_Convert_ComparisonFunc( ComparisonFunc::Enum e )
{
	switch( e ) {
		case ComparisonFunc::Always :		return D3D11_COMPARISON_ALWAYS;
		case ComparisonFunc::Never :		return D3D11_COMPARISON_NEVER;
		case ComparisonFunc::Less :			return D3D11_COMPARISON_LESS;
		case ComparisonFunc::Equal :		return D3D11_COMPARISON_EQUAL;
		case ComparisonFunc::Greater :		return D3D11_COMPARISON_ALWAYS;
		case ComparisonFunc::Not_Equal :	return D3D11_COMPARISON_NOT_EQUAL;
		case ComparisonFunc::Less_Equal :	return D3D11_COMPARISON_LESS_EQUAL;
		case ComparisonFunc::Greater_Equal :return D3D11_COMPARISON_GREATER_EQUAL;
		mxNO_SWITCH_DEFAULT;
	}
	return D3D11_COMPARISON_ALWAYS;
}

static D3D11_STENCIL_OP D3D11_Convert_StencilOp( StencilOp::Enum e )
{
	switch( e ) {
		case StencilOp::KEEP :		return D3D11_STENCIL_OP_KEEP;
		case StencilOp::ZERO :		return D3D11_STENCIL_OP_ZERO;
		case StencilOp::INCR_WRAP :		return D3D11_STENCIL_OP_INCR;
		case StencilOp::DECR_WRAP :		return D3D11_STENCIL_OP_DECR;
		case StencilOp::REPLACE :	return D3D11_STENCIL_OP_REPLACE;
		case StencilOp::INCR_SAT :	return D3D11_STENCIL_OP_INCR_SAT;
		case StencilOp::DECR_SAT :	return D3D11_STENCIL_OP_DECR_SAT;
		case StencilOp::INVERT :	return D3D11_STENCIL_OP_INVERT;
		mxNO_SWITCH_DEFAULT;
	}
	return D3D11_STENCIL_OP_KEEP;
}

static void D3D11_Convert_DepthStencil_Description( const DepthStencilDescription& input, D3D11_DEPTH_STENCIL_DESC &output )
{
	output.DepthEnable		= input.enableDepthTest;
	output.DepthWriteMask	= input.enableDepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	output.DepthFunc		= D3D11_Convert_ComparisonFunc( input.depthFunction );

	output.StencilEnable			= input.enableStencil;
	output.StencilReadMask			= input.stencilReadMask;
	output.StencilWriteMask			= input.stencilWriteMask;

	output.FrontFace.StencilFailOp			= D3D11_Convert_StencilOp(input.frontFace.stencilFailOp);
	output.FrontFace.StencilDepthFailOp		= D3D11_Convert_StencilOp(input.frontFace.depthFailOp);
	output.FrontFace.StencilPassOp			= D3D11_Convert_StencilOp(input.frontFace.stencilPassOp);
	output.FrontFace.StencilFunc			= D3D11_Convert_ComparisonFunc(input.frontFace.stencilFunction);

	output.BackFace.StencilFailOp			= D3D11_Convert_StencilOp(input.backFace.stencilFailOp);
	output.BackFace.StencilDepthFailOp		= D3D11_Convert_StencilOp(input.backFace.depthFailOp);
	output.BackFace.StencilPassOp			= D3D11_Convert_StencilOp(input.backFace.stencilPassOp);
	output.BackFace.StencilFunc				= D3D11_Convert_ComparisonFunc(input.backFace.stencilFunction);
}

static D3D11_FILL_MODE D3D11_Convert_FillMode( FillMode::Enum e )
{
	switch( e ) {
		case FillMode::Solid :		return D3D11_FILL_SOLID;
		case FillMode::Wireframe :	return D3D11_FILL_WIREFRAME;
		mxNO_SWITCH_DEFAULT;
	}
	return D3D11_FILL_SOLID;
}

static D3D11_CULL_MODE D3D11_Convert_CullMode( CullMode::Enum e )
{
	switch( e ) {
		case CullMode::None :	return D3D11_CULL_NONE;
		case CullMode::Back :	return D3D11_CULL_BACK;
		case CullMode::Front :	return D3D11_CULL_FRONT;
		mxNO_SWITCH_DEFAULT;
	}
	return D3D11_CULL_NONE;
}

static void D3D11_Convert_Rasterizer_Description( const RasterizerDescription& input, D3D11_RASTERIZER_DESC &output )
{
	output.FillMode					= D3D11_Convert_FillMode( input.fillMode );
	output.CullMode					= D3D11_Convert_CullMode( input.cullMode );
	output.FrontCounterClockwise	= TRUE;
	output.DepthBias				= 0;
	output.DepthBiasClamp			= 0.000000f;
	output.SlopeScaledDepthBias		= 0.000000f;
	output.DepthClipEnable			= input.enableDepthClip;
	output.ScissorEnable			= input.enableScissor;
	output.MultisampleEnable		= input.enableMultisample;
	output.AntialiasedLineEnable	= input.enableAntialiasedLine;
}

static D3D11_TEXTURE_ADDRESS_MODE D3D11_Convert_TextureAddressMode( TextureAddressMode::Enum e )
{
	switch( e ) {
		case TextureAddressMode::Wrap :			return D3D11_TEXTURE_ADDRESS_WRAP;
		case TextureAddressMode::Clamp :		return D3D11_TEXTURE_ADDRESS_CLAMP;
		case TextureAddressMode::Border :		return D3D11_TEXTURE_ADDRESS_BORDER;
		case TextureAddressMode::Mirror :		return D3D11_TEXTURE_ADDRESS_MIRROR;		
		case TextureAddressMode::MirrorOnce :	return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		mxNO_SWITCH_DEFAULT;
	}
	return D3D11_TEXTURE_ADDRESS_WRAP;
}

static D3D11_FILTER D3D11_Convert_TextureFilter( TextureFilter::Enum e, bool enableComparison )
{
	if( enableComparison )
	{
		switch( e )
		{
		case TextureFilter::Min_Mag_Mip_Point :               return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		case TextureFilter::Min_Mag_Point_Mip_Linear :        return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
		case TextureFilter::Min_Point_Mag_Linear_Mip_Point :  return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case TextureFilter::Min_Point_Mag_Mip_Linear :        return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
		case TextureFilter::Min_Linear_Mag_Mip_Point :        return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
		case TextureFilter::Min_Linear_Mag_Point_Mip_Linear : return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case TextureFilter::Min_Mag_Linear_Mip_Point :        return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		case TextureFilter::Min_Mag_Mip_Linear :              return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		case TextureFilter::Anisotropic :                     return D3D11_FILTER_COMPARISON_ANISOTROPIC;
			mxNO_SWITCH_DEFAULT;
		}
	}
	else
	{
		switch( e )
		{
		case TextureFilter::Min_Mag_Mip_Point :                          return D3D11_FILTER_MIN_MAG_MIP_POINT;
		case TextureFilter::Min_Mag_Point_Mip_Linear :                   return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		case TextureFilter::Min_Point_Mag_Linear_Mip_Point :             return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case TextureFilter::Min_Point_Mag_Mip_Linear :                   return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		case TextureFilter::Min_Linear_Mag_Mip_Point :                   return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		case TextureFilter::Min_Linear_Mag_Point_Mip_Linear :            return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case TextureFilter::Min_Mag_Linear_Mip_Point :                   return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case TextureFilter::Min_Mag_Mip_Linear :                         return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		case TextureFilter::Anisotropic :                                return D3D11_FILTER_ANISOTROPIC;
			mxNO_SWITCH_DEFAULT;
		}
	}
	return D3D11_FILTER_MIN_MAG_MIP_POINT;
}

static void D3D11_Convert_Sampler_Description( const SamplerDescription& input, D3D11_SAMPLER_DESC &output )
{
	bool enableComparison = (input.comparison != ComparisonFunc::Never && input.comparison != ComparisonFunc::Always);
	output.Filter	= D3D11_Convert_TextureFilter( input.filter, enableComparison );
	output.AddressU	= D3D11_Convert_TextureAddressMode( input.addressU );
	output.AddressV	= D3D11_Convert_TextureAddressMode( input.addressV );
	output.AddressW	= D3D11_Convert_TextureAddressMode( input.addressW );
	output.MipLODBias		= input.mipLODBias;
	output.MaxAnisotropy	= input.maxAnisotropy;
	output.ComparisonFunc	= D3D11_Convert_ComparisonFunc( input.comparison );
	output.BorderColor[0]	= input.borderColor[0];
	output.BorderColor[1]	= input.borderColor[1];
	output.BorderColor[2]	= input.borderColor[2];
	output.BorderColor[3]	= input.borderColor[3];
	output.MinLOD			= input.minLOD;
	output.MaxLOD			= input.maxLOD;
}

static D3D11_BLEND D3D11_Convert_BlendMode( BlendFactor::Enum e )
{
	switch( e ) {
	case BlendFactor::ZERO :				return D3D11_BLEND_ZERO;
	case BlendFactor::ONE :				return D3D11_BLEND_ONE;
	case BlendFactor::SRC_COLOR :			return D3D11_BLEND_SRC_COLOR;
	case BlendFactor::INV_SRC_COLOR :		return D3D11_BLEND_INV_SRC_COLOR;
	case BlendFactor::SRC_ALPHA :			return D3D11_BLEND_SRC_ALPHA;
	case BlendFactor::INV_SRC_ALPHA :		return D3D11_BLEND_INV_SRC_ALPHA;
	case BlendFactor::DST_ALPHA :		return D3D11_BLEND_DEST_ALPHA;
	case BlendFactor::INV_DST_ALPHA :	return D3D11_BLEND_INV_DEST_ALPHA;
	case BlendFactor::DST_COLOR :		return D3D11_BLEND_DEST_COLOR;
	case BlendFactor::INV_DST_COLOR :	return D3D11_BLEND_INV_DEST_COLOR;
	case BlendFactor::SRC_ALPHA_SAT :		return D3D11_BLEND_SRC_ALPHA_SAT;
	case BlendFactor::BLEND_FACTOR :		return D3D11_BLEND_BLEND_FACTOR;
	case BlendFactor::INV_BLEND_FACTOR :	return D3D11_BLEND_INV_BLEND_FACTOR;
	case BlendFactor::SRC1_COLOR :		return D3D11_BLEND_SRC1_COLOR;
	case BlendFactor::INV_SRC1_COLOR :	return D3D11_BLEND_INV_SRC1_COLOR;
	case BlendFactor::SRC1_ALPHA :		return D3D11_BLEND_SRC1_ALPHA;
	case BlendFactor::INV_SRC1_ALPHA :	return D3D11_BLEND_INV_SRC1_ALPHA;
	mxNO_SWITCH_DEFAULT;
	}
	return D3D11_BLEND_ONE;
}

static D3D11_BLEND_OP D3D11_Convert_BlendOp( BlendOp::Enum e )
{
	switch( e ) {
	case BlendOp::MIN :			return D3D11_BLEND_OP_MIN;
	case BlendOp::MAX :			return D3D11_BLEND_OP_MAX;
	case BlendOp::ADD :			return D3D11_BLEND_OP_ADD;
	case BlendOp::SUBTRACT :	return D3D11_BLEND_OP_SUBTRACT;
	case BlendOp::REV_SUBTRACT :return D3D11_BLEND_OP_REV_SUBTRACT;
	mxNO_SWITCH_DEFAULT;
	}
	return D3D11_BLEND_OP_ADD;
}

static UINT8 D3D11_Convert_ColorWriteMask( BColorWriteMask flags )
{
	UINT8 result = 0;
	if( flags & ColorWriteMask::ENABLE_RED )	result |= D3D11_COLOR_WRITE_ENABLE_RED;
	if( flags & ColorWriteMask::ENABLE_GREEN )	result |= D3D11_COLOR_WRITE_ENABLE_GREEN;
	if( flags & ColorWriteMask::ENABLE_BLUE )	result |= D3D11_COLOR_WRITE_ENABLE_BLUE;
	if( flags & ColorWriteMask::ENABLE_ALPHA )	result |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
	return result;
}

static void D3D11_Convert_Blend_Description( const BlendDescription& input, D3D11_BLEND_DESC &output )
{
	output.AlphaToCoverageEnable	= input.enableAlphaToCoverage;
	output.IndependentBlendEnable	= false;//input.enableIndependentBlend;

	output.RenderTarget[0].BlendEnable		= input.enableBlending;
	output.RenderTarget[0].SrcBlend			= D3D11_Convert_BlendMode( input.color.sourceFactor );
	output.RenderTarget[0].DestBlend		= D3D11_Convert_BlendMode( input.color.destinationFactor );
	output.RenderTarget[0].BlendOp			= D3D11_Convert_BlendOp( input.color.operation );
	output.RenderTarget[0].SrcBlendAlpha	= D3D11_Convert_BlendMode( input.alpha.sourceFactor );
	output.RenderTarget[0].DestBlendAlpha	= D3D11_Convert_BlendMode( input.alpha.destinationFactor );
	output.RenderTarget[0].BlendOpAlpha		= D3D11_Convert_BlendOp( input.alpha.operation );
	output.RenderTarget[0].RenderTargetWriteMask	= D3D11_Convert_ColorWriteMask( input.writeMask );
}

static D3D11_PRIMITIVE_TOPOLOGY D3D11_ConvertPrimitiveTopology( Topology::Enum topology )
{
	switch( topology )
	{
		case Topology::Undefined :		return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		case Topology::PointList :		return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		case Topology::LineList :		return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		case Topology::LineStrip :		return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case Topology::TriangleList :	return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case Topology::TriangleStrip :	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case Topology::TriangleFan :	/* fallthrough */
		default:						ptERROR("Invalid primitive topology");
	}
	return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
}


template< class OBJECT, class DESCRIPTION >
UINT32 FindObjectIndexByHash( THandleManager< OBJECT >& objects, const DESCRIPTION& info, UINT32 hash, const char* debugName )
{
	if( !objects.NumValidItems() ) {
		return -1;
	}
	THandleManager< OBJECT >::Iterator	it( objects );
	while( it.IsValid() )
	{
		OBJECT& o = it.Value();
		if( o.m_checksum == hash ) {
			ptWARN("Found duplicate object of type '%s' (name: '%s')",
				typeid(OBJECT).name(), debugName ? debugName : "<?>" );
			ptBREAK;//<= remove this
			return objects.GetContainedItemIndex( &o );
		}
		it.MoveToNext();
	}
	return -1;
}

//---------------------------------------------------------------------------

	void driverCreateDefaultStates()
	{
		//me.depthStencilStates.Reserve(MAX_CREATED_RENDER_STATES);
		//me.rasterizerStates.Reserve(MAX_CREATED_RENDER_STATES);
		//me.samplerStates.Reserve(MAX_CREATED_RENDER_STATES);
		//me.blendStates.Reserve(MAX_CREATED_RENDER_STATES);
		//me.colorTargets.Reserve(MAX_CREATED_RENDER_TARGETS);
		//me.textures.Reserve(MAX_CREATED_TEXTURES);
		//me.inputLayouts.Reserve(MAX_CREATED_INPUT_LAYOUTS);
		//me.buffers.Reserve(MAX_CREATED_MESHES);
		//me.shaders.Reserve(MAX_CREATED_SHADERS);
	}

	static ERet CreateBackBuffer( const DXGI_SWAP_CHAIN_DESC& scd );
	static ERet ReleaseBackBuffer();

	ERet driverInitialize( const void* context )
	{
		mxINITIALIZE_PRIVATE_DATA( gDriverData );

 		HWND hWnd = (HWND) context;

		dxPtr< IDXGIFactory >		factory;
		dxPtr< IDXGIAdapter >		adapter;
		dxPtr< IDXGIOutput >		output0;
		dxPtr< IDXGISwapChain >		swapChain;

		DeviceVendor::Enum		deviceVendor;
		chkRET_X_IF_NOT(SetupDXGI( factory, adapter, output0, deviceVendor ), ERR_UNKNOWN_ERROR);

		dxPtr< ID3D11Device >			device;
		dxPtr< ID3D11DeviceContext >	deviceContext;

		chkRET_X_IF_NOT(CreateDirect3DDevice( adapter, device, deviceContext ), ERR_UNKNOWN_ERROR);
		chkRET_X_IF_NOT(CheckFeatureLevel( device ), ERR_UNKNOWN_ERROR);

		LogDeviceInfo( device );

		DXGI_SWAP_CHAIN_DESC	swapChainDesc;
		GetSwapChainDescription( hWnd, me.device, swapChainDesc );
		chkRET_X_IF_NOT(CreateDirect3DSwapChain( swapChainDesc, factory, device, swapChain ), ERR_UNKNOWN_ERROR);

		me.device			= device;
		me.swapChain		= swapChain;
		me.output			= output0;

		mxZERO_OUT(me.resolution);

		driverCreateDefaultStates();

		DXGI_SWAP_CHAIN_DESC	swapChainInfo;
		me.swapChain->GetDesc( &swapChainInfo );
		mxDO(CreateBackBuffer( swapChainInfo ));

		me.immediateContext.Initialize(deviceContext);

		return ALL_OK;
	}

	void driverShutdown()
	{
		me.immediateContext.Shutdown();

		ReleaseBackBuffer();

		ptPRINT("D3D11: checking live objects...");
		TDestroyLiveObjects( me.programs );
		TDestroyLiveObjects( me.shaders );
		TDestroyLiveObjects( me.buffers );
		TDestroyLiveObjects( me.inputLayouts );
		TDestroyLiveObjects( me.textures );
		TDestroyLiveObjects( me.colorTargets );
		TDestroyLiveObjects( me.depthTargets );
		TDestroyLiveObjects( me.buffers );
		TDestroyLiveObjects( me.blendStates );
		TDestroyLiveObjects( me.samplerStates );
		TDestroyLiveObjects( me.rasterizerStates );
		TDestroyLiveObjects( me.depthStencilStates );

		mxZERO_OUT(me.resolution);

		me.output = NULL;
		me.swapChain = NULL;
		me.device = NULL;

		mxSHUTDOWN_PRIVATE_DATA( gDriverData );
	}

	ERet ReleaseBackBuffer()
	{
		//unresolved external symbol:
		//dxCHK(D3DX11UnsetAllDeviceObjects(me.deviceContext));

		return ALL_OK;
	}

	ERet CreateBackBuffer( const DXGI_SWAP_CHAIN_DESC& scd )
	{
		// Initialize back buffer. Grab back buffer texture and create main depth-stencil texture.
		// The application can re-query interfaces after calling ResizeBuffers() via IDXGISwapChain::GetBuffer().

		// Get a pointer to the back buffer.
		ID3D11Texture2D *	colorTexture = NULL;
		dxTRY(me.swapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**) &colorTexture ));
		//dxSetDebugName(colorTexture, "Main depth-stencil");

		// Create a render-target view for accessing the back buffer.
		ID3D11RenderTargetView *	renderTargetView = NULL;
		dxTRY(me.device->CreateRenderTargetView( colorTexture, NULL, &renderTargetView ));

		me.resolution.width = scd.BufferDesc.Width;
		me.resolution.height = scd.BufferDesc.Height;

		const HColorTarget handle = { me.colorTargets.Alloc() };
		ColorTargetD3D11 &	newRT = me.colorTargets[ handle.id ];
		newRT.m_colorTexture = colorTexture;
		newRT.m_RTV = renderTargetView;
		newRT.m_SRV = NULL;
		newRT.m_width = scd.BufferDesc.Width;
		newRT.m_height = scd.BufferDesc.Height;
		newRT.m_format = scd.BufferDesc.Format;
		newRT.m_flags = 0;

		return ALL_OK;
	}

	HContext driverGetMainContext()
	{
		HContext handle = { &me.immediateContext };
		return handle;
	}

	ColorTargetD3D11::ColorTargetD3D11()
	{
		m_colorTexture = NULL;
		m_RTV = NULL;
		m_SRV = NULL;
		m_width = 0;
		m_height = 0;
		m_format = DXGI_FORMAT_UNKNOWN;
		m_flags = 0;
	}
	void ColorTargetD3D11::Create( const ColorTargetDescription& rtInfo )
	{
		const TextureFormatInfoD3D11& format = gs_textureFormats[ rtInfo.format ];
		const DXGI_FORMAT nativeFormat = format.m_fmt;

		D3D11_TEXTURE2D_DESC	texDesc;
		texDesc.Width				= rtInfo.width;
		texDesc.Height				= rtInfo.height;
		texDesc.MipLevels			= 1;
		texDesc.ArraySize			= 1;
		texDesc.Format				= nativeFormat;
		texDesc.SampleDesc.Count	= 1;
		texDesc.SampleDesc.Quality	= 0;
		texDesc.Usage				= D3D11_USAGE_DEFAULT;
		texDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags		= 0;
		texDesc.MiscFlags			= 0;

		dxCHK(me.device->CreateTexture2D( &texDesc, NULL, &m_colorTexture ));
		dxCHK(me.device->CreateRenderTargetView( m_colorTexture, NULL, &m_RTV ));
		dxCHK(me.device->CreateShaderResourceView( m_colorTexture, NULL, &m_SRV ));

		dxSetDebugName(m_colorTexture, rtInfo.name.SafeGetPtr());

		m_width = rtInfo.width;
		m_height = rtInfo.height;
		m_format = nativeFormat;
		m_flags = 0;
	}
	void ColorTargetD3D11::Destroy()
	{
		DBGOUT("Destroying render target %ux%u", m_width, m_height );
		SAFE_RELEASE(m_colorTexture);
		SAFE_RELEASE(m_RTV);
		SAFE_RELEASE(m_SRV);
		m_width = 0;
		m_height = 0;
		m_format = DXGI_FORMAT_UNKNOWN;
		m_flags = 0;
	}

	DepthTargetD3D11::DepthTargetD3D11()
	{
		m_depthTexture = NULL;
		m_DSV = NULL;
		m_SRV = NULL;
		m_width = 0;
		m_height = 0;
		m_flags = 0;
	}
	void DepthTargetD3D11::Create( const DepthTargetDescription& dtInfo )
	{
		DXGI_FORMAT depthStencilFormat = D3D11_Convert_DepthStencilFormat( dtInfo.format );

		DXGI_FORMAT textureFormat = depthStencilFormat;
		// NOTE: the backing resource must be created as a typeless surface
		// so that it can be accessed in shaders via a shader resource view
		if( dtInfo.sample ) {
			textureFormat = DXGI_GetDepthStencil_Typeless_Format( textureFormat );
		}

		UINT sampleCount = 1;

		UINT textureBindFlags = D3D11_BIND_DEPTH_STENCIL;
		if( dtInfo.sample ) {
			textureBindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}

		D3D11_TEXTURE2D_DESC	texDesc;
		texDesc.Width				= dtInfo.width;
		texDesc.Height				= dtInfo.height;
		texDesc.MipLevels			= 1;
		texDesc.ArraySize			= 1;
		texDesc.Format				= textureFormat;
		texDesc.SampleDesc.Count	= sampleCount;
		texDesc.SampleDesc.Quality	= 0;
		texDesc.Usage				= D3D11_USAGE_DEFAULT;
		texDesc.BindFlags			= textureBindFlags;
		texDesc.CPUAccessFlags		= 0;
		texDesc.MiscFlags			= 0;
		dxCHK(me.device->CreateTexture2D( &texDesc, NULL, &m_depthTexture ));

#if 1
		D3D11_DEPTH_STENCIL_VIEW_DESC	dsvDesc;
		dsvDesc.Format				= depthStencilFormat;
		dsvDesc.ViewDimension		= (sampleCount > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags				= 0;//D3D11_DSV_READ_ONLY_DEPTH;	//< this flag can only be used with feature level D3D11 and above
		dsvDesc.Texture2D.MipSlice	= 0;
		dxCHK(me.device->CreateDepthStencilView( m_depthTexture, &dsvDesc, &m_DSV ));
#else
		dxCHK(me.device->CreateDepthStencilView( m_depthTexture, NULL, &m_DSV ));
#endif

		if( dtInfo.sample )
		{
			DXGI_FORMAT dsSRVFormat = DXGI_GetDepthStencil_SRV_Format( depthStencilFormat );

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			{
				srvDesc.Format = dsSRVFormat;
				srvDesc.ViewDimension = (sampleCount > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = 1;
				srvDesc.Texture2D.MostDetailedMip = 0;
			}
			dxCHK(me.device->CreateShaderResourceView( m_depthTexture, &srvDesc, &m_SRV ));
		}

		dxSetDebugName(m_depthTexture, dtInfo.name.SafeGetPtr());
		dxSetDebugName(m_DSV, dtInfo.name.SafeGetPtr());
		dxSetDebugName(m_SRV, dtInfo.name.SafeGetPtr());

		m_width = dtInfo.width;
		m_height = dtInfo.height;
		m_flags = 0;
	}
	void DepthTargetD3D11::Destroy()
	{
		DBGOUT("Destroying depth target %ux%u", m_width, m_height );
		SAFE_RELEASE(m_depthTexture);
		SAFE_RELEASE(m_DSV);
		SAFE_RELEASE(m_SRV);
		m_width = 0;
		m_height = 0;
		m_flags = 0;
	}

	HColorTarget driverCreateColorTarget( const ColorTargetDescription& rtInfo )
	{
		const HColorTarget handle = { me.colorTargets.Alloc() };
		ColorTargetD3D11 &	newRT = me.colorTargets[ handle.id ];
		newRT.Create( rtInfo );
		return handle;
	}
	void driverDeleteColorTarget( HColorTarget rt )
	{
		ColorTargetD3D11 &	renderTarget = me.colorTargets[ rt.id ];
		renderTarget.Destroy();
		me.colorTargets.Free( rt.id );
	}
	HDepthTarget driverCreateDepthTarget( const DepthTargetDescription& dtInfo )
	{
		const HDepthTarget handle = { me.depthTargets.Alloc() };
		DepthTargetD3D11 &	newDT = me.depthTargets[ handle.id ];
		newDT.Create( dtInfo );
		return handle;
	}
	void driverDeleteDepthTarget( HDepthTarget dt )
	{
		DepthTargetD3D11 &	depthTarget = me.depthTargets[ dt.id ];
		depthTarget.Destroy();
		me.depthTargets.Free( dt.id );
	}

	DepthStencilStateD3D11::DepthStencilStateD3D11()
	{
		m_ptr = NULL;
	}
	void DepthStencilStateD3D11::Create( const DepthStencilDescription& dsInfo )
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		D3D11_Convert_DepthStencil_Description( dsInfo, depthStencilDesc );
		dxCHK(me.device->CreateDepthStencilState( &depthStencilDesc, &m_ptr ));
		dxSetDebugName(m_ptr, dsInfo.name.ToPtr());
	}
	void DepthStencilStateD3D11::Destroy()
	{
		SAFE_RELEASE(m_ptr);
	}
	HDepthStencilState driverCreateDepthStencilState( const DepthStencilDescription& dsInfo )
	{
		const HDepthStencilState handle = { me.depthStencilStates.Alloc() };
		me.depthStencilStates[ handle.id ].Create( dsInfo );
		return handle;
	}

	RasterizerStateD3D11::RasterizerStateD3D11()
	{
		m_ptr = NULL;
	}
	void RasterizerStateD3D11::Create( const RasterizerDescription& rsInfo )
	{
		D3D11_RASTERIZER_DESC rasterizerDesc;
		D3D11_Convert_Rasterizer_Description( rsInfo, rasterizerDesc );
		dxCHK(me.device->CreateRasterizerState( &rasterizerDesc, &m_ptr ));
		dxSetDebugName(m_ptr, rsInfo.name.ToPtr());
	}
	void RasterizerStateD3D11::Destroy()
	{
		SAFE_RELEASE(m_ptr);
	}
	HRasterizerState driverCreateRasterizerState( const RasterizerDescription& rsInfo )
	{
		const HRasterizerState handle = { me.rasterizerStates.Alloc() };
		me.rasterizerStates[ handle.id ].Create( rsInfo );
		return handle;
	}

	SamplerStateD3D11::SamplerStateD3D11()
	{
		m_ptr = NULL;
	}
	void SamplerStateD3D11::Create( const SamplerDescription& ssInfo )
	{
		D3D11_SAMPLER_DESC samplerDesc;
		D3D11_Convert_Sampler_Description( ssInfo, samplerDesc );
		dxCHK(me.device->CreateSamplerState( &samplerDesc, &m_ptr ));
		dxSetDebugName(m_ptr, ssInfo.name.SafeGetPtr());
	}
	void SamplerStateD3D11::Destroy()
	{
		SAFE_RELEASE(m_ptr);
	}
	HSamplerState driverCreateSamplerState( const SamplerDescription& ssInfo )
	{
		const HSamplerState handle = { me.samplerStates.Alloc() };
		me.samplerStates[ handle.id ].Create( ssInfo );
		return handle;
	}

	BlendStateD3D11::BlendStateD3D11()
	{
		m_blendState = NULL;
	}
	void BlendStateD3D11::Create( const BlendDescription& bsInfo )
	{
		D3D11_BLEND_DESC blendDesc;
		D3D11_Convert_Blend_Description( bsInfo, blendDesc );
		dxCHK(me.device->CreateBlendState( &blendDesc, &m_blendState ));
		dxSetDebugName(m_blendState, bsInfo.name.ToPtr());
	}
	void BlendStateD3D11::Destroy()
	{
		SAFE_RELEASE(m_blendState);
	}
	HBlendState driverCreateBlendState( const BlendDescription& bsInfo )
	{
		const HBlendState handle = { me.blendStates.Alloc() };
		me.blendStates[ handle.id ].Create( bsInfo );
		return handle;
	}

	void driverDeleteDepthStencilState( HDepthStencilState ds )
	{
		me.depthStencilStates[ ds.id ].Destroy();
		me.depthStencilStates.Free( ds.id );
	}
	void driverDeleteRasterizerState( HRasterizerState rs )
	{	
		me.rasterizerStates[ rs.id ].Destroy();
		me.rasterizerStates.Free( rs.id );
	}
	void driverDeleteSamplerState( HSamplerState ss )
	{
		me.samplerStates[ ss.id ].Destroy();
		me.samplerStates.Free( ss.id );
	}
	void driverDeleteBlendState( HBlendState bs )
	{
		me.blendStates[ bs.id ].Destroy();
		me.blendStates.Free( bs.id );
	}

	InputLayoutD3D11::InputLayoutD3D11()
	{
		m_layout = NULL;
		m_checksum = 0;
		m_numSlots = 0;
		mxZERO_OUT(m_strides);
	}
	static const char* gs_semanticName[VertexAttribute::Count] = {
		"Position",
		"Color", "Color",
		"Normal", "Tangent", "Binormal",
		"TexCoord",	"TexCoord",	"TexCoord",	"TexCoord",	"TexCoord",	"TexCoord",	"TexCoord",	"TexCoord",
		"BoneWeights", "BoneIndices",
	};
	static const UINT gs_semanticIndex[VertexAttribute::Count] = {
		0,
		0, 1,
		0, 0, 0,
		0, 1, 2, 3, 4, 5, 6, 7,
		0, 0,
	};
	// [attrib_type] [attrib_count] [is_normalized]
	static const DXGI_FORMAT gs_attribType[AttributeType::Count][4][2] = {
		// Signed Byte
		{
			{ DXGI_FORMAT_R8_SINT,            DXGI_FORMAT_R8_SNORM           },
			{ DXGI_FORMAT_R8G8_SINT,          DXGI_FORMAT_R8G8_SNORM         },
			{ DXGI_FORMAT_R8G8B8A8_SINT,      DXGI_FORMAT_R8G8B8A8_SNORM     },
			{ DXGI_FORMAT_R8G8B8A8_SINT,      DXGI_FORMAT_R8G8B8A8_SNORM     },
		},
		// Unsigned Byte
		{
			{ DXGI_FORMAT_R8_UINT,            DXGI_FORMAT_R8_UNORM           },
			{ DXGI_FORMAT_R8G8_UINT,          DXGI_FORMAT_R8G8_UNORM         },
			{ DXGI_FORMAT_R8G8B8A8_UINT,      DXGI_FORMAT_R8G8B8A8_UNORM     },
			{ DXGI_FORMAT_R8G8B8A8_UINT,      DXGI_FORMAT_R8G8B8A8_UNORM     },
		},
		// Signed Short
		{
			{ DXGI_FORMAT_R16_SINT,           DXGI_FORMAT_R16_SNORM          },
			{ DXGI_FORMAT_R16G16_SINT,        DXGI_FORMAT_R16G16_SNORM       },
			{ DXGI_FORMAT_R16G16B16A16_SINT,  DXGI_FORMAT_R16G16B16A16_SNORM },
			{ DXGI_FORMAT_R16G16B16A16_SINT,  DXGI_FORMAT_R16G16B16A16_SNORM },
		},
		// Unsigned Short
		{
			{ DXGI_FORMAT_R16_UINT,           DXGI_FORMAT_R16_UNORM          },
			{ DXGI_FORMAT_R16G16_UINT,        DXGI_FORMAT_R16G16_UNORM       },
			{ DXGI_FORMAT_R16G16B16A16_UINT,  DXGI_FORMAT_R16G16B16A16_UNORM },
			{ DXGI_FORMAT_R16G16B16A16_UINT,  DXGI_FORMAT_R16G16B16A16_UNORM },
		},

		// Half
		{
			{ DXGI_FORMAT_R16_FLOAT,          DXGI_FORMAT_R16_FLOAT          },
			{ DXGI_FORMAT_R16G16_FLOAT,       DXGI_FORMAT_R16G16_FLOAT       },
			{ DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT },
			{ DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT },
		},
		// Float
		{
			{ DXGI_FORMAT_R32_FLOAT,          DXGI_FORMAT_R32_FLOAT          },
			{ DXGI_FORMAT_R32G32_FLOAT,       DXGI_FORMAT_R32G32_FLOAT       },
			{ DXGI_FORMAT_R32G32B32_FLOAT,    DXGI_FORMAT_R32G32B32_FLOAT    },
			{ DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT },
		},
	};

	// [attrib_type] [is_normalized]
	static const char* gs_nameInShader[AttributeType::Count][2] = {
		// Signed Byte
		{ "int", "float" },
		// Unsigned Byte
		{ "uint", "float" },
		// Signed Short
		{ "int", "float" },
		// Unsigned Short
		{ "uint", "float" },

		// Half
		{ "half", "float" },
		// Float
		{ "float", "float" },
	};

	void InputLayoutD3D11::Create( const VertexDescription& desc, UINT32 hash )
	{
		// Compile a dummy vertex shader and retrieve its input signature.
		typedef TStaticList< ANSICHAR, 1024 >	CodeSnippet;

		D3D11_INPUT_ELEMENT_DESC	elementDescriptions[LLGL_MAX_VERTEX_ATTRIBS];
		mxASSERT(desc.attribCount <= ARRAYSIZE(elementDescriptions));

		UINT	numInputSlots = 1;

		CodeSnippet				dummyVertexShaderCode(_InitZero);
		CodeSnippet::OStream	stream = dummyVertexShaderCode.GetOStream();
		TextStream				tw( stream );
		{
			tw << "void F(\n";
			for( int i = 0; i < desc.attribCount; i++ )
			{
				const VertexElement& element = desc.attribsArray[ i ];

				numInputSlots = largest(numInputSlots, element.inputSlot+1);

				D3D11_INPUT_ELEMENT_DESC &	elemDesc = elementDescriptions[i];
				elemDesc.SemanticName			= gs_semanticName[element.semantic];
				elemDesc.SemanticIndex			= gs_semanticIndex[element.semantic];
				elemDesc.Format					= gs_attribType[element.type][element.dimension][element.normalized];
				elemDesc.InputSlot				= element.inputSlot;
				elemDesc.AlignedByteOffset		= desc.attribOffsets[i];
				elemDesc.InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
				elemDesc.InstanceDataStepRate	= 0;

				char shaderSemantic[64];
				sprintf_s(shaderSemantic, "%s%u", elemDesc.SemanticName, elemDesc.SemanticIndex );

				char shaderTypeName[32];
				sprintf_s(shaderTypeName, "%s%u", gs_nameInShader[element.type][element.normalized], element.dimension+1 );

 				tw.PrintF("%s _%u : %s%s\n"
					,shaderTypeName
					,i
					,shaderSemantic
					, (i != desc.attribCount-1) ? "," : ""
				);
			}
			tw << "){}\n";
			;
		}

		DBGOUT("Input signature: %s", dummyVertexShaderCode.ToPtr());

		for( int i = 0; i < desc.attribCount; i++ )
		{
			const D3D11_INPUT_ELEMENT_DESC& element = elementDescriptions[i];
			DBGOUT("%s[%u]: %s at %u in slot %u", element.SemanticName, element.SemanticIndex, DXGI_FORMAT_ToChars(element.Format), element.AlignedByteOffset, element.InputSlot);
		}

		const D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		mxASSERT(featureLevel == D3D_FEATURE_LEVEL_11_0);
		const char* vertexShaderProfile = "vs_5_0";

		dxPtr< ID3DBlob >	compiledCode;

		dxCHK(::D3DX11CompileFromMemory(
			dummyVertexShaderCode.ToPtr(),
			dummyVertexShaderCode.Num(),
			NULL,	// Optional. The name of the shader file. Use either this or pSrcData.
			NULL,	// const D3D_SHADER_MACRO* defines
			NULL,	// ID3DInclude* includeHandler
			"F",	// const char* entryPoint,
			vertexShaderProfile,
			0,		// Shader compile options
			0,		// Effect compile options.
			NULL,	// ID3DX11ThreadPump* pPump
			&compiledCode.Ptr,
			NULL,	// Optional. A pointer to an ID3D10Blob that contains compiler error messages, or NULL if there were no errors.
			NULL		// HRESULT* pHResult
		));

		mxASSERT(compiledCode.IsValid());

		const void* bytecode = compiledCode->GetBufferPointer();
		size_t bytecodeLength = compiledCode->GetBufferSize();

		mxASSERT_PTR(bytecode);
		mxASSERT(bytecodeLength > 0);

		// Create a new vertex declaration.
		dxCHK(me.device->CreateInputLayout(
			elementDescriptions,
			desc.attribCount,
			bytecode,
			bytecodeLength,
			&m_layout
		));

		m_checksum = hash;
		m_numSlots = numInputSlots;
		TCopyStaticPODArray( m_strides, desc.streamStrides, numInputSlots );
	}
	void InputLayoutD3D11::Destroy()
	{
		SAFE_RELEASE(m_layout);
		m_checksum = 0;
		m_numSlots = 0;
		mxZERO_OUT(m_strides);
	}
	HInputLayout driverCreateInputLayout( const VertexDescription& desc, const char* name )
	{
		const UINT32 hash = MurmurHash32( &desc, sizeof(desc) );
		for( int i = 0; i < me.inputLayouts.Num(); i++ )
		{
			const InputLayoutD3D11& layout = me.inputLayouts[ i ];
			if( layout.m_checksum == hash ) {
				DBGOUT("Returning existing input layout '%s'", name ? name : "<?>");
				const HInputLayout handle = { i };
				return handle;
			}
		}
		DBGOUT("Creating input layout '%s'", name ? name : "<?>");
		const HInputLayout handle = { me.inputLayouts.Alloc() };
		InputLayoutD3D11 &	layout = me.inputLayouts[ handle.id ];
		layout.Create( desc, hash );
		dxSetDebugName( layout.m_layout, name );
		return handle;
	}
	void driverDeleteInputLayout( HInputLayout handle )
	{
		InputLayoutD3D11 &	layout = me.inputLayouts[ handle.id ];
		layout.Destroy();
		me.inputLayouts.Free( handle.id );
	}

	static ID3D11Buffer* D3D11_CreateBuffer(
		const UINT size,
		const void* data,
		const bool dynamic,
		const UINT bindFlags,
		ID3D11Device* device
	){
		mxASSERT( size > 0 );

		D3D11_BUFFER_DESC	bufferDesc;
		bufferDesc.BindFlags		= bindFlags;
		bufferDesc.Usage			= dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth		= size;
		bufferDesc.CPUAccessFlags	= dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		bufferDesc.MiscFlags		= 0;
		bufferDesc.StructureByteStride	= 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = data;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		ID3D11Buffer *	d3dBuffer;
		dxCHK(device->CreateBuffer(
			&bufferDesc,
			data ? &initData : NULL,
			&d3dBuffer
		));

		return d3dBuffer;
	}

	BufferD3D11::BufferD3D11()
	{
		m_ptr = NULL;
	}
	void BufferD3D11::Destroy()
	{
		SAFE_RELEASE(m_ptr);
	}

	HBuffer driverCreateBuffer( EBufferType type, const void* data, UINT size )
	{
		const HBuffer handle = { me.buffers.Alloc() };
		BufferD3D11& newBuffer = me.buffers[ handle.id ];

		const bool dynamic = (data == NULL);

		if( type == Buffer_Uniform ) {
			newBuffer.m_ptr = D3D11_CreateBuffer( size, data, true, D3D11_BIND_CONSTANT_BUFFER, me.device );
		}
		else if( type == Buffer_Vertex ) {
			newBuffer.m_ptr = D3D11_CreateBuffer( size, data, dynamic, D3D11_BIND_VERTEX_BUFFER, me.device );
		}
		else if( type == Buffer_Index ) {
			newBuffer.m_ptr = D3D11_CreateBuffer( size, data, dynamic, D3D11_BIND_INDEX_BUFFER, me.device );
		}
		else {
			mxUNREACHABLE;
		}

		return handle;
	}
	void driverDeleteBuffer( HBuffer handle )
	{
		me.buffers[ handle.id ].Destroy();
		me.buffers.Free( handle.id );
	}

	ShaderD3D11::ShaderD3D11()
	{
		m_ptr = NULL;
	}
	void ShaderD3D11::Create( EShaderType shaderType, const void* compiledBytecode, UINT bytecodeLength )
	{
		//UINT32 disassembleFlags = 0;
		//ID3DBlob* disassembly = NULL;
		//D3DDisassemble(
		//	compiledBytecode,
		//	bytecodeLength,
		//	disassembleFlags,
		//	NULL,	// LPCSTR szComments
		//	&disassembly
		//	);
		//if(disassembly){
		//	Util_SaveDataToFile(disassembly->GetBufferPointer(), disassembly->GetBufferSize(),
		//		Str::SPrintF<String64>("R:/%s.asm",g_shaderTypeName[shaderType].buffer).ToPtr());
		//	disassembly->Release();
		//	disassembly = NULL;
		//}

		ID3D11ClassLinkage* linkage = NULL;

		switch( shaderType )
		{
		case ShaderVertex :
			dxCHK(me.device->CreateVertexShader( compiledBytecode, bytecodeLength, linkage, &m_VS ));
			break;

		case ShaderHull :
			dxCHK(me.device->CreateHullShader( compiledBytecode, bytecodeLength, linkage, &m_HS ));
			break;

		case ShaderDomain :
			dxCHK(me.device->CreateDomainShader( compiledBytecode, bytecodeLength, linkage, &m_DS ));
			break;

		case ShaderGeometry :
			dxCHK(me.device->CreateGeometryShader( compiledBytecode, bytecodeLength, linkage, &m_GS ));
			break;

		case ShaderFragment :
			dxCHK(me.device->CreatePixelShader( compiledBytecode, bytecodeLength, linkage, &m_PS ));
			break;

		//case ShaderCompute :
		//	dxCHK(me.device->CreateComputeShader( compiledBytecode, bytecodeLength, linkage, &m_CS ));
		//	break;

		mxNO_SWITCH_DEFAULT;
		}
	}
	void ShaderD3D11::Destroy()
	{
		SAFE_RELEASE(m_ptr);
	}

	HShader driverCreateShader( EShaderType shaderType, const void* compiledBytecode, UINT bytecodeLength )
	{
		const HShader handle = { me.shaders.Alloc() };
		me.shaders[ handle.id ].Create( shaderType, compiledBytecode, bytecodeLength );
		return handle;
	}
	void driverDeleteShader( HShader handle )
	{
		me.shaders[ handle.id ].Destroy();
		me.shaders.Free( handle.id );
	}
	HProgram driverCreateProgram( const ProgramDescription& pd )
	{
		const HProgram handle = { me.programs.Alloc() };
		ProgramD3D11& program = me.programs[ handle.id ];
		program.VS = pd.shaders[ShaderVertex];
		program.HS = pd.shaders[ShaderHull];
		program.DS = pd.shaders[ShaderDomain];
		program.GS = pd.shaders[ShaderGeometry];
		program.PS = pd.shaders[ShaderFragment];

#if LL_DEBUG_LEVEL >= 3
		mxASSERT_PTR(pd.bindings);
		program.name = pd.name;
		program.bindings = *pd.bindings;		
#endif
		return handle;
	}
	void driverDeleteProgram( HProgram handle )
	{
		me.programs.Free( handle.id );
	}

	TextureD3D11::TextureD3D11()
	{
		m_resource = NULL;
		m_SRV = NULL;
		//m_type = TextureType::TEXTURE_2D;
		m_numMips = 0;
	}
	void TextureD3D11::Create( const void* _data, UINT _size )
	{
		const UINT32 magicNum = *(UINT32*) _data;
		if( magicNum == TEXTURE_MAGIC_NUM )
		{
			const TextureHeader& header = *(TextureHeader*) _data;
			const void* imageData = mxAddByteOffset(_data, sizeof(TextureHeader));

			const PixelFormat::Enum textureFormat = (PixelFormat::Enum) header.format;

			TextureImage image;
			image.data		= imageData;
			image.size		= header.size;
			image.width		= header.width;
			image.height	= header.height;
			image.depth		= header.depth;
			image.format	= textureFormat;
			image.numMips	= header.numMips;
			image.isCubeMap	= false;

			this->CreateInternal(image);
		}
		else if( magicNum == DDS_MAGIC_NUM )
		{
			TextureImage	image;
			//DDS_Parse( _data, _size, image );
			UNDONE;

			this->CreateInternal(image);
		}
		else
		{
			//int tex_x, tex_y, tex_comp;
			//void* tex_data = stbi_load_from_memory( (const unsigned char*)_data, (int)_size, &tex_x, &tex_y, &tex_comp, 0 );
			//if( tex_data )
			//{
			//	TextureImage image;
			//	image.data		= tex_data;
			//	image.size		= tex_x * tex_y * sizeof(UINT32);
			//	image.width		= tex_x;
			//	image.height	= tex_y;
			//	image.depth		= 1;
			//	image.format	= PixelFormat::RGBA8;
			//	image.numMips	= 1;
			//	image.isCubeMap	= false;

			//	this->CreateInternal(image);
			//}
			//else
			{
				ptERROR("Unknown texture type");
			}
		}

		mxASSERT_PTR(m_resource);
		mxASSERT_PTR(m_SRV);
	}
	void TextureD3D11::Create( const Texture2DDescription& txInfo, const void* imageData )
	{
		TextureImage image;
		image.data		= imageData;
		image.size		= CalculateTextureSize(txInfo.width, txInfo.height, txInfo.format, txInfo.numMips);
		image.width		= txInfo.width;
		image.height	= txInfo.height;
		image.depth		= 1;
		image.format	= txInfo.format;
		image.numMips	= txInfo.numMips;
		image.isCubeMap	= false;

		this->CreateInternal( image, txInfo.dynamic );
	}
	void TextureD3D11::Destroy()
	{
		SAFE_RELEASE(m_resource);
		SAFE_RELEASE(m_SRV);
		//m_type = TextureType::TEXTURE_2D;
		m_numMips = 0;
	}
	void TextureD3D11::CreateInternal( const TextureImage& _image, bool _dynamic )
	{
		const PixelFormat::Enum textureFormat = _image.format;
		const TextureFormatInfoD3D11& format = gs_textureFormats[ textureFormat ];

		if(!_dynamic)
			mxASSERT_PTR(_image.data);
#if 0
		MipLevel	mips[ LLGL_MAX_TEXTURE_MIP_LEVELS ];
		mxASSERT(_image.numMips <= mxCOUNT_OF(mips));

		D3D11_SUBRESOURCE_DATA	initialData[ LLGL_MAX_TEXTURE_MIP_LEVELS ];
		if( _image.data != NULL )
		{
			ParseMipLevels( _image, 0, mips, mxCOUNT_OF(mips) );
			for( UINT lodIndex = 0; lodIndex < _image.numMips; lodIndex++ )
			{
				const MipLevel& mipLevel = mips[ lodIndex ];
				initialData[ lodIndex ].pSysMem	= mipLevel.data;
				initialData[ lodIndex ].SysMemPitch	= mipLevel.pitch;
				initialData[ lodIndex ].SysMemSlicePitch = 0;
			}
		}
#endif
		const int numSides = _image.isCubeMap ? 6 : 1;
		const int numSrd = _image.numMips * numSides;
		D3D11_SUBRESOURCE_DATA* initialData = NULL;

		if( _image.data )
		{
			initialData = (D3D11_SUBRESOURCE_DATA*) alloca( numSrd * sizeof(D3D11_SUBRESOURCE_DATA) );
			int kk = 0;
			for( UINT sideIndex = 0; sideIndex < numSides; sideIndex++ )
			{
				MipLevel	mips[ LLGL_MAX_TEXTURE_MIP_LEVELS ];
				mxASSERT(_image.numMips <= mxCOUNT_OF(mips));

				ParseMipLevels( _image, sideIndex, mips, mxCOUNT_OF(mips) );

				for( UINT lodIndex = 0; lodIndex < _image.numMips; lodIndex++ )
				{
					const MipLevel& mip = mips[ lodIndex ];
					initialData[ kk ].pSysMem	= mip.data;
					initialData[ kk ].SysMemPitch	= mip.pitch;
					initialData[ kk ].SysMemSlicePitch = 0;
					++kk;
				}
			}
		}

		D3D11_TEXTURE2D_DESC	texDesc;
		texDesc.Width			= _image.width;
		texDesc.Height			= _image.height;
		texDesc.MipLevels		= _image.numMips;
		texDesc.Format			= format.m_fmt;
		texDesc.SampleDesc.Count	= 1;
		texDesc.SampleDesc.Quality	= 0;
		texDesc.Usage			= _dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		texDesc.BindFlags		= D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags	= _dynamic ? D3D11_CPU_ACCESS_WRITE : 0;

		D3D11_SHADER_RESOURCE_VIEW_DESC	srvDesc;
		srvDesc.Format				= texDesc.Format;

		if( _image.isCubeMap )
		{
			texDesc.ArraySize	= 6;
			texDesc.MiscFlags	= D3D11_RESOURCE_MISC_TEXTURECUBE;

			srvDesc.ViewDimension	= D3D11_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip	= 0;
			srvDesc.TextureCube.MipLevels = _image.numMips;
		}
		else
		{
			texDesc.ArraySize	= 1;
			texDesc.MiscFlags	= 0;

			srvDesc.ViewDimension	= D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip	= 0;
			srvDesc.Texture2D.MipLevels = _image.numMips;
		}

		dxCHK(me.device->CreateTexture2D( &texDesc, initialData, &m_texture2D ));
		dxCHK(me.device->CreateShaderResourceView( m_resource, NULL, &m_SRV ));
		//m_type = TextureType::TEXTURE_2D;
		m_numMips = _image.numMips;
	}

	HTexture driverCreateTexture( const void* data, UINT size )
	{
		const HTexture textureHandle = { me.textures.Alloc() };
		TextureD3D11 &	newTexture = me.textures[ textureHandle.id ];
		newTexture.Create( data, size );
		return textureHandle;
	}
	HTexture driverCreateTexture2D( const Texture2DDescription& txInfo, const void* imageData )
	{
		const HTexture textureHandle = { me.textures.Alloc() };
		TextureD3D11 &	newTexture2D = me.textures[ textureHandle.id ];
		newTexture2D.Create( txInfo, imageData );
		return textureHandle;
	}
	HTexture driverCreateTexture3D( const Texture3DDescription& txInfo, const Memory* initialData )
	{
		UNDONE;
		const HTexture handle = { LLGL_NULL_HANDLE };
		return handle;
	}
	void driverDeleteTexture( HTexture tx )
	{
		TextureD3D11 &	texture = me.textures[ tx.id ];
		texture.Destroy();
		me.textures.Free( tx.id );
	}

	static inline HResource MakeResourceHandle( EShaderResourceType type, UINT16 index )
	{
		HResource handle;
		handle.id = (index << SRT_NumBits) | type;
		return handle;
	}
	static inline ID3D11ShaderResourceView* GetResourceByHandle( HResource handle )
	{
		if( handle.IsNull() ) {
			return NULL;
		}
		ID3D11ShaderResourceView* pSRV = NULL;
		const UINT type = handle.id & ((1 << SRT_NumBits)-1);
		const UINT index = ((UINT)handle.id >> SRT_NumBits);

		//if( type == SRT_Buffer ) {
		//	pSRV = me.buffers[ index ].m_SRV;
		//}
		if( type == SRT_Texture ) {
			pSRV = me.textures[ index ].m_SRV;
		}
		else if( type == SRT_ColorSurface ) {
			pSRV = me.colorTargets[ index ].m_SRV;
		}
		else if( type == SRT_DepthSurface ) {
			pSRV = me.depthTargets[ index ].m_SRV;
		}
		mxASSERT_PTR(pSRV);

		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		pSRV->GetDesc(&desc);

		return pSRV;
	}

	HResource driverGetShaderResource( HBuffer br )
	{
		return MakeResourceHandle( SRT_Buffer, br.id );
	}
	HResource driverGetShaderResource( HTexture tx )
	{
		return MakeResourceHandle( SRT_Texture, tx.id );
	}
	HResource driverGetShaderResource( HColorTarget rt )
	{
		return MakeResourceHandle( SRT_ColorSurface, rt.id );
	}
	HResource driverGetShaderResource( HDepthTarget dt )
	{
		return MakeResourceHandle( SRT_DepthSurface, dt.id );
	}

	ProgramD3D11::ProgramD3D11()
	{
		VS.SetNil();
		HS.SetNil();
		DS.SetNil();
		GS.SetNil();
		PS.SetNil();
	}
	void ProgramD3D11::Destroy()
	{
		// Nothing.
	}

	void* GetDirect3DDevice()
	{
		return me.device;
	}
	void* GetDirect3DSwapChain()
	{
		return me.swapChain;
	}


	void driverUpdateVideoMode()
	{
#if 0
		DXGI_SWAP_CHAIN_DESC	swapChainInfo;
		me.swapChain->GetDesc( &swapChainInfo );

		if( me.resetFlags != 0
		|| swapChainInfo.BufferDesc.Width != (UINT)me.resolution.width
		|| swapChainInfo.BufferDesc.Height != (UINT)me.resolution.height )
		{
			swapChainInfo.BufferDesc.Width = (UINT)me.resolution.width;
			swapChainInfo.BufferDesc.Height = (UINT)me.resolution.height;

			ReleaseBackBuffer();

			if( me.resetFlags & RESET_FULLSCREEN ) {
				UNDONE;
			}
			if( me.resetFlags & RESET_VSYNC ) {
				UNDONE;
			}

			//	A swapchain cannot be resized unless all outstanding references
			//	to its back buffers have been released.
			//	The application must release all of its direct and indirect references
			//	on the backbuffers in order for ResizeBuffers to succeed.
			//	Direct references are held by the application after calling AddRef on a resource.
			//	Indirect references are held by views to a resource,
			//	binding a view of the resource to a device context,
			//	a command list that used the resource,
			//	a command list that used a view to that resource,
			//	a command list that executed another command list that used the resource, etc.

			//	Before calling ResizeBuffers,
			//	ensure that the application releases all references
			//	(by calling the appropriate number of Release invocations) on the resources,
			//	any views to the resource, any command lists that use either the resources or views,
			//	and ensure that neither the resource, nor a view is still bound to a device context.
			//	ClearState can be used to ensure this.
			//	If a view is bound to a deferred context,
			//	then the partially built command list must be discarded as well
			//	(by calling ClearState, FinishCommandList, then Release on the command list).

			dxCHK(me.swapChain->ResizeBuffers(
				swapChainInfo.BufferCount,
				swapChainInfo.BufferDesc.Width,
				swapChainInfo.BufferDesc.Height,
				swapChainInfo.BufferDesc.Format,
				DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
			));

			// Recreate render target and depth-stencil.
			CreateBackBuffer( swapChainInfo );

			me.resetFlags = 0;
		}
#endif
	}

	// redundancy checking

// overwrite differing values with nulls - this option is enabled by default (better for debugging/less instructions)
#define UNBIND_UNUSED_RESOURCES_ANYWAY    (0)

    struct DifferenceDelta
    {
        UINT32    delta;    // bit mask of changed states
        UINT32    first;    // valid only if delta != 0
        UINT32    last;    // valid only if delta != 0
        UINT32    count;    // valid only if delta != 0
    public:
        template< typename HANDLE_TYPE, const UINT MAX_COUNT >
        inline DifferenceDelta( const HANDLE_TYPE (&_old)[MAX_COUNT], const HANDLE_TYPE (&_new)[MAX_COUNT] )
        {
            UINT32    mask = 0;
            for( UINT i = 0; i < MAX_COUNT; i++ )
            {
                UINT32 diff = MapTo01( _old[i].id ^ _new[i].id );

            #if UNBIND_UNUSED_RESOURCES_ANYWAY
                mask |= (diff << i);
            #else
                mask |= _new[i].IsValid() ? (diff << i) : 0;
            #endif
            }
            DWORD nMinSlot, nMaxSlot;
            _BitScanForward( &nMinSlot, mask );
            _BitScanReverse( &nMaxSlot, mask );

            this->delta = mask;
            this->first = nMinSlot;
            this->last = nMaxSlot;
            this->count = nMaxSlot - nMinSlot + 1;
        }
    };


	// could use memcmp() to determine if the arrays are different
	template< typename HANDLE_TYPE, const UINT MAX_COUNT >
	static inline UINT32 CalculateDifference( const HANDLE_TYPE (&_old)[MAX_COUNT], const HANDLE_TYPE (&_new)[MAX_COUNT] )
	{
		UINT32	mask = 0;
		for( UINT i = 0; i < MAX_COUNT; i++ )
		{
			UINT32 diff = MapTo01( _old[i].id ^ _new[i].id );
			mask |= (diff << i);
		}
		return mask;
	}

	void driverSubmitFrame( CommandBuffer & commands, UINT size )
	{
		driverUpdateVideoMode();

		me.immediateContext.EndFrame();

		dxCHK(me.swapChain->Present(
			0,		// UINT SyncInterval
			0		// UINT Flags
		));
	}

	DeviceContext::DeviceContext()
	{
		m_deviceContext = NULL;

		this->ResetState();
	}
	DeviceContext::~DeviceContext()
	{
	}
	void DeviceContext::Initialize( ID3D11DeviceContext* context11 )
	{
		SAFE_ADDREF(context11);
		m_deviceContext = context11;
	}
	void DeviceContext::Shutdown()
	{
		// Cleanup (aka make the runtime happy)
		m_deviceContext->ClearState();
		SAFE_RELEASE(m_deviceContext);
	}
	void DeviceContext::ResetState()
	{
		//m_renderTargets = UINT32_Replicate(LLGL_NULL_HANDLE);

		mxZERO_OUT(m_viewport);

		m_rasterizerState.SetNil();
		m_depthStencilState.SetNil();
		m_stencilReference = 0;
		m_blendState.SetNil();
		m_sampleMask = ~0;
		memset(m_blendFactor, 0, sizeof(m_blendFactor));

		m_currentProgram.SetNil();
		memset(m_currentShaders, ~0, sizeof(m_currentShaders));

		m_currentInputLayout.SetNil();

		mxZERO_OUT(m_streamOffsets);
		mxZERO_OUT(m_streamStrides);
		m_numInputSlots = 0;

		m_currentIndexBuffer.SetNil();

		m_primitiveTopology = Topology::Undefined;
	}
	void DeviceContext::SubmitView( const ViewState& view )
	{
		const UINT numRenderTargets = view.targetCount;

		// Unbind shader resources
		// avoid D3D11 WARNING: ID3D11DeviceContext::OMSetRenderTargets:
		// Resource being set to OM DepthStencil is still bound on input!
		// [ STATE_SETTING WARNING #9: DEVICE_OMSETRENDERTARGETS_HAZARD]
		if( view.targetCount || view.depthTarget.IsValid() )
		{
			ID3D11ShaderResourceView *	shaderResourceViews	[LLGL_MAX_BOUND_SHADER_TEXTURES] = { NULL };
			//ID3D11UnorderedAccessView *    unorderedAccessViews[D3D11_PS_CS_UAV_REGISTER_COUNT] = { NULL };

			m_deviceContext->VSSetShaderResources( 0, mxCOUNT_OF(shaderResourceViews), shaderResourceViews );
			m_deviceContext->GSSetShaderResources( 0, mxCOUNT_OF(shaderResourceViews), shaderResourceViews );
			m_deviceContext->PSSetShaderResources( 0, mxCOUNT_OF(shaderResourceViews), shaderResourceViews );
		}


		ID3D11DepthStencilView* depthStencilView = NULL;
		if( view.depthTarget.IsValid() )
		{
			depthStencilView = me.depthTargets[ view.depthTarget.id ].m_DSV;

			UINT clearDepthStencilFlags = 0;
			if( view.flags & ClearDepth )      clearDepthStencilFlags |= D3D11_CLEAR_DEPTH;
			if( view.flags & ClearStencil )    clearDepthStencilFlags |= D3D11_CLEAR_STENCIL;
			if( clearDepthStencilFlags ) {
				m_deviceContext->ClearDepthStencilView( depthStencilView, clearDepthStencilFlags, view.depth, view.stencil );
			}
		}

		//const UINT32 nRTMask = CalculateDifference( currentColorTargets, view.colorTargets );
		//if( nRTMask )
		//{
		//    TCopyStaticPODArray( currentColorTargets, view.colorTargets );
		//    ...
		//}
		{
			ID3D11RenderTargetView *	pRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { NULL };
			for( UINT iRT = 0; iRT < numRenderTargets; iRT++ )
			{
				pRTVs[ iRT ] = me.colorTargets[ view.colorTargets[ iRT ].id ].m_RTV;
				if( view.flags & (1UL << iRT) ) {
					m_deviceContext->ClearRenderTargetView( pRTVs[ iRT ], view.clearColors[ iRT ] );
				}
			}
			//ASSERT_SIZEOF_ARE_EQUAL(m_renderTargets, view.colorTargets);
			//if( m_renderTargets != *(UINT32*)view.colorTargets )
			//{
			//	m_renderTargets = *(UINT32*)view.colorTargets;
			//}
			m_deviceContext->OMSetRenderTargets( numRenderTargets, pRTVs, depthStencilView );
		}

		{
			ASSERT_SIZEOF(Viewport, sizeof(UINT64));
			if( *(UINT64*)&m_viewport != *(UINT64*)&view.viewport )
			{
				m_viewport = view.viewport;

				D3D11_VIEWPORT    viewport11;
				viewport11.TopLeftX    = view.viewport.x;
				viewport11.TopLeftY    = view.viewport.y;
				viewport11.Width       = view.viewport.width;
				viewport11.Height      = view.viewport.height;
				viewport11.MinDepth    = 0.0f;
				viewport11.MaxDepth    = 1.0f;
				m_deviceContext->RSSetViewports( 1, &viewport11 );
			}
		}
	}
	void DeviceContext::UpdateBuffer( HBuffer handle, UINT32 start, const void* data, UINT32 size )
	{
		mxASSERT2(start == 0, "Unimplemented");

		BufferD3D11& bufferD3D = me.buffers[ handle.id ];

		D3D11_MAPPED_SUBRESOURCE	mappedData;
		if(SUCCEEDED(m_deviceContext->Map( bufferD3D.m_ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData )))
		{
			memcpy( mappedData.pData, data, size );
			m_deviceContext->Unmap( bufferD3D.m_ptr, 0 );
		}
	}
	void DeviceContext::UpdateTexture2( HTexture handle, const void* data, UINT32 size )
	{
		TextureD3D11& textureD3D = me.textures[ handle.id ];
#if 0
		m_deviceContext->UpdateSubresource(
			textureD3D.m_resource,	// ID3D11Resource *pDstResource
			0,	// UINT DstSubresource
			NULL,	// const D3D11_BOX *pDstBox
			data,	// const void *pSrcData
			0,	// UINT SrcRowPitch
			0	// UINT SrcDepthPitch
		);
#else
		D3D11_MAPPED_SUBRESOURCE	mappedData;
		mxZERO_OUT(mappedData);

		dxCHK(m_deviceContext->Map( textureD3D.m_resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData ));

		if( mappedData.pData )
		{
			memcpy( mappedData.pData, data, size );
			m_deviceContext->Unmap( textureD3D.m_resource, 0 );
		}
#endif
	}
	void* DeviceContext::MapBuffer( HBuffer _handle, UINT32 _start, EMapMode _mode, UINT32 _size )
	{
		BufferD3D11& bufferD3D = me.buffers[ _handle.id ];

		const D3D11_MAP mapType = ConvertMapModeD3D(_mode);
		const UINT iSubresource = 0;
		const UINT mapFlags = 0;

		D3D11_MAPPED_SUBRESOURCE	mappedData;
		if(SUCCEEDED(m_deviceContext->Map( bufferD3D.m_ptr, iSubresource, mapType, mapFlags, &mappedData )))
		{
			return mxAddByteOffset(mappedData.pData, _start);
		}
		return NULL;
	}
	void DeviceContext::UnmapBuffer( HBuffer _handle )
	{
		BufferD3D11& bufferD3D = me.buffers[ _handle.id ];

		const UINT iSubresource = 0;

		m_deviceContext->Unmap( bufferD3D.m_ptr, iSubresource );
	}
	////void CopyResource( source, destination );
	//void GenerateMips( HColorTarget target );
	//void ResolveTexture( HColorTarget target );
	//void ReadPixels( HColorTarget source, void *destination, UINT32 bufferSize );
	void DeviceContext::SetRasterizerState( HRasterizerState rasterizerState )
	{
		if( m_rasterizerState != rasterizerState )
		{
			m_rasterizerState = rasterizerState;
			m_deviceContext->RSSetState( me.rasterizerStates[ rasterizerState.id ].m_ptr );
		}
	}
	void DeviceContext::SetDepthStencilState( HDepthStencilState depthStencilState, UINT8 stencilReference )
	{
		if( m_depthStencilState != depthStencilState || m_stencilReference != stencilReference )
		{
			m_depthStencilState = depthStencilState;
			m_stencilReference = stencilReference;
			m_deviceContext->OMSetDepthStencilState( me.depthStencilStates[ depthStencilState.id ].m_ptr, stencilReference );
		}
	}
	void DeviceContext::SetBlendState( HBlendState blendState, const float* blendFactor /*= NULL*/, UINT32 sampleMask /*= ~0*/ )
	{
		static const float s_defaultBlendFactors[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		if( !blendFactor ) {
			blendFactor = s_defaultBlendFactors;
		}
		if( m_blendState != blendState || m_sampleMask != sampleMask || memcmp(m_blendFactor, blendFactor, 4*sizeof(float)) )
		{
			m_blendState = blendState;
			m_sampleMask = sampleMask;
			memcpy(m_blendFactor, blendFactor, 4*sizeof(float));
			m_deviceContext->OMSetBlendState( me.blendStates[ blendState.id ].m_blendState, blendFactor, sampleMask );
		}
	}
	void DeviceContext::SubmitBatch( const DrawCall& batch )
	{
		ID3D11Buffer *				constantBuffers		[LLGL_MAX_BOUND_UNIFORM_BUFFERS] = { NULL };
		ID3D11SamplerState *		samplerStates		[LLGL_MAX_BOUND_SHADER_SAMPLERS] = { NULL };
		ID3D11ShaderResourceView *	shaderResourceViews	[LLGL_MAX_BOUND_SHADER_TEXTURES] = { NULL };
		//ID3D11UnorderedAccessView *    unorderedAccessViews[D3D11_PS_CS_UAV_REGISTER_COUNT] = { NULL };

		// bind constant buffers
		for( UINT iCB = 0; iCB < mxCOUNT_OF(batch.CBs); iCB++ )
		{
			const HBuffer hCB = batch.CBs[ iCB ];
			if( hCB.IsValid() ) {
				constantBuffers[ iCB ] = me.buffers[ hCB.id ].m_ptr;
			} else {
				constantBuffers[ iCB ] = NULL;
			}
		}
		m_deviceContext->VSSetConstantBuffers( 0, mxCOUNT_OF(constantBuffers), constantBuffers );
		m_deviceContext->GSSetConstantBuffers( 0, mxCOUNT_OF(constantBuffers), constantBuffers );
		m_deviceContext->PSSetConstantBuffers( 0, mxCOUNT_OF(constantBuffers), constantBuffers );

		// bind sampler states
		for( UINT iSS = 0; iSS < mxCOUNT_OF(batch.SSs); iSS++ )
		{
			const HSamplerState hSS = batch.SSs[ iSS ];
			if( hSS.IsValid() ) {
				samplerStates[ iSS ] = me.samplerStates[ hSS.id ].m_ptr;
			} else {
				samplerStates[ iSS ] = NULL;
			}
		}
		m_deviceContext->VSSetSamplers( 0, mxCOUNT_OF(samplerStates), samplerStates );
		m_deviceContext->GSSetSamplers( 0, mxCOUNT_OF(samplerStates), samplerStates );
		m_deviceContext->PSSetSamplers( 0, mxCOUNT_OF(samplerStates), samplerStates );

		// bind shader resources
		for( UINT iSR = 0; iSR < mxCOUNT_OF(batch.SRs); iSR++ )
		{
			const HResource hSR = batch.SRs[ iSR ];
			shaderResourceViews[ iSR ] = GetResourceByHandle( hSR );
		}
		m_deviceContext->VSSetShaderResources( 0, mxCOUNT_OF(shaderResourceViews), shaderResourceViews );
		m_deviceContext->GSSetShaderResources( 0, mxCOUNT_OF(shaderResourceViews), shaderResourceViews );
		m_deviceContext->PSSetShaderResources( 0, mxCOUNT_OF(shaderResourceViews), shaderResourceViews );

		// bind shaders
		mxASSERT(batch.program.IsValid());
		if( m_currentProgram != batch.program )
		{
			m_currentProgram = batch.program;

			const ProgramD3D11& program = me.programs[ batch.program.id ];

#if LL_DEBUG_LEVEL >= 3
			{
				const ProgramBindingsOGL& bindings = program.bindings;

				for( UINT32 iCB = 0; iCB < bindings.cbuffers.Num(); iCB++ )
				{
					const CBufferBindingOGL& cbuffer = bindings.cbuffers[ iCB ];
					if( !batch.CBs[ cbuffer.slot ].IsValid() ) {
						DBGOUT("'%s' expects UBO '%s' at [%d] of size %d",
							program.name.c_str(), cbuffer.name.c_str(), cbuffer.slot, cbuffer.size);
					}
				}
				for( UINT32 iTS = 0; iTS < bindings.samplers.Num(); iTS++ )
				{
					const SamplerBindingOGL& binding = bindings.samplers[ iTS ];
					if( !batch.SRs[ binding.slot ].IsValid() ) {
						DBGOUT("'%s' expects resource '%s' at [%d]",
							program.name.c_str(), binding.name.c_str(), binding.slot);
					}
					if( !batch.SSs[ binding.slot ].IsValid() ) {
						DBGOUT("'%s' expects sampler '%s' at [%d]",
							program.name.c_str(), binding.name.c_str(), binding.slot);
					}
				}
				
			}
#endif

			if( m_currentShaders[ShaderVertex] != program.VS )
			{
				m_currentShaders[ShaderVertex] = program.VS;
				if( program.VS.IsValid() )
				{
					const ShaderD3D11& vertexShader = me.shaders[ program.VS.id ];
					m_deviceContext->VSSetShader( vertexShader.m_VS, NULL, 0 );
				}
				else
				{
					m_deviceContext->VSSetShader( NULL, NULL, 0 );
				}
			}

			if( m_currentShaders[ShaderGeometry] != program.GS )
			{
				m_currentShaders[ShaderGeometry] = program.GS;
				if( program.GS.IsValid() )
				{
					const ShaderD3D11& geometryShader = me.shaders[ program.GS.id ];
					m_deviceContext->GSSetShader( geometryShader.m_GS, NULL, 0 );
				}
				else
				{
					m_deviceContext->GSSetShader( NULL, NULL, 0 );
				}
			}

			if( m_currentShaders[ShaderGeometry] != program.PS )
			{
				m_currentShaders[ShaderGeometry] = program.PS;
				if( program.PS.IsValid() )
				{
					const ShaderD3D11& pixelShader = me.shaders[ program.PS.id ];
					m_deviceContext->PSSetShader( pixelShader.m_PS, NULL, 0 );
				}
				else
				{
					m_deviceContext->PSSetShader( NULL, NULL, 0 );
				}
			}
		}

		// bind input layout
		if( m_currentInputLayout != batch.inputLayout )
		{
			m_currentInputLayout = batch.inputLayout;

			if( batch.inputLayout.IsValid() )
			{
				const InputLayoutD3D11& inputLayout = me.inputLayouts[ batch.inputLayout.id ];
				mxASSERT( inputLayout.m_numSlots <= LLGL_MAX_VERTEX_STREAMS );
				TCopyStaticPODArray( m_streamStrides, inputLayout.m_strides );
				m_numInputSlots = inputLayout.m_numSlots;
				m_deviceContext->IASetInputLayout( inputLayout.m_layout );
			}
			else
			{
				mxZERO_OUT(m_streamStrides);
				m_numInputSlots = 0;
				m_deviceContext->IASetInputLayout( NULL );
			}
		}

		// bind vertex buffers
		ID3D11Buffer *    vertexBuffers[LLGL_MAX_VERTEX_STREAMS] = { NULL };
		if( m_numInputSlots )
		{
			for( UINT iVB = 0; iVB < m_numInputSlots; iVB++ )
			{
				vertexBuffers[ iVB ] = me.buffers[ batch.VB[ iVB ].id ].m_ptr;
			}
			m_deviceContext->IASetVertexBuffers( 0, m_numInputSlots, vertexBuffers, m_streamStrides, m_streamOffsets );
		}
		else
		{
			// D3D11 INFO: ID3D11DeviceContext::IASetVertexBuffers:
			// Since NumBuffers is 0, the operation effectively does nothing.
			// This is probably not intentional, nor is the most efficient way to achieve this operation.
			// Avoid calling the routine at all.
			// [ STATE_SETTING INFO #240: DEVICE_IASETVERTEXBUFFERS_BUFFERS_EMPTY]
			//m_deviceContext->IASetVertexBuffers( 0, 0, NULL, NULL, NULL );
		}

		// bind index buffer
		if( m_currentIndexBuffer != batch.IB )
		{
			m_currentIndexBuffer = batch.IB;
			if( batch.IB.IsValid() )
			{
				ID3D11Buffer *	indexBuffer = me.buffers[ batch.IB.id ].m_ptr;
				const DXGI_FORMAT	indexFormat = batch.b32bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
				m_deviceContext->IASetIndexBuffer( indexBuffer, indexFormat, 0 );
			}
			else
			{
				m_deviceContext->IASetIndexBuffer( NULL, DXGI_FORMAT_UNKNOWN, 0 );
			}
		}

		// set primitive topology
		if( m_primitiveTopology != (UINT)batch.topology )
		{
			m_primitiveTopology = batch.topology;
			D3D11_PRIMITIVE_TOPOLOGY primTypeD3D = D3D11_ConvertPrimitiveTopology( (Topology::Enum)batch.topology );
			m_deviceContext->IASetPrimitiveTopology( primTypeD3D );
		}

		if( *((UINT64*)&batch.scissor) )
		{
			D3D11_RECT	rect;
			rect.left = batch.scissor.left;
			rect.top = batch.scissor.top;
			rect.right = batch.scissor.right;
			rect.bottom = batch.scissor.bottom;
			m_deviceContext->RSSetScissorRects( 1, &rect );
		}
		else
		{
			m_deviceContext->RSSetScissorRects( 0, NULL );
		}

		// execute a draw call
		if( batch.indexCount ) {
			m_deviceContext->DrawIndexed( batch.indexCount, batch.startIndex, batch.baseVertex );
		} else {
			m_deviceContext->Draw( batch.vertexCount, batch.baseVertex );
		}
	}

	void DeviceContext::EndFrame()
	{
		// Cleanup (aka make the runtime happy)
		m_deviceContext->ClearState();

		this->ResetState();
	}

	void SaveScreenshot( const char* _where )
	{
		ID3D11DeviceContext* deviceContext = me.immediateContext.m_deviceContext;

		ID3D11Texture2D* pSrcResource = NULL;
#if 0
		ColorTargetD3D11& source = me.colorTargets[ command->source.id ];
		pSrcResource = source.m_colorTexture;
#else
		dxCHK(me.swapChain->GetBuffer( 0, IID_ID3D11Texture2D, (void**)&pSrcResource ));
#endif

#if USE_D3DX
		::D3DX11SaveTextureToFileA( deviceContext, pSrcResource, D3DX11_IFF_JPG, _where );
#else
		D3D11_TEXTURE2D_DESC srcDesc;
		pSrcResource->GetDesc(&srcDesc);
UNDONE;
		D3D11_TEXTURE2D_DESC dstDesc;
		dstDesc.Width = srcDesc.Width;
		dstDesc.Height = srcDesc.Height;
		dstDesc.MipLevels = 1;
		dstDesc.ArraySize = 1;
		dstDesc.Format = srcDesc.Format;
		dstDesc.SampleDesc.Count = 1;
		dstDesc.SampleDesc.Quality = 0;
		dstDesc.Usage = D3D11_USAGE_STAGING;
		dstDesc.BindFlags = 0;
		dstDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		dstDesc.MiscFlags = 0;

		ID3D11Texture2D* pDstResource = NULL;
		if(SUCCEEDED(me.device->CreateTexture2D( &dstDesc, NULL, &pDstResource )))
		{
			if( srcDesc.SampleDesc.Count == 1 )
			{
				deviceContext->CopyResource( pDstResource, pSrcResource );
			}
			else
			{
				UNDONE;
			}

			ScopedLock_D3D11 lockedTexture( deviceContext, pDstResource, D3D11_MAP_READ );

			const UINT32 rowPitchInBytes = lockedTexture.GetMappedData().RowPitch;
			//const UINT32 depthPitchInBytes = lockedTexture.GetMappedData().DepthPitch;
			//const UINT bytesToCopy = smallest(command->bufferSize, depthPitchInBytes);
			//memcpy(command->destination, lockedTexture.ToVoidPtr(), bytesToCopy);
			const UINT32 textureSize = rowPitchInBytes * srcDesc.Height;

			//FILE* file = fopen(_where,"w");
			FILE* file = fopen("___test.dds","w");
			fwrite( lockedTexture.ToVoidPtr(), 1, textureSize, file );
			fclose( file );

			SAFE_RELEASE(pDstResource);
		}
#endif
	}

}//namespace llgl

#endif // LLGL_Driver == LLGL_Driver_Direct3D_11



#if 0
            case RC_ReadPixels :
                {
                    const ReadPixelsCommand* command = commands.ReadNoCopy< ReadPixelsCommand >();

                    ColorTargetD3D11& source = me.colorTargets[ command->source.id ];

                    ID3D11Texture2D* pSrcResource = source.m_colorTexture;

                    D3D11_TEXTURE2D_DESC srcDesc;
                    pSrcResource->GetDesc(&srcDesc);

                    D3D11_TEXTURE2D_DESC dstDesc;
                    dstDesc.Width = source.m_width;
                    dstDesc.Height = source.m_height;
                    dstDesc.MipLevels = 1;
                    dstDesc.ArraySize = 1;
                    dstDesc.Format = srcDesc.Format;
                    dstDesc.SampleDesc.Count = 1;
                    dstDesc.SampleDesc.Quality = 0;
                    dstDesc.Usage = D3D11_USAGE_STAGING;
                    dstDesc.BindFlags = 0;
                    dstDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                    dstDesc.MiscFlags = 0;

                    ID3D11Texture2D* pDstResource = NULL;
                    if(SUCCEEDED(me.device->CreateTexture2D( &dstDesc, NULL, &pDstResource )))
                    {
                        me.deviceContext->CopyResource( pDstResource, pSrcResource );

                        ScopedLock_D3D11 lockedTexture( me.deviceContext, pDstResource, D3D11_MAP_READ );

                        //const UINT rowPitchInBytes = lockedTexture.GetMappedData().RowPitch;
                        const UINT depthPitchInBytes = lockedTexture.GetMappedData().DepthPitch;
                        const UINT bytesToCopy = smallest(command->bufferSize, depthPitchInBytes);
                        memcpy(command->destination, lockedTexture.ToVoidPtr(), bytesToCopy);

                        SAFE_RELEASE(pDstResource);
                    }
                } break;
#endif


mxNO_EMPTY_FILE

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
