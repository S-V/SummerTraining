/*
=============================================================================
	File:	Token.cpp
	Desc:
=============================================================================
*/
#include <Core/Core_PCH.h>
#pragma hdrstop

// for strtoul()
#include <cstdlib>

#include <Base/Util/Color.h>
#include <Core/Text/Token.h>
#include <Core/Text/TextWriter.h>

mxBEGIN_REFLECT_ENUM( TokenTypeT )
	mxREFLECT_ENUM_ITEM( Bad, ETokenType::TT_Bad ),
	mxREFLECT_ENUM_ITEM( Name, ETokenType::TT_Name ),
	mxREFLECT_ENUM_ITEM( Number, ETokenType::TT_Number ),
	mxREFLECT_ENUM_ITEM( String, ETokenType::TT_String ),
	mxREFLECT_ENUM_ITEM( Literal, ETokenType::TT_Literal ),
	mxREFLECT_ENUM_ITEM( Punctuation, ETokenType::TT_Punctuation ),
	mxREFLECT_ENUM_ITEM( Directive, ETokenType::TT_Directive ),
mxEND_REFLECT_ENUM

#if 0
const char* ETokenKind_To_Chars( TokenTypeT _type )
{
	switch( _type )
	{
	case TC_Bad:	return "Bad";
	case TT_Number:	return "integer";
	case TC_FloatNumber:	return "float";
	case TC_Char:	return "char";
	case TT_String:	return "string";
	case TT_Name:	return "identifier";
	case TC_Punctuation:	return "punctuation";
	case TT_Directive:	return "directive";
		mxNO_SWITCH_DEFAULT;
	}
}
#endif

/*
-----------------------------------------------------------------------------
	Location
-----------------------------------------------------------------------------
*/

/*
-----------------------------------------------------------------------------
	Token
-----------------------------------------------------------------------------
*/
Token::Token()
{
	type = TT_Bad;
	flags.m_value = 0;
}

Token::Token( const Token& other )
{
	*this = other;
}

Token::~Token()
{
}

Token& Token::operator = ( const Token &other )
{
	text		= other.text;
	type		= other.type;
	//m_loc		= other.m_loc;
	//m_numberType= other.m_numberType;
	return *this;
}
//
//TokenTypeT Token::GetKind() const
//{
//	return m_kind;
//}
//
//const char* Token::ToPtr() const
//{
//	return text.ToPtr();
//}
//
//const UINT Token::Length() const
//{
//	return this->GetText().Length();
//}
//
////const TokenFlags Token::GetFlags() const
////{
////	return m_flags;
////}
//
//const ENumberType Token::GetNumberType() const
//{
//	return m_numberType;
//}
//
//const String& Token::GetText() const
//{
//	return text;
//}
//
//const Location& Token::GetLocation() const
//{
//	return m_loc;
//}

bool Token::operator == ( const Token& other ) const
{
	return text == other.text
		&& type == other.type
		;
}

bool Token::operator != ( const Token& other ) const
{
	return !(*this == other);
}

bool Token::GetIntegerValue( int *_value ) const
{
	mxASSERT_PTR(_value);
	chkRET_FALSE_IF_NOT(type == TT_Number && (flags & TF_Integer) );

	//if( m_numberType == TF_Decimal )
	{
		const long int longIntValue = ::atol( this->text.c_str() );
		*_value = longIntValue;
	}
	//else if( m_numberType == TF_Hex )
	//{
	//	const unsigned long ulongValue = strtoul( this->ToPtr(), NULL, 16 );
	//	*_value = (int)ulongValue;
	//}

	return true;
}

bool Token::GetFloatValue( float *_value ) const
{
	chkRET_FALSE_IF_NIL(_value);
	chkRET_FALSE_IF_NOT(type == TT_Number && (flags & TF_Float) );

	const double floatValue = ::atof( this->text.c_str() );
	*_value = floatValue;

	return true;
}

