// Living without D3DX:
// http://blogs.msdn.com/b/chuckw/archive/2013/08/21/living-without-d3dx.aspx

#include "Graphics/Graphics_PCH.h"
#pragma hdrstop
#include <Graphics/Device.h>

#if LLGL_Driver_Is_Direct3D

#include "d3d_common.h"

// Returns a string corresponding to the error code.
const char* D3D_GetErrorCodeString( HRESULT errorCode )
{
	switch ( errorCode )
	{
		case D3DERR_INVALIDCALL :
			return "The method call is invalid";

		case D3DERR_WASSTILLDRAWING :
			return "The previous blit operation that is transferring information to or from this surface is incomplete";

		case E_FAIL :
			return "Attempted to create a device with the debug layer enabled and the layer is not installed";

		case E_INVALIDARG :
			return "An invalid parameter was passed to the returning function";

		case E_OUTOFMEMORY :
			return "Direct3D could not allocate sufficient memory to complete the call";

		case S_FALSE :
			return "Alternate success value, indicating a successful but nonstandard completion (the precise meaning depends on context";

		case S_OK :
			return "No error occurred"; 

		// DXGI-specific error codes

		case DXGI_ERROR_DEVICE_HUNG :
			return "DXGI_ERROR_DEVICE_HUNG"; 

		case DXGI_ERROR_DEVICE_REMOVED :
			return "DXGI_ERROR_DEVICE_REMOVED"; 

		case DXGI_ERROR_DEVICE_RESET :
			return "DXGI_ERROR_DEVICE_RESET"; 

		case DXGI_ERROR_DRIVER_INTERNAL_ERROR :
			return "DXGI_ERROR_DRIVER_INTERNAL_ERROR"; 

		case DXGI_ERROR_INVALID_CALL :
			return "DXGI_ERROR_INVALID_CALL"; 

		// Direct3D 11 specific error codes

		case D3D11_ERROR_FILE_NOT_FOUND :
			return "The file was not found";

		case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS :
			return "There are too many unique instances of a particular type of state object";

		case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS :
			return "There are too many unique instances of a particular type of view object";

		case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD :
			return "Deferred context map without initial discard";

		default:
			/* fallthrough */;
	}

	return ::DXGetErrorDescriptionA( errorCode );
}
//--------------------------------------------------------------//
HRESULT dxWARN( HRESULT errorCode, const char* message, ... )
{
	char	buffer[ 2048 ];
	mxGET_VARARGS_A( buffer, message );
	ptWARN( "%s\nReason:\n %s\n", buffer, D3D_GetErrorCodeString(errorCode) );
	return errorCode;
}
//--------------------------------------------------------------//
HRESULT dxERROR( HRESULT errorCode, const char* message, ... )
{
	char	buffer[ 2048 ];
	mxGET_VARARGS_A( buffer, message );
	ptWARN( "%s\nReason:\n %s\n", buffer, D3D_GetErrorCodeString(errorCode) );
	return errorCode;
}
//--------------------------------------------------------------//
HRESULT dxERROR( HRESULT errorCode )
{
	ptWARN( "Error:\n%s\n", D3D_GetErrorCodeString( errorCode ) );
	return errorCode;
}
//--------------------------------------------------------------//
ERet D3D_HRESULT_To_EResult( HRESULT errorCode )
{
	switch ( errorCode )
	{
		case D3DERR_INVALIDCALL :
			return ERR_INVALID_FUNCTION_CALL;

		case E_INVALIDARG :
			return ERR_INVALID_PARAMETER;

		case E_OUTOFMEMORY :
			return ERR_OUT_OF_MEMORY;

		case S_FALSE :
		case S_OK :
			return ALL_OK;

		// Direct3D 11 specific error codes

		case D3D11_ERROR_FILE_NOT_FOUND :
			return ERR_FAILED_TO_OPEN_FILE;

		case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS :
		case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS :
			return ERR_TOO_MANY_OBJECTS;

		default:
			/* fallthrough */;
	}
	return ERet::ERR_UNKNOWN_ERROR;
}

