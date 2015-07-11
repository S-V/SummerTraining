/*
=============================================================================
	File:	Preprocessor.cpp
	Desc:
	ToDo:	the merging operator: ##
			and stringizing operator: #
=============================================================================
*/
#include <Core/Core_PCH.h>
#pragma hdrstop
#include <Core/Text/Lexer.h>
#include <Core/Text/TextWriter.h>
#include <Core/Text/Preprocessor.h>

#define PP_ENABLE_DEBUG	0

#if PP_ENABLE_DEBUG
	#define ppDEBUG( X )	X
#else
	#define ppDEBUG( X )
#endif


enum { MAX_TOKENS_ON_LINE = 64 };
typedef TStaticList< Token, MAX_TOKENS_ON_LINE > PPTokenList;

static void Dump_PPTokenList( const Token* tokens, int numTokens )
{
	ptPRINT("PPTokenList:\n");
	for( int iToken = 0; iToken < numTokens; iToken++ )
	{
		const Token& token = tokens[ iToken ];
		ptPRINT("%s ", token.text.c_str());
	}
	ptPRINT("\n");
}
template< class TOKEN_LIST >
static void Dump_PPTokenList( const TOKEN_LIST& tokens )
{
	Dump_PPTokenList( tokens.ToPtr(), tokens.Num() );
}

template< class STRING >
static void PPTokenList_To_String( const Token* tokens, int numTokens, STRING &outString )
{
	mxASSERT(outString.IsEmpty());
	for( int iToken = 0; iToken < numTokens; iToken++ )
	{
		const Token& token = tokens[ iToken ];
		Str::Append( outString, token.text );
	}
}

template< class TOKEN_LIST >
static bool ReadExprOnCurrLine( Lexer& lexer, TOKEN_LIST & output )
{
	const int lastLineNum = lexer.GetCurrentLine();

	Token	token;
	while( lexer.PeekToken( token ) )
	{
		if( lexer.GetCurrentLine() > lastLineNum )
		{
			break;
		}
		if( output.Num() >= MAX_TOKENS_ON_LINE )
		{
			lexer.Error("line too long");
			return false;
		}
		SkipToken( lexer );

		if( token == '#' )
		{
			lexer.Error("invalid token");
			return false;
		}

		output.Add( token );
	}

	return output.NonEmpty();
}

// Copied from 'Lexer::SkipRestOfLine'
static bool ExpectEmptyRestOfLine( Lexer& lexer )
{
	const int lastLineNum = lexer.GetCurrentLine();

	int numReadTokens = 0;

	Token	token;
	while( lexer.PeekToken( token ) )
	{
		if( lexer.GetCurrentLine() > lastLineNum )
		{
			break;
		}
		SkipToken( lexer );
		numReadTokens++;
	}

	chkRET_FALSE_IF_NOT(numReadTokens == 0);

	return true;
}

/*
-----------------------------------------------------------------------------
	SimpleFileInclude
-----------------------------------------------------------------------------
*/
SimpleFileInclude::SimpleFileInclude( const char* pathToSources )
{
	Str::CopyS(m_pathToSources, pathToSources);
	Str::StripFileName(m_pathToSources);
	Str::NormalizePath(m_pathToSources);
}

SimpleFileInclude::~SimpleFileInclude()
{
	for( int i = 0; i < m_openedFiles.Num(); i++ )
	{
		Util_DeleteString( m_openedFiles[ i ] );
	}
	m_openedFiles.Empty();
}

bool SimpleFileInclude::OpenFile( const char* fileName, char **fileData, UINT32 *fileSize )
{
	if(mxSUCCEDED(Util_LoadFileToString( fileName, fileData, fileSize )))
	{
		m_openedFiles.Add( *fileData );
		return true;
	}
	Str::Copy( m_lastFilePath, m_pathToSources );
	Str::AppendS( m_lastFilePath, fileName );
	if(mxSUCCEDED(Util_LoadFileToString( m_lastFilePath.ToPtr(), fileData, fileSize )))
	{
		m_openedFiles.Add( *fileData );
		return true;
	}
	m_lastFilePath.Empty();
	return false;
}

void SimpleFileInclude::CloseFile( char* fileData )
{
	const int fileIndex = m_openedFiles.FindIndexOf(fileData);
	chkRET_IF_NOT(fileIndex != INDEX_NONE);

	Util_DeleteString( fileData );
	m_openedFiles.RemoveAt_Fast( fileIndex );
}

