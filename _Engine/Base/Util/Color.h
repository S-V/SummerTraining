/*
=============================================================================
	File:	Color.h
	Desc:	Color representation.
=============================================================================
*/

#ifndef __MX_COLOR_H__
#define __MX_COLOR_H__

#include <Base/Object/Reflection.h>

// The per-color weighting to be used for luminance calculations in RGB order.
extern const FLOAT LUMINANCE_VECTOR[ 3 ];

mxSWIPED("idLib, Doom3/Prey SDK");
/*
================
ColorFloatToByte
================
*/
mxFORCEINLINE BYTE ColorFloatToByte( FLOAT c )
{
	DWORD colorMask = FLOATSIGNBITSET(c) ? 0 : 255;
	return (BYTE) ( ( (DWORD) ( c * 255.0f ) ) & colorMask );
}

#ifdef __MX_MATH_H_DEFINED__

mxSWIPED("idLib, Doom3/Prey SDK");
/*
================
PackColor
================
*/
mxFORCEINLINE DWORD PackColor( const Vec4D &color )
{
	DWORD dw, dx, dy, dz;

	dx = ColorFloatToByte( color.x );
	dy = ColorFloatToByte( color.y );
	dz = ColorFloatToByte( color.z );
	dw = ColorFloatToByte( color.w );

#if defined(_WIN32) || defined(__linux__)
	return ( dx << 0 ) | ( dy << 8 ) | ( dz << 16 ) | ( dw << 24 );
#elif defined(MACOS_X)
	return ( dx << 24 ) | ( dy << 16 ) | ( dz << 8 ) | ( dw << 0 );
#else
#error OS define is required!
#endif
}
mxSWIPED("idLib, Doom3/Prey SDK");
/*
================
UnpackColor
================
*/
mxFORCEINLINE void UnpackColor( const DWORD color, Vec4D &unpackedColor )
{
#if defined(_WIN32) || defined(__linux__)
	unpackedColor.Set( ( ( color >> 0 ) & 255 ) * ( 1.0f / 255.0f ),
		( ( color >> 8 ) & 255 ) * ( 1.0f / 255.0f ), 
		( ( color >> 16 ) & 255 ) * ( 1.0f / 255.0f ),
		( ( color >> 24 ) & 255 ) * ( 1.0f / 255.0f ) );
#elif defined(MACOS_X)
	unpackedColor.Set( ( ( color >> 24 ) & 255 ) * ( 1.0f / 255.0f ),
		( ( color >> 16 ) & 255 ) * ( 1.0f / 255.0f ), 
		( ( color >> 8 ) & 255 ) * ( 1.0f / 255.0f ),
		( ( color >> 0 ) & 255 ) * ( 1.0f / 255.0f ) );
#else
#error OS define is required!
#endif
}
mxSWIPED("idLib, Doom3/Prey SDK");
/*
================
PackColor
================
*/
mxFORCEINLINE DWORD PackColor( const Vec3D &color )
{
	DWORD dx, dy, dz;

	dx = ColorFloatToByte( color.x );
	dy = ColorFloatToByte( color.y );
	dz = ColorFloatToByte( color.z );

#if defined(_WIN32) || defined(__linux__)
	return ( dx << 0 ) | ( dy << 8 ) | ( dz << 16 );
#elif defined(MACOS_X)
	return ( dy << 16 ) | ( dz << 8 ) | ( dx << 0 );
#else
#error OS define is required!
#endif
}
mxSWIPED("idLib, Doom3/Prey SDK");
/*
================
UnpackColor
================
*/
mxFORCEINLINE void UnpackColor( const DWORD color, Vec3D &unpackedColor )
{
#if defined(_WIN32) || defined(__linux__)
	unpackedColor.Set( ( ( color >> 0 ) & 255 ) * ( 1.0f / 255.0f ),
		( ( color >> 8 ) & 255 ) * ( 1.0f / 255.0f ), 
		( ( color >> 16 ) & 255 ) * ( 1.0f / 255.0f ) );
#elif defined(MACOS_X)
	unpackedColor.Set( ( ( color >> 16 ) & 255 ) * ( 1.0f / 255.0f ),
		( ( color >> 8 ) & 255 ) * ( 1.0f / 255.0f ),
		( ( color >> 0 ) & 255 ) * ( 1.0f / 255.0f ) );
#else
#error OS define is required!
#endif
}

#endif // #ifdef __MX_MATH_H_DEFINED__


struct R8G8B8 { UINT8 R,G,B; };

union R8G8B8A8
{
	struct { UINT8 R,G,B,A; };
	UINT32 asU32;
};



/*
==========================================================================

		Color representation.

==========================================================================
*/

//
//	SColor - a 32 bit ARGB color.
//
class SColor {
public:
	UINT8	A, R, G, B;	// The color is stored in A8R8G8B8 format.

public:
	mxFORCEINLINE SColor()
	{}

	mxFORCEINLINE SColor( UINT32 argb )
	{
		FromUInt32( argb );
	}

	mxFORCEINLINE SColor( UINT8 alpha, UINT8 red, UINT8 green, UINT8 blue )
		: A( alpha ), R( red ), G( green ), B( blue )
	{}

	mxFORCEINLINE UINT32 AsUInt32() const
	{
		return (A << 24) | (R << 16) | (G << 8) | (B);
	}

	mxFORCEINLINE void FromUInt32( UINT32 argb )
	{
		A = ( (argb & 0xFF000000) >> 24 );
		R = ( (argb & 0x00FF0000) >> 16 );
		G = ( (argb & 0x0000FF00) >> 8 );
		B = ( (argb & 0x000000FF) );
	}

