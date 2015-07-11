/*
=============================================================================
	File:	NetProtocol.h
	Desc:	
=============================================================================
*/
#pragma once

// for AssetID
#include <Core/Asset.h>

struct EdCmd_Base : public CStruct
{
	String		debugText;
public:
	mxDECLARE_CLASS( EdCmd_Base, CStruct );
	mxDECLARE_REFLECTION;
};

struct EdCmd_AssetDatabaseInfo : public EdCmd_Base
{
	String		assetDbName;
public:
	mxDECLARE_CLASS( EdCmd_AssetDatabaseInfo, CStruct );
	mxDECLARE_REFLECTION;
};

struct EdCmd_ReloadAsset : public EdCmd_Base
{
	AssetID				assetId;
	String				assetPath;
	TBuffer< BYTE >		assetData;
public:
	mxDECLARE_CLASS( EdCmd_ReloadAsset, EdCmd_Base );
	mxDECLARE_REFLECTION;
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