const char* SimpleFileInclude::CurrentFilePath() const
{
	return m_lastFilePath.ToPtr();
}

/*
-----------------------------------------------------------------------------
	MultiFileInclude
-----------------------------------------------------------------------------
*/
MultiFileInclude::MultiFileInclude()
{
}

MultiFileInclude::MultiFileInclude( const StringListT& sourcePaths )
{
	m_searchPaths.SetNum(sourcePaths.Num());
	for( int i = 0; i < sourcePaths.Num(); i++ )
	{
		Str::Copy(m_searchPaths[i], sourcePaths[i]);
	}
}

MultiFileInclude::~MultiFileInclude()
{
	for( int i = 0; i < m_openedFiles.Num(); i++ )
	{
		Util_DeleteString( m_openedFiles[ i ] );
	}
	m_openedFiles.Empty();
}

void MultiFileInclude::AddSearchPath( const char* path )
{
	String128	tmp;
	Str::CopyS(tmp, path);
	Str::StripFileName(tmp);
	Str::NormalizePath(tmp);

	m_searchPaths.AddUnique( tmp );
}

bool MultiFileInclude::OpenFile( const char* fileName, char **fileData, UINT32 *fileSize )
{
	if(mxSUCCEDED(Util_LoadFileToString( fileName, fileData, fileSize )))
	{
		m_openedFiles.Add( *fileData );
		return true;
	}
	for( int i = 0; i < m_searchPaths.Num(); i++ )
	{
		const String& sourcePath = m_searchPaths[i];

		Str::Copy( m_lastFilePath, sourcePath );
		Str::AppendS( m_lastFilePath, fileName );

		if(mxSUCCEDED(Util_LoadFileToString( m_lastFilePath.ToPtr(), fileData, fileSize )))
		{
			m_openedFiles.Add( *fileData );
			this->AddSearchPath( m_lastFilePath.ToPtr() );
			return true;
		}
	}
	m_lastFilePath.Empty();
	return false;
}

void MultiFileInclude::CloseFile( char* fileData )
{
	const int fileIndex = m_openedFiles.FindIndexOf(fileData);
	chkRET_IF_NOT(fileIndex != INDEX_NONE);
	Util_DeleteString( fileData );
	m_openedFiles.RemoveAt_Fast( fileIndex );
}

const char* MultiFileInclude::CurrentFilePath() const
{
	return m_lastFilePath.ToPtr();
}

/*
-----------------------------------------------------------------------------
	Preprocessor
-----------------------------------------------------------------------------
*/
Preprocessor::Preprocessor()
{
}

Preprocessor::~Preprocessor()
{
}

void Preprocessor::SetOptions( const PPOptions& options )
{
	m_options = options;
}

ERet Preprocessor::PreprocessFile( const char* fileName, AFileInclude* fileInclude, TokenStream &output )
{
	chkRET_X_IF_NIL( fileName, ERR_NULL_POINTER_PASSED );

	char*	fileData = NULL;
	UINT32	fileSize = 0;

	const char*	filePath = NULL;

	if(mxFAILED(Util_LoadFileToString(fileName, &fileData, &fileSize)))
	{
		if( fileInclude && fileInclude->OpenFile(fileName, &fileData, &fileSize) )
		{
			filePath = fileInclude->CurrentFilePath();
		}
		else
		{
			return ERR_FAILED_TO_OPEN_FILE;
		}
	}

	ppDEBUG(ptPRINT("! Opening file: \"%s\"\n", fileName));

	Lexer	lexer( fileData, fileSize );
	lexer.SetFileName( filePath ? filePath : fileName );
	lexer.SetLineNumber( 0 );

	mxDO(PreprocessText( lexer, fileInclude, output ));

	if( filePath )
	{
		fileInclude->CloseFile(fileData);
	}
	else
	{
		Util_DeleteString(fileData);
	}

	return ALL_OK;
}

ERet Preprocessor::PreprocessText( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output )
{
	m_files.Empty();
	output.Empty();

	chkRET_X_IF_NOT(PreprocessFileRecursive( lexer, fileInclude, output ), ERR_FAILED_TO_PARSE_DATA);

	return ALL_OK;
}