	mxFORCEINLINE void SetARGB( UINT8 alpha, UINT8 red, UINT8 green, UINT8 blue )
	{
		A = alpha;
		R = red;
		G = green;
		B = blue;
	}

	mxFORCEINLINE INT Brightness() const
	{
		return (2*(INT)R + 3*(INT)G + 1*(INT)B) >> 3;
	}

public:
	static SColor	GetRandomColor();

public:
	// Shift count and bit mask for A, R, G, B components
	enum
	{
		AlphaShift  = 24,
		RedShift    = 16,
		GreenShift  = 8,
		BlueShift   = 0
	};

	enum
	{
		AlphaMask   = 0xff000000,
		RedMask     = 0x00ff0000,
		GreenMask   = 0x0000ff00,
		BlueMask    = 0x000000ff
	};

	// Assemble A, R, G, B values into a 32-bit integer
	static inline UINT32 MakeARGB( BYTE a, BYTE r, BYTE g, BYTE b )
	{
		return (((UINT32) (b) << BlueShift)
			| ((UINT32) (g) << GreenShift)
			| ((UINT32) (r) << RedShift)
			| ((UINT32) (a) << AlphaShift))
			;
	}

	//
	//	Some common colors.
	//
	// Colors taken from http://www.keller.com/rgb.html
	//
	enum
	{
		MAROON				= 0xFF800000,
		DARKRED				= 0xFF8B0000,
		RED					= 0xFFFF0000,
		LIGHTPINK			= 0xFFFFB6C1,
		CRIMSON				= 0xFFDC143C,
		PALEVIOLETRED		= 0xFFDB7093,
		HOTPINK				= 0xFFFF69B4,
		DEEPPINK			= 0xFFFF1493,
		MEDIUMVIOLETRED 	= 0xFFC71585,
		PURPLE				= 0xFF800080,
		DARKMAGENTA			= 0xFF8B008B,
		ORCHID				= 0xFFDA70D6,
		THISTLE				= 0xFFD8BFD8,
		PLUM				= 0xFFDDA0DD,
		VIOLET				= 0xFFEE82EE,
		FUCHSIA 			= 0xFFFF00FF,
		MAGENTA 			= 0xFFFF00FF,
		MEDIUMORCHID		= 0xFFBA55D3,
		DARKVIOLET 			= 0xFF9400D3,
		DARKORCHID 			= 0xFF9932CC,
		BLUEVIOLET 			= 0xFF8A2BE2,
		INDIGO				= 0xFF4B0082,
		MEDIUMPURPLE		= 0xFF9370DB,
		SLATEBLUE			= 0xFF6A5ACD,
		MEDIUMSLATEBLUE 	= 0xFF7B68EE,
		DARKBLUE			= 0xFF00008B,
		MEDIUMBLUE			= 0xFF0000CD,
		BLUE 				= 0xFF0000FF,
		NAVY 				= 0xFF000080,
		MIDNIGHTBLUE		= 0xFF191970,
		DARKSLATEBLUE		= 0xFF483D8B,
		ROYALBLUE			= 0xFF4169E1,
		CORNFLOWERBLUE		= 0xFF6495ED,
		LIGHTSTEELBLUE		= 0xFFB0C4DE,
		ALICEBLUE			= 0xFFF0F8FF,
		GHOSTWHITE			= 0xFFF8F8FF,
		LAVENDER			= 0xFFE6E6FA,
		DODGERBLUE			= 0xFF1E90FF,
		STEELBLUE			= 0xFF4682B4,
		DEEPSKYBLUE			= 0xFF00BFFF,
		SLATEGRAY			= 0xFF708090,
		LIGHTSLATEGRAY		= 0xFF778899,
		LIGHTSKYBLUE		= 0xFF87CEFA,
		SKYBLUE				= 0xFF87CEEB,
		LIGHTBLUE			= 0xFFADD8E6,
		TEAL				= 0xFF008080,
		DARKCYAN			= 0xFF008B8B,
		DARKTURQUOISE		= 0xFF00CED1,
		CYAN				= 0xFF00FFFF,
		MEDIUMTURQUOISE 	= 0xFF48D1CC,
		CADETBLUE			= 0xFF5F9EA0,
		PALETURQUOISE		= 0xFFAFEEEE,
		LIGHTCYAN			= 0xFFE0FFFF,
		AZURE				= 0xFFF0FFFF,
		LIGHTSEAGREEN		= 0xFF20B2AA,
		TURQUOISE			= 0xFF40E0D0,
		POWDERBLUE			= 0xFFB0E0E6,
		DARKSLATEGRAY		= 0xFF2F4F4F,
		AQUAMARINE			= 0xFF7FFFD4,
		MEDIUMSPRINGGREEN	= 0xFF00FA9A,
		MEDIUMAQUAMARINE	= 0xFF66CDAA,
		SPRINGGREEN			= 0xFF00FF7F,
		MEDIUMSEAGREEN		= 0xFF3CB371,
		SEAGREEN			= 0xFF2E8B57,
		LIMEGREEN 			= 0xFF32CD32,
		DARKGREEN 			= 0xFF006400,
		GREEN				= 0xFF008000,
		LIME				= 0xFF00FF00,
		FORESTGREEN			= 0xFF228B22,
		DARKSEAGREEN		= 0xFF8FBC8F,
		LIGHTGREEN			= 0xFF90EE90,
		PALEGREEN			= 0xFF98FB98,
		MINTCREAM			= 0xFFF5FFFA,
		HONEYDEW			= 0xFFF0FFF0,
		CHARTREUSE			= 0xFF7FFF00,
		LAWNGREEN			= 0xFF7CFC00,
		OLIVEDRAB			= 0xFF6B8E23,
		DARKOLIVEGREEN		= 0xFF556B2F,
		YELLOWGREEN			= 0xFF9ACD32,
		GREENYELLOW			= 0xFFADFF2F,
		BEIGE				= 0xFFF5F5DC,
		LINEN				= 0xFFFAF0E6,
		LIGHTGOLDENRODYELLOW = 0xFFFAFAD2,
		OLIVE				= 0xFF808000,
		YELLOW				= 0xFFFFFF00,
		LIGHTYELLOW			= 0xFFFFFFE0,
		IVORY				= 0xFFFFFFF0,
		DARKKHAKI			= 0xFFBDB76B,
		KHAKI				= 0xFFF0E68C,
		PALEGOLDENROD		= 0xFFEEE8AA,
		WHEAT				= 0xFFF5DEB3,
		GOLD				= 0xFFFFD700,
		LEMONCHIFFON		= 0xFFFFFACD,
		PAPAYAWHIP			= 0xFFFFEFD5,
		DARKGOLDENROD		= 0xFFB8860B,
		GOLDENROD			= 0xFFDAA520,
		ANTIQUEWHITE		= 0xFFFAEBD7,
		CORNSILK			= 0xFFFFF8DC,
		OLDLACE				= 0xFFFDF5E6,
		MOCCASIN			= 0xFFFFE4B5,
		NAVAJOWHITE			= 0xFFFFDEAD,
		ORANGE 				= 0xFFFFA500,
		BISQUE 				= 0xFFFFE4C4,
		TAN					= 0xFFD2B48C,
		DARKORANGE			= 0xFFFF8C00,
		BURLYWOOD			= 0xFFDEB887,
		SADDLEBROWN			= 0xFF8B4513,
		SANDYBROWN			= 0xFFF4A460,
		BLANCHEDALMOND		= 0xFFFFEBCD,
		LAVENDERBLUSH		= 0xFFFFF0F5,
		SEASHELL			= 0xFFFFF5EE,
		FLORALWHITE 		= 0xFFFFFAF0,
		SNOW				= 0xFFFFFAFA,
		PERU				= 0xFFCD853F,
		PEACHPUFF			= 0xFFFFDAB9,
		CHOCOLATE			= 0xFFD2691E,
		SIENNA				= 0xFFA0522D,
		LIGHTSALMON 		= 0xFFFFA07A,
		CORAL				= 0xFFFF7F50,
		DARKSALMON			= 0xFFE9967A,
		MISTYROSE 			= 0xFFFFE4E1,
		ORANGERED 			= 0xFFFF4500,
		SALMON 				= 0xFFFA8072,
		TOMATO 				= 0xFFFF6347,
		ROSYBROWN			= 0xFFBC8F8F,
		PINK				= 0xFFFFC0CB,
		INDIANRED			= 0xFFCD5C5C,
		LIGHTCORAL			= 0xFFF08080,
		BROWN				= 0xFFA52A2A,
		FIREBRICK			= 0xFFB22222,
		BLACK				= 0xFF000000,
		DIMGRAY				= 0xFF696969,
		GRAY				= 0xFF808080,
		DARKGRAY			= 0xFFA9A9A9,
		SILVER				= 0xFFC0C0C0,
		LIGHTGREY			= 0xFFD3D3D3,
		GAINSBORO			= 0xFFDCDCDC,
		WHITESMOKE			= 0xFFF5F5F5,
		WHITE				= 0xFFFFFFFF,
		GREY				= 0xff888888,
		GREY25 				= 0xff404040,
		GREY50 				= 0xff808080,
		GREY75 				= 0xffc0c0c0,
	};

