#include "Renderer/Renderer_PCH.h"
#pragma hdrstop
#include <Renderer/Texture.h>
#include <Renderer/Shader.h>
#include <Renderer/Mesh.h>
#if 0

UINT ShaderFeature_VertexFormat::NumCombinations()
{
	return VertexType::Count;
}

void ShaderFeature_VertexFormat::GetDefines( UINT mask, TArray<XShaderMacro> &defines )
{
	if( mask & BIT(VertexType::Static) ) {
		//
	}
	if( mask & BIT(VertexType::Skinned) ) {
		//
	}
}

XShaderKey ShaderFeature_VertexFormat::ComposeMask( VertexType::Enum eVertexFormat )
{
	return eVertexFormat;
}

int F_SelectShader(const rxMesh& mesh, const FxShader& technique)
{
	// iterate over supported shader features and construct a (compact) bit string
	// for searching for a shader variation that is best-suited for the given context

	XShaderKey key = 0;

//	for( UINT i = 0; i < technique.features )

	UINT shift = 0;

	if( technique.features & SFT_VertexFormat )
	{
		key = ShaderFeature_VertexFormat::ComposeMask(mesh.m_vertexFormat);
		shift += ShaderFeature_VertexFormat::NumBits;
	}

	//@todo: binary search or even direct array addressing;
	//generate shader permutations per pass, not per technique?

	for( int i = 0; i < technique.permutations.Num(); i++ )
	{
		if( technique.permutations[i] == key )
		{
			return i;
		}
	}

	return -1;
}
#endif
//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