bool Preprocessor::DefineConstant( const String& defineName, const String& defineValue )
{
	chkRET_FALSE_IF_NOT( defineName.NonEmpty() );
	chkRET_FALSE_IF_NOT( defineValue.NonEmpty() );

	Token	nameToken;
	nameToken.type = TT_Name;
	Str::Copy( nameToken.text, defineName );

	chkRET_FALSE_IF_NOT( !m_defines.Contains(nameToken) );

	PPMacro & newMacro = m_defines.Add( nameToken ).value;
	Token & valueToken = newMacro.tokens.Add();

	Lexer	lexer( defineValue.ToPtr(), defineValue.Length() );
	chkRET_FALSE_IF_NOT(lexer.ReadToken(valueToken));

	return true;
}

void Preprocessor::RemoveAllDefines()
{
	m_defines.Empty();
}

int Preprocessor::NumDefines() const
{
	return m_defines.NumEntries();
}

void Preprocessor::GatherDefines( String & outDefines ) const
{
	THashMap< Token, PPMacro >::ConstIterator	iter( m_defines );
	while( iter )
	{
		const Token& name = iter.Key();
		const PPMacro& macro = iter.Value();

		TLocalString< 256 >	valueString;
		PPTokenList_To_String( macro.tokens.ToPtr(), macro.tokens.Num(), valueString );

		TLocalString< 256 >	macroString;
		Str::SPrintF( macroString, "#define %s  %s\n", name.text.c_str(), valueString.ToPtr() );

		Str::Append( outDefines, macroString );

		++iter;
	}
}

bool Preprocessor::PreprocessFileRecursive( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output )
{
	if( m_options.flags & PPOptions::InsertLine )
	{
		Token& newToken = output.m_tokens.Add();
		lexer.GetLocation( newToken.location );
		newToken.type = TT_Directive;
		Str::SPrintF( newToken.text, "#line %u \"%s\"\n", lexer.GetCurrentLine(), lexer.GetFileName() );
	}

	String256	fileName;
	Str::CopyS(fileName, lexer.GetFileName());
	Str::StripPath(fileName);

	PPFileContext& fileContext = m_files.Add();
	Str::Copy( fileContext.name, fileName );

	Token	token;
	while( lexer.ReadToken( token ) )
	{
		//DBGOUT("Preprocessor::ParseSourceFile(): read token: '%s' (%u)\n", token.text.c_str(), token.text.Length());
		if( token == '#' ) {
			chkRET_FALSE_IF_NOT(Process_Directive(lexer, fileInclude, output));
			continue;
		}
		if( !CheckIfSkipping() ) {
			chkRET_FALSE_IF_NOT(TryExpandDefine(token.location, token, output));
		}	
	}

	if( fileContext.indents.NonEmpty() )
	{
		lexer.Error("expected #endif, but reached end of file");
		return false;
	}

	ppDEBUG(ptPRINT("! Finished parsing: \"%s\"\n", fileName));

	m_files.PopLast();

	if(PP_ENABLE_DEBUG)
	{
		ptPRINT("======= Current defines: =======\n");
		String	definesString;
		this->GatherDefines( definesString );
		ptPRINT("%s\n", definesString.ToPtr());
		ptPRINT("======= [END] =======\n\n");
	}

	return true;
}

bool Preprocessor::Process_Directive( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output )
{
	Token	token;
	chkRET_FALSE_IF_NOT(lexer.ReadToken( token ));

	if( token == "include" )
	{
		chkRET_FALSE_IF_NOT(Process_Directive_Include(lexer, fileInclude, output));
	}
	else if( token == "define" )
	{
		chkRET_FALSE_IF_NOT(Process_Directive_Define(lexer, fileInclude, output));
	}
	else if( token == "undef" )
	{
		chkRET_FALSE_IF_NOT(Process_Directive_Undef(lexer, fileInclude, output));
	}
	else if( token == "ifdef" )
	{
		chkRET_FALSE_IF_NOT(Process_Directive_Ifdef(lexer, fileInclude, output));
	}
	else if( token == "ifndef" )
	{
		chkRET_FALSE_IF_NOT(Process_Directive_Ifndef(lexer, fileInclude, output));
	}
	else if( token == "if" )
	{
		chkRET_FALSE_IF_NOT(Process_Directive_If(lexer, fileInclude, output));
	}
	else if( token == "else" )
	{
		chkRET_FALSE_IF_NOT(Process_Directive_Else(lexer, fileInclude, output));
	}
	else if( token == "elif" )
	{
		chkRET_FALSE_IF_NOT(Process_Directive_Elif(lexer, fileInclude, output));
	}
	else if( token == "endif" )
	{
		chkRET_FALSE_IF_NOT(Process_Directive_Endif(lexer, fileInclude, output));
	}
	else if( token == "error" )
	{
		chkRET_FALSE_IF_NOT(Process_Directive_Error(lexer, fileInclude, output));
	}
	else
	{
		lexer.Error("invalid preprocessor directive: '%s'", token.text.c_str());
	}
	return true;
}

