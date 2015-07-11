/*
=============================================================================
	File:	Token.h
	Desc:	
=============================================================================
*/
#pragma once

#include <Core/Text/NameTable.h>

class RGBAf;

enum ETokenType
{
	TT_Bad,			// unknown token or EoF
	TT_Name,		// arbitrary identifier
	TT_Number,		// any number (int, float)
	TT_String,		// "string" constant
	TT_Literal,		// literal 'character' constant
	TT_Punctuation,	// ...
	TT_Directive,	// e.g.: #line, #pragma
	//TT_Keyword,	// reserved word
};
mxDECLARE_ENUM( ETokenType, UINT32, TokenTypeT );


enum ETokenFlags
{
	// number sub types
	TF_Integer	= BIT(0),	// integer
	TF_Float	= BIT(1),	// floating point number

	// integer number flags
	TF_Long		= BIT(2),	// long int
	TF_Unsigned	= BIT(3),	// unsigned int
	TF_Decimal	= BIT(4),	// decimal number
	TF_Binary	= BIT(5),	// binary number
	TF_Octal	= BIT(6),	// octal number
	TF_Hex		= BIT(7),	// hexadecimal number

	// floating-point number flags
	TF_Double_Precision	 = BIT(8)	// double
};
mxDECLARE_FLAGS( ETokenFlags, UINT32, TokenFlags );

/*
-----------------------------------------------------------------------------
	Location contains file-line number info
	which is used for tracking position in a source file.
-----------------------------------------------------------------------------
*/
struct Location
{
	NameID	file;
	int		line : 24;
	int		column : 8;
public:
	Location()
	{
		line = 1;
		column = 1;
	}
};

/*
-----------------------------------------------------------------------------
	Token

	The Token class is used to represent a single lexed token,
	it is a single lexeme from a source file.

	Tokens are intended to be used by the lexer/preprocess and parser libraries,
	but are not intended to live beyond them
	(for example, they should not live in the ASTs).

// NOTE: Token can be used in retail builds for parsing INI files
// and we'd better avoid strings with dynamic memory allocation
-----------------------------------------------------------------------------
*/
struct Token
{
	String32		text;	// raw text of this token
	TokenTypeT		type;	// the actual flavor of token this is ( TT_* )
	TokenFlags		flags;
	Location	location;	// position of this token in a source file

public:
	Token();
	Token( const Token& other );
	~Token();

	Token& operator = ( const Token &other );

	bool operator == ( const Token& other ) const;
	bool operator != ( const Token& other ) const;

	template< UINT N >
	bool operator == ( const char (&s)[N] ) const {
		return strcmp( text.c_str(), s ) == 0;
	}
	template< UINT N >
	bool operator != ( const char (&s)[N] ) const {
		return !(*this == s);
	}

	bool operator == ( const char c ) const {
		return text.c_str()[0] == c && text.c_str()[1] == '\0';
	}
	bool operator != ( const char c ) const {
		return !(*this == c);
	}

public:
	bool GetIntegerValue( int *_value ) const;
	bool GetFloatValue( float *_value ) const;

	int GetIntegerValue() const;
	float GetFloatValue() const;

	bool IsSign() const { return type == TT_Punctuation && Str::EqualS(text,"-"); }
};

template<>
struct THashTrait< Token >
{
	static mxFORCEINLINE UINT GetHashCode( const Token& token )
	{
		return FNV32_StringHash( token.text.c_str() );
	}
};
template<>
struct TEqualsTrait< Token >
{
	static mxFORCEINLINE bool Equals( const Token& a, const Token& b )
	{
		return (a == b);
	}
};

typedef TArray< Token > TokenList;

template< UINT BUFFER_SIZE >
inline
void FormatTextMessageV( char (&buffer)[BUFFER_SIZE], const Location& location, const char* format, va_list args )
{
	char temp[ 4096 ];
	const int count = vsnprintf_s( temp, mxCOUNT_OF(temp), mxCOUNT_OF(temp) - 1, format, args );
	mxUNUSED(count);

	//NOTE: formatted for visual studio debug output (click -> go to source location)
	sprintf(
		buffer, mxCOUNT_OF(buffer),
		"%s(%d,%d): %s\n",
		location.file.c_str(), location.line, location.Column(), temp
		);
}


void Lex_PrintF(
				const Location& location,
				const char* format, ...
				);
void Lex_PrintV(
				const Location& location,
				const char* format, va_list args,
				const char* additional_info = NULL
				);

// emit sound signal
void Lex_ErrorBeep();


#define LexError( LEXER, ... )		(LEXER).Error( __VA_ARGS__ )
#define LexWarning( LEXER, ... )	(LEXER).Warning( __VA_ARGS__ )


/*
-----------------------------------------------------------------------------
	ATokenWriter
-----------------------------------------------------------------------------
*/
struct ATokenWriter
{
	virtual void AddToken( const Token& newToken ) = 0;

protected:
	virtual ~ATokenWriter() {}
};

