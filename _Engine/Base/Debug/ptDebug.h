/*
=============================================================================
	File:	Debug.h
	Desc:	Code for debug utils, assertions and other very useful things.
	Note:	Some debug utils depend heavily on the platform.
	References:
	Assertions in Game Development:
	http://www.itshouldjustworktm.com/?p=155
=============================================================================
*/
#pragma once

/*
=============================================================================
	
		Run-time checks and assertions.

=============================================================================
*/

//
//	DBGOUT - The same as Print(), but works in debug mode only.
//
#if MX_DEBUG
	extern void mxVARARGS DBGOUT( const char* format, ... );
#else
	inline void mxVARARGS DBGOUT( const char* format, ... ) {}
#endif

//
//	EAssertBehaviour - used to specify actions taken on assert failures.
//
enum EAssertBehaviour
{
	AB_Break = 0,		// trigger a breakpoint
	AB_Ignore = 1,		// continue and don't report this assert from now on
	AB_Continue = 2,	// continue execution, simply return from the assertion failure
	AB_FatalError = 3,	// trigger a breakpoint
};

// user assertion failure handler should contain user logic (wrapped in your favourite GUI) and writing to log;
// NOTE: 'message' is optional, can be null
typedef EAssertBehaviour (*UserAssertCallback)( const char* expr, const char* file, const char* func, unsigned int line, const char* message );

//! Set a user-defined assertion-failure callback.
/** This function registers a user-defined routine to call in the event of an
* assertion failure.
* \param cb A callback routine to call that is of type UserAssertCallback.
* \return Returns the previous callback, if one was registered.
* \note The user-defined callback should return 0 to have the program simply return
* from the assertion failure and not process it any further,
* nonzero to continue with assertion processing for the host OS.
*/
extern UserAssertCallback SetUserAssertCallback( UserAssertCallback cb );


// internal function;
// returns true if the assertion should trigger a breakpoint
//
extern bool ZZ_OnAssertionFailed(
	const char* expression,
	const char* filename, const char* function, int line,
	bool* bIgnoreAlways = NULL,
	const char* message = NULL
	);


/*
=============================================================================
	mxASSERT asserts that the statement x is true, and continue processing.

	If the expression is true, continue processing.

	If the expression is false, trigger a debug breakpoint.

	These asserts are only present in DEBUG builds.


	Examples of usage:
		mxASSERT( a > b );
		mxASSERT2( x == y, "X must be equal to Y!" );

	NOTE: Asserts should not have any side effects.

	In debug mode the compiler will generate code for asserts.
	When using MVC++, asserts trigger a break point interrupt inside a macro.
	This way the program brakes on the line of the assert.
=============================================================================
*/