	// This code is taken from GDI+.
	// Common color constants

	enum
	{
		AliceBlue            = 0xFFF0F8FF,
		AntiqueWhite         = 0xFFFAEBD7,
		Aqua                 = 0xFF00FFFF,
		Aquamarine           = 0xFF7FFFD4,
		Azure                = 0xFFF0FFFF,
		Beige                = 0xFFF5F5DC,
		Bisque               = 0xFFFFE4C4,
		Black                = 0xFF000000,
		BlanchedAlmond       = 0xFFFFEBCD,
		Blue                 = 0xFF0000FF,
		BlueViolet           = 0xFF8A2BE2,
		Brown                = 0xFFA52A2A,
		BurlyWood            = 0xFFDEB887,
		CadetBlue            = 0xFF5F9EA0,
		Chartreuse           = 0xFF7FFF00,
		Chocolate            = 0xFFD2691E,
		Coral                = 0xFFFF7F50,
		CornflowerBlue       = 0xFF6495ED,
		Cornsilk             = 0xFFFFF8DC,
		Crimson              = 0xFFDC143C,
		Cyan                 = 0xFF00FFFF,
		DarkBlue             = 0xFF00008B,
		DarkCyan             = 0xFF008B8B,
		DarkGoldenrod        = 0xFFB8860B,
		DarkGray             = 0xFFA9A9A9,
		DarkGreen            = 0xFF006400,
		DarkKhaki            = 0xFFBDB76B,
		DarkMagenta          = 0xFF8B008B,
		DarkOliveGreen       = 0xFF556B2F,
		DarkOrange           = 0xFFFF8C00,
		DarkOrchid           = 0xFF9932CC,
		DarkRed              = 0xFF8B0000,
		DarkSalmon           = 0xFFE9967A,
		DarkSeaGreen         = 0xFF8FBC8B,
		DarkSlateBlue        = 0xFF483D8B,
		DarkSlateGray        = 0xFF2F4F4F,
		DarkTurquoise        = 0xFF00CED1,
		DarkViolet           = 0xFF9400D3,
		DeepPink             = 0xFFFF1493,
		DeepSkyBlue          = 0xFF00BFFF,
		DimGray              = 0xFF696969,
		DodgerBlue           = 0xFF1E90FF,
		Firebrick            = 0xFFB22222,
		FloralWhite          = 0xFFFFFAF0,
		ForestGreen          = 0xFF228B22,
		Fuchsia              = 0xFFFF00FF,
		Gainsboro            = 0xFFDCDCDC,
		GhostWhite           = 0xFFF8F8FF,
		Gold                 = 0xFFFFD700,
		Goldenrod            = 0xFFDAA520,
		Gray                 = 0xFF808080,
		Green                = 0xFF008000,
		GreenYellow          = 0xFFADFF2F,
		Honeydew             = 0xFFF0FFF0,
		HotPink              = 0xFFFF69B4,
		IndianRed            = 0xFFCD5C5C,
		Indigo               = 0xFF4B0082,
		Ivory                = 0xFFFFFFF0,
		Khaki                = 0xFFF0E68C,
		Lavender             = 0xFFE6E6FA,
		LavenderBlush        = 0xFFFFF0F5,
		LawnGreen            = 0xFF7CFC00,
		LemonChiffon         = 0xFFFFFACD,
		LightBlue            = 0xFFADD8E6,
		LightCoral           = 0xFFF08080,
		LightCyan            = 0xFFE0FFFF,
		LightGoldenrodYellow = 0xFFFAFAD2,
		LightGray            = 0xFFD3D3D3,
		LightGreen           = 0xFF90EE90,
		LightPink            = 0xFFFFB6C1,
		LightSalmon          = 0xFFFFA07A,
		LightSeaGreen        = 0xFF20B2AA,
		LightSkyBlue         = 0xFF87CEFA,
		LightSlateGray       = 0xFF778899,
		LightSteelBlue       = 0xFFB0C4DE,
		LightYellow          = 0xFFFFFFE0,
		Lime                 = 0xFF00FF00,
		LimeGreen            = 0xFF32CD32,
		Linen                = 0xFFFAF0E6,
		Magenta              = 0xFFFF00FF,
		Maroon               = 0xFF800000,
		MediumAquamarine     = 0xFF66CDAA,
		MediumBlue           = 0xFF0000CD,
		MediumOrchid         = 0xFFBA55D3,
		MediumPurple         = 0xFF9370DB,
		MediumSeaGreen       = 0xFF3CB371,
		MediumSlateBlue      = 0xFF7B68EE,
		MediumSpringGreen    = 0xFF00FA9A,
		MediumTurquoise      = 0xFF48D1CC,
		MediumVioletRed      = 0xFFC71585,
		MidnightBlue         = 0xFF191970,
		MintCream            = 0xFFF5FFFA,
		MistyRose            = 0xFFFFE4E1,
		Moccasin             = 0xFFFFE4B5,
		NavajoWhite          = 0xFFFFDEAD,
		Navy                 = 0xFF000080,
		OldLace              = 0xFFFDF5E6,
		Olive                = 0xFF808000,
		OliveDrab            = 0xFF6B8E23,
		Orange               = 0xFFFFA500,
		OrangeRed            = 0xFFFF4500,
		Orchid               = 0xFFDA70D6,
		PaleGoldenrod        = 0xFFEEE8AA,
		PaleGreen            = 0xFF98FB98,
		PaleTurquoise        = 0xFFAFEEEE,
		PaleVioletRed        = 0xFFDB7093,
		PapayaWhip           = 0xFFFFEFD5,
		PeachPuff            = 0xFFFFDAB9,
		Peru                 = 0xFFCD853F,
		Pink                 = 0xFFFFC0CB,
		Plum                 = 0xFFDDA0DD,
		PowderBlue           = 0xFFB0E0E6,
		Purple               = 0xFF800080,
		Red                  = 0xFFFF0000,
		RosyBrown            = 0xFFBC8F8F,
		RoyalBlue            = 0xFF4169E1,
		SaddleBrown          = 0xFF8B4513,
		Salmon               = 0xFFFA8072,
		SandyBrown           = 0xFFF4A460,
		SeaGreen             = 0xFF2E8B57,
		SeaShell             = 0xFFFFF5EE,
		Sienna               = 0xFFA0522D,
		Silver               = 0xFFC0C0C0,
		SkyBlue              = 0xFF87CEEB,
		SlateBlue            = 0xFF6A5ACD,
		SlateGray            = 0xFF708090,
		Snow                 = 0xFFFFFAFA,
		SpringGreen          = 0xFF00FF7F,
		SteelBlue            = 0xFF4682B4,
		Tan                  = 0xFFD2B48C,
		Teal                 = 0xFF008080,
		Thistle              = 0xFFD8BFD8,
		Tomato               = 0xFFFF6347,
		Transparent          = 0x00FFFFFF,
		Turquoise            = 0xFF40E0D0,
		Violet               = 0xFFEE82EE,
		Wheat                = 0xFFF5DEB3,
		White                = 0xFFFFFFFF,
		WhiteSmoke           = 0xFFF5F5F5,
		Yellow               = 0xFFFFFF00,
		YellowGreen          = 0xFF9ACD32
	};

private:
	// This function is never called, it just contains compile-time assertions.
	void StaticChecks()
	{
		mxSTATIC_ASSERT( sizeof( SColor ) == sizeof( INT32 ) );
	}
};