//-------------------------------------------------------------------------------------------------------------//
mxBEGIN_REFLECT_ENUM( DXGI_FORMAT_ENUM )
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_UNKNOWN					  ),

	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32G32B32A32_TYPELESS       ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32G32B32A32_FLOAT		  ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32G32B32A32_UINT           ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32G32B32A32_SINT           ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32G32B32_TYPELESS          ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32G32B32_FLOAT             ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32G32B32_UINT              ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32G32B32_SINT              ),

	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16G16B16A16_TYPELESS       ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16G16B16A16_FLOAT          ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16G16B16A16_UNORM          ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16G16B16A16_UINT           ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16G16B16A16_SNORM          ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16G16B16A16_SINT           ),

	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32G32_TYPELESS             ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32G32_FLOAT                ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32G32_UINT                 ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32G32_SINT                 ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32G8X24_TYPELESS           ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_D32_FLOAT_S8X24_UINT        ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     ),

	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R10G10B10A2_TYPELESS        ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R10G10B10A2_UNORM           ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R10G10B10A2_UINT            ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R11G11B10_FLOAT             ),

	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8G8B8A8_TYPELESS           ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8G8B8A8_UNORM              ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8G8B8A8_UINT               ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8G8B8A8_SNORM              ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8G8B8A8_SINT               ),

	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16G16_TYPELESS             ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16G16_FLOAT                ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16G16_UNORM                ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16G16_UINT                 ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16G16_SNORM                ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16G16_SINT                 ),

	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32_TYPELESS                ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_D32_FLOAT                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32_FLOAT                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32_UINT                    ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R32_SINT                    ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R24G8_TYPELESS              ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_D24_UNORM_S8_UINT           ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R24_UNORM_X8_TYPELESS       ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_X24_TYPELESS_G8_UINT        ),

	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8G8_TYPELESS               ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8G8_UNORM                  ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8G8_UINT                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8G8_SNORM                  ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8G8_SINT                   ),

	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16_TYPELESS                ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16_FLOAT                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_D16_UNORM                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16_UNORM                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16_UINT                    ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16_SNORM                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R16_SINT                    ),

	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8_TYPELESS                 ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8_UNORM                    ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8_UINT                     ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8_SNORM                    ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8_SINT                     ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_A8_UNORM                    ),

	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R1_UNORM                    ),

	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R9G9B9E5_SHAREDEXP          ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R8G8_B8G8_UNORM             ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_G8R8_G8B8_UNORM             ),

	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC1_TYPELESS                ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC1_UNORM                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC1_UNORM_SRGB              ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC2_TYPELESS                ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC2_UNORM                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC2_UNORM_SRGB              ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC3_TYPELESS                ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC3_UNORM                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC3_UNORM_SRGB              ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC4_TYPELESS                ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC4_UNORM                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC4_SNORM                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC5_TYPELESS                ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC5_UNORM                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC5_SNORM                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_B5G6R5_UNORM                ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_B5G5R5A1_UNORM              ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_B8G8R8A8_UNORM              ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_B8G8R8X8_UNORM              ),
#if 0
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_B8G8R8A8_TYPELESS           ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_B8G8R8X8_TYPELESS           ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC6H_TYPELESS               ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC6H_UF16                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC6H_SF16                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC7_TYPELESS                ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC7_UNORM                   ),
	mxREFLECT_ENUM_ITEM1( DXGI_FORMAT_BC7_UNORM_SRGB              ),
#endif
mxEND_REFLECT_ENUM;

//-------------------------------------------------------------------------------------------------------------//

const char* DXGI_FORMAT_ToChars( DXGI_FORMAT format )
{
	return mxGET_ENUM_TYPE( DXGI_FORMAT_ENUM ).GetStringByValue( format );
}