/*
-----------------------------------------------------------------------------
	TokenStream
-----------------------------------------------------------------------------
*/
struct TokenStream : public ATokenWriter
{
	TArray< Token >	m_tokens;

public:
	void Empty();

	//=-- ATokenWriter
	virtual void AddToken( const Token& newToken ) override;

	bool DumpToFile( const char* filename, bool bHumanReadable = false, const char* prefix = NULL ) const;
	void Dump( AStreamWriter & stream ) const;
	void Dump_HumanReadable( AStreamWriter & stream ) const;

	//void IncrementLineNumbers( UINT incByAmount );
};

/*
-----------------------------------------------------------------------------
	ATokenReader
-----------------------------------------------------------------------------
*/
struct ATokenReader
{
	virtual bool ReadToken( Token &newToken ) = 0;
	virtual bool PeekToken( Token &nextToken ) = 0;
	virtual bool EndOfFile() const = 0;
	virtual Location GetLocation() const = 0;

	virtual void Debug( const char* format, ... );
	virtual void Message( const char* format, ... );
	virtual void Warning( const char* format, ... );
	virtual void Error( const char* format, ... );

	// for debugging
	virtual bool IsValid() const { return true; }

protected:
	virtual ~ATokenReader() {}
};

/*
-----------------------------------------------------------------------------
	TokenListReader
-----------------------------------------------------------------------------
*/
class TokenListReader : public ATokenReader
{
	const Token *	m_tokenList;
	const UINT		m_numTokens;
	UINT			m_currToken;

public:
	typedef ATokenReader Super;

	TokenListReader( const TokenStream& source );
	TokenListReader( const Token* tokens, UINT numTokens );

	virtual bool ReadToken( Token &nextToken ) override;
	virtual bool PeekToken( Token &outToken ) override;
	virtual bool EndOfFile() const override;
	virtual Location GetLocation() const override;

	virtual bool IsValid() const override;

	const Token& CurrentToken() const;

	struct State
	{
		UINT currToken;
	};
	void SaveState( State & state ) const;
	void RestoreState( const State& state );

	virtual void Error( const char* format, ... ) override;
};

bool SkipToken( ATokenReader& _lexer );
TokenTypeT PeekTokenType( ATokenReader& _lexer );
bool PeekTokenChar( ATokenReader& _lexer, const char tokenChar );
bool PeekTokenString( ATokenReader& _lexer, const char* tokenString );
bool ExpectTokenType( ATokenReader& _lexer, TokenTypeT _type, Token &_token );

bool ExpectAnyToken( ATokenReader& _lexer, Token &_token );
bool ExpectChar( ATokenReader& _lexer, char c ) ;
inline bool ExpectString( ATokenReader& _lexer, Token &token ) {
	return ExpectTokenType( _lexer, TT_String, token );
}
inline bool ExpectIdentifier( ATokenReader& _lexer, Token &token ) {
	return ExpectTokenType( _lexer, TT_Name, token );
}

// expect a certain token, reads the token when available
bool Expect( ATokenReader& _lexer, const char* tokenString );

bool expectBool( ATokenReader& _lexer, bool &value );

bool expectUInt8( ATokenReader& _lexer, UINT8 &value, UINT _min = 0, UINT _max = MAX_UINT8 );

bool expectUInt( ATokenReader& _lexer, UINT & value, UINT _min = 0, UINT _max = MAX_UINT32 );

bool expectInt( ATokenReader& _lexer, Token & _token, int & value, int _min = MIN_INT32, int _max = MAX_INT32 );
bool expectInt( ATokenReader& _lexer, int & _value, int _min = MIN_INT32, int _max = MAX_INT32 );
int expectInt( ATokenReader& _lexer, int _min = MIN_INT32, int _max = MAX_INT32 );

bool expectFloat( ATokenReader& _lexer, Token &token, float & value, float _min = -1e6f, float _max = +1e6f );
bool expectFloat( ATokenReader& _lexer, float &value, float _min = -1e6f, float _max = +1e6f );
float expectFloat( ATokenReader& _lexer, float _min = -1e6f, float _max = +1e6f );

//expectIdentifier
bool expectName( ATokenReader& _lexer, String &tokenString );

bool expectString( ATokenReader& _lexer, String &stringConstant );

bool expectRGBA( ATokenReader& _lexer, float * value );
bool expectRGBA( ATokenReader& _lexer, RGBAf & value );

bool readOptionalChar( ATokenReader& _lexer, char tokenChar );
bool readOptionalSemicolon( ATokenReader& _lexer );

// parse matrices with floats
int Parse1DMatrix( ATokenReader& _lexer, int x, float *m );
int Parse2DMatrix( ATokenReader& _lexer, int y, int x, float *m );
int Parse3DMatrix( ATokenReader& _lexer, int z, int y, int x, float *m );

//== MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
//---------------------------------------------------------------------------
//
#define shREAD_OPTIONAL( NAME, VALUE )\
		if( _token == (NAME) ) {\
			_lexer.SkipToken();\
			chkRET_FALSE_IF_NOT(expectName( _lexer, (VALUE) ));\
			continue;\
		}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
