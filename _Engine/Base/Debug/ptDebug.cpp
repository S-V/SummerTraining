/*
=============================================================================
	File:	Debug.cpp
	Desc:	Code for debug utils, assertions and other very useful things.
	ToDo:	print stack trace
=============================================================================
*/

#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>


namespace Debug
{
	static EAssertBehaviour StandardUserAssertCallback( const char* expr, const char* file, const char* func, unsigned int line, const char* message )
	{
		return AB_Break;
	}


	template< int SIZE >
	int tMy_snprintfcat( char (&buf)[SIZE], char const* fmt, ... )
	{
		int result;
		va_list args;
		int len = strnlen( buf, SIZE);

		va_start( args, fmt);
		result = My_vsnprintf( buf + len, SIZE - len, fmt, args);
		va_end( args);

		return result + len;
	}

	static EAssertBehaviour DefaultUserAssertCallback( const char* expr, const char* file, const char* func, unsigned int line, const char* message )
	{
#if (mxPLATFORM == mxPLATFORM_WINDOWS)

		char  buffer[ 2048 ] = { 0 };
		int	  length = 0;

		length = tMy_snprintfcat(buffer, "%s(%d): Assertion failed in '%s', '%s'", file, line, expr, func);
		if( message != NULL ) {
			length = tMy_snprintfcat(buffer, ", '%s'.\n", message);
		}

		mxGetLog().VWrite( LL_Info, buffer, length );

		if( ::IsDebuggerPresent() )
		{
			sprintf_s( buffer, "Assertion failed:\n\n '%s'\n\n in file %s, function '%s', line %d, %s.\n",
				expr, file, func, line, message ? message : "no additional info" );

			strcat( buffer, "\nDo you wish to debug?\nYes - 'Debug Break', No - 'Exit', Cancel or Close - 'Don't bother me again!'\n" );
			const int result = ::MessageBoxA( NULL, buffer, ("Assertion failed"), MB_YESNOCANCEL | MB_ICONERROR );
			if( IDYES == result ) {
				return AB_Break;
			}
			elif( IDCANCEL == result ) {
				return AB_Ignore;
			}
		}

		return AB_FatalError;
#else

	#error Unsupported platform!

#endif

	}

	static UserAssertCallback userAssertCallback = &DefaultUserAssertCallback;

}//Debug

UserAssertCallback SetUserAssertCallback( UserAssertCallback cb )
{
	UserAssertCallback prevCallback = 
		(Debug::userAssertCallback == Debug::DefaultUserAssertCallback)
	?
		NULL
	:
		Debug::userAssertCallback
	;

	Debug::userAssertCallback = cb ? cb : Debug::DefaultUserAssertCallback;

	return prevCallback;
}

#if MX_DEBUG
	void mxVARARGS DBGOUT( const char* format, ... )
	{
		va_list	args;
		va_start( args, format );
		mxGetLog().PrintV( LL_Debug, format, args );
		va_end( args );
	}
#endif // MX_DEBUG

/*
================================
		ZZ_OnAssertionFailed
================================
*/
bool ZZ_OnAssertionFailed(
	const char* expression,
	const char* filename, const char* function, int line,
	bool* bIgnoreAlways,
	const char* message
	)
{
	const EAssertBehaviour eAction = (*Debug::userAssertCallback)( expression, filename, function, line, message );
	if( eAction == AB_Break )
	{
		return true;
	}
	if( eAction == AB_Ignore )
	{
		if( bIgnoreAlways != NULL ) {
			*bIgnoreAlways = true;
		}
		return false;
	}
	if( eAction == AB_Continue )
	{
		return false;
	}
	if( eAction == AB_FatalError )
	{
		ptERROR( "Assertion failed" );
		return true;
	}
	return false;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
