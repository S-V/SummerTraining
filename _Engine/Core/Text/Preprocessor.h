/*
=============================================================================
	File:	Preprocessor.h
	Desc:	
=============================================================================
*/
#pragma once

#include <Core/Text/Token.h>
#include <Core/Text/Lexer.h>

/*
===============================================================================

	C/C++ compatible pre-compiler

===============================================================================
*/

struct AFileInclude
{
	virtual bool OpenFile( const char* fileName, char **fileData, UINT32 *fileSize ) = 0;
	virtual void CloseFile( char* fileData ) = 0;

	// should return the full path to the currently opened file (for debugging)
	virtual const char* CurrentFilePath() const { return ""; }

protected:
	virtual ~AFileInclude() {}
};

/*
-----------------------------------------------------------------------------
	SimpleFileInclude
-----------------------------------------------------------------------------
*/
class SimpleFileInclude : public AFileInclude
{
	String256			m_pathToSources;
	TArray< ANSICHAR* >	m_openedFiles;
	String256			m_lastFilePath;

public:
	SimpleFileInclude( const char* pathToSources );
	~SimpleFileInclude();

	virtual bool OpenFile( const char* fileName, char **fileData, UINT32 *fileSize ) override;
	virtual void CloseFile( char* fileData ) override;

	virtual const char* CurrentFilePath() const override;
};

/*
-----------------------------------------------------------------------------
	MultiFileInclude
-----------------------------------------------------------------------------
*/
class MultiFileInclude : public AFileInclude
{
	TArray< String128 >		m_searchPaths;
	TArray< ANSICHAR* >		m_openedFiles;
	String256				m_lastFilePath;

public:
	MultiFileInclude();
	MultiFileInclude( const StringListT& sourcePaths );
	~MultiFileInclude();

	void AddSearchPath( const char* path );

	virtual bool OpenFile( const char* fileName, char **fileData, UINT32 *fileSize ) override;
	virtual void CloseFile( char* fileData ) override;

	virtual const char* CurrentFilePath() const override;
};

struct PPDefine
{
	Token	name;
	Token	value;
};

struct PPMacro
{
	TokenList	tokens;	// tokens on the right hand side of #define name
};

// indents used for conditional compilation directives:
// #if, #else, #elif, #ifdef, #ifndef
struct PPIndent
{
	enum EType
	{
		IF,
		ELSE,
		ELIF,
		IFDEF,
		IFNDEF,
	};

	//Token	name;
	EType	type;
	bool	skip;	// true if skipping source because of conditional compilation
};

// pushed onto stack for each included source file
struct PPFileContext
{
	String				name;		// name of file
	//String				path;		// path for opening
	//String				text;		// file contents
	TArray< PPIndent >	indents;	// indent stack
};

// preprocessor options
struct PPOptions
{
	enum Flags {
		InsertLine = BIT(0),	// insert #line directives
		None = 0
	};

	TBits< Flags, UINT32 >	flags;

public:
	PPOptions()
	{
		flags = None;
	}
};

// for evaluating expressions
typedef signed long int PPValue;

/*
-----------------------------------------------------------------------------
	Preprocessor
-----------------------------------------------------------------------------
*/
class Preprocessor
{
public:
	Preprocessor();
	~Preprocessor();

	void SetOptions( const PPOptions& options );

	ERet PreprocessFile( const char* fileName, AFileInclude* fileInclude, TokenStream &output );
	ERet PreprocessText( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output );

	bool DefineConstant( const String& defineName, const String& defineValue );

	void RemoveAllDefines();

	int NumDefines() const;

	// for debugging
	void GatherDefines( String & outDefines ) const;

private:
	bool PreprocessFileRecursive( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output );

	bool Process_Directive( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output );

	// #include "filename.ext"
	bool Process_Directive_Include( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output );
	bool Process_Directive_Define( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output );
	bool Process_Directive_Undef( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output );
	bool Process_Directive_Ifdef( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output );
	bool Process_Directive_Ifndef( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output );
	bool Process_Directive_If( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output );
	bool Process_Directive_Else( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output );
	bool Process_Directive_Elif( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output );
	bool Process_Directive_Endif( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output );
	bool Process_Directive_Error( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output );
	bool Process_Pragma_Once( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output );

	void PushIndent( PPIndent::EType type, bool skip );
	bool PopIndent( PPIndent::EType *type = NULL, bool *skip = NULL );

	bool EvaluateTokens( Lexer& lexer, PPValue *value ) const;

	bool Evaluate_Expression( ATokenReader& lexer, PPValue *value ) const;
	// E || E
	bool Evaluate_LogicalOr( ATokenReader& lexer, PPValue *value ) const;
	// E && E
	bool Evaluate_LogicalAnd( ATokenReader& lexer, PPValue *value ) const;
	// E | E ( E or E )
	bool Evaluate_BitOr( ATokenReader& lexer, PPValue *value ) const;
	// E ^ E ( E xor E )
	bool Evaluate_BitXor( ATokenReader& lexer, PPValue *value ) const;
	// E & E ( E and E )
	bool Evaluate_BitAnd( ATokenReader& lexer, PPValue *value ) const;
	// ==, != .
	bool Evaluate_Equality( ATokenReader& lexer, PPValue *value ) const;
	// >, >=, <, <= .
	bool Evaluate_Relational( ATokenReader& lexer, PPValue *value ) const;
	// <<, >>, >>> .
	bool Evaluate_Shifts( ATokenReader& lexer, PPValue *value ) const;
	// +, - .
	bool Evaluate_Terms( ATokenReader& lexer, PPValue *value ) const;
	// *, /, % .
	bool Evaluate_Factors( ATokenReader& lexer, PPValue *value ) const;
	bool Evaluate_Atom( ATokenReader& lexer, PPValue *value ) const;

	bool TryExpandDefine( const Location& origLoc, const Token& token, TokenStream &output );

private:
	// returns true if skipping source because of conditional compilation
	bool CheckIfSkipping() const;

	// calculates how many files have been opened (for debugging)
	int GetFilesNestingLevel() const;
	// calculates how many #directives have been entered (for debugging)
	int GetDefinesNestingLevel() const;

private:
	TArray< PPFileContext >		m_files;	// stack of files that have been #included
	THashMap< Token, PPMacro >	m_defines;	// macros that have been #defined
	PPOptions					m_options;	//
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
