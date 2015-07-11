/*
=============================================================================
	Graphics materials describe how geometry is rendered.
=============================================================================
*/
#pragma once

#include <Core/Asset.h>
#include <Core/ObjectModel.h>
#include <Graphics/Effects.h>
#include <Renderer/Texture.h>

enum { MAX_TEXTURE_LAYERS = 8 };

struct MaterialHeader_d
{
	UINT32	fourCC;
	UINT32	numUniforms;	// number of float4 values
	UINT32	numResources;	// number of used textures
	UINT32	materialHash;
};

struct TextureLayer : public CStruct
{
	rxTexture *	texture;
	SamplerID	sampler;	// internal sampler index
public:
	mxDECLARE_CLASS( TextureLayer, CStruct );
	mxDECLARE_REFLECTION;
};

/*
--------------------------------------------------------------
	rxMaterial represents a graphics material.
	Material = shader code (technique) + shader data (parameters).
--------------------------------------------------------------
*/
struct rxMaterial : public CStruct
{
	TPtr< FxShader >		m_shader;	// pointer to shader technique, must always be valid
	ParameterBuffer			m_uniforms;	// uniform shader parameters are kept here
	TBuffer< TextureLayer >	m_textures;	// pointers to shader resources such as textures
	UINT32					m_sort;		// similar materials will have similar hashes
public:
	mxDECLARE_CLASS( rxMaterial, CStruct );
	mxDECLARE_REFLECTION;

	rxMaterial();

	ERet Slow_SetUniform( const char* name, const void* data );

public:
	ERet Load( AStreamReader& stream, Clump* clump );

	static AssetTypeT GetAssetType() { return AssetTypes::MATERIAL; }

	static ERet Load( Assets::LoadContext2 & context );
	static ERet Online( Assets::LoadContext2 & context );
	static void Offline( Assets::LoadContext2 & context );
	static void Destruct( Assets::LoadContext2 & context );

public:
	static const UINT32 MAX_UNIFORM_BUFFER_SIZE = 65536;
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