//
//	RGBf - represents a color in RGB space; consists of 3 floating-point values.
//
class RGBf {
public:
	union {
		struct {
			FLOAT	R, G, B;	// The red, the green and the blue components.
		};
		struct {
			FLOAT	x, y, z;
		};
	};

public:
	mxFORCEINLINE RGBf()
	{}

	mxFORCEINLINE RGBf( FLOAT red, FLOAT green, FLOAT blue )
		: R( red ), G( green ), B( blue )
	{}

#ifdef __MX_MATH_H_DEFINED__
	mxFORCEINLINE RGBf( const Vec3D& v )
		: R( v.x ), G( v.y ), B( v.z )
	{}
#endif // #ifdef __MX_MATH_H_DEFINED__

	mxFORCEINLINE RGBf( const RGBf& other )
		: R( other.R ), G( other.G ), B( other.B )
	{}


	// Sets all three components of a color.

	mxFORCEINLINE void Set( FLOAT red, FLOAT green, FLOAT blue )
	{
		R = clampf( red, 0.0f, 1.0f );
		G = clampf( green, 0.0f, 1.0f );
		B = clampf( blue, 0.0f, 1.0f );
	}

	mxFORCEINLINE FLOAT * ToPtr()
	{
		return &R;
	}
	mxFORCEINLINE const FLOAT * ToPtr() const
	{
		return &R;
	}

#ifdef __MX_MATH_H_DEFINED__
	mxFORCEINLINE Vec3D& AsVec3D()
	{
		return reinterpret_cast< Vec3D& >( *this );
	}
	mxFORCEINLINE const Vec3D& AsVec3D() const
	{
		return reinterpret_cast< const Vec3D& >( *this );
	}
#endif // #ifdef __MX_MATH_H_DEFINED__