int Token::GetIntegerValue() const
{
	int result = ::atoi( text.ToPtr() );
	return result;
}

float Token::GetFloatValue() const
{
	double result = ::atof( text.ToPtr() );
	return result;
}

void Lex_PrintF(
				const Location& location,
				const char* format, ...
				)
{
	va_list	args;
	va_start( args, format );
	Lex_PrintV( location, format, args );
	va_end( args );
}

void Lex_PrintV(
				const Location& location,
				const char* format, va_list args,
				const char* additional_info
				)
{
	ptPRINT_VARARGS_BODY(
		format, args,
		//NOTE: formatted for visual studio debug output (click -> go to source location)
		ptPRINT("%s(%d,%d): %s%s\n", location.file.c_str(), location.line, location.column, additional_info ? additional_info : "", ptr_)
		);
}

void Lex_ErrorBeep()
{
	mxBeep(200);
}

///*
//-----------------------------------------------------------------------------
//	TokenStringWriter
//-----------------------------------------------------------------------------
//*/
//TokenStringWriter::TokenStringWriter( String &destination )
//	: m_dest( destination )
//{
//
//}
//
//void TokenStringWriter::AddToken( const Token& newToken )
//{
//	m_dest.Append( newToken.ToPtr(), newToken.Length() );
//}

/*
-----------------------------------------------------------------------------
	TokenStream
-----------------------------------------------------------------------------
*/
static void InsertLineDirective( const Token& token, AStreamWriter & stream )
{
	String256	directive;
	Str::SPrintF(directive, "#line %u \"%s\"\n", token.location.line, token.location.file.c_str());
	//Str::SPrintF(directive, "#line %u\n", loc.Line());	//GLSL doesn't support file names in #line
	stream.Write( directive.ToPtr(), directive.Length() );
}

static void PutToken( const Token& token, AStreamWriter & stream )
{
	const UINT32 tokenLength = token.text.Length();
	if( tokenLength > 0 ) {
		stream.Write( token.text.c_str(), tokenLength );
	}
}

void TokenStream::Dump( AStreamWriter & stream ) const
{
	for( UINT iToken = 0; iToken < m_tokens.Num(); iToken++ )
	{
		const Token& token = m_tokens[ iToken ];

		mxASSERT(token.text.Length() > 0);

		PutToken(token, stream);

		stream.Write(" ",1);
	}
}

void TokenStream::Dump_HumanReadable( AStreamWriter & stream ) const
{
	TextWriter	writer( stream );

	UINT prevLine = 1;
mxTIDY_UP_FILE;
	for( UINT iToken = 0; iToken < m_tokens.Num(); iToken++ )
	{
		const Token& token = m_tokens[ iToken ];
		mxASSERT(token.text.Length() > 0);

		const UINT currLine = token.location.line;
		if( currLine != prevLine )
		{
			writer.NewLine();
			//InsertLineDirective(token, stream);
			prevLine = currLine;
		}

		//char nextTokenChar = '\0';
		//if( iToken < m_tokens.Num()-1 )
		//{
		//	const Token& nextToken = m_tokens[ iToken + 1 ];
		//	nextTokenChar = nextToken.GetText()[0];
		//}

		//writer.Putf("Token: '%s' (%u)\n", token.text.c_str(), token.text.Length());
		PutToken(token, writer.GetStream());

		//const UINT tokenLength = token.text.Length();
		//if( tokenLength > 0 )
		//{
		//	writer.InsertTabs();
		//	stream.Write( token.text.c_str(), tokenLength );
		//}
		//if( token == '{' )
		//{
		//	writer.IncreaseIndentation();
		//}
		//else if( token == '}' )
		//{
		//	writer.DecreaseIndentation();
		//	//writer.NewLine();
		//}
		////if( token.location.Column() + token.text.Length() )
		//else
			writer.Emit(" ",1);

//		if( token.type == TT_Directive )
//		{
//			writer.NewLine();
//		}
//		else if( token == '{' )
//		{
//			writer.NewLine();
//		//	InsertLineDirective(token, stream);
//			writer.IncreaseIndentation();
//			//writer.Putf("");
//		}
//		else if( token == '}' )
//		{
//			writer.DecreaseIndentation();
//			writer.NewLine();
//	//		InsertLineDirective(token, stream);
//			//writer.DecreaseIndentation();
//		}
//		else if( token == ';' )
//		{
//			writer.NewLine();
////			InsertLineDirective(token, stream);
//			//if( nextTokenChar != '{' )
//			//{
//			//	writer.InsertTabs();
//			//}
//		}
//		else
//		{
//			//if( token.type == TC_Punctuation )
//			//{
//			//	Lex_PrintF(token.location, "writing whitespace after '%s'",token.text.c_str());
//			//}
//			writer.Emit(" ",1);
//		}
	}
}