#if MX_DEBUG

	#define mxASSERT( expr )\
		mxMACRO_BEGIN\
			static bool bIgnoreAlways = 0;\
			if(!bIgnoreAlways)\
				if(!(expr))\
					{ptDBG_BREAK;\
						if(ZZ_OnAssertionFailed( #expr, __FILE__, __FUNCTION__, __LINE__, &bIgnoreAlways ))\
							ptBREAK;\
					}\
		mxMACRO_END

	#define mxASSERT2( expr, message )\
		mxMACRO_BEGIN\
			static bool bIgnoreAlways = 0;\
			if(!bIgnoreAlways)\
				if(!(expr))\
					{ptDBG_BREAK;\
						if(ZZ_OnAssertionFailed( #expr, __FILE__, __FUNCTION__, __LINE__, &bIgnoreAlways, message ))\
							ptBREAK;\
					}\
		mxMACRO_END

#else // ! MX_DEBUG

	#define mxASSERT( expr )			mxNOOP;(void)(expr)
	#define mxASSERT2( expr, message )	mxNOOP;(void)(expr)

#endif // ! MX_DEBUG


#define mxASSERT_PTR( ptr )			mxASSERT( mxIsValidHeapPointer((ptr)) )

/*
=============================================================================
	CHK(x) is always evaluated, returns the (boolean) value of the passed (logical) expression.

	e.g.:
		bool	b;

		b = CHK(5>3);
		b = CHK(5>1);
		b = CHK(5>1) && CHK(4<1);	// <- will break on this line (4<1)
		b = CHK(5>9);				// <- and here

	Real-life example:
		bool Mesh::isOk() const
		{
			return CHK(mesh != null)
				&& CHK(numBatches > 0)
				&& CHK(batches != null)
				;
		}
=============================================================================
*/
#if MX_DEBUG
	#define CHK( expr )		( (expr) ? true : (ZZ_OnAssertionFailed(#expr,__FILE__,__FUNCTION__,__LINE__),ptDBG_BREAK,false) )
#else
	#define CHK( expr )		(expr)
#endif // ! MX_DEBUG

//
// FullAsserts are never removed in release builds, they slow down a lot.
//
#define AlwaysAssert( expr )		\
	mxMACRO_BEGIN					\
	static bool bIgnoreAlways = 0;	\
	if(!bIgnoreAlways)				\
		if(!(expr))					\
			if(ZZ_OnAssertionFailed( #expr, __FILE__, __FUNCTION__, __LINE__, &bIgnoreAlways ))ptDBG_BREAK;\
	mxMACRO_END


#define AlwaysAssertX( expr, message )	\
	mxMACRO_BEGIN						\
	static bool bIgnoreAlways = 0;		\
	if(!bIgnoreAlways)					\
		if(!(expr))						\
			if(ZZ_OnAssertionFailed( #expr, __FILE__, __FUNCTION__, __LINE__, &bIgnoreAlways, message ))ptBREAK;\
	mxMACRO_END



/*
another version:
#define AlwaysAssert( expr )	\
	do { if(!(expr)) ZZ_OnAssertionFailed( #expr, __FILE__, __FUNCTION__, __LINE__ ); } while(0)

#define AlwaysAssertX( expr, message )	\
	do { if(!(expr)) ZZ_OnAssertionFailed( message, #expr, __FILE__, __FUNCTION__, __LINE__ ); } while(0)
*/

/* old code:
#define AlwaysAssert( expr )	\
	(void)( (expr) ? 1 : (ZZ_OnAssertionFailed( #expr, __FILE__, __FUNCTION__, __LINE__ ) ))

#define AlwaysAssertX( expr, message )	\
	(void)( (expr) ? 1 : (ZZ_OnAssertionFailed( message, #expr, __FILE__, __FUNCTION__, __LINE__ ) ))
*/

#define mxASSUME( x )		{ mxASSERT((x)); mxOPT_HINT(x); }


/*
The verify(x) macro just returns true or false in release mode, but breaks
in debug mode.  That way the code can take a non-fatal path in release mode
if something that's not supposed to happen happens.

if ( !verify(game) ) {
	// This should never happen!
	return;
}

(this is taken from Quake 4 SDK)
*/

#if MX_DEBUG
	#define mxVERIFY( x )		( (x) ? true :\
		(ZZ_OnAssertionFailed( #x, __FILE__, __FUNCTION__, __LINE__ ) ? ptBREAK : false) )
#else
	#define mxVERIFY( x )		((x))
#endif

//
// Causes a breakpoint exception in debug builds, causes fatal error in release builds.
//
#if MX_DEBUG
	#define mxENSURE( expr )	do{ ptBREAK_IF( !(expr) ) }while(0)
#else
	#define mxENSURE( expr )	if( !(expr) ) {ptERROR("Fatal error!");}
#endif // ! MX_DEBUG

/*
=============================================================================
		Exception handling.
=============================================================================
*/
#if MX_EXCEPTIONS_ENABLED
	#define mxTHROW(x)		throw(x)
	#define mxCATCH(x)		catch(x)
	#define mxCATCH_ALL		catch(...)
#else
	#define mxTHROW(x)		ptERROR("Unhandled exception occured. The program will now exit.\n")
	#define mxCATCH(x)		if(0)
	#define mxCATCH_ALL		if(0)
#endif //MX_EXCEPTIONS_ENABLED

/*
=============================================================================
		Debug macros.
=============================================================================
*/

// Everything inside the parentheses will be discarded in release builds.
#if MX_DEBUG
	#define mxDEBUG_CODE( code )		code
#else
	#define mxDEBUG_CODE( code )		mxNOOP
#endif // ! MX_DEBUG

/*
=============================================================================
	IF_DEBUG is a C/C++ comment-out macro.

	The preprocessor must support the '//' character sequence
	as a single line comment.

	Example:
	IF_DEBUG outputDevice << "Generating a random world " << world.Name();
=============================================================================
*/
#if MX_DEBUG
#	define IF_DEBUG		// Debug version - enable code on line.
#else
#	define IF_DEBUG		mxPT_SLASH(/)	// Release version - comment out line.
#endif // ! MX_DEBUG

/*
=============================================================================

	checked_cast< TypeTo, TypeFrom >

	Works as a static_cast, except that it would use dynamic_cast 
	to catch programming errors in debug mode.
	Uses fast static_cast in Release build, but checks cast with an mxASSERT() in Debug.

	Example:

	(Both 'rxPointLight' and 'rxSpotLight' inherit 'rxLocalLight'.)

	rxPointLight * newLight = new_one(rxPointLight());

	rxSpotLight * pSpotLight = checked_cast< rxSpotLight*>( newLight );	// <= will break here

=============================================================================
*/
#if MX_DEBUG && MX_CPP_RTTI_ENABLED

	template < class TypeTo, class TypeFrom >
	inline TypeTo checked_cast( TypeFrom ptr )
	{
		if ( ptr )
		{
			TypeTo dtto = dynamic_cast< TypeTo >( ptr );
			mxASSERT( dtto != NULL );
			TypeTo stto = static_cast< TypeTo >( ptr );
			mxASSERT( dtto == stto );
			return stto;
		}
		return NULL;
	}

#else // if !defined( MX_DEBUG )

	template < class TypeTo, class TypeFrom >
	inline TypeTo checked_cast( TypeFrom ptr )
	{
		return static_cast< TypeTo >( ptr );
	}

#endif // ! MX_DEBUG

/*

Other versions:

template <class TypeTo, class TypeFrom>
TypeTo checked_cast(TypeFrom p)
{
	mxASSERT(dynamic_cast<TypeTo>(p));
	return static_cast<TypeTo>(p);
}

// perform a static_cast asserted by a dynamic_cast
template <class Type, class SourceType>
Type static_cast_checked(SourceType item)
{
	mxASSERT(!item || dynamic_cast<Type>(item));
	return static_cast<Type>(item);
}

#if MX_DEBUG
#	define checked_cast    dynamic_cast
#else // if !defined( MX_DEBUG )
#	define checked_cast    static_cast
#endif // ! MX_DEBUG


=== For using on references:

/// perform a static_cast asserted by a dynamic_cast
template <class Type, class SourceType>
Type* static_cast_checked(SourceType *item)
{
	mxASSERT(!item || dynamic_cast<Type*>(item));
	return static_cast<Type*>(item);
}

/// overload for reference
template <class Type, class SourceType>
Type &static_cast_checked(SourceType &item)
{
	mxASSERT(dynamic_cast<Type *>(&item));
	return static_cast<Type&>(item);
}

=== Use it like this:
	Derived d;
	Base* pbase = static_cast_checked<Base>(&d);
	Base& rbase = static_cast_checked<Base>(d);
*/

//----------------------------------------------------------------------------
//	Use this to mark unreachable locations,
//	such as the unreachable default case of a switch
//	or code paths that should not be executed
//	(e.g. overriden member function Write() of some class named ReadOnlyFile).
//	Unreachables are removed in release builds.
//----------------------------------------------------------------------------

#if MX_DEBUG
	#define  mxUNREACHABLE				mxASSERT2(false, "Unreachable location")
	#define  mxUNREACHABLE2( text )		mxASSERT2(false, text)
#else
#	define  mxUNREACHABLE				mxOPT_HINT( 0 )
#	define  mxUNREACHABLE2( message )	mxOPT_HINT( 0 )
#endif // ! MX_DEBUG

#if MX_DEBUG
#	define  mxDBG_UNREACHABLE			mxASSERT2(false, "Unreachable location")
#	define  mxDBG_UNREACHABLE2(...)		mxASSERT2(false, __VA_ARGS__)
#else
#	define  mxDBG_UNREACHABLE
#	define  mxDBG_UNREACHABLE2(...)
#endif // ! MX_DEBUG

//----------------------------------------------------------------------------
//	Use this to mark unimplemented features
//	which are not supported yet and will cause a crash.
//  They are not removed in release builds.
//----------------------------------------------------------------------------
#if MX_DEBUG
	#define  Unimplemented				{mxASSERT2(false, "Unimplemented feature");}
	#define  UnimplementedX( message )	{ptERROR("%s(%d): Unimplemented feature in %s", __FILE__, __LINE__, __FUNCTION__ );}
#else
	#define  Unimplemented				ptBREAK
	#define  UnimplementedX( message )	ptBREAK
#endif // ! MX_DEBUG


#if MX_DEBUG
	#define  UNDONE		{mxASSERT2(false, "Unimplemented feature");}
#else
	#define  UNDONE		ptBREAK
#endif

/*
=============================================================================
	
		Compile-time checks and assertions.

		NOTE: expressions to be checked
		must be compile-time constants!
=============================================================================
*/

/*
============================================================================
	StaticAssert( expression )
	STATIC_ASSERT2( expression, message )

	Fires at compile-time !

	Usage:
		StaticAssert( sizeof(*void) == 4, size_of_void_must_be_4_bytes );
============================================================================
*/
#if CPP_0X

	#define mxSTATIC_ASSERT( expression )		static_assert((expression))

#else

	#define mxSTATIC_ASSERT( expression )\
		typedef char PP_JOIN_TOKEN(CHECK_LINE_, __LINE__)	[ (expression) ? 1 : -1 ];\

	//// This macro has no runtime side affects as it just defines an enum
	//// whose name depends on the current line,
	//// and whose value will give a divide by zero error at compile time if the assertion is false.
	//// removed because of warning C4804: '/' : unsafe use of type 'bool' in operation
	////#define mxSTATIC_ASSERT( expression )\
	////	enum { PP_JOIN_TOKEN(CHECK_LINE_, __LINE__) = 1/(!!(expression)) }

#endif // CPP_0X


#define mxSTATIC_ASSERT2( expression, message )		\
	struct ERROR_##message {						\
		ERROR_##message() {							\
			int _##message[ (expression) ? 1 : -1 ];\
			(void)_##message;/* inhibit warning: unreferenced local variable */\
		}											\
	}



/*
============================================================================
	STATIC_CHECK( expression )
	STATIC_CHECK2( expression, message )

	Fires at link-time !
	( unresolved external symbol void CompileTimeChecker<0>(void); )

	Usage:
		STATIC_CHECK2( sizeof(*void) == 4, size_of_void_must_be_4_bytes );

	Note: should only be used in source files, not header files.
============================================================================
* /

namespace debug
{
	// We create a specialization for true, but not for false.
	template< bool > struct CompileTimeChecker;
	template<> struct CompileTimeChecker< true > {
		CompileTimeChecker( ... );
	};
}//end of namespace debug



#define STATIC_CHECK( expr )							\
	debug::CompileTimeChecker < (expr) != false > ();


#define STATIC_CHECK2( expr, msg )						\
class ERROR_##msg {										\
	debug::CompileTimeChecker< (expr) != false > error;	\
}
*/



/*
=================================================================
	DELAYED_ASSERT( expression );

	Valid only in function scope !

	Fires at link-time !
	( unresolved external symbol void StaticAssert<0>(void); )

	Usage example:
			void  Foo()
			{
				DELAYED_ASSERT( a == b );

				// ....
			}
=================================================================
* /
namespace debug {

	template< bool > void	StaticAssert();
	template<> inline void	StaticAssert< true >() {}

}//end of namespace debug

#define DELAYED_ASSERT( expression )					\
		::debug::StaticAssert< expression != false >();	\
*/

//--------------------------------------------------------------------------------------------------------------
//	These can be very useful compile-time assertions :
/* COMPILE_TIME_ASSERT is for enforcing boolean/integral conditions at compile time.
   Since it is purely a compile-time mechanism that generates no code, the check
   is left in even if _DEBUG is not defined. */
//--------------------------------------------------------------------------------------------------------------

// This one is valid in function scope only!
#define mxCOMPILE_TIME_ASSERT( x )				{ typedef int ZZ_compile_time_assert_failed[ (x) ? 1 : -1 ]; }

// This one is valid in function scope only!
#define mxCOMPILE_TIME_ASSERT2( x )				switch(0) { case 0: case !!(x) : ; }

// This one is valid in file and function scopes.
#define mxFILE_SCOPED_COMPILE_TIME_ASSERT( x )	extern int ZZ_compile_time_assert_failed[ (x) ? 1 : -1 ]

#define ASSERT_SIZEOF( type, size )				mxSTATIC_ASSERT( sizeof( type ) == size )
#define ASSERT_OFFSETOF( type, field, offset )	mxSTATIC_ASSERT( offsetof( type, field ) == offset )
#define ASSERT_SIZEOF_8_BYTE_MULTIPLE( type )	mxSTATIC_ASSERT( ( sizeof( type ) & 8 ) == 0 )
#define ASSERT_SIZEOF_16_BYTE_MULTIPLE( type )	mxSTATIC_ASSERT( ( sizeof( type ) & 15 ) == 0 )
#define ASSERT_SIZEOF_32_BYTE_MULTIPLE( type )	mxSTATIC_ASSERT( ( sizeof( type ) & 31 ) == 0 )
#define ASSERT_SIZEOF_ARE_EQUAL( type1, type2 )	mxSTATIC_ASSERT( sizeof( type1 ) == sizeof( type2 ) )

#define mxSTATIC_ASSERT_ISPOW2( X )				mxSTATIC_ASSERT( TIsPowerOfTwo< (X) >::value )

/*
============================================================================
	CHECK_STORAGE( var, required_range )

	// bad explanation, look at the example below
	var - variable (e.g. 'c', where 'c' is of type 'char')
	required_range - [0..max_alowed_value] (e.g. 256)

	Fires at compile-time !

	Usage:
		unsigned char c;
		enum { Required_Range = 257 };	// 'c' must hold up to 257 different values without overflow
		CHECK_STORAGE( c, Required_Range ); // <- error, 'c' can only hold 256 different values

	Multiple checks should be enclosed into brackets:
		{ CHECK_STORAGE( var1, range1 ); }
		{ CHECK_STORAGE( var2, range2 ); }
		etc.
============================================================================
*/

namespace debug
{
	template< unsigned BytesInVariable, unsigned long RequiredRange >
	struct TStorageChecker
	{
		enum {
			bEnoughStorage =  TPow2< BYTES_TO_BITS(BytesInVariable) >::value >= RequiredRange
		};
		//static const bool bEnoughStorage = ( TPow2< BYTES_TO_BITS(BytesInVariable) >::value >= RequiredRange );
	};

	template< unsigned BitsInVariable, unsigned long RequiredRange >
	struct TStorageCheckerBits
	{
		enum {
			bEnoughStorage =  TPow2< BitsInVariable >::value >= RequiredRange
		};
	};

}//end of namespace debug


#define CHECK_STORAGE( size_of_var, required_range )\
	class PP_JOIN_TOKEN(StorageChecker_,__LINE__) {\
		int a[ debug::TStorageChecker< (size_of_var), (required_range) >::bEnoughStorage ? 1 : -1 ];\
	}

#define CHECK_STORAGE_BITS( size_of_var_in_bits, max_value )\
	class PP_JOIN_TOKEN(StorageChecker_,__LINE__) {\
		int a[ debug::TStorageCheckerBits< (size_of_var_in_bits), (max_value) >::bEnoughStorage ? 1 : -1 ];\
	}

//--------------------------------------------------------------------------------------------------------------

//
//	COMPILE_TIME_MSG( msg )
//
//	Writes a message into the output window.
//
//	Usage:
//	#pragma COMPILE_TIME_MSG( Fix this before final release! )
//
#define mxCOMPILE_TIME_MSG( msg )		message( __FILE__ "(" TO_STR(__LINE__) "): " #msg )

//TODO: compile-time warnings
#define mxCOMPILE_TIME_WARNING( x, msg )	mxCOMPILE_TIME_ASSERT((x))

//--------------------------------------------------------------------------------------------------------------

#if MX_DEBUG
	#define mxDBG_TRACE_CALL		DBGOUT("%s (%s:%d)\n",__FUNCTION__,__FILE__,__LINE__)
#else
	#define mxDBG_TRACE_CALL
#endif // MX_DEBUG


#if MX_DEBUG

	#define DBG_DO_INTERVAL(what,milliseconds)\
	{\
		static UINT prevTime = mxGetTimeInMilliseconds();\
		const UINT currTime = mxGetTimeInMilliseconds();\
		if( currTime - prevTime > milliseconds )\
		{\
			what;\
			prevTime = currTime;\
		}\
	}

#else

	#define DBG_DO_INTERVAL(what,milliseconds)

#endif // MX_DEBUG


#if MX_DEBUG
	#define mxDBG_BREAK_IF_EQUAL_STR( STR1, STR2 )		if(mxStrEquAnsi( (STR1), (STR2) ))	ptBREAK;
#else
	#define mxDBG_BREAK_IF_EQUAL_STR( STR1, STR2 )
#endif // MX_DEBUG


#define chkPATH_EXISTS( X )\
	if( !OS::IO::PathExists( (X).ToPtr() ) ) {\
		ptERROR( "Path '%s' doesn't exist.", (X).ToPtr() );\
		return false;\
	}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