	// Clamps all components to the specified range.
	mxFORCEINLINE void ClampTo( FLOAT min, FLOAT max )
	{
		R = clampf( R, min, max );
		G = clampf( G, min, max );
		B = clampf( B, min, max );
	}

	// Clamps all components to [0..1].
	mxFORCEINLINE void Saturate()
	{
		ClampTo( 0.0f, 1.0f );
	}

	// Returns 'true' if all components are within the specified range.
	mxFORCEINLINE
	bool InsideRange( FLOAT min = 0.0f, FLOAT max = 1.0f ) const
	{
		return IsInRangeInc( R, min, max )
			&& IsInRangeInc( G, min, max )
			&& IsInRangeInc( B, min, max );
	}

	// extracts luminance
	mxFORCEINLINE FLOAT Brightness() const
	{
		return R * LUMINANCE_VECTOR[0] + G * LUMINANCE_VECTOR[1] + B * LUMINANCE_VECTOR[2];
	}

	// Returns a reference to the i-th component of a color. The value of i must be 0, 1, or 2. 
	FLOAT &			operator [] ( UINT i );

	//	Returns a constant reference to the i-th component of a color. The value of i must be 0, 1, or 2. 
	const FLOAT &	operator [] ( UINT i ) const;

	//	Sets all three components to the value f. 
	RGBf &		operator = ( FLOAT f );

	//	Adds the color c. 
	RGBf &		operator += ( const RGBf& c );

	//	Subtracts the color c. 
	RGBf &		operator -= ( const RGBf& c );

	//	Multiplies by the color c. 
	RGBf &		operator *= ( const RGBf& c );

	//	Multiplies all three components by t. 
	RGBf &		operator *= ( FLOAT t );

	//	Divides all three components by t.
	RGBf &		operator /= ( FLOAT t );

	RGBf & operator = ( const RGBf& other );

	// Returns the negation of the color c.
	friend RGBf operator - ( const RGBf& c );	

	// Returns the sum of the colors color1 and color2.
	friend RGBf operator + ( const RGBf& color1, const RGBf& color2 );

	//	Returns the difference of the colors color1 and color2. 
	friend RGBf operator - ( const RGBf& color1, const RGBf& color2 );

	//	Returns the product of the colors color1 and color2. 
	friend RGBf operator * ( const RGBf& color1, const RGBf& color2 );

	//	Returns the product of the color c and the scalar t. 
	friend RGBf operator * ( const RGBf& c, FLOAT t );

	//	Returns the product of the color c and the scalar t. 
	friend RGBf operator * ( FLOAT t, const RGBf& c );

	//	Returns the product of the color c and the inverse of the scalar t. 
	friend RGBf operator / ( const RGBf& c, FLOAT t );

	//	Returns a boolean value indicating the equality of the colors color1 and color2. 
	friend bool operator == ( const RGBf& color1, const RGBf& color2 );

	//	Returns a boolean value indicating the inequality of the colors color1 and color2.
	friend bool operator != ( const RGBf& color1, const RGBf& c2 );
};

mxDECLARE_POD_TYPE(SColor);


//
//	RGBAf - a 128-bit color in RGBA space; consists of 4 floating-point values.
//
class RGBAf {
public:
	union {
		struct {
			FLOAT	R, G, B, A;
		};
		struct {
			FLOAT	x, y, z, w;
		};
		Vector4		q;
	};

public:
	mxFORCEINLINE RGBAf()
	{}