void TokenStream::Empty()
{
	m_tokens.Empty();
}

void TokenStream::AddToken( const Token& newToken )
{
	mxTODO("compare locations of the last tokens? define operator < (SourceLoc a,b) ?");
	m_tokens.Add( newToken );
}

bool TokenStream::DumpToFile( const char* filename, bool bHumanReadable, const char* prefix ) const
{
	FileWriter	file( filename );
	chkRET_FALSE_IF_NOT(file.IsOpen());

	if( prefix != NULL )
	{
		const UINT len = strlen( prefix );
		if( len > 0 ) {
			file.Write( prefix, len );
		}
	}

	if( bHumanReadable ) {
		this->Dump_HumanReadable( file );
	} else {
		this->Dump( file );
	}

	return true;
}

/*
-----------------------------------------------------------------------------
	ATokenReader
-----------------------------------------------------------------------------
*/
void ATokenReader::Debug( const char* format, ... )
{
#if MX_DEBUG
	va_list	args;
	va_start( args, format );
	Lex_PrintV( this->GetLocation(), format, args );
	va_end( args );
#endif // MX_DEBUG
}

void ATokenReader::Message( const char* format, ... )
{
	va_list	args;
	va_start( args, format );
	Lex_PrintV( this->GetLocation(), format, args );
	va_end( args );
}

void ATokenReader::Warning( const char* format, ... )
{
	va_list	args;
	va_start( args, format );
	Lex_PrintV( this->GetLocation(), format, args, "warning: " );
	va_end( args );
	Lex_ErrorBeep();
}

void ATokenReader::Error( const char* format, ... )
{
	va_list	args;
	va_start( args, format );
	Lex_PrintV( this->GetLocation(), format, args, "error: " );
	va_end( args );
	Lex_ErrorBeep();
}

/*
-----------------------------------------------------------------------------
	TokenListReader
-----------------------------------------------------------------------------
*/
TokenListReader::TokenListReader( const TokenStream& source )
	: m_tokenList( source.m_tokens.ToPtr() ), m_numTokens( source.m_tokens.Num() )
{
	mxASSERT(m_numTokens > 0);
	m_currToken = 0;
}

TokenListReader::TokenListReader( const Token* tokens, UINT numTokens )
	: m_tokenList( tokens ), m_numTokens( numTokens )
{
	mxASSERT(numTokens > 0);
	m_currToken = 0;
}

bool TokenListReader::ReadToken( Token &nextToken )
{
	if( m_currToken < m_numTokens )
	{
		nextToken = m_tokenList[ m_currToken++ ];
		//DBGOUT("ReadToken: %s\n", outToken.ToPtr());
		return true;
	}
	return false;
}

bool TokenListReader::PeekToken( Token &outToken )
{
	State	oldState;
	this->SaveState(oldState);
	bool bOk = this->ReadToken(outToken);
	this->RestoreState(oldState);
	return bOk;
}

bool TokenListReader::EndOfFile() const
{
	return (m_currToken >= m_numTokens);
}

Location TokenListReader::GetLocation() const
{
	return this->CurrentToken().location;
}