bool Preprocessor::Process_Directive_Include( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output )
{
	if( CheckIfSkipping() )
	{
		lexer.SkipRestOfLine();
		return true;
	}

	const int prevLineNum = lexer.GetCurrentLine();

	Token	fileNameToken;
	chkRET_FALSE_IF_NOT(ExpectString( lexer, fileNameToken ));

	if( prevLineNum != lexer.GetCurrentLine() )
	{
		lexer.Error("#include without file name");
		return false;
	}

	const char* includedFileName = fileNameToken.text.c_str();

	ppDEBUG(lexer.Debug("#include \"%s\"", includedFileName));

	for( int iFile = 0; iFile < m_files.Num(); iFile++ ) {
		const PPFileContext& file = m_files[iFile];
		if( Str::EqualS( file.name, includedFileName ) ) {
			ptERROR("recursive #include : '%s'", includedFileName);
			return false;
		}
	}

	// Open include
	if( fileInclude != NULL )
	{
		char* includedFileData = NULL;
		UINT32 includedFileSize = 0;
		chkRET_FALSE_IF_NOT(fileInclude->OpenFile( includedFileName, &includedFileData, &includedFileSize ));

		Lexer	newLexer( includedFileData, includedFileSize );
		newLexer.SetFileName( fileInclude->CurrentFilePath() );
		newLexer.SetLineNumber( 0 );
		chkRET_FALSE_IF_NOT(this->PreprocessFileRecursive(newLexer, fileInclude, output));

		fileInclude->CloseFile( includedFileData );

		return true;
	}
	else
	{
		lexer.Error("failed to #include \"%s\"", includedFileName);
		return false;
	}
}

bool Preprocessor::Process_Directive_Define( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output )
{
	if( CheckIfSkipping() )
	{
		lexer.SkipRestOfLine();
		return true;
	}

	const int lastLineNum = lexer.GetCurrentLine();

	Token	defineName;
	chkRET_FALSE_IF_NOT(ExpectIdentifier( lexer, defineName ));

	if( lastLineNum != lexer.GetCurrentLine() )
	{
		lexer.Error("#define without name");
		return false;
	}

	// check if the define already exists
	if( m_defines.Find( defineName ) )
	{
		lexer.Error("'%s' has already been defined", defineName.text.c_str());
		return false;
	}

	PPMacro &	newMacro = m_defines.Add( defineName ).value;

	const char* textAfterDefineName = lexer.CurrentPtr();

	Token	nextToken;
	chkRET_FALSE_IF_NOT(lexer.PeekToken( nextToken ));

	const char* textBeforeNextToken = lexer.CurrentPtr();
	const bool bHasWhitespaceBeforeNextToken = textBeforeNextToken > textAfterDefineName;

	if( lastLineNum == nextToken.location.line )
	{
		// next token is on the same line as the macro

		if( nextToken == '(' && !bHasWhitespaceBeforeNextToken )
		{
			lexer.Error("#defines with macro parameters are not supported");
			return false;
		}

		chkRET_FALSE_IF_NOT(ReadExprOnCurrLine(lexer, newMacro.tokens));

		if( PP_ENABLE_DEBUG )
		{
			String defineValue;
			PPTokenList_To_String( newMacro.tokens.ToPtr(), newMacro.tokens.Num(), defineValue );
			lexer.Debug("#define %s %s", defineName.text.c_str(), defineValue.ToPtr());
		}
	}

	if( PP_ENABLE_DEBUG )
	{
		lexer.Debug("#define %s", defineName.text.c_str());
	}

	return true;
}

bool Preprocessor::Process_Directive_Undef( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output )
{
	if( CheckIfSkipping() )
	{
		lexer.SkipRestOfLine();
		return true;
	}

	const int lastLineNum = lexer.GetCurrentLine();

	Token	defineName;
	chkRET_FALSE_IF_NOT(ExpectIdentifier( lexer, defineName ));

	if( lastLineNum != lexer.GetCurrentLine() )
	{
		lexer.Error("#undef without name");
		return false;
	}

	if( m_defines.Contains( defineName ) )
	{
		lexer.Debug("#undef '%s'", defineName.text.c_str());
		m_defines.Remove( defineName );
	}
	else
	{
		lexer.Warning("'%s' is undefined, #undef has no effect", defineName.text.c_str());
	}

	return true;
}