	mxFORCEINLINE RGBAf( Vector4Arg q )
		: q( q )
	{}

	mxFORCEINLINE explicit RGBAf( FLOAT f )
		: R( f ), G( f ), B( f ), A( f )
	{}

#ifdef __MX_MATH_H_DEFINED__
	mxFORCEINLINE explicit RGBAf( const Vec4D& v )
	{
		*this = *(RGBAf*) &v;
	}
#endif

	mxFORCEINLINE RGBAf( FLOAT red, FLOAT green, FLOAT blue, FLOAT alpha = 1.0f )
		: R( red ), G( green ), B( blue ), A( alpha )
	{}
	mxFORCEINLINE RGBAf( const RGBAf& other )
		: R( other.R ), G( other.G ), B( other.B ), A( other.A )
	{}
	mxFORCEINLINE RGBAf( const SColor& argb )
	{
		this->FromARGB(argb.AsUInt32());
	}
	mxFORCEINLINE RGBAf( const RGBf& rgb )
	{
		this->R = rgb.R;
		this->G = rgb.G;
		this->B = rgb.B;
		this->A = 1.0f;
	}

	// Sets all four components of a color.

	mxFORCEINLINE void Set( FLOAT red, FLOAT green, FLOAT blue, FLOAT alpha = 1.0f )
	{
		this->R = clampf( red, 0.0f, 1.0f );
		this->G = clampf( green, 0.0f, 1.0f );
		this->B = clampf( blue, 0.0f, 1.0f );
		this->A = clampf( alpha, 0.0f, 1.0f );
	}
	mxFORCEINLINE void SetAll( FLOAT what )
	{
		this->R = clampf( what, 0.0f, 1.0f );
		this->G = clampf( what, 0.0f, 1.0f );
		this->B = clampf( what, 0.0f, 1.0f );
		this->A = clampf( what, 0.0f, 1.0f );
	}

	mxFORCEINLINE FLOAT* ToPtr()
	{
		return (FLOAT*) this;
	}
	mxFORCEINLINE const FLOAT* ToPtr() const
	{
		return (const FLOAT*) this;
	}

#ifdef __MX_MATH_H_DEFINED__
	mxFORCEINLINE Vec4D& AsVec4D()
	{
		return reinterpret_cast< Vec4D& >( *this );
	}
	mxFORCEINLINE const Vec4D& AsVec4D() const
	{
		return reinterpret_cast< const Vec4D& >( *this );
	}
#endif

	mxFORCEINLINE RGBAf& FromRGBA( UINT32 rgba )
	{
		const FLOAT f = 1.0f / 255.0f;
		this->R = f * (FLOAT) UINT8( rgba >> 24 );
		this->G = f * (FLOAT) UINT8( rgba >> 16 );
		this->B = f * (FLOAT) UINT8( rgba >> 8 );
		this->A = f * (FLOAT) UINT8( rgba >> 0 );
		return *this;
	}
	mxFORCEINLINE RGBAf& FromARGB( UINT32 argb )
	{
		const FLOAT f = 1.0f / 255.0f;
		this->R = f * (FLOAT) UINT8( argb >> 16 );
		this->G = f * (FLOAT) UINT8( argb >>  8 );
		this->B = f * (FLOAT) UINT8( argb >>  0 );
		this->A = f * (FLOAT) UINT8( argb >> 24 );
		return *this;
	}
	mxFORCEINLINE SColor ToARGB() const
	{
		return SColor(
			UINT8( this->A * 255.0f ),
			UINT8( this->R * 255.0f ),
			UINT8( this->G * 255.0f ),
			UINT8( this->B * 255.0f )
		);
	}
	
	mxFORCEINLINE RGBf ToRGB() const
	{
		return RGBf( R, G, B );
	}

	mxFORCEINLINE UINT32 ToInt32() const
	{
		R8G8B8A8	rgba;
		rgba.R = ColorFloatToByte( this->R );
		rgba.G = ColorFloatToByte( this->G );
		rgba.B = ColorFloatToByte( this->B );
		rgba.A = ColorFloatToByte( this->A );
		return rgba.asU32;
	}

	mxFORCEINLINE RGBf &		AsRGB()			{ return *(RGBf*)this; }
	mxFORCEINLINE const RGBf &	AsRGB() const	{ return *(RGBf*)this; }

	mxFORCEINLINE RGBAf operator + ( FLOAT f ) const
	{
		return RGBAf( R + f, G + f, B + f, A + f );
	}
	mxFORCEINLINE RGBAf	operator += ( FLOAT f )
	{
		this->R += f;
		this->G += f;
		this->B += f;
		this->A += f;
		return *this;
	}
	mxFORCEINLINE RGBAf operator * ( FLOAT f ) const
	{
		return RGBAf( R * f, G * f, B * f, A * f );
	}
	mxFORCEINLINE RGBAf & operator *= ( FLOAT f )
	{
		this->R *= f;
		this->G *= f;
		this->B *= f;
		this->A *= f;
		return *this;
	}
	mxFORCEINLINE RGBAf operator * ( const RGBAf& other ) const
	{
		return RGBAf( R * other.R, G * other.G, B * other.B, A * other.A );
	}
	mxFORCEINLINE RGBAf operator / ( FLOAT f ) const
	{
		mxASSERT(f!=0.0f);
		FLOAT inv = 1.0f / f;
		return RGBAf( R * inv, G * inv, B * inv, A * inv );
	}
	RGBAf & operator /= ( FLOAT f )
	{
		mxASSERT(f!=0.0f);
		FLOAT inv = 1.0f / f;
		this->R *= inv;
		this->G *= inv;
		this->B *= inv;
		this->A *= inv;
		return *this;
	}
	mxFORCEINLINE RGBAf & operator = ( const RGBAf& other )
	{
		this->R = other.R;
		this->G = other.G;
		this->B = other.B;
		this->A = other.A;
		return *this;
	}