bool TokenListReader::IsValid() const
{
	return (m_numTokens > 0) && !this->EndOfFile();
}

const Token& TokenListReader::CurrentToken() const
{
	if( EndOfFile() )
	{
		// return the last token
		return m_tokenList[ m_numTokens-1 ];
	}
	mxASSERT(m_currToken < m_numTokens);
	return m_tokenList[ m_currToken ];
}

void TokenListReader::SaveState( State &state ) const
{
	state.currToken = m_currToken;
}

void TokenListReader::RestoreState( const State& state )
{
	m_currToken = state.currToken;
}

void TokenListReader::Error( const char* format, ... )
{
	this->Debug("current token: '%s'", this->CurrentToken().text.c_str());
	char	buffer[4096];
	mxGET_VARARGS_A(buffer, format);
	Super::Error( buffer );
}


bool SkipToken( ATokenReader& _lexer )
{
	Token	dummyToken;
	return _lexer.ReadToken( dummyToken );
}
TokenTypeT PeekTokenType( ATokenReader& _lexer )
{
	Token	nextToken;
	_lexer.PeekToken( nextToken );
	return nextToken.type;
}
bool PeekTokenChar( ATokenReader& _lexer, const char tokenChar )
{
	Token	nextToken;
	chkRET_FALSE_IF_NOT( _lexer.PeekToken( nextToken ) );
	return nextToken == tokenChar;
}
bool PeekTokenString( ATokenReader& _lexer, const char* tokenString )
{
	Token	nextToken;
	RET_FALSE_IF_NOT( _lexer.PeekToken( nextToken ) );
	return Str::EqualS( nextToken.text, tokenString );
}
bool ExpectTokenType( ATokenReader& _lexer, TokenTypeT _type, Token &_token )
{
	if ( !ExpectAnyToken( _lexer, _token ) ) {
		return false;
	}
	if( _token.type != _type )
	{
		LexError(_lexer, "expected '%s' but found '%s'",
			GetTypeOf_TokenTypeT().GetStringByValue(_type), _token.text.c_str() );
		return false;
	}
	return true;
}
bool ExpectAnyToken( ATokenReader& _lexer, Token &_token )
{
	if ( !_lexer.ReadToken( _token ) ) {
		LexError(_lexer, "couldn't read expected token" );
		return false;
	}
	return true;
}
bool ExpectChar( ATokenReader& _lexer, char c )
{
	Token	token;
	if( !_lexer.ReadToken( token ) ) {
		LexError(_lexer, "couldn't read expected '%c' : end of file reached", c);
		return false;
	}

	if( token == c ) {
		return true;
	}

	//LexError(*this, "expected a character constant '%c', but read '%s'", c, token.text.c_str());
	LexError(_lexer, "expected '%c', but read '%s'", c, token.text.c_str());

	return false;
}
#if 0
bool ExpectString( Token &token )
{
	if( !this->ReadToken( token ) )
	{
		LexError(*this, "couldn't read expected string");
		return false;
	}
	if( token.type != TT_String )
	{
		LexError(*this, "expected a string literal, but read '%s'", token.text.c_str());
		return false;
	}
	return true;
}

bool ExpectInt( Token &token )
{
	if( !this->ReadToken( token ) )
	{
		LexError(*this, "couldn't read expected integer number");
		return false;
	}

	if ( token.type == TC_Punctuation && token == "-" ) {
		this->ExpectTokenType( TT_Number, &token );
		return -((signed int) token.GetIntegerValue());
	}

	if( token.type != TT_Number )
	{
		LexError(*this, "expected an integer number, but read '%s'", token.text.c_str());
		return false;
	}
	return true;
}

bool ExpectFloat( Token &token )
{
	if( !this->ReadToken( token ) )
	{
		LexError(*this, "couldn't read expected floating-point number");
		return false;
	}
	if( token.type != TC_FloatNumber )
	{
		LexError(*this, "expected an floating-point number, but read '%s'", token.text.c_str());
		return false;
	}
	return true;
}

