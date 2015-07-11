#pragma once

//	graphics driver type
struct GAPI {
	enum Enum {
		None = 0,	// can be used for applications which don't need visualization

		Direct3D_9,	// D3D9 without shaders (fixed-function pipeline, D3D7-like).
		Direct3D_9S,// D3D9 using shaders only.

		Direct3D_10,//  on Windows Vista and above (which is always shaders only).
		Direct3D_10_1,
		Direct3D_11,

		OpenGL,		// OpenGL without shaders.
		OpenGLS,	// OpenGL using shaders only.
		OpenGLES,	// OpenGL ES 1.1
		OpenGLES20Mobile,	// OpenGL ES 2.0 mobile variant
		OpenGLES20Desktop,	// OpenGL ES 2.0 desktop variant (i.e. NaCl)

		GCM,		// Sony PlayStation 3 GCM
		Xenon,		// Xbox 360
		Hollywood,	// Nintendo Wii

		Molehill,	// Flash 11 Stage3D

		Unknown,

		AutomaticSelection,	// Select the best GAPI automatically.

		MAX	//<= Marker. Don't use!
	};
};

struct DeviceVendor {
	enum Enum {
		Vendor_Unknown	= 0,
		Vendor_3DLABS	= 1,	// 3Dlabs, Inc. Ltd
		Vendor_ATI		= 2,	// ATI Technologies Inc. / Advanced Micro Devices, Inc.
		Vendor_Intel	= 3,	// Intel Corporation
		Vendor_Matrox	= 4,	// Matrox Electronic Systems Ltd.
		Vendor_NVidia	= 5,	// NVIDIA Corporation
		Vendor_S3		= 6,	// S3 Graphics Co., Ltd
		Vendor_SIS		= 7,	// SIS
	};
	static Enum FourCCToVendorEnum( UINT32 fourCC );
	static const char* GetVendorString( Enum vendorId );
};

struct EImageFileFormat {
	enum Enum {
		IFF_Unknown,
		IFF_BMP,
		IFF_JPG,
		IFF_PNG,
		IFF_DDS,
		IFF_TIFF,
		IFF_GIF,
		IFF_WMP,
	};
	// e.g. ".jpg", ".bmp"
	static const char* GetFileExtension( Enum e );
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