bool Preprocessor::Process_Directive_Ifdef( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output )
{
	const int lastLineNum = lexer.GetCurrentLine();

	Token	defineName;
	chkRET_FALSE_IF_NOT(ExpectIdentifier( lexer, defineName ));

	if( lastLineNum != lexer.GetCurrentLine() )
	{
		lexer.Error("#ifdef without name");
		return false;
	}

	chkRET_FALSE_IF_NOT(ExpectEmptyRestOfLine(lexer));

	const bool bSkip = !m_defines.Contains( defineName );
	ppDEBUG(lexer.Debug("#ifdef %s (skip: %d)", defineName.ToPtr(), bSkip));
	PushIndent( PPIndent::IFDEF, bSkip );

	return true;
}

bool Preprocessor::Process_Directive_Ifndef( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output )
{
	const int lastLineNum = lexer.GetCurrentLine();

	Token	defineName;
	chkRET_FALSE_IF_NOT(ExpectIdentifier( lexer, defineName ));

	if( lastLineNum != lexer.GetCurrentLine() )
	{
		lexer.Error("#ifndef without name");
		return false;
	}

	chkRET_FALSE_IF_NOT(ExpectEmptyRestOfLine(lexer));

	const bool bSkip = m_defines.Contains( defineName );
	ppDEBUG(lexer.Debug("#ifndef %s (skip: %d)", defineName.ToPtr(), bSkip));
	PushIndent( PPIndent::IFNDEF, bSkip );

	return true;
}

bool Preprocessor::Process_Directive_If( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output )
{
	PPValue	value;
	chkRET_FALSE_IF_NOT(EvaluateTokens(lexer, &value));

	const bool bSkip = (value == 0);
	ppDEBUG(lexer.Debug("#if %d (skip: %d)", value, bSkip));
	PushIndent( PPIndent::IF, bSkip );

	return true;
}

bool Preprocessor::Process_Directive_Else( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output )
{
	PPIndent::EType type;
	bool skip = false;

	if( !PopIndent( &type, &skip ) ) {
		lexer.Error( "misplaced #else" );
		return false;
	}
	if( type == PPIndent::ELSE ) {
		lexer.Error( "#else after #else" );
		return false;
	}

	chkRET_FALSE_IF_NOT(ExpectEmptyRestOfLine(lexer));

	PushIndent( PPIndent::ELSE, !skip );

	return true;
}

bool Preprocessor::Process_Directive_Elif( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output )
{
	PPIndent::EType type;
	bool skip = false;
	if( !PopIndent( &type, &skip ) ) {
		lexer.Error( "misplaced #elif" );
		return false;
	}
	if( type == PPIndent::ELSE ) {
		lexer.Error( "misplaced #elif" );
		return false;
	}

	if( skip )
	{
		PushIndent( PPIndent::ELIF, true );
	}
	else
	{
		PPValue	value;
		chkRET_FALSE_IF_NOT(EvaluateTokens(lexer, &value));

		skip = (value == 0);
		ppDEBUG(lexer.Debug("#elif %d (skip: %d)", value, skip));
		PushIndent( PPIndent::ELIF, skip );
	}

	return true;
}

bool Preprocessor::Process_Directive_Endif( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output )
{
	PPIndent::EType	type;
	if( !PopIndent( &type ) ) {
		lexer.Error( "misplaced #endif" );
		return false;
	}

	ppDEBUG(lexer.Debug("#endif // (files: %u, depth: %u)", GetFilesNestingLevel(), GetDefinesNestingLevel()));

	chkRET_FALSE_IF_NOT(ExpectEmptyRestOfLine(lexer));

	return true;
}

bool Preprocessor::Process_Directive_Error( Lexer& lexer, AFileInclude* fileInclude, TokenStream &output )
{
	if( CheckIfSkipping() )
	{
		lexer.SkipRestOfLine();
		return true;
	}

	String	errorMessage;
	chkRET_FALSE_IF_NOT(lexer.ReadRestOfLine( errorMessage ));

	lexer.Error("#%s", errorMessage.ToPtr());

	return false;
}