bool ExpectConstant( Token &token )
{
	if( !this->ReadToken( token ) )
	{
		LexError(*this, "couldn't read expected constant");
		return false;
	}
	if( !TokenIsConstant( token ) )
	{
		LexError(*this, "expected a constant, but read '%s'", token.text.c_str());
		return false;
	}
	return true;
}
#endif

bool Expect( ATokenReader& _lexer, const char* tokenString )
{
	Token	token;
	if( !_lexer.ReadToken( token ) )
	{
		LexError(_lexer, "couldn't read expected '%s'", tokenString);
		return false;
	}
	if( !Str::Equal( token.text, Chars(tokenString) ) )
	{
		LexError(_lexer, "expected '%s', but read '%s'", tokenString, token.text.c_str());
		return false;
	}
	return true;
}

bool expectBool( ATokenReader& _lexer, bool &value )
{
	Token	token;
	chkRET_FALSE_IF_NOT(ExpectAnyToken(_lexer, token));

	String64	tokenValue( token.text );
	Str::ToUpper( tokenValue );

	if( Str::Equal(tokenValue, Chars("TRUE")) || token == '1' ) {
		value = true;
		return true;
	}
	if( Str::Equal(tokenValue, Chars("FALSE")) || token == '0' ) {
		value = false;
		return true;
	}

	_lexer.Error("expected a boolean value, but got %s", token.text.c_str());
	return false;
}

bool expectFloat( ATokenReader& _lexer, float & value, float _min, float _max )
{
	Token	token;
	return expectFloat( _lexer, token, value, _min, _max );
}

bool expectFloat( ATokenReader& _lexer, Token &_token, float &_value, float _min, float _max )
{
	if ( !ExpectAnyToken( _lexer, _token ) ) {
		return false;
	}
	if ( _token.IsSign() ) {
		if( !ExpectTokenType( _lexer, TT_Number, _token ) ) {
			return false;
		}
		_value = -_token.GetFloatValue();
	}
	else if( _token.type != TT_Number ) {
		_lexer.Error("expected a floating point number, but got '%s'", _token.text.c_str());
		return false;
	}
	else {
		_value = _token.GetFloatValue();
	}
	if( _value < _min || _value > _max )
	{
		_lexer.Error("expected a floating point number in range [%f, %f], but got %f", _min, _max, _value);
		return false;
	}
	return true;
}

float expectFloat( ATokenReader& _lexer, float _min, float _max )
{
	float value = 0;
	if( !expectFloat( _lexer, value, _min, _max ) ) {
		return 0;
	}
	return value;
}

bool expectUInt( ATokenReader& _lexer, UINT & value, UINT _min, UINT _max )
{
	Token	token;
	if( !ExpectAnyToken( _lexer, token ) ) {
		return false;
	}

	int integerValue;
	const bool bOk = token.GetIntegerValue( &integerValue );

	if( !bOk || integerValue < _min || integerValue > _max )
	{
		_lexer.Error("expected an unsigned integer number in range [%u, %u], but got %s", _min, _max, token.text.c_str());
		return false;
	}

	value = (UINT)integerValue;

	return true;
}

bool expectUInt8( ATokenReader& _lexer, UINT8 &value, UINT _min, UINT _max )
{
	UINT temp;
	chkRET_FALSE_IF_NOT(expectUInt(_lexer, temp, _min, _max ));
	value = temp;
	return true;
}

bool expectInt( ATokenReader& _lexer, int & value, int _min, int _max )
{
	Token	token;
	return expectInt( _lexer, token, value, _min, _max );
}

