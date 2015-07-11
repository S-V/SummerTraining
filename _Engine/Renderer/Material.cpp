/*
=============================================================================
	Graphics material.
=============================================================================
*/
#include "Renderer/Renderer_PCH.h"
#pragma hdrstop
#include <Renderer/Renderer.h>
#include <Renderer/Material.h>

mxDEFINE_CLASS( TextureLayer );
mxBEGIN_REFLECTION( TextureLayer )
	mxMEMBER_FIELD(texture),
	mxMEMBER_FIELD(sampler),
mxEND_REFLECTION

/*
-----------------------------------------------------------------------------
	rxMaterial
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS( rxMaterial );
mxBEGIN_REFLECTION( rxMaterial )
	mxMEMBER_FIELD(m_shader),
	mxMEMBER_FIELD(m_uniforms),
	mxMEMBER_FIELD(m_textures),
	mxMEMBER_FIELD(m_sort),
mxEND_REFLECTION
rxMaterial::rxMaterial()
{
	m_shader = nil;
	m_sort = 0;
}

ERet rxMaterial::Slow_SetUniform( const char* name, const void* data )
{
	FxShader* shader = m_shader;
	mxASSERT(shader->localCBs.Num()==1);
	const FxCBuffer& rCB = shader->localCBs[0];
	const FxUniform* pUniform = FindByName( rCB.uniforms, name );
	if( !pUniform ) {
		return ERR_OBJECT_NOT_FOUND;
	}
	void* pDestination = m_uniforms.ToPtr();
	pDestination = mxAddByteOffset( pDestination, pUniform->offset );
	memcpy( pDestination, data, pUniform->size );
	return ALL_OK;
}

ERet rxMaterial::Load( AStreamReader& stream, Clump* clump )
{
	MaterialHeader_d	header;
	mxDO(stream.Get(header));

	AssetID	shaderID;
	mxDO(ReadAssetID(stream, &shaderID));

	FxShader* shader = GetAsset< FxShader >( shaderID, clump );
	chkRET_X_IF_NIL(shader,ERR_OBJECT_NOT_FOUND);

	m_shader = shader;

	m_textures.SetNum(header.numResources);
	for( UINT32 iSR = 0; iSR < header.numResources; iSR++ )
	{
		AssetID	resourceID;
		mxDO(ReadAssetID( stream, &resourceID ));
		m_textures[iSR].texture = GetAsset< rxTexture >( resourceID, clump );

		SamplerID samplerID;
		mxDO(stream.Get(samplerID));
		m_textures[iSR].sampler = samplerID;
	}

	m_uniforms.SetNum(header.numUniforms);
	if( header.numUniforms ) {
		mxDO(stream.Read( m_uniforms.ToPtr(), m_uniforms.GetDataSize() ));
	}

	m_sort = header.materialHash;

	return ALL_OK;
}

ERet rxMaterial::Load( Assets::LoadContext2 & context )
{
	Clump* clump = context.clump;

	rxMaterial* material = static_cast< rxMaterial* >( context.o );

	material->Load(context, clump);

	return ALL_OK;
}
ERet rxMaterial::Online( Assets::LoadContext2 & context )
{

	return ALL_OK;
}
void rxMaterial::Offline( Assets::LoadContext2 & context )
{

}
void rxMaterial::Destruct( Assets::LoadContext2 & context )
{

}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