void Preprocessor::PushIndent( PPIndent::EType type, bool skip )
{
	mxASSERT(m_files.NonEmpty());
	PPFileContext & file = m_files.GetLast();
	PPIndent & newIndent = file.indents.Add();
	newIndent.type = type;
	newIndent.skip = skip;
}

bool Preprocessor::PopIndent( PPIndent::EType *type, bool *skip )
{
	mxASSERT(m_files.NonEmpty());
	PPFileContext & file = m_files.GetLast();
	if( file.indents.NonEmpty() )
	{
		PPIndent & lastIndent = file.indents.GetLast();
		if( type != NULL ) {
			*type = lastIndent.type;
		}
		if( skip != NULL ) {
			*skip = lastIndent.skip;
		}
		file.indents.PopLast();
		return true;
	}
	return false;
}

bool Preprocessor::EvaluateTokens( Lexer& lexer, PPValue *value ) const
{
	PPTokenList		tokens;
	chkRET_FALSE_IF_NOT(ReadExprOnCurrLine(lexer, tokens));
	ppDEBUG(Dump_PPTokenList(tokens));

	TokenListReader	tokenReader( tokens.ToPtr(), tokens.Num() );
	chkRET_FALSE_IF_NOT(Evaluate_Expression(tokenReader, value));

	return true;
}

bool Preprocessor::Evaluate_Expression( ATokenReader& lexer, PPValue *value ) const
{
	return Evaluate_LogicalOr(lexer, value);
}

// E || E
bool Preprocessor::Evaluate_LogicalOr( ATokenReader& lexer, PPValue *value ) const
{
	PPValue L, R;

	chkRET_NIL_IF_NOT(Evaluate_LogicalAnd( lexer, &L ));

	while( PeekTokenString( lexer, "||" ) )
	{
		SkipToken( lexer );
		chkRET_NIL_IF_NOT(Evaluate_LogicalAnd( lexer, &R ));
		L = (L || R);
	}

	*value = L;
	return true;
}

// E && E
bool Preprocessor::Evaluate_LogicalAnd( ATokenReader& lexer, PPValue *value ) const
{
	PPValue L, R;

	chkRET_NIL_IF_NOT(Evaluate_BitOr( lexer, &L ));

	while( PeekTokenString( lexer, "&&" ) )
	{
		SkipToken( lexer );
		chkRET_NIL_IF_NOT(Evaluate_BitOr( lexer, &R ));
		L = (L && R);
	}

	*value = L;
	return true;
}

// E | E ( E or E )
bool Preprocessor::Evaluate_BitOr( ATokenReader& lexer, PPValue *value ) const
{
	PPValue L, R;

	chkRET_NIL_IF_NOT(Evaluate_BitXor( lexer, &L ));

	Token	nextToken;
	lexer.PeekToken( nextToken );

	while( nextToken == '|' )
	{
		SkipToken( lexer );
		chkRET_NIL_IF_NOT(Evaluate_BitXor( lexer, &R ));
		L = (L | R);
		lexer.PeekToken( nextToken );
	}

	*value = L;
	return true;
}

// E ^ E ( E xor E )
bool Preprocessor::Evaluate_BitXor( ATokenReader& lexer, PPValue *value ) const
{
	PPValue L, R;

	chkRET_NIL_IF_NOT(Evaluate_BitAnd( lexer, &L ));

	Token	nextToken;
	lexer.PeekToken( nextToken );

	while( nextToken == '^' )
	{
		SkipToken( lexer );
		chkRET_NIL_IF_NOT(Evaluate_BitAnd( lexer, &R ));
		L = (L ^ R);
		lexer.PeekToken( nextToken );
	}

	*value = L;
	return true;
}

// E & E ( E and E )
bool Preprocessor::Evaluate_BitAnd( ATokenReader& lexer, PPValue *value ) const
{
	PPValue L, R;

	chkRET_NIL_IF_NOT(Evaluate_Equality( lexer, &L ));

	Token	nextToken;
	lexer.PeekToken( nextToken );

	while( nextToken == '&' )
	{
		SkipToken( lexer );
		chkRET_NIL_IF_NOT(Evaluate_Equality( lexer, &R ));
		L = (L & R);
		lexer.PeekToken( nextToken );
	}

	*value = L;
	return true;
}