mxSWIPED("bgfx");
const TextureFormatInfoD3D11 gs_textureFormats[] =
{
	{ DXGI_FORMAT_BC1_UNORM,          DXGI_FORMAT_BC1_UNORM,             DXGI_FORMAT_UNKNOWN           }, // BC1
	{ DXGI_FORMAT_BC2_UNORM,          DXGI_FORMAT_BC2_UNORM,             DXGI_FORMAT_UNKNOWN           }, // BC2
	{ DXGI_FORMAT_BC3_UNORM,          DXGI_FORMAT_BC3_UNORM,             DXGI_FORMAT_UNKNOWN           }, // BC3
	{ DXGI_FORMAT_BC4_UNORM,          DXGI_FORMAT_BC4_UNORM,             DXGI_FORMAT_UNKNOWN           }, // BC4
	{ DXGI_FORMAT_BC5_UNORM,          DXGI_FORMAT_BC5_UNORM,             DXGI_FORMAT_UNKNOWN           }, // BC5
#ifdef Direct3D_11_1_Plus
	{ DXGI_FORMAT_BC6H_SF16,          DXGI_FORMAT_BC6H_SF16,             DXGI_FORMAT_UNKNOWN           }, // BC6H
	{ DXGI_FORMAT_BC7_UNORM,          DXGI_FORMAT_BC7_UNORM,             DXGI_FORMAT_UNKNOWN           }, // BC7
#else
	{ DXGI_FORMAT_UNKNOWN,          DXGI_FORMAT_UNKNOWN,				DXGI_FORMAT_UNKNOWN           }, // BC6H
	{ DXGI_FORMAT_UNKNOWN,          DXGI_FORMAT_UNKNOWN,				DXGI_FORMAT_UNKNOWN           }, // BC7
#endif
	{ DXGI_FORMAT_UNKNOWN,            DXGI_FORMAT_UNKNOWN,               DXGI_FORMAT_UNKNOWN           }, // ETC1
	{ DXGI_FORMAT_UNKNOWN,            DXGI_FORMAT_UNKNOWN,               DXGI_FORMAT_UNKNOWN           }, // ETC2
	{ DXGI_FORMAT_UNKNOWN,            DXGI_FORMAT_UNKNOWN,               DXGI_FORMAT_UNKNOWN           }, // ETC2A
	{ DXGI_FORMAT_UNKNOWN,            DXGI_FORMAT_UNKNOWN,               DXGI_FORMAT_UNKNOWN           }, // ETC2A1
	{ DXGI_FORMAT_UNKNOWN,            DXGI_FORMAT_UNKNOWN,               DXGI_FORMAT_UNKNOWN           }, // PTC12
	{ DXGI_FORMAT_UNKNOWN,            DXGI_FORMAT_UNKNOWN,               DXGI_FORMAT_UNKNOWN           }, // PTC14
	{ DXGI_FORMAT_UNKNOWN,            DXGI_FORMAT_UNKNOWN,               DXGI_FORMAT_UNKNOWN           }, // PTC12A
	{ DXGI_FORMAT_UNKNOWN,            DXGI_FORMAT_UNKNOWN,               DXGI_FORMAT_UNKNOWN           }, // PTC14A
	{ DXGI_FORMAT_UNKNOWN,            DXGI_FORMAT_UNKNOWN,               DXGI_FORMAT_UNKNOWN           }, // PTC22
	{ DXGI_FORMAT_UNKNOWN,            DXGI_FORMAT_UNKNOWN,               DXGI_FORMAT_UNKNOWN           }, // PTC24

	{ DXGI_FORMAT_UNKNOWN,            DXGI_FORMAT_UNKNOWN,               DXGI_FORMAT_UNKNOWN           }, // Unknown

	{ DXGI_FORMAT_R1_UNORM,           DXGI_FORMAT_R1_UNORM,              DXGI_FORMAT_UNKNOWN           }, // R1
	{ DXGI_FORMAT_R8_UNORM,           DXGI_FORMAT_R8_UNORM,              DXGI_FORMAT_UNKNOWN           }, // R8
	{ DXGI_FORMAT_R16_UINT,           DXGI_FORMAT_R16_UINT,              DXGI_FORMAT_UNKNOWN           }, // R16
	{ DXGI_FORMAT_R16_FLOAT,          DXGI_FORMAT_R16_FLOAT,             DXGI_FORMAT_UNKNOWN           }, // R16F
	{ DXGI_FORMAT_R32_UINT,           DXGI_FORMAT_R32_UINT,              DXGI_FORMAT_UNKNOWN           }, // R32
	{ DXGI_FORMAT_R32_FLOAT,          DXGI_FORMAT_R32_FLOAT,             DXGI_FORMAT_UNKNOWN           }, // R32F
	{ DXGI_FORMAT_R8G8_UNORM,         DXGI_FORMAT_R8G8_UNORM,            DXGI_FORMAT_UNKNOWN           }, // RG8
	{ DXGI_FORMAT_R16G16_UNORM,       DXGI_FORMAT_R16G16_UNORM,          DXGI_FORMAT_UNKNOWN           }, // RG16
	{ DXGI_FORMAT_R16G16_FLOAT,       DXGI_FORMAT_R16G16_FLOAT,          DXGI_FORMAT_UNKNOWN           }, // RG16F
	{ DXGI_FORMAT_R32G32_UINT,        DXGI_FORMAT_R32G32_UINT,           DXGI_FORMAT_UNKNOWN           }, // RG32
	{ DXGI_FORMAT_R32G32_FLOAT,       DXGI_FORMAT_R32G32_FLOAT,          DXGI_FORMAT_UNKNOWN           }, // RG32F
	{ DXGI_FORMAT_B8G8R8A8_UNORM,     DXGI_FORMAT_B8G8R8A8_UNORM,        DXGI_FORMAT_UNKNOWN           }, // BGRA8
	{ DXGI_FORMAT_R8G8B8A8_UNORM,     DXGI_FORMAT_R8G8B8A8_UNORM,        DXGI_FORMAT_UNKNOWN           }, // RGBA8
	{ DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R16G16B16A16_UNORM,    DXGI_FORMAT_UNKNOWN           }, // RGBA16
	{ DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT,    DXGI_FORMAT_UNKNOWN           }, // RGBA16F
	{ DXGI_FORMAT_R32G32B32A32_UINT,  DXGI_FORMAT_R32G32B32A32_UINT,     DXGI_FORMAT_UNKNOWN           }, // RGBA32
	{ DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,    DXGI_FORMAT_UNKNOWN           }, // RGBA32F
	{ DXGI_FORMAT_B5G6R5_UNORM,       DXGI_FORMAT_B5G6R5_UNORM,          DXGI_FORMAT_UNKNOWN           }, // R5G6B5
#ifdef Direct3D_11_1_Plus
	{ DXGI_FORMAT_B4G4R4A4_UNORM,     DXGI_FORMAT_B4G4R4A4_UNORM,        DXGI_FORMAT_UNKNOWN           }, // RGBA4
#else
	{ DXGI_FORMAT_UNKNOWN,				DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN           }, // RGBA4
#endif
	{ DXGI_FORMAT_B5G5R5A1_UNORM,     DXGI_FORMAT_B5G5R5A1_UNORM,        DXGI_FORMAT_UNKNOWN           }, // RGB5A1
	{ DXGI_FORMAT_R10G10B10A2_UNORM,  DXGI_FORMAT_R10G10B10A2_UNORM,     DXGI_FORMAT_UNKNOWN           }, // RGB10A2
	{ DXGI_FORMAT_R11G11B10_FLOAT,    DXGI_FORMAT_R11G11B10_FLOAT,       DXGI_FORMAT_UNKNOWN           }, // R11G11B10F
#if 0
	{ DXGI_FORMAT_UNKNOWN,            DXGI_FORMAT_UNKNOWN,               DXGI_FORMAT_UNKNOWN           }, // UnknownDepth
	{ DXGI_FORMAT_R16_TYPELESS,       DXGI_FORMAT_R16_UNORM,             DXGI_FORMAT_D16_UNORM         }, // D16
	{ DXGI_FORMAT_R24G8_TYPELESS,     DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT }, // D24
	{ DXGI_FORMAT_R24G8_TYPELESS,     DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT }, // D24S8
	{ DXGI_FORMAT_R24G8_TYPELESS,     DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT }, // D32
	{ DXGI_FORMAT_R32_TYPELESS,       DXGI_FORMAT_R32_FLOAT,             DXGI_FORMAT_D32_FLOAT         }, // D16F
	{ DXGI_FORMAT_R32_TYPELESS,       DXGI_FORMAT_R32_FLOAT,             DXGI_FORMAT_D32_FLOAT         }, // D24F
	{ DXGI_FORMAT_R32_TYPELESS,       DXGI_FORMAT_R32_FLOAT,             DXGI_FORMAT_D32_FLOAT         }, // D32F
	{ DXGI_FORMAT_R24G8_TYPELESS,     DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT }, // D0S8
#endif
};
mxSTATIC_ASSERT(PixelFormat::MAX == mxCOUNT_OF(gs_textureFormats) );