	mxFORCEINLINE friend bool operator == ( const RGBAf& color1, const RGBAf& color2 )
	{
		return ( color1.R == color2.R )
			&& ( color1.G == color2.G )
			&& ( color1.B == color2.B )
			&& ( color1.A == color2.A );
	}

	mxFORCEINLINE friend RGBAf operator + ( const RGBAf& color1, const RGBAf& color2 )
	{
		return RGBAf(
			color1.R + color2.R,
			color1.G + color2.G,
			color1.B + color2.B,
			color1.A + color2.A
		);
	}

	// extracts luminance
	mxFORCEINLINE FLOAT Brightness() const
	{
		return AsRGB().Brightness();
	}

	//--------------------------------------------------------------//
	//	Reflection.
	//--------------------------------------------------------------//

	//mxDECLARE_REFLECTION;

	//mxFORCEINLINE Reflect::EFieldType reflectionFieldType() const
	//{ return Reflect::EFieldType::Type_ColorRGBA; }

	//--------------------------------------------------------------//
	//	Serializers.
	//--------------------------------------------------------------//
/*
	friend inline AStreamReader & operator >> ( AStreamReader& archive, RGBAf& c )
	{
		return archive >> c.x >> c.y >> c.z >> c.w;
	}
	friend inline AStreamWriter & operator << ( AStreamWriter& archive, const RGBAf& c )
	{
		return archive << c.x << c.y << c.z << c.w;
	}
*/

public:
	static const RGBAf		BLACK;
	static const RGBAf		DARK_GREY;
	static const RGBAf		GRAY;	//MID_GREY
	static const RGBAf		LIGHT_GREY;
	static const RGBAf		WHITE;

	static const RGBAf		RED;
	static const RGBAf		GREEN;
	static const RGBAf		BLUE;
	static const RGBAf		YELLOW;
	static const RGBAf		MAGENTA;
	static const RGBAf		CYAN;
	static const RGBAf		ORANGE;
	static const RGBAf		PURPLE;
	static const RGBAf		PINK;
	static const RGBAf		BROWN;

	static const RGBAf		BEACH_SAND;
	static const RGBAf		LIGHT_YELLOW_GREEN;
	static const RGBAf		DARK_YELLOW_GREEN;
	static const RGBAf		DARKBROWN;