// ==, != .
bool Preprocessor::Evaluate_Equality( ATokenReader& lexer, PPValue *value ) const
{
	PPValue L, R, E;

	chkRET_NIL_IF_NOT(Evaluate_Relational( lexer, &L ));

	mxLOOP_FOREVER
	{
		Token	nextToken;
		lexer.PeekToken( nextToken );

		// E -> E == E
		if ( nextToken == "==" ) {
			SkipToken( lexer );
			chkRET_NIL_IF_NOT(Evaluate_Relational( lexer, &R ));
			E = (L == R);
			L = E;
			continue;
		}
		// E -> E != E
		if ( nextToken == "!=" ) {
			SkipToken( lexer );
			chkRET_NIL_IF_NOT(Evaluate_Relational( lexer, &R ));
			E = (L != R);
			L = E;
			continue;
		}
		break;		 
	}

	*value = L;
	return true;
}

// >, >=, <, <= .
bool Preprocessor::Evaluate_Relational( ATokenReader& lexer, PPValue *value ) const
{
	PPValue L, R;

	chkRET_NIL_IF_NOT(Evaluate_Shifts( lexer, &L ));

	mxLOOP_FOREVER
	{
		Token	nextToken;
		lexer.PeekToken( nextToken );

		// E -> E < E
		if ( nextToken == '<' ) {
			SkipToken( lexer );
			chkRET_NIL_IF_NOT(Evaluate_Shifts( lexer, &R ));
			L = (L < R);
			continue;
		}
		// E -> E <= E
		if ( nextToken == "<=" ) {
			SkipToken( lexer );
			chkRET_NIL_IF_NOT(Evaluate_Shifts( lexer, &R ));
			L = (L <= R);
			continue;
		}
		// E -> E > E
		if ( nextToken == '>' ) {
			SkipToken( lexer );
			chkRET_NIL_IF_NOT(Evaluate_Shifts( lexer, &R ));
			L = (L > R);
			continue;
		}
		// E -> E >= E
		if ( nextToken == ">=" ) {
			SkipToken( lexer );
			chkRET_NIL_IF_NOT(Evaluate_Shifts( lexer, &R ));
			L = (L >= R);
			continue;
		}
		break;		 
	}

	*value = L;
	return true;
}

// <<, >>, >>> .
bool Preprocessor::Evaluate_Shifts( ATokenReader& lexer, PPValue *value ) const
{
	PPValue L, R;

	chkRET_NIL_IF_NOT(Evaluate_Terms( lexer, &L ));

	mxLOOP_FOREVER
	{
		Token	nextToken;
		lexer.PeekToken( nextToken );

		// E -> E << E
		if ( nextToken == "<<" ) {
			SkipToken( lexer );
			chkRET_NIL_IF_NOT(Evaluate_Terms( lexer, &R ));
			L = (L << R);
			continue;
		}
		// E -> E >> E
		if ( nextToken == ">>" ) {
			SkipToken( lexer );
			chkRET_NIL_IF_NOT(Evaluate_Terms( lexer, &R ));
			L = (L >> R);
			continue;
		}
		// E -> E >>> E
		if ( nextToken == ">>>" ) {
			SkipToken( lexer );
			chkRET_NIL_IF_NOT(Evaluate_Terms( lexer, &R ));
			L = ((unsigned long long)L >> (unsigned long long)R);
			continue;
		}
		break;		 
	}

	*value = L;
	return true;
}

// +, - .
bool Preprocessor::Evaluate_Terms( ATokenReader& lexer, PPValue *value ) const
{
	PPValue L, R;

	chkRET_NIL_IF_NOT(Evaluate_Factors( lexer, &L ));

	mxLOOP_FOREVER
	{
		Token	nextToken;
		lexer.PeekToken( nextToken );

		// E -> E + E
		if ( nextToken == '+' ) {
			SkipToken( lexer );
			chkRET_NIL_IF_NOT(Evaluate_Factors( lexer, &R ));
			L = L + R;
			continue;
		}
		// E -> E - E
		if ( nextToken == '-' ) {
			SkipToken( lexer );
			chkRET_NIL_IF_NOT(Evaluate_Factors( lexer, &R ));
			L = L - R;
			continue;
		}
		break;		 
	}

	*value = L;
	return true;
}

