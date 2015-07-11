#pragma once

#include <Core/Asset.h>
#include <Graphics/Device.h>

struct rxTexture : public CStruct
{
	HTexture		m_texture;	// handle of hardware texture
	HResource		m_resource;	// shader resource handle
	UINT16			pad;
public:
	mxDECLARE_CLASS( rxTexture, CStruct );
	mxDECLARE_REFLECTION;
	rxTexture();

public:
	static AssetTypeT GetAssetType() { return AssetTypes::TEXTURE; }

	static ERet Loader( Assets::LoadContext2 & context );
	static ERet Online( Assets::LoadContext2 & context );
	static void Offline( Assets::LoadContext2 & context );
	static void Unloader( Assets::LoadContext2 & context );
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