	//
	// colors taken from http://prideout.net/archive/colors.php
	//
	static const RGBAf		aliceblue			;
	static const RGBAf		antiquewhite		;
	static const RGBAf		aqua				;
	static const RGBAf		aquamarine			;
	static const RGBAf		azure				;
	static const RGBAf		beige				;
	static const RGBAf		bisque				;
	static const RGBAf		black				;
	static const RGBAf		blanchedalmond		;
	static const RGBAf		blue				;
	static const RGBAf		blueviolet			;
	static const RGBAf		brown				;
	static const RGBAf		burlywood			;
	static const RGBAf		cadetblue			;
	static const RGBAf		chartreuse			;
	static const RGBAf		chocolate			;
	static const RGBAf		coral				;
	static const RGBAf		cornflowerblue		;
	static const RGBAf		cornsilk			;
	static const RGBAf		crimson				;
	static const RGBAf		cyan				;
	static const RGBAf		darkblue			;
	static const RGBAf		darkcyan			;
	static const RGBAf		darkgoldenrod		;
	static const RGBAf		darkgray			;
	static const RGBAf		darkgreen			;
	static const RGBAf		darkgrey			;
	static const RGBAf		darkkhaki			;
	static const RGBAf		darkmagenta			;
	static const RGBAf		darkolivegreen		;
	static const RGBAf		darkorange			;
	static const RGBAf		darkorchid			;
	static const RGBAf		darkred				;
	static const RGBAf		darksalmon			;
	static const RGBAf		darkseagreen		;
	static const RGBAf		darkslateblue		;
	static const RGBAf		darkslategray		;
	static const RGBAf		darkslategrey		;
	static const RGBAf		darkturquoise		;
	static const RGBAf		darkviolet			;
	static const RGBAf		deeppink			;
	static const RGBAf		deepskyblue			;
	static const RGBAf		dimgray				;
	static const RGBAf		dimgrey				;
	static const RGBAf		dodgerblue			;
	static const RGBAf		firebrick			;
	static const RGBAf		floralwhite			;
	static const RGBAf		forestgreen			;
	static const RGBAf		fuchsia				;
	static const RGBAf		gainsboro			;
	static const RGBAf		ghostwhite			;
	static const RGBAf		gold				;
	static const RGBAf		goldenrod			;
	static const RGBAf		gray				;
	static const RGBAf		green				;
	static const RGBAf		greenyellow			;
	static const RGBAf		grey				;
	static const RGBAf		honeydew			;
	static const RGBAf		hotpink				;
	static const RGBAf		indianred			;
	static const RGBAf		indigo				;
	static const RGBAf		ivory				;
	static const RGBAf		khaki				;
	static const RGBAf		lavender			;
	static const RGBAf		lavenderblush		;
	static const RGBAf		lawngreen			;
	static const RGBAf		lemonchiffon		;
	static const RGBAf		lightblue			;
	static const RGBAf		lightcoral			;
	static const RGBAf		lightcyan			;
	static const RGBAf		lightgoldenrodyellow;
	static const RGBAf		lightgray			;
	static const RGBAf		lightgreen			;
	static const RGBAf		lightgrey			;
	static const RGBAf		lightpink			;
	static const RGBAf		lightsalmon			;
	static const RGBAf		lightseagreen		;
	static const RGBAf		lightskyblue		;
	static const RGBAf		lightslategray		;
	static const RGBAf		lightslategrey		;
	static const RGBAf		lightsteelblue		;
	static const RGBAf		lightyellow			;
	static const RGBAf		lime				;
	static const RGBAf		limegreen			;
	static const RGBAf		linen				;
	static const RGBAf		magenta				;
	static const RGBAf		maroon				;
	static const RGBAf		mediumaquamarine	;
	static const RGBAf		mediumblue			;
	static const RGBAf		mediumorchid		;
	static const RGBAf		mediumpurple		;
	static const RGBAf		mediumseagreen		;
	static const RGBAf		mediumslateblue		;
	static const RGBAf		mediumspringgreen	;
	static const RGBAf		mediumturquoise		;
	static const RGBAf		mediumvioletred		;
	static const RGBAf		midnightblue		;
	static const RGBAf		mintcream			;
	static const RGBAf		mistyrose			;
	static const RGBAf		moccasin			;
	static const RGBAf		navajowhite			;
	static const RGBAf		navy				;
	static const RGBAf		oldlace				;
	static const RGBAf		olive				;
	static const RGBAf		olivedrab			;
	static const RGBAf		orange				;
	static const RGBAf		orangered			;
	static const RGBAf		orchid				;
	static const RGBAf		palegoldenrod		;
	static const RGBAf		palegreen			;
	static const RGBAf		paleturquoise		;
	static const RGBAf		palevioletred		;
	static const RGBAf		papayawhip			;
	static const RGBAf		peachpuff			;
	static const RGBAf		peru				;
	static const RGBAf		pink				;
	static const RGBAf		plum				;
	static const RGBAf		powderblue			;
	static const RGBAf		purple				;
	static const RGBAf		red					;
	static const RGBAf		rosybrown			;
	static const RGBAf		royalblue			;
	static const RGBAf		saddlebrown			;
	static const RGBAf		salmon				;
	static const RGBAf		sandybrown			;
	static const RGBAf		seagreen			;
	static const RGBAf		seashell			;
	static const RGBAf		sienna				;
	static const RGBAf		silver				;
	static const RGBAf		skyblue				;
	static const RGBAf		slateblue			;
	static const RGBAf		slategray			;
	static const RGBAf		slategrey			;
	static const RGBAf		snow				;
	static const RGBAf		springgreen			;
	static const RGBAf		steelblue			;
	static const RGBAf		tan					;
	static const RGBAf		teal				;
	static const RGBAf		thistle				;
	static const RGBAf		tomato				;
	static const RGBAf		turquoise			;
	static const RGBAf		violet				;
	static const RGBAf		wheat				;
	static const RGBAf		white				;
	static const RGBAf		whitesmoke			;
	static const RGBAf		yellow				;
	static const RGBAf		yellowgreen			;
};

mxDECLARE_POD_TYPE(RGBAf);
mxDECLARE_STRUCT(RGBAf);

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

mxFORCEINLINE RGBf & RGBf::operator = ( const RGBf& other )
{
	this->R = other.R;
	this->G = other.G;
	this->B = other.B;
	return *this;
}

mxFORCEINLINE
RGBf & RGBf::operator = ( FLOAT f )
{
	this->R = f;
	this->G = f;
	this->B = f;
	return *this;
}

mxFORCEINLINE
RGBf operator + ( const RGBf& color1, const RGBf& color2 )
{
	return RGBf(
		color1.R + color2.R,
		color1.G + color2.G,
		color1.B + color2.B
	);
}

mxFORCEINLINE
RGBf operator * ( FLOAT t, const RGBf& c )
{
	return RGBf(
		c.R * t,
		c.G * t,
		c.B * t
	);
}

mxFORCEINLINE
RGBf & RGBf::operator *= ( FLOAT f )
{
	this->R *= f;
	this->G *= f;
	this->B *= f;
	return *this;
}

mxFORCEINLINE
RGBf & RGBf::operator /= ( FLOAT t )
{
	mxASSERT(t!=0.0f);
	FLOAT inv = 1.0f / t;
	this->R *= inv;
	this->G *= inv;
	this->B *= inv;
	return *this;
}

mxFORCEINLINE
RGBf operator * ( const RGBf& c, FLOAT t )
{
	return RGBf(
		c.R * t,
		c.G * t,
		c.B * t
	);
}

mxFORCEINLINE
bool operator == ( const RGBf& color1, const RGBf& color2 )
{
	return ( color1.R == color2.R )
		&& ( color1.G == color2.G )
		&& ( color1.B == color2.B );
}

mxFORCEINLINE
RGBf operator * ( const RGBf& color1, const RGBf& color2 )
{
	return RGBf(
		color1.R * color2.R,
		color1.G * color2.G,
		color1.B * color2.B
	);
}

#endif // !__MX_COLOR_H__

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
