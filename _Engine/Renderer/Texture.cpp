/*
=============================================================================
	File:	Texture.cpp
	Desc:	
	References:

	Knowing which mipmap levels are needed
	http://home.comcast.net/~tom_forsyth/blog.wiki.html
=============================================================================
*/
#include "Renderer/Renderer_PCH.h"
#pragma hdrstop
#include <Core/ObjectModel.h>
#include <Renderer/Texture.h>

mxDEFINE_CLASS( rxTexture );
mxBEGIN_REFLECTION( rxTexture )
	//mxMEMBER_FIELD( pad ),
mxEND_REFLECTION

rxTexture::rxTexture()
{
	m_texture.SetNil();
	m_resource.SetNil();
}

ERet rxTexture::Loader( Assets::LoadContext2 & context )
{
	//Clump* clump = context.clump;
	//rxTexture* texture = static_cast< rxTexture* >( context.o );
	//todo: read header
	return ALL_OK;
}
ERet rxTexture::Online( Assets::LoadContext2 & context )
{
	rxTexture* texture = static_cast< rxTexture* >( context.o );

	const UINT32 textureSize = context.GetSize();

	ScopedStackAlloc	tempAlloc( gCore.frameAlloc );
	void* textureData = tempAlloc.AllocA( textureSize );
	mxDO(context.Read( textureData, textureSize ));

	texture->m_texture = llgl::CreateTexture( textureData, textureSize );
	texture->m_resource = llgl::AsResource( texture->m_texture );

	return ALL_OK;
}
void rxTexture::Offline( Assets::LoadContext2 & context )
{
	rxTexture* texture = static_cast< rxTexture* >( context.o );

	if( texture->m_texture.IsValid() ) {
		llgl::DeleteTexture(texture->m_texture);
		texture->m_texture.SetNil();
	}
	texture->m_resource.SetNil();
}
void rxTexture::Unloader( Assets::LoadContext2 & context )
{
	Clump* clump = context.clump;
	rxTexture* texture = static_cast< rxTexture* >( context.o );
	clump->Free( texture );
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
