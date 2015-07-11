/*
=============================================================================
=============================================================================
*/
#pragma once

#include <Core/Core.h>
#include <TxTSupport/TxTCommon.h>

namespace SON
{

class TextConfig : public AConfigFile
{
	SON::Node *		m_root;
	ByteBuffer32	m_text;
	SON::Allocator	m_nodes;
public:
	TextConfig();
	~TextConfig();
	ERet LoadFromFile( const char* path );
	virtual const char* FindString( const char* _key ) const override;
	virtual bool FindInteger( const char* _key, int &_value ) const override;
	virtual bool FindSingle( const char* _key, float &_value ) const override;
	virtual bool FindBoolean( const char* _key, bool &_value ) const override;
};

}//namespace SON

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
