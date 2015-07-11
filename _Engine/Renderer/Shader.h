#pragma once

#include <Graphics/Geometry.h>
#include <Graphics/Effects.h>
#if 0

class FxShader;
class rxMesh;
class rxMaterial;


// static shader switches are set by the artist during development;
// they are used to create different material shader variations;
struct StaticSwitch
{
	String	name;
	String	value;
};



enum EShaderFeatureType
{
	// for selecting shaders based on a mesh's vertex format
	SFT_VertexFormat,

	SFT_Lighting,

	SFT_Count,
	SFT_Bits = 16,
};

struct XShaderMacro
{
	const char*	name;
	const char*	value;
};

// for describing vertex formats supported by a material shader
struct ShaderFeature_VertexFormat
{
	enum { NumBits = VertexType::NumBits };

	// NOTE: number of shader combinations can be less than (1 << NumBits), some combinations can be invalid
	static UINT NumCombinations();

	static XShaderKey GetBitMask( UINT combination );

	static void GetDefines( UINT mask, TArray<XShaderMacro> &defines );

	// feature-specific
	static XShaderKey ComposeMask( VertexType::Enum eVertexFormat );
};

int F_SelectShader(const rxMesh& mesh, const FxShader& technique);
#endif

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