//--------------------------------------------------------------------------------------
// Returns the BPP for a particular format
//--------------------------------------------------------------------------------------

UINT DXGI_FORMAT_BitsPerPixel( DXGI_FORMAT format )
{
	switch( format )
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 96;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT: 
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return 64;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
		return 32;

#ifdef DXGI_1_1_FORMATS
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return 32;
#endif // DXGI_1_1_FORMATS

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
		return 16;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
		return 8;

	case DXGI_FORMAT_R1_UNORM:
		return 1;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
		return 4;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
		return 8;

#ifdef DXGI_1_1_FORMATS
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return 8;
#endif // DXGI_1_1_FORMATS

		mxNO_SWITCH_DEFAULT; // unhandled format
	}
	return 0;
}

//--------------------------------------------------------------------------------------
// Helper functions to create SRGB formats from typeless formats and vice versa
//--------------------------------------------------------------------------------------

DXGI_FORMAT DXGI_FORMAT_MAKE_SRGB( DXGI_FORMAT format )
{
	switch( format )
	{
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
		return DXGI_FORMAT_BC1_UNORM_SRGB;
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
		return DXGI_FORMAT_BC2_UNORM_SRGB;
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
		return DXGI_FORMAT_BC3_UNORM_SRGB;
	};
	return format;
}

