/*
=============================================================================
=============================================================================
*/
#include <Base/Base.h>
#include <TxTSupport/TxTConfig.h>
#include <TxTSupport/TxTReader.h>

namespace SON
{

TextConfig::TextConfig()
{
	m_root = nil;
}
TextConfig::~TextConfig()
{
	//
}
ERet TextConfig::LoadFromFile( const char* path )
{
	FileReader	stream;
	mxTRY(stream.Open( path, FileRead_NoErrors ));

	const size_t fileSize = stream.GetSize();
	mxDO(m_text.SetNum( fileSize ));

	char* buffer = (char*) m_text.ToPtr();
	mxDO(stream.Read( buffer, fileSize ));

	SON::Parser		parser;
	parser.buffer = buffer;
	parser.length = fileSize;
	parser.file = path;

	m_root = SON::ParseBuffer(parser, m_nodes);
	chkRET_X_IF_NIL(m_root, ERR_FAILED_TO_PARSE_DATA);
	chkRET_X_IF_NOT(parser.errorCode == 0, ERR_FAILED_TO_PARSE_DATA);

	return ALL_OK;
}
const char* TextConfig::FindString( const char* _key ) const
{
	const SON::Node* valueNode = SON::FindValue( m_root, _key );
	return valueNode ? SON::AsString( valueNode ) : nil;
}
bool TextConfig::FindInteger( const char* _key, int &_value ) const
{
	const SON::Node* valueNode = SON::FindValue( m_root, _key );
	if( valueNode ) {
		_value = SON::AsDouble(valueNode);
		return true;
	}
	return false;
}
bool TextConfig::FindSingle( const char* _key, float &_value ) const
{
	const SON::Node* valueNode = SON::FindValue( m_root, _key );
	if( valueNode ) {
		_value = SON::AsDouble(valueNode);
		return true;
	}
	return false;
}
bool TextConfig::FindBoolean( const char* _key, bool &_value ) const
{
	const SON::Node* valueNode = SON::FindValue( m_root, _key );
	if( valueNode ) {
		_value = SON::AsBoolean(valueNode);
		return true;
	}
	return false;
}

}//namespace SON

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
