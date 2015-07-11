/*
=============================================================================
	File:	Lexer.h
	Desc:	A simple lexicographical text parser (aka scanner, tokenizer).
=============================================================================
*/
#pragma once

#include <Core/Text/Token.h>

/*
-----------------------------------------------------------------------------
	LexerOptions
-----------------------------------------------------------------------------
*/
struct LexerOptions
{
	enum Flags
	{
		// allow the dollar sign '$' in identifiers?
		DollarsInIdentifiers
	};
	
	TBits< Flags, UINT32 >	m_flags;
	
public:
	LexerOptions();
};

struct LexerState
{
	const char *readPtr;
	int	line;
	int	column;

public:
	LexerState();
	void Clear();
};

/*
-----------------------------------------------------------------------------
	The Lexer class provides the mechanics of lexing tokens
	out of a source buffer and deciding what they mean.
-----------------------------------------------------------------------------
*/
class Lexer : public ATokenReader
{
public:
	Lexer();
	Lexer( const void* source, int length, const char* file = NULL );
	~Lexer();

	// filename is used for line number info/diagnostics
	void SetFileName( const char* fileName );

	virtual bool ReadToken( Token &newToken ) override;
	virtual bool PeekToken( Token &nextToken ) override;
	virtual bool EndOfFile() const override;
	virtual Location GetLocation() const override;

	bool ReadLine( Token & token );
	bool SkipRestOfLine();

	// parse the rest of the line
	bool ReadRestOfLine( String &text );

	// parse a braced section into a string
	bool ParseBracedSection( String &text, bool skipBraces = true );
	bool ParseBracedSection( ATokenWriter &destination );

	void SaveState( LexerState &state ) const;
	void RestoreState( const LexerState& state );

	int GetCurrentLine() const;
	int GetCurrentColumn() const;
	const char* GetFileName() const;

	// get location as delta between current and previous states
	void GetLocation( Location & out ) const;

	const char* CurrentPtr() const;

public_internal:
	void SetLineNumber( int newLineNum );

private:
	void Initialize();
	void Shutdown();

private:
	bool ReadNextTokenInternal( Token &token );
	bool ExpectNextCharInternal( char c );

	bool SkipWhiteSpaces();

	// unsafe internal functions
	char CurrChar() const;	// CurrentChar()
	char PeekChar() const;
	char PrevChar() const;	// PreviousChar()
	char ReadChar();	// eats one char and returns the next char
	void SkipChar();

	bool ReadName( Token &token );
	bool ReadString( Token &token );
	bool ReadNumber( Token &token );
	bool ReadInteger( Token &token );
	bool ReadFloat( Token &token );
	bool ReadHexadecimal( Token &token );
	bool ReadOctalInteger( Token &token );
	bool ReadBinaryInteger( Token &token );

	bool ParsePunctuation( Token &token );

	void IncrementLineCounter();

private:
	const char* m_source;	// pointer to the buffer containing the script
	int		m_length;	// size of the whole source file

	// current lexer state
	const char*	m_curr;		// current position in the source file
	int		m_currentLine;
	int		m_currentColumn;

	NameID		m_fileName;

	LexerOptions	m_options;
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
