#include <Graphics/Graphics_PCH.h>
#pragma hdrstop

#include <Graphics/Device.h>
#include "DDS_Reader.h"
#include "image.h"

// "DDS " or ' SDD' on little-endian machines
#define DDS_MAGIC	0x20534444

#define DDS_MAGIC_SIZE			sizeof(UINT32)
#define DDS_HEADER_SIZE			sizeof(DDS_HEADER)
#define DDS_IMAGE_DATA_OFFSET	(DDS_HEADER_SIZE + DDS_MAGIC_SIZE)


#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |       \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif /* defined(MAKEFOURCC) */

#define DDS_DXT1 MAKEFOURCC('D', 'X', 'T', '1')
#define DDS_DXT2 MAKEFOURCC('D', 'X', 'T', '2')
#define DDS_DXT3 MAKEFOURCC('D', 'X', 'T', '3')
#define DDS_DXT4 MAKEFOURCC('D', 'X', 'T', '4')
#define DDS_DXT5 MAKEFOURCC('D', 'X', 'T', '5')
#define DDS_ATI1 MAKEFOURCC('A', 'T', 'I', '1')
#define DDS_BC4U MAKEFOURCC('B', 'C', '4', 'U')
#define DDS_ATI2 MAKEFOURCC('A', 'T', 'I', '2')
#define DDS_BC5U MAKEFOURCC('B', 'C', '5', 'U')

#define DDS_A8R8G8B8	21
#define D3DFMT_A16B16G16R16  36
#define D3DFMT_A16B16G16R16F 113

// Subset here matches D3D10_RESOURCE_DIMENSION and D3D11_RESOURCE_DIMENSION
enum DDS_RESOURCE_DIMENSION
{
    DDS_DIMENSION_TEXTURE1D	= 2,
    DDS_DIMENSION_TEXTURE2D	= 3,
    DDS_DIMENSION_TEXTURE3D	= 4,
};

// Subset here matches D3D10_RESOURCE_MISC_FLAG and D3D11_RESOURCE_MISC_FLAG
enum DDS_RESOURCE_MISC_FLAG
{
   DDS_RESOURCE_MISC_TEXTURECUBE = 0x4L,
};

enum DDS_MISC_FLAGS2
{
    DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
};

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA

const DDS_PIXELFORMAT DDSPF_DXT1 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','1'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT2 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','2'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT3 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','3'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT4 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','4'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT5 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','5'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_A8R8G8B8 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };

const DDS_PIXELFORMAT DDSPF_A1R5G5B5 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 };

const DDS_PIXELFORMAT DDSPF_A4R4G4B4 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 };

const DDS_PIXELFORMAT DDSPF_R8G8B8 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };

const DDS_PIXELFORMAT DDSPF_R5G6B5 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 };

// This indicates the DDS_HEADER_DXT10 extension is present (the format is in dxgiFormat)
const DDS_PIXELFORMAT DDSPF_DX10 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','1','0'), 0, 0, 0, 0, 0 };

#define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 
#define DDS_HEADER_FLAGS_MIPMAP         0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH          0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE     0x00080000  // DDSD_LINEARSIZE

// DDS_header.dwFlags
#define DDSD_CAPS                   0x00000001
#define DDSD_HEIGHT                 0x00000002
#define DDSD_WIDTH                  0x00000004
#define DDSD_PITCH                  0x00000008
#define DDSD_PIXELFORMAT            0x00001000
#define DDSD_MIPMAPCOUNT            0x00020000
#define DDSD_LINEARSIZE             0x00080000
#define DDSD_DEPTH                  0x00800000

#define DDPF_ALPHAPIXELS            0x00000001
#define DDPF_ALPHA                  0x00000002
#define DDPF_FOURCC                 0x00000004
#define DDPF_INDEXED                0x00000020
#define DDPF_RGB                    0x00000040
#define DDPF_YUV                    0x00000200
#define DDPF_LUMINANCE              0x00020000

#define DDSCAPS_COMPLEX             0x00000008
#define DDSCAPS_TEXTURE             0x00001000
#define DDSCAPS_MIPMAP              0x00400000

#define DDSCAPS2_CUBEMAP            0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX  0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX  0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY  0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY  0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ  0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ  0x00008000

#define DDSCAPS2_CUBEMAP_ALLFACES	(DDSCAPS2_CUBEMAP_POSITIVEX|DDSCAPS2_CUBEMAP_NEGATIVEX \
									 |DDSCAPS2_CUBEMAP_POSITIVEY|DDSCAPS2_CUBEMAP_NEGATIVEY \
									 |DDSCAPS2_CUBEMAP_POSITIVEZ|DDSCAPS2_CUBEMAP_NEGATIVEZ)

#define DDSCAPS2_VOLUME             0x00200000

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000 // DDSCAPS_TEXTURE
#define DDS_SURFACE_FLAGS_MIPMAP  0x00400008 // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008 // DDSCAPS_COMPLEX

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_FLAGS_VOLUME 0x00200000 // DDSCAPS2_VOLUME


//--------------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------------

#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )


const DDS_HEADER* DDS_ParseHeader( const void* _data, UINT _size, UINT *_offset /*= NULL*/ )
{
	// Need at least enough data to fill the header and magic number to be a valid DDS
	if( _size < (sizeof(DDS_HEADER)+sizeof(UINT32)) ) {
		return NULL;
	}

	// DDS files always start with the same magic number ("DDS ")
	UINT32 magicNumber = *(UINT32*) _data;
	if( magicNumber != DDS_MAGIC ) {
		return NULL;
	}

	const DDS_HEADER* header = c_cast(DDS_HEADER* ) mxAddByteOffset( _data, sizeof(UINT32) );

	// Verify header to validate DDS file
	if( (header->size != sizeof(DDS_HEADER)) || (header->pixelFormat.size != sizeof(DDS_PIXELFORMAT)) )
	{
		return NULL;
	}

	if( (header->surfaceFlags & DDSCAPS_TEXTURE) == 0 ) {
		return NULL;
	}

	// Check for DX10 extension
	bool bDXT10Header = false;
	if( (header->pixelFormat.flags & DDS_FOURCC) && (MAKEFOURCC( 'D', 'X', '1', '0' ) == header->pixelFormat.fourCC) )
	{
		// Must be long enough for both headers and magic value
		if( _size < (sizeof(DDS_HEADER)+sizeof(UINT32)+sizeof(DDS_HEADER_DXT10)) )
		{
			return false;
		}
		bDXT10Header = true;
	}

	mxASSERT2(bDXT10Header == false, "DX10 DDS headers are not supported!");

	// setup the pointers in the process request
	if( _offset )
	{
		*_offset = sizeof(UINT32) + sizeof(DDS_HEADER) + (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);
	}

//	const void* textureData = mxAddByteOffset( _data, _offset );
//	UINT32 dataSize = _size - _offset;

	return header;
}

struct DDS_Format_Mapping
{
	UINT32				DDS_format;
	PixelFormat::Enum	engine_format;
};
static const DDS_Format_Mapping gs_DDS_Format_Mappings[] =
{
	{ DDS_DXT1,                  PixelFormat::BC1     },
	{ DDS_DXT2,                  PixelFormat::BC2     },
	{ DDS_DXT3,                  PixelFormat::BC2     },
	{ DDS_DXT4,                  PixelFormat::BC3     },
	{ DDS_DXT5,                  PixelFormat::BC3     },
	{ DDS_ATI1,                  PixelFormat::BC4     },
	{ DDS_BC4U,                  PixelFormat::BC4     },
	{ DDS_ATI2,                  PixelFormat::BC5     },
	{ DDS_BC5U,                  PixelFormat::BC5     },
	//{ D3DFMT_A16B16G16R16,       PixelFormat::RGBA16  },
	//{ D3DFMT_A16B16G16R16F,      PixelFormat::RGBA16F },
	//{ DDPF_RGB|DDPF_ALPHAPIXELS, PixelFormat::BGRA8   },
	//{ DDPF_INDEXED,              PixelFormat::L8      },
	//{ DDPF_LUMINANCE,            PixelFormat::L8      },
	//{ DDPF_ALPHA,                PixelFormat::L8      },
	{ DDS_A8R8G8B8,				PixelFormat::BGRA8     },
};

static PixelFormat::Enum FindEngineFormat( UINT32 _DDS_format )
{
	for( UINT i = 0; i < mxCOUNT_OF(gs_DDS_Format_Mappings); i++ )
	{
		const DDS_Format_Mapping& mapping = gs_DDS_Format_Mappings[ i ];
		if( mapping.DDS_format == _DDS_format ) {
			return mapping.engine_format;
		}
	}
	UNDONE;
	return PixelFormat::Unknown;
}

ERet DDS_Parse( const void* _data, UINT _size, TextureImage &_dds )
{
	UINT32 offset = 0;
	const DDS_HEADER* header = DDS_ParseHeader( _data, _size, &offset );
	chkRET_X_IF_NIL(header, ERR_INVALID_PARAMETER);

	const UINT32 width = header->width;
	const UINT32 height = header->height;
	const UINT32 depth = header->depth;

	// > 1 if (header->surfaceFlags & DDS_SURFACE_FLAGS_MIPMAP)
	const UINT32 mipCount = largest(header->mipMapCount, 1);

	const bool isCubeMap = ((header->cubemapFlags & DDSCAPS2_CUBEMAP) != 0);
	if( isCubeMap )
	{
		if((header->cubemapFlags & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
		{
			// partial cube maps are not supported - we require all six faces to be defined
			return ERR_UNSUPPORTED_FEATURE;
		}
	}

	if( depth > 0 ) {
		return ERR_UNSUPPORTED_FEATURE;
	}

	const UINT32 pixelFormatFlags = header->pixelFormat.flags;

	//const bool hasAlpha = ((pixelFormatFlags & DDPF_ALPHAPIXELS) != 0);
	const UINT32 ddsFormat = (pixelFormatFlags & DDPF_FOURCC) ? header->pixelFormat.fourCC : pixelFormatFlags;

	const PixelFormat::Enum engineFormat = FindEngineFormat( ddsFormat );
	chkRET_X_IF_NOT(engineFormat != PixelFormat::Unknown, ERR_UNSUPPORTED_FEATURE);

	using namespace bgfx;
	const ImageBlockInfo& pixelFormat = s_imageBlockInfo[ engineFormat ];

	const UINT32 bitsPerPixel = pixelFormat.bitsPerPixel;
	const UINT32 texelsInBlock = pixelFormat.blockWidth * pixelFormat.blockHeight;
	const UINT32 blockSizeBytes = texelsInBlock * bitsPerPixel / BITS_IN_BYTE;

	mxUNUSED(bitsPerPixel);
	mxUNUSED(texelsInBlock);
	mxUNUSED(blockSizeBytes);

	_dds.data		= mxAddByteOffset( _data, offset );
	_dds.size		= _size - offset;
	_dds.format		= engineFormat;
	_dds.width		= width;
	_dds.height		= height;
	_dds.depth		= depth;
	_dds.numMips	= mipCount;
	_dds.isCubeMap	= isCubeMap;

	return ALL_OK;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