// *, /, % .
bool Preprocessor::Evaluate_Factors( ATokenReader& lexer, PPValue *value ) const
{
	PPValue L, R;

	chkRET_NIL_IF_NOT(Evaluate_Atom( lexer, &L ));

	mxLOOP_FOREVER
	{
		Token	nextToken;
		lexer.PeekToken( nextToken );

		// E -> E * E
		if ( nextToken == '*' ) {
			SkipToken( lexer );
			chkRET_NIL_IF_NOT(Evaluate_Atom( lexer, &R ));
			L = L * R;
			continue;
		}
		// E -> E / E
		if ( nextToken == '/' ) {
			SkipToken( lexer );
			chkRET_NIL_IF_NOT(Evaluate_Atom( lexer, &R ));
			L = L / R;
			continue;
		}
		// E -> E % E
		if ( nextToken == '%' ) {
			SkipToken( lexer );
			chkRET_NIL_IF_NOT(Evaluate_Atom( lexer, &R ));
			L = L % R;
			continue;
		}
		break;		 
	}

	*value = L;
	return true;
}

bool TokenIsPPConstant( const Token& token )
{
	const TokenTypeT kind = token.type;
	return kind == TT_Number
		|| kind == TT_String
		|| kind == TT_Literal
		;
}

bool GetTokenValue( const Token& token, PPValue *value )
{
	const TokenTypeT kind = token.type;
	if( kind == TT_Number )
	{
		const long longIntValue = ::atol( token.text.c_str() );
		*value = longIntValue;
		return true;
	}
	if( kind == TT_Literal )
	{
		*value = token.text.c_str()[0];
		return true;
	}
	if( kind == TT_String )
	{
		*value = 1;
		return true;
	}
	return false;
}

bool Preprocessor::Evaluate_Atom( ATokenReader& lexer, PPValue *value ) const
{
	Token	token;
	chkRET_NIL_IF_NOT(lexer.ReadToken( token ));

	ppDEBUG(lexer.Debug("Evaluate_Atom: %s", token.text.c_str()));

	if( TokenIsPPConstant( token ) )
	{
		chkRET_FALSE_IF_NOT(GetTokenValue( token, value ));
		return true;
	}

	if( token.type == TT_Name )
	{
		const PPMacro* existingMacro = m_defines.Find( token );
		if( existingMacro != NULL )
		{
			const TokenList & tokens = existingMacro->tokens;
			chkRET_FALSE_IF_NOT( tokens.NonEmpty() );

			TokenListReader	tokenReader( tokens.ToPtr(), tokens.Num() );
			chkRET_FALSE_IF_NOT(Evaluate_Expression(tokenReader, value));
			return true;
		}
		else
		{
			lexer.Error("undefined token: '%s'", token.text.c_str());
			return false;
		}
	}

	// E -> ( E )
	if( token == '(' )
	{
		chkRET_NIL_IF_NOT(Evaluate_Expression( lexer, value ));
		chkRET_NIL_IF_NOT(ExpectChar( lexer, ')'));
		return true;
	}

	lexer.Error("invalid token: '%s'", token.text.c_str());
	return false;
}

bool Preprocessor::TryExpandDefine( const Location& origLoc, const Token& token, TokenStream &output )
{
	// if the token is a name
	if( token.type == TT_Name )
	{
		// check if the name is a define macro
		const PPMacro* define = m_defines.Find( token );
		// if it is a define macro
		if( define != NULL )
		{
			// expand the defined macro
			for( int iToken = 0; iToken < define->tokens.Num(); iToken++ )
			{
				const Token& param = define->tokens[ iToken ];
				TryExpandDefine( origLoc, param, output );
			}
			return true;
		}
	}

	// not a define
	output.m_tokens.Add( token );

	//DBGOUT("Preprocessor::TryExpandDefine(): Added token: '%s' (%u)\n", token.text.c_str(), token.text.Length());

	// so that the inserted tokens refer to the original source file location
	output.m_tokens.GetLast().location = origLoc;

	return true;
}

bool Preprocessor::CheckIfSkipping() const
{
	for( int iFile = 0; iFile < m_files.Num(); iFile++ )
	{
		const PPFileContext& file = m_files[iFile];
		for( int i = 0; i < file.indents.Num(); i++ )
		{
			if( file.indents[i].skip ) {
				return true;
			}
		}
	}
	return false;
}

int Preprocessor::GetFilesNestingLevel() const
{
	return m_files.Num();
}

int Preprocessor::GetDefinesNestingLevel() const
{
	int depth = 0;
	for( int iFile = 0; iFile < m_files.Num(); iFile++ ) {
		depth += m_files[iFile].indents.Num();
	}
	return depth;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