//--------------------------------------------------------------------------------------
DXGI_FORMAT DXGI_FORMAT_MAKE_TYPELESS( DXGI_FORMAT format )
{
	switch( format )
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
		return DXGI_FORMAT_R8G8B8A8_TYPELESS;

	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC1_UNORM:
		return DXGI_FORMAT_BC1_TYPELESS;
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC2_UNORM:
		return DXGI_FORMAT_BC2_TYPELESS;
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC3_UNORM:
		return DXGI_FORMAT_BC3_TYPELESS;
	};
	return format;
}

//-------------------------------------------------------------------------------------------------------------//

bool DXGI_FORMAT_IsBlockCompressed( DXGI_FORMAT format )
{
	switch (format)
	{
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return true;
		break;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
		return true;
		break;

#ifdef DXGI_1_1_FORMATS
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return true;
		break;
#endif // DXGI_1_1_FORMATS

	default:
		// Nothing.
		break;
	}
	return false;
}
//-------------------------------------------------------------------------------------------------------------//
void DXGI_FORMAT_GetSurfaceInfo( UINT width, UINT height, DXGI_FORMAT fmt, UINT* pNumBytes, UINT* pRowBytes, UINT* pNumRows )
{
	UINT numBytes = 0;
	UINT rowBytes = 0;
	UINT numRows = 0;

	bool bc = true;
	int bcnumBytesPerBlock = 16;
	switch (fmt)
	{
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		bcnumBytesPerBlock = 8;
		break;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
		break;

#ifdef DXGI_1_1_FORMATS
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		break;
#endif // DXGI_1_1_FORMATS

	default:
		bc = false;
		break;
	}

	if( bc )
	{
		int numBlocksWide = 0;
		if( width > 0 ) {
			numBlocksWide = largest( 1, width / 4 );
		}
		int numBlocksHigh = 0;
		if( height > 0 ) {
			numBlocksHigh = largest( 1, height / 4 );
		}
		rowBytes = numBlocksWide * bcnumBytesPerBlock;
		numRows = numBlocksHigh;
	}
	else
	{
		UINT bpp = DXGI_FORMAT_BitsPerPixel( fmt );
		rowBytes = ( width * bpp + 7 ) / 8; // round up to nearest byte
		numRows = height;
	}
	numBytes = rowBytes * numRows;
	if( pNumBytes != NULL ) {
		*pNumBytes = numBytes;
	}
	if( pRowBytes != NULL ) {
		*pRowBytes = rowBytes;
	}
	if( pNumRows != NULL ) {
		*pNumRows = numRows;
	}
}
//-------------------------------------------------------------------------------------------------------------//
const char* DXGI_ScanlineOrder_ToStr( DXGI_MODE_SCANLINE_ORDER scanlineOrder )
{
	switch( scanlineOrder )
	{
	case DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED :			return "DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED";
	case DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE :			return "DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE";
	case DXGI_MODE_SCANLINE_ORDER_UPPER_FIELD_FIRST :	return "DXGI_MODE_SCANLINE_ORDER_UPPER_FIELD_FIRST";
	case DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST :	return "DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST";
	mxNO_SWITCH_DEFAULT;
	}
	return "Unknown";
}
//-------------------------------------------------------------------------------------------------------------//
const char* DXGI_ScalingMode_ToStr( DXGI_MODE_SCALING scaling )
{
	switch( scaling )
	{
	case DXGI_MODE_SCALING_UNSPECIFIED :return "DXGI_MODE_SCALING_UNSPECIFIED";
	case DXGI_MODE_SCALING_CENTERED :	return "DXGI_MODE_SCALING_CENTERED";
	case DXGI_MODE_SCALING_STRETCHED :	return "DXGI_MODE_SCALING_STRETCHED";
	mxNO_SWITCH_DEFAULT;
	}
	return "Unknown";
}