bool expectInt( ATokenReader& _lexer, Token & _token, int & _value, int _min, int _max )
{
	if ( !ExpectAnyToken( _lexer, _token ) ) {
		return false;
	}
	if ( _token.IsSign() ) {
		if( !ExpectTokenType( _lexer, TT_Number, _token ) ) {
			return false;
		}
		_value = -_token.GetIntegerValue();
	}
	else if( _token.type != TT_Number ) {
		_lexer.Error("expected an integer number, but got '%s'", _token.text.c_str());
		return false;
	}
	else {
		_value = _token.GetIntegerValue();
	}
	if( _value < _min || _value > _max )
	{
		_lexer.Error("expected an integer number in range [%i, %i], but got %i", _min, _max, _value);
		return false;
	}
	return true;
}

int expectInt( ATokenReader& _lexer, int _min, int _max )
{
	int value = 0;
	if( !expectInt( _lexer, value, _min, _max ) ) {
		return 0;
	}
	return value;
}

bool expectName( ATokenReader& _lexer, String &tokenString )
{
	Token	token;
	chkRET_FALSE_IF_NOT(ExpectIdentifier( _lexer, token ));

	mxASSERT(token.text.Length() > 0);

	tokenString.Copy( token.text );

	return true;
}

bool expectString( ATokenReader& _lexer, String &stringConstant )
{
	Token	token;
	chkRET_FALSE_IF_NOT(ExpectString( _lexer, token ));

	stringConstant.Copy( token.text );

	return true;
}

bool expectRGBA( ATokenReader& _lexer, float * value )
{
	chkRET_FALSE_IF_NOT(Expect(_lexer, "RGBA"));
	chkRET_FALSE_IF_NOT(ExpectChar(_lexer, '('));

	chkRET_FALSE_IF_NOT(expectFloat( _lexer, value[0], 0.0f, 1.0f ));
	chkRET_FALSE_IF_NOT(expectFloat( _lexer, value[1], 0.0f, 1.0f ));
	chkRET_FALSE_IF_NOT(expectFloat( _lexer, value[2], 0.0f, 1.0f ));
	chkRET_FALSE_IF_NOT(expectFloat( _lexer, value[3], 0.0f, 1.0f ));

	chkRET_FALSE_IF_NOT(ExpectChar(_lexer, ')'));

	return true;
}

bool expectRGBA( ATokenReader& _lexer, RGBAf & value )
{
	return expectRGBA( _lexer, value.ToPtr() );
}

bool readOptionalChar( ATokenReader& _lexer, char tokenChar )
{
	Token	nextToken;
	if( _lexer.PeekToken( nextToken ) )
	{
		if( nextToken == tokenChar )
		{
			SkipToken( _lexer );
		}
		return true;
	}
	// end of file reached
	return false;
}

bool readOptionalSemicolon( ATokenReader& _lexer )
{
	return readOptionalChar( _lexer, ';' );
}

int Parse1DMatrix( ATokenReader& _lexer, int x, float *m ) {
	int i;

	if ( !Expect(_lexer,  "(" ) ) {
		return false;
	}

	for ( i = 0; i < x; i++ ) {
		m[i] = expectFloat( _lexer );
	}

	if ( !Expect(_lexer,  ")" ) ) {
		return false;
	}
	return true;
}

int Parse2DMatrix( ATokenReader& _lexer, int y, int x, float *m ) {
	int i;

	if ( !Expect(_lexer,  "(" ) ) {
		return false;
	}

	for ( i = 0; i < y; i++ ) {
		if ( !Parse1DMatrix( _lexer, x, m + i * x ) ) {
			return false;
		}
	}

	if ( !Expect(_lexer,  ")" ) ) {
		return false;
	}
	return true;
}

int Parse3DMatrix( ATokenReader& _lexer, int z, int y, int x, float *m ) {
	int i;

	if ( !Expect(_lexer,  "(" ) ) {
		return false;
	}

	for ( i = 0 ; i < z; i++ ) {
		if ( !Parse2DMatrix( _lexer, y, x, m + i * x*y ) ) {
			return false;
		}
	}

	if ( !Expect(_lexer,  ")" ) ) {
		return false;
	}
	return true;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
