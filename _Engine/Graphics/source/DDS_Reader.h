//--------------------------------------------------------------------------------------
// DDS.h
//
// This header defines constants and structures that are useful when parsing 
// DDS files.  DDS files were originally designed to use several structures
// and constants that are native to DirectDraw and are defined in ddraw.h,
// such as DDSURFACEDESC2 and DDSCAPS2.  This file defines similar 
// (compatible) constants and structures so that one can use DDS files 
// without needing to include ddraw.h.
//--------------------------------------------------------------------------------------
#pragma once

#pragma pack(push,1)
struct DDS_PIXELFORMAT
{
    UINT32 	size;		// Structure size; set to 32 (bytes).
    UINT32 	flags;		// Values which indicate what type of data is in the surface. 
    UINT32 	fourCC;		// Four-character codes for specifying compressed or custom formats.
    UINT32 	RGBBitCount;// Number of bits in an RGB (possibly including alpha) format. Valid when dwFlags includes DDPF_RGB, DDPF_LUMINANCE, or DDPF_YUV.
    UINT32 	RBitMask;	// Red (or luminance or Y) mask for reading color data. For instance, given the A8R8G8B8 format, the red mask would be 0x00ff0000.
    UINT32 	GBitMask;	// Green (or U) mask for reading color data. For instance, given the A8R8G8B8 format, the green mask would be 0x0000ff00.
    UINT32 	BBitMask;	// Blue (or V) mask for reading color data. For instance, given the A8R8G8B8 format, the blue mask would be 0x000000ff.
    UINT32 	ABitMask;	// Alpha mask for reading alpha data. dwFlags must include DDPF_ALPHAPIXELS or DDPF_ALPHA. For instance, given the A8R8G8B8 format, the alpha mask would be 0xff000000.
};
struct DDS_HEADER
{
    UINT32 			size;	// Size of structure. This member must be set to 124.
    UINT32 			flags;	// Flags to indicate which members contain valid data.

	UINT32 			height;	// Surface height (in pixels).
    UINT32 			width;	// Surface width (in pixels).

	// The pitch or number of bytes per scan line in an uncompressed texture;
	// the total number of bytes in the top level texture for a compressed texture.
	UINT32 			pitchOrLinearSize;

	// Depth of a volume texture (in pixels) (if DDS_HEADER_FLAGS_VOLUME is set in flags), otherwise unused.
	UINT32 			depth;

    UINT32 			mipMapCount;	// Number of mipmap levels, otherwise unused.

    UINT32 			reserved1[11];	// Unused.

    DDS_PIXELFORMAT	pixelFormat;	// The pixel format (see DDS_PIXELFORMAT).

    UINT32 			surfaceFlags;	// Specifies the complexity of the surfaces stored.
    UINT32 			cubemapFlags;	// Additional detail about the surfaces stored.

	UINT32 			reserved2[3];	// Unused.
};
// DDS header extension to handle resource arrays.
struct DDS_HEADER_DXT10
{
    UINT32	dxgiFormat;	// DXGI_FORMAT
    UINT32	resourceDimension;	// D3D10_RESOURCE_DIMENSION
    UINT32	miscFlag; // see D3D11_RESOURCE_MISC_FLAG
    UINT32	arraySize;
    UINT32	reserved; // see DDS_MISC_FLAGS2
};
#pragma pack(pop)

ASSERT_SIZEOF( DDS_HEADER, 124 );
ASSERT_SIZEOF( DDS_HEADER_DXT10, 20 );

// returns a valid pointer to DDS file header and the offset to the start of the texture data
const DDS_HEADER* DDS_ParseHeader( const void* _data, UINT _size, UINT *_offset = NULL );


ERet DDS_Parse( const void* _data, UINT _size, TextureImage &_dds );

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