DXGI_FORMAT DXGI_GetDepthStencil_Typeless_Format( DXGI_FORMAT depthStencilFormat )
{
	DXGI_FORMAT textureFormat = DXGI_FORMAT_UNKNOWN;
	if( depthStencilFormat == DXGI_FORMAT_D16_UNORM ) {
		textureFormat = DXGI_FORMAT_R16_TYPELESS;
	} else if( depthStencilFormat == DXGI_FORMAT_D24_UNORM_S8_UINT ) {
		textureFormat = DXGI_FORMAT_R24G8_TYPELESS;
	} else if( depthStencilFormat == DXGI_FORMAT_D32_FLOAT ) {
		textureFormat = DXGI_FORMAT_R32_TYPELESS;
	} else if( depthStencilFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT ) {
		textureFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	} else {
		String64 message;
		Str::SPrintF(message,"not a valid depth format: %d", depthStencilFormat);
		mxUNREACHABLE2(message.ToPtr());
	}
	return textureFormat;
}

DXGI_FORMAT DXGI_GetDepthStencilView_Format( DXGI_FORMAT depthStencilFormat )
{
	DXGI_FORMAT depthStencilViewFormat = DXGI_FORMAT_UNKNOWN;
	if( depthStencilFormat == DXGI_FORMAT_D16_UNORM ) {
		depthStencilViewFormat = DXGI_FORMAT_R16_FLOAT;
	} else if( depthStencilFormat == DXGI_FORMAT_D24_UNORM_S8_UINT ) {
		depthStencilViewFormat = DXGI_FORMAT_R24G8_TYPELESS;
	} else if( depthStencilFormat == DXGI_FORMAT_D32_FLOAT ) {
		depthStencilViewFormat = DXGI_FORMAT_R32_FLOAT;
	} else if( depthStencilFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT ) {
		depthStencilViewFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	} else {
		String64 message;
		Str::SPrintF(message,"not a valid depth format: %d", depthStencilFormat);
		mxUNREACHABLE2(message.ToPtr());
	}
	return depthStencilViewFormat;
}

DXGI_FORMAT DXGI_GetDepthStencil_SRV_Format( DXGI_FORMAT depthStencilFormat )
{
	DXGI_FORMAT shaderResourceViewFormat = DXGI_FORMAT_UNKNOWN;
	if( depthStencilFormat == DXGI_FORMAT_D16_UNORM ) {
		shaderResourceViewFormat = DXGI_FORMAT_R16_UNORM;
	} else if( depthStencilFormat == DXGI_FORMAT_D24_UNORM_S8_UINT ) {
		shaderResourceViewFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	} else if( depthStencilFormat == DXGI_FORMAT_D32_FLOAT ) {
		shaderResourceViewFormat = DXGI_FORMAT_R32_FLOAT;
	} else if( depthStencilFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT ) {
		shaderResourceViewFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	} else {
		String64 message;
		Str::SPrintF(message,"not a valid depth format: %d", depthStencilFormat);
		mxUNREACHABLE2(message.ToPtr());
	}
	return shaderResourceViewFormat;
}

//-------------------------------------------------------------------------------------------------------------//
mxBEGIN_REFLECT_ENUM( D3D_FEATURE_LEVEL_ENUM )
	mxREFLECT_ENUM_ITEM( D3D_FEATURE_LEVEL_9_1, D3D_FEATURE_LEVEL_9_1	),
	mxREFLECT_ENUM_ITEM( D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_2	),
	mxREFLECT_ENUM_ITEM( D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_3	),
	mxREFLECT_ENUM_ITEM( D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_10_0	),
	mxREFLECT_ENUM_ITEM( D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_1	),
	mxREFLECT_ENUM_ITEM( D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_0	),
mxEND_REFLECT_ENUM;

const char* D3D_FeatureLevelToStr( D3D_FEATURE_LEVEL featureLevel )
{
	return mxGET_ENUM_TYPE( D3D_FEATURE_LEVEL_ENUM ).GetStringByValue( featureLevel );
}

D3D11_MAP ConvertMapModeD3D( EMapMode mapMode )
{
	switch( mapMode )
	{
	case Map_Read :	return D3D11_MAP_READ;
	case Map_Write :	return D3D11_MAP_WRITE;
	case Map_Read_Write :	return D3D11_MAP_READ_WRITE;
	case Map_Write_Discard :	return D3D11_MAP_WRITE_DISCARD;
	case Map_Write_DiscardRange :	return D3D11_MAP_WRITE_NO_OVERWRITE;
		mxNO_SWITCH_DEFAULT;
	}
	return D3D11_MAP_WRITE_DISCARD;
}

#endif // LLGL_Driver_Is_Direct3D

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
