/*
=============================================================================
	File:	Types.h
	Desc:	Definitions for common types for scalability, portability, etc.
	Note:	This file uses other base types defined
			in platform-specific headers.
=============================================================================
*/
#pragma once

/*
=============================================================================
	Low-level type definitions
=============================================================================
*/
//
// The null pointer type.
//
#ifndef NULL
#define NULL	0
#endif

#if !CPP_0X
#define nullptr	0
#endif

// this was introduced to do less typing
#define nil		nullptr

// macro for converting pointers to boolean values
//
#if CPP_0X
	#define PtrToBool(pointer_to_variable)	((pointer_to_variable) != nil)
#else
	#define PtrToBool(pointer_to_variable)	(pointer_to_variable)
#endif

//
//	Boolean types.
//

//
//	mxBool32 - guaranteed to be 32 bytes in size (for performance-related reasons).
//	Thus comparisons like bool32 == true will not work as expected.
//
typedef INT32	mxBool32;

//
//	FASTBOOL - 'false' is zero, 'true' is any non-zero value.
//	Thus comparisons like bool32 == true will not work as expected.
//
typedef signed int		FASTBOOL;

// 'false' is zero, 'true' is one.
typedef unsigned int	mxBool01;

/*
=============================================================================
	Numeric Limits
=============================================================================
*/

// Minimum 8-bit signed integer.
#define MIN_INT8	((UINT8)~MAX_INT8)
// Maximum 8-bit signed integer.// 0x7F
#define	MAX_INT8	((INT8)(MAX_UINT8 >> 1))
// Maximum 8-bit unsigned integer.// 0xFFU
#define	MAX_UINT8	((UINT8)~((UINT8)0))

// Minimum 16-bit signed integer.
#define	MIN_INT16	((INT16)~MAX_INT16)
// Maximum 16-bit signed integer.// 0x7FFF
#define	MAX_INT16	((INT16)(MAX_UINT16 >> 1))
// Maximum 16-bit unsigned integer.// 0xFFFFU
#define	MAX_UINT16	((UINT16)~((UINT16)0))

// Minimum 32-bit signed integer.
#define	MIN_INT32	((INT32)~MAX_INT32)
// Maximum 32-bit signed integer.// 0x7FFFFFFF
#define	MAX_INT32	((INT32)(MAX_UINT32 >> 1))
// Maximum 32-bit unsigned integer.// 0xFFFFFFFFU
#define	MAX_UINT32	((UINT32)~((UINT32)0))

// Minimum 64-bit signed integer.
#define	MIN_INT64	((INT64)~MAX_INT64);
// Maximum 64-bit signed integer.//0x7FFFFFFFFFFFFFFF
#define	MAX_INT64	((INT64)(MAX_UINT64 >> 1))
// Maximum 64-bit unsigned integer.//0xFFFFFFFFFFFFFFFUL
#define	MAX_UINT64	((UINT64)~((UINT64)0))

#define MAX_FLOAT32		3.402823466e+38F
#define MIN_FLOAT32		1.175494351e-38F

#define MAX_FLOAT64		179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0
#define MIN_FLOAT64		2.225073858507201383090e-308

// smallest positive number such that (1.0 + FLOAT_EPSILON) != 1.0
#define	FLOAT32_EPSILON		1.192092896e-07f
#define FLOAT64_EPSILON		2.220446049250313080847e-16

//---------------------------------------------------------------------------
//	Definitions of useful mathematical constants.
//---------------------------------------------------------------------------

//
// Pi
//
#define		mxPI			float( 3.1415926535897932384626433832795028841971693993751 )
#define		mxPI_f64		DOUBLE( 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798 )
#define		mxTWO_PI		float( 6.28318530717958647692528676655901 )	// pi * 2
#define		mxHALF_PI		float( 1.5707963267948966192313216916395 )	// pi / 2
#define		mxPI_DIV_4		float( 0.78539816339744830961566084581975 )	// pi / 4
#define		mxINV_PI		float( 0.31830988618379067153776752674508 )	// 1 / pi
#define		mxINV_TWOPI		float( 0.15915494309189533576888376337254 )	// 1 / (pi * 2)
#define		mx2_DIV_PI		float( 0.63661977236758134307553505349016 )	// 2 / pi
#define		mxSQRT_PI		float( 1.772453850905516027298167483341 )	// sqrt(pi)
#define		mxINV_SQRT_PI	float( 0.564189583547756286948f )			// 1 / sqrt(pi)
#define		mx2_SQRT_PI		float( 1.1283791670955125738961589031216 )	// 2 / sqrt(pi)
#define		mxPI_SQUARED	float( 9.869604401089358618834490999873 )	// pi * pi

#define		mxTANGENT_30	float( 0.57735026918962576450914878050196f )// tan(30)
#define		mx2_TANGENT_30	float( (mxTANGENT_30 * 2.0f)	// 2*tan(30)

//
// Golden ratio ( phi = float( 0.5 * (Sqrt(5) + 1) ).
//
#define		mxPHI			float( 1.61803398874989484820f )
#define		mxPHI_f64		DOUBLE( 1.6180339887498948482045868343656 )

//
// e
//
#define		mxE				float( 2.71828182845904523536f )
#define		mxE_f64			DOUBLE( 2.7182818284590452353602874713526624977572470936999 )
#define		mxLOG2E			float( 1.44269504088896340736f )	// log2(e)
#define		mxLOG10E		float( 0.434294481903251827651f )	// log10(e)

#define		mxLN10			float( 2.30258509299404568402f )	// ln(10)

#define		mxINV_LOG2		float( 3.32192809488736234787f )	// 1.0 / log10(2)
#define		mxLN2			float( 0.69314718055994530941723212145818f )	// ln(2)
#define		mxINV_LN2		float( 1.44269504089f )				// 1.0f / ln(2)

#define		mxINV_3			float( 0.33333333333333333333f )	// 1/3
#define		mxINV_6			float( 0.16666666666666666666f )	// 1/6
#define		mxINV_7			float( 0.14285714285714285714f )	// 1/7
#define		mxINV_9			float( 0.11111111111111111111f )	// 1/9
#define		mxINV_255		float( 0.00392156862745098039f )	// 1/255

#define		mxSQRT_2		float( 1.4142135623730950488016887242097f )	// sqrt(2)
#define		mxINV_SQRT_2	float( 0.7071067811865475244008443621048490f )	// 1 / sqrt(2)

#define		mxSQRT_3		float( 1.7320508075688772935274463415059f )	// sqrt(3)
#define		mxINV_SQRT_3	float( 0.57735026918962576450f )	// 1 / sqrt(3)

#define		mxSQRT5			float( 2.236067977499789696409173668731f )

// Euler's constant
#define		mxEULER			float( 0.5772156649015328606065 )

// degrees to radians multiplier
#define		mxDEG2RAD		float( 0.0174532925199432957692369076848861 )	// mxPI / 180.0f

// radians to degrees multiplier
#define		mxRAD2DEG		float( 57.2957795130823208767981548141052 )	// 180.0f / mxPI

// seconds to milliseconds multiplier
#define		mxSEC2MS		1000

// milliseconds to seconds multiplier
#define		mxMS2SEC		float( 0.001f )


#define mxSLERP_DELTA	0.999f

//--------------------------------------------------
//	Universal constants.
//--------------------------------------------------

//
// Atomic physics constants.
//
namespace KernPhysik
{
	// Unified atomic mass.
	const double	u  = 1.66053873e-27f;

	// Electron mass.
	const double	me = 9.109373815276e-31f;

	// Electron charge.
	const double	e  = 1.602176462e-16f;

	// Proton mass.
	const double	mp = 1.67262158e-27f;

	// Neutron mass.
	const double	mn = 1.67492716e-27f;

};//End of KernPhysik

//
// Bulk physics constants.
//
namespace Physik
{
	// Boltzmann constant
	const double k	= 1.3806503e-23f;

	// Electric field constant / permittivity of free space
	const double e	= 8.854187817e-12f;

	// Universal Gravitational Constant [m^3 kg^(-1) s^(-2)]
	const double G = 6.67384e-11;

	// Impedance of free space
	const double Z	= 376.731f;

	// Speed of light in vacuum
	const double c	= 2.99792458e8f;

	// Magnetic field constant / Permeability of a vacuum
	const double mu	= 1.2566370614f;

	// Planck constant
	const double h = 6.62606876e-34f;

	// Stefan-Boltzmann constant
	const double sigma = 5.670400e-8f;

	// Astronomical unit
	const double AU = 149.59787e11f;

	// Standard gravity acceleration.
	const double EARTH_GRAVITY	= 9.81f;
	// mass of the Earth [kg]
	const double EARTH_MASS = 5.97219e+24;
	// radius of the Earth [m]
	const double EARTH_RADIUS = 6.3781e+6;

};//End of Physik

//
//	Conversions.
//
#define DEG2RAD( a )		( (a) * mxDEG2RAD )
#define RAD2DEG( a )		( (a) * mxRAD2DEG )

#define SEC2MS( t )			( mxFloatToInt( (t) * mxSEC2MS ) )
#define MS2SEC( t )			( mxIntToFloat( (t) ) * mxMS2SEC )

#define ANGLE2SHORT( x )	( mxFloatToInt( (x) * 65536.0f / 360.0f ) & 65535 )
#define SHORT2ANGLE( x )	( (x) * ( 360.0f / 65536.0f ) ); }

#define ANGLE2BYTE( x )		( mxFloatToInt( (x) * 256.0f / 360.0f ) & 255 )
#define BYTE2ANGLE( x )		( (x) * ( 360.0f / 256.0f ) )

//--------------------------------------------------------------//

#define BIG_NUMBER			999999.0f
#define SMALL_NUMBER		1.e-6f

// Value indicating invalid index.
// Use "-1" with unsigned values to indicate a bad/uninitialized state.

#define INDEX_NONE_16	(UINT16)(~0u)
#define INDEX_NONE_32	(~0u)
#define INDEX_NONE_64	(~0ull)

#define INDEX_NONE			INDEX_NONE_32
#define INDEX_NONE_SIZE_T	INDEX_NONE_64

// ENoInit - Tag for suppressing initialization (also used to mark uninitialized variables).
enum ENoInit { _NoInit };

// EInitDefault - means 'initialize with default value'.
enum EInitDefault { _InitDefault = 0 };

// EInitZero - Used to mark data initialized with invalid values.
enum EInitInvalid { _InitInvalid = -1 };

// EInitZero - Used to mark data initialized with zeros.
enum EInitZero { _InitZero = 0 };

// EInitIdentity - Used to mark data initialized with default Identity/One value.
enum EInitIdentity { _InitIdentity = 1 };

// EInitInfinity - Provided for convenience.
enum EInitInfinity { _InitInfinity = -1 };

// for creating (slow) named constructors
enum EInitSlow { _InitSlow = -1 };

enum EInitCustom { _InitCustom };

//enum EInPlaceSerializationHelper { _FinishedLoadingFlag };
struct _FinishedLoadingFlag { _FinishedLoadingFlag(){} };

enum EDontAddRef { _DontAddRef };
enum EDontCopy { _DontCopy };

// relative or absolute mode
enum ERelAbs
{
	kAbsolute,
	kRelative
};

//
// Helper templates.
//
template< typename T >
struct Equal
{
	inline bool operator()(const T& a, const T& b)	const { return a==b; }
};
template< typename T >
struct Less
{
	inline bool operator()(const T& a, const T& b)	const { return a<b; }
};
template< typename T >
struct Greater
{
	inline bool operator()(const T& a, const T& b)	const { return a>b; }
};

template< typename T > inline T	Max( T x, T y ) { return ( x > y ) ? x : y; }
template< typename T > inline T	Min( T x, T y ) { return ( x < y ) ? x : y; }
template< typename T > inline INT	MaxIndex( T x, T y ) { return  ( x > y ) ? 0 : 1; }
template< typename T > inline INT	MinIndex( T x, T y ) { return ( x < y ) ? 0 : 1; }

template< typename T > inline T	Max3( T x, T y, T z ) { return ( x > y ) ? ( ( x > z ) ? x : z ) : ( ( y > z ) ? y : z ); }
template< typename T > inline T	Min3( T x, T y, T z ) { return ( x < y ) ? ( ( x < z ) ? x : z ) : ( ( y < z ) ? y : z ); }
template< typename T > inline INT	Max3Index( T x, T y, T z ) { return ( x > y ) ? ( ( x > z ) ? 0 : 2 ) : ( ( y > z ) ? 1 : 2 ); }
template< typename T > inline INT	Min3Index( T x, T y, T z ) { return ( x < y ) ? ( ( x < z ) ? 0 : 2 ) : ( ( y < z ) ? 1 : 2 ); }

template< typename T > inline T	Abs( T x )		{ return ( x >= 0 ) ? x : -x; }
template< typename T > inline T	Sign( T x )		{ return ( x > 0 ) ? 1 : ( ( x < 0 ) ? -1 : 0 ); }
template< typename T > inline T	Square( T x )	{ return x * x; }
template< typename T > inline T	Cube( T x )		{ return x * x * x; }

template< typename T > inline T	Clamp( const T value, const T Min, const T Max )	{ return (value < Min) ? Min : (value > Max) ? Max : value; }
template< typename T > inline T	Saturate( const T value )						{ return Clamp<T>( value, 0, 1 ); }

template< typename T > inline T	Mean( const T a, const T b )	{ return (a + b) * (T)0.5f; }

template< typename T > inline T	Wrap( const T a, const T min, const T max )	{ return (a < min)? max - (min - a) : (a > max)? min - (max - a) : a; }

template< typename T > inline bool	IsInRange( const T value, const T Min, const T Max )	{ return (value > Min) && (value < Max); }
template< typename T > inline bool	IsInRangeInc( const T value, const T Min, const T Max )	{ return (value >= Min) && (value <= Max); }


template< typename T >
inline void TSetMin( T& a, const T& b ) {
    if( b < a ) {
		a = b;
	}
}

template< typename T >
inline void TSetMax( T& a, const T& b ) {
    if( a < b ) {
		a = b;
	}
}

template< typename T >
inline const T& TGetClamped( const T& a, const T& lo, const T& hi )
{
	return (a < lo) ? lo : (hi < a ? hi : a);
}

// maps negative values to -1, zero to 0 and positive to +1
template< typename T >
inline T MapToUnitRange( const T& value ) {
	return T( (value > 0) - (value < 0) );
}

//static inline
//FLOAT SafeReciprocal( FLOAT x )
//{
//	return (fabs(x) > 1e-4f) ? (1.0f / x) : 0.0f;
//}

//
//	TSwap - exchanges values of two elements.
//
template< typename type >
inline void TSwap( type & a, type & b )
{
	const type  temp = a;
	a = b;
	b = temp;
}

/*
	swap in C++ 11:

	template <class T> swap(T& a, T& b)
	{
		T tmp(std::move(a));
		a = std::move(b);   
		b = std::move(tmp);
	}
*/

//
//	TSwap16 - exchanges values of two 16-byte aligned elements.
//
template< typename type >
inline void TSwap16( type & a, type & b )
{
	mxALIGN_16( const type  temp ) = a;
	a = b;
	b = temp;
}

//template <class T> inline void Exchange( T & X, T & Y )
//{
//	const T Tmp = X;
//	X = Y;
//	Y = Tmp;
//}


// TODO: endian swap
template< typename TYPE >
inline void TOrderPointers( TYPE *& pA, TYPE *& pB )
{
	if( (ULONG)pA > (ULONG)pB )
	{
		TSwap( pA, pB );
	}
}

template< typename TYPE >
inline void TSort2( TYPE & oA, TYPE & oB )
{
	if( oA > oB )
	{
		TSwap( oA, oB );
	}
}

template< typename TYPE >
inline void TSort3( TYPE & a, TYPE & b, TYPE & c )
{
	if( a > b ) {
		TSwap( a, b );
	}
	if( b > c ) {
		TSwap( b, c );
	}
	if( a > b ) {
		TSwap( a, b );
	}
	if( b > c ) {
		TSwap( b, c );
	}
}

// Figure out if a type is a pointer
template< typename TYPE > struct is_pointer
{
	enum { VALUE = false };
};
template< typename TYPE > struct is_pointer< TYPE* >
{
	enum { VALUE = true };
};

// Removes the pointer from a type
template< typename TYPE > struct strip_pointer
{
	typedef TYPE Type;
};
template< typename TYPE > struct strip_pointer< TYPE* >
{
	typedef TYPE Type;
};

template< typename TYPE >
bool IsSignedType( const TYPE t ) {
	return TYPE( -1 ) < 0;
}

//
// The "Empty Member" C++ Optimization
// See:
// http://www.cantrip.org/emptyopt.html
// http://www.tantalon.com/pete/cppopt/final.htm#EmptyMemberOptimization
//
template< class Base, class Member >
struct BaseOpt : Base {
	Member m;
	BaseOpt( Base const& b, Member const& mem ) 
		: Base(b), m(mem)
	{ }
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

template< class T >
inline T* ConstructInPlace( T* mem )
{
	return ::new(mem) T;
}

template< class T >
inline T* ConstructInPlace( T* mem, const T* copy )
{
	return ::new(mem) T( *copy );
}

template< class T, class T2 >
inline T* ConstructInPlace( T* mem, T2 t2 )
{
	return ::new(mem) T( t2 );
}

template< class T, class T2, class T3 >
inline T* ConstructInPlace( T* mem, T2 t2, T3 t3 )
{
	return ::new(mem) T( t2, t3 );
}

template< class T, class T2, class T3, class T4 >
inline T* ConstructInPlace( T* mem, T2 t2, T3 t3, T4 t4 )
{
	return ::new(mem) T( t2, t3, t4 );
}

template< class T, class T2, class T3, class T4, class T5 >
inline T* ConstructInPlace( T* mem, T2 t2, T3 t3, T4 t4, T5 t5 )
{
	return ::new(mem) T( t2, t3, t4, t5 );
}

//---------------------------------------------------------------------------
// Methods to invoke the constructor, copy constructor, and destructor
//---------------------------------------------------------------------------

template< class T >
inline void Construct( T* pMemory )
{
	::new( pMemory ) T;
}

template< class T >
inline void CopyConstruct( T* pMemory, T const& src )
{
	::new( pMemory ) T(src);
}

// Destructs an object without freeing the memory associated with it.
template< class T >
inline void Destruct( T* ptr )
{
	ptr->~T();
//#if MX_DEBUG
//	memset( ptr, FREED_MEM_ID, sizeof(T) );
//#endif
}

template< class T >
inline void FDestructorCallback( void* o )
{
	((T*)o)->~T();
}

// Constructs an array of objects with placement ::new.
template< typename T >
inline T * TConstructN( T* basePtr, size_t objectCount )
{
	for( size_t i = 0; i < objectCount; i++ )
	{
		::new ( basePtr + i ) T();
	}
	return basePtr;
}

// Explicitly calls destructors for an array of objects.
//
template< typename T >
inline void TDestructN( T* basePtr, size_t objectCount )
{
#if 1
	for( size_t i = 0; i < objectCount; i++ )
	{
		(basePtr + i)->~T();
	}
#else
	mxASSERT( objectCount != 0 );

	T* start = basePtr;
	T* current = basePtr + objectCount - 1;

	// Destroy in reverse order, to match construction order.
	while( current-- > start )
	{
		current->~T();
	}
#endif
//#if MX_DEBUG
//	memset( basePtr, FREED_MEM_ID, sizeof(T) * objectCount );
//#endif
}


template< typename T >
inline void TDestructOne_IfNonPOD( T & o )
{
	if( !TypeTrait<T>::IsPlainOldDataType )
	{
		o.~T();
	}
}

template< typename T >
inline void TConstructN_IfNonPOD( T* basePtr, size_t objectCount )
{
	if( !TypeTrait<T>::IsPlainOldDataType )
	{
		TConstructN< T >( basePtr, objectCount );
	}
}

template< typename T >
inline void TDestructN_IfNonPOD( T* basePtr, size_t objectCount )
{
	if( !TypeTrait<T>::IsPlainOldDataType )
	{
		TDestructN< T >( basePtr, objectCount );
	}
}

template< typename T >
inline void TCopyArray( T* dest, const T* src, size_t count )
{
	if( TypeTrait<T>::IsPlainOldDataType )
	{
		memcpy( dest, src, count * sizeof(T) );
	}
	else
	{
		for( size_t i = 0; i < count; i++ )
		{
			dest[ i ] = src[ i ];
		}
	}
}

template< typename T >
inline void TCopyConstructArray( T* dest, const T* src, size_t count )
{
	if( TypeTrait<T>::IsPlainOldDataType )
	{
		memcpy( dest, src, count * sizeof(T) );
	}
	else
	{
		for( size_t i = 0; i < count; i++ )
		{
			new (&dest[i]) T( src[i] );
		}
	}
}

template< typename TYPE1, typename TYPE2, const UINT SIZE >
static inline void TCopyStaticPODArray( TYPE1 (&dst)[SIZE], const TYPE2 (&src)[SIZE] )
{
	for( UINT i = 0; i < SIZE; i++ ) {
		dst[i] = src[i];
	}
}
template< typename TYPE1, typename TYPE2 >
static inline void TCopyStaticPODArray( TYPE1 *dst, const TYPE2* src, UINT count )
{
	for( UINT i = 0; i < count; i++ ) {
		dst[i] = src[i];
	}
}

template< typename T >
inline void TSetArray( T* dest, size_t count, const T& srcValue )
{
	// memset - The value is passed as an int, but the function fills the block of memory
	// using the unsigned char conversion of this value
	//if( TypeTrait<T>::IsPlainOldDataType
	//	&& ( sizeof srcValue <= sizeof int )
	//	)
	//{
	//	//if( count )
	//	memset( dest, &srcValue, count * sizeof(T) );
	//}
	//else
	{
		for( size_t i = 0; i < count; i++ )
		{
			dest[ i ] = srcValue;
		}
	}
}

template< typename T >
inline void TMoveArray( T* dest, const T* src, size_t count )
{
	if( TypeTrait<T>::IsPlainOldDataType )
	{
		memmove( dest, src, count * sizeof(T) );
	}
	else
	{
		for( size_t i = 0; i < count; i++ )
		{
			dest[ i ] = src[ i ];
		}
	}
}


template< typename SIZE_TYPE, UINT NUM_BUCKETS >
void Build_Offset_Table_1D(const SIZE_TYPE (&counts)[NUM_BUCKETS],
						   SIZE_TYPE (&offsets)[NUM_BUCKETS])
{
	UINT	numSoFar = 0;

	for( UINT iBucket = 0; iBucket < NUM_BUCKETS; iBucket++ )
	{
		offsets[ iBucket ] = numSoFar;

		numSoFar += counts[ iBucket ];
	}
}

// builds a 2D table of offsets for indexing into sorted list
// (similar to prefix sum computation in radix sort algorithm)
//
template< UINT NUM_ROWS, UINT NUM_COLUMNS >
void Build_Offset_Table_2D(const UINT counts[NUM_ROWS][NUM_COLUMNS],
						UINT offsets[NUM_ROWS][NUM_COLUMNS])
{
	UINT	numSoFar = 0;

	for( UINT iRow = 0; iRow < NUM_ROWS; iRow++ )
	{
		for( UINT iCol = 0; iCol < NUM_COLUMNS; iCol++ )
		{
			// items in cell [iRow][iCol] start at 'numSoFar'
			offsets[iRow][iCol] = numSoFar;

			numSoFar += counts[iRow][iCol];
		}
	}
}

template< UINT COUNT >
UINT Calculate_Sum_1D( const UINT (&elements)[COUNT] )
{
	UINT totalCount = 0;
	for( UINT i = 0; i < COUNT; i++ )
	{
		totalCount += elements[ i ];
	}
	return totalCount;
}

template< UINT NUM_ROWS, UINT NUM_COLUMNS >
UINT Calculate_Sum_2D( const UINT (&elements)[NUM_ROWS][NUM_COLUMNS] )
{
	UINT totalCount = 0;
	for( UINT iRow = 0; iRow < NUM_ROWS; iRow++ )
	{
		for( UINT iCol = 0; iCol < NUM_COLUMNS; iCol++ )
		{
			totalCount += elements[iRow][iCol];
		}
	}
	return totalCount;
}

//
//	DbgNamedObject - named object.
//
//	NOTE: its memory layout and size are different in debug and release versions!
//
template< UINT BUFFER_SIZE = 32 >
struct DbgNamedObject
{
#if MX_DEBUG
		// NOTE: the name will be truncated to the size of the embedded buffer
		inline void DbgSetName( const char* _str )
		{
			chkRET_IF_NIL(_str);
			const UINT len = smallest( strlen(_str), mxCOUNT_OF(m__DebugName)-1 );
			strncpy( m__DebugName, _str, len );
			m__DebugName[len] = '\0';
		}
		inline const char* DbgGetName() const
		{
			return m__DebugName;
		}
	protected:
		inline DbgNamedObject()
		{
			m__DebugName[0] = '?';
			m__DebugName[1] = '\0';
		}
	private:
		char	m__DebugName[ BUFFER_SIZE ];
#else
		inline void DbgSetName( const char* _str )
		{
			mxUNUSED(_str);
		}
		inline const char* DbgGetName() const
		{
			return "";
		}
#endif // MX_DEBUG
};//struct DbgNamedObject

// memory usage stats
//
struct MemStatsCollector
{
	size_t	staticMem;	// size of non-allocated memory, in bytes
	size_t	dynamicMem;	// size of allocated memory, in bytes

public:
	inline MemStatsCollector()
	{
		Reset();
	}
	inline void Reset()
	{
		staticMem = 0;
		dynamicMem = 0;
	}
};


template< typename T >
static inline
T* GetStaticArrayPtr( const T& e0 )
{
	static T s_array[] = { e0 };
	return s_array;
}
template< typename T >
static inline
T* GetStaticArrayPtr( const T& e0, const T& e1 )
{
	static T s_array[] = { e0, e1 };
	return s_array;
}
template< typename T >
static inline
T* GetStaticArrayPtr( const T& e0, const T& e1, const T& e2 )
{
	static T s_array[] = { e0, e1, e2 };
	return s_array;
}
template< typename T >
static inline
T* GetStaticArrayPtr( const T& e0, const T& e1, const T& e2, const T& e3 )
{
	static T s_array[] = { e0, e1, e2, e3 };
	return s_array;
}
template< typename T >
static inline
T* GetStaticArrayPtr( const T& e0, const T& e1, const T& e2, const T& e3, const T& e4 )
{
	static T s_array[] = { e0, e1, e2, e3, e4 };
	return s_array;
}
template< typename T >
static inline
T* GetStaticArrayPtr( const T& e0, const T& e1, const T& e2, const T& e3, const T& e4, const T& e5 )
{
	static T s_array[] = { e0, e1, e2, e3, e4, e5 };
	return s_array;
}
template< typename T >
static inline
T* GetStaticArrayPtr( const T& e0, const T& e1, const T& e2, const T& e3, const T& e4, const T& e5, const T& e6 )
{
	static T s_array[] = { e0, e1, e2, e3, e4, e5, e6 };
	return s_array;
}
template< typename T >
static inline
T* GetStaticArrayPtr( const T& e0, const T& e1, const T& e2, const T& e3, const T& e4, const T& e5, const T& e6, const T& e7 )
{
	static T s_array[] = { e0, e1, e2, e3, e4, e5, e6, e7 };
	return s_array;
}
template< typename T >
static inline
T* GetStaticArrayPtr( const T& e0, const T& e1, const T& e2, const T& e3, const T& e4, const T& e5, const T& e6, const T& e7, const T& e8 )
{
	static T s_array[] = { e0, e1, e2, e3, e4, e5, e6, e7, e8 };
	return s_array;
}

template< typename A, typename B >
static inline void CopyPOD( A &dest, const B& src )
{
	mxSTATIC_ASSERT( sizeof dest == sizeof src );
	memcpy( &dest, &src, sizeof dest );
}

//
//	mxZERO_OUT( x )
//
//#define mxZERO_OUT( x )		MemZero( &(x), sizeof(x) )

// Useful function to clean the structure.
template <class T>
inline void mxZERO_OUT( T &t ) { MemZero( &t, sizeof(t) ); }

//--------------------------------------------------------------//
//	Basic I/O.
//--------------------------------------------------------------//

//
//	AStreamReader
//
class AStreamReader : public DbgNamedObject< 60 >
{
public:
	virtual size_t Tell() const = 0;
	// Return the size of data (length of the file if this is a file), in bytes.
	virtual size_t GetSize() const = 0;
	virtual ERet Read( void *buffer, size_t size ) = 0;

	inline ERet SerializeMemory( void *pDest, const size_t size )
	{
		return this->Read( pDest, size );
	}
	template< typename TYPE >
	inline ERet Get( TYPE &value )
	{
		return this->Read( &value, sizeof(TYPE) );
	}
	template< typename TYPE >
	inline void SerializeArray( TYPE * a, UINT count )
	{
		if( TypeTrait<TYPE>::IsPlainOldDataType ) {
			if( count ) {
				this->SerializeMemory( a, count * sizeof a[0] );
			}
		} else {
			for( UINT i = 0; i < count; i++ ) {
				*this >> a[i];
			}
		}
	}
protected:
	inline AStreamReader() {}
	virtual ~AStreamReader();
private:
	PREVENT_COPY(AStreamReader);
};

//
//	AStreamWriter
//
class AStreamWriter : public DbgNamedObject< 60 >
{
public:
	virtual ERet Write( const void* buffer, size_t size ) = 0;

	inline ERet SerializeMemory( const void* source, const size_t size )
	{
		return this->Write( source, size );
	}
	template< typename TYPE >
	inline ERet Put( const TYPE& value )
	{
		return this->Write( &value, sizeof(TYPE) );
	}
	template< typename TYPE >
	inline void SerializeArray( TYPE * a, UINT count )
	{
		if( TypeTrait<TYPE>::IsPlainOldDataType ) {
			if( count ) {
				this->SerializeMemory( a, count * sizeof a[0] );
			}
		} else {
			for( UINT i = 0; i < count; i++ ) {
				*this << a[i];
			}
		}
	}

	virtual void VPreallocate( UINT32 sizeHint ) {}

protected:
	inline AStreamWriter() {}
	virtual ~AStreamWriter();
private:
	PREVENT_COPY(AStreamWriter);
};

#define mxDECLARE_STREAMABLE( TYPE )\
	friend AStreamWriter& operator << ( AStreamWriter& stream, const TYPE& o );\
	friend AStreamReader& operator >> ( AStreamReader& stream, TYPE& o );

#define mxIMPLEMENT_FUNCTION_READ_SINGLE( TYPE, FUNCTION )	\
	mxFORCEINLINE TYPE FUNCTION( AStreamReader & stream )	\
	{	\
		TYPE value;	\
		stream.Get(value);	\
		return value;	\
	}

#define mxIMPLEMENT_FUNCTION_WRITE_SINGLE( TYPE, FUNCTION )\
	mxFORCEINLINE ERet FUNCTION( AStreamWriter & stream, const TYPE& value )\
	{\
		return stream.SerializeMemory( &value, sizeof(TYPE) );\
	}

class AObject;

/*
-----------------------------------------------------------------------------
	mxArchive
	used for 'manual' binary serialization
-----------------------------------------------------------------------------
*/
struct mxArchive
{
	// serialization of POD objects
	virtual void SerializeMemory( void* ptr, size_t size ) = 0;

	// serialization of (references to) non-POD objects
	virtual void SerializePointer( AObject *& o ) {Unimplemented;}

	// registers a top-level object
	virtual void InsertRootObject( AObject* o ) {Unimplemented;}

public:
	virtual AStreamWriter* IsWriter() {return nil;}
	virtual AStreamReader* IsReader() {return nil;}

	// IsSaving()
	inline bool IsWriting() { return this->IsWriter() != nil; }
	// IsLoading()
	inline bool IsReading() { return this->IsReader() != nil; }

protected:
	virtual ~mxArchive()
	{}
};

//	Type traits.
//TODO: type traits from Qt?
//
//	TypeTrait<T> - used for carrying compile-time type information.
//
//	Can be specialized for custom types.
//
//	NOTE: these templates could also be done with enums.
//
template< typename TYPE >
struct TypeTrait
{
#if 0 // this is not needed for now; and all memory managers allocate aligned memory
	static bool NeedsDestructor()
	{
		return true;
	}
	static bool NeedsDefaultConstructor()
	{
		return true;
	}
	static bool NeedsCopyConstructor()
	{
		return true;
	}
	// returns true if this class is copied via "operator =" (i.e. it's not bitwise copyable via memcpy())
	static bool NeedsAssignment()
	{
		return true;
	}
	static size_t GetAlignment()
	{
		return EFFICIENT_ALIGNMENT;
	}
#else
	// Plain Old Data types don't need constructors, destructors and assignment operators being called.
	// They can be memset and memcopied.
	enum
	{
		IsPlainOldDataType = false
	};
#endif
};

#define mxDECLARE_POD_TYPE( TYPE )\
	template< > struct TypeTrait< TYPE > {\
		enum { IsPlainOldDataType = true };\
	};\
	template< > struct TypeTrait< const TYPE > {\
		enum { IsPlainOldDataType = true };\
	};\
	inline AStreamWriter& operator << ( AStreamWriter &outStream, const TYPE& o )\
	{\
		outStream.Write( &o, sizeof(TYPE) );\
		return outStream;\
	}\
	inline AStreamReader& operator >> ( AStreamReader& inStream, TYPE &o )\
	{\
		inStream.Read( &o, sizeof(TYPE) );\
		return inStream;\
	}\
	template< class SERIALIZER >\
	inline SERIALIZER& operator & ( SERIALIZER & serializer, TYPE & o )\
	{\
		serializer.SerializeMemory( &o, sizeof(TYPE) );\
		return serializer;\
	}\
	inline mxArchive& Serialize( mxArchive & serializer, TYPE & o )\
	{\
		serializer.SerializeMemory( &o, sizeof(TYPE) );\
		return serializer;\
	}\
	inline mxArchive& operator && ( mxArchive & serializer, TYPE & o )\
	{\
		serializer.SerializeMemory( &o, sizeof(TYPE) );\
		return serializer;\
	}\
	inline void F_UpdateMemoryStats( MemStatsCollector& stats, const TYPE& o )\
	{\
		stats.staticMem += sizeof o;\
	}


mxDECLARE_POD_TYPE(ANSICHAR);
mxDECLARE_POD_TYPE(UNICODECHAR);
mxDECLARE_POD_TYPE(INT8);
mxDECLARE_POD_TYPE(UINT8);
mxDECLARE_POD_TYPE(INT16);
mxDECLARE_POD_TYPE(UINT16);
mxDECLARE_POD_TYPE(INT32);
mxDECLARE_POD_TYPE(UINT32);
mxDECLARE_POD_TYPE(INT64);
mxDECLARE_POD_TYPE(UINT64);
mxDECLARE_POD_TYPE(LONG);
mxDECLARE_POD_TYPE(ULONG);
mxDECLARE_POD_TYPE(FLOAT);
mxDECLARE_POD_TYPE(DOUBLE);

//
//	TEnum< enumName, storage > - a wrapper to store an enum with explicit size.
//
template< typename ENUM, typename STORAGE = UINT32 >
class TEnum {
public:
	typedef ENUM					ENUM_TYPE;
	typedef TEnum< ENUM, STORAGE >	THIS_TYPE;

	inline TEnum()
	{}

	inline TEnum( ENUM e )
	{
		m_value = static_cast< STORAGE >( e );
	}

	inline operator ENUM () const
	{
		return static_cast< ENUM >( m_value );
	}

	inline void operator = ( ENUM e )
	{
		m_value = static_cast< STORAGE >( e );
	}

	inline bool operator == ( ENUM e ) const
	{
		return (m_value == static_cast< STORAGE >( e ));
	}
	inline bool operator != ( ENUM e ) const
	{
		return (m_value != static_cast< STORAGE >( e ));
	}

	inline STORAGE AsInt() const
	{
		return m_value;
	}

	// ??? overload 'forbidden' operators like '+', '-', '/', '<<', etc
	// to catch programming errors early?
	inline friend AStreamWriter& operator << ( AStreamWriter& file, const THIS_TYPE& o )
	{
		file << o.m_value;
		return file;
	}
	inline friend AStreamReader& operator >> ( AStreamReader& file, THIS_TYPE& o )
	{
		file >> o.m_value;
		return file;
	}
	inline friend mxArchive& operator && ( mxArchive & serializer, THIS_TYPE & o )
	{
		serializer && o.m_value;
		return serializer;
	}

private:
	STORAGE		m_value;
};

#if MX_DEBUG

	// view names in debugger
	#define mxDECLARE_ENUM_TYPE( enumName, storage, newType )	\
		typedef enumName	newType;

#else // !MX_DEBUG

	#define mxDECLARE_ENUM_TYPE( enumName, storage, newType )	\
		typedef TEnum< enumName, storage >	newType;

#endif

//
//	TBool - a boolean type with explicit storage type.
//
template< typename STORAGE = UINT32 >
class TBool {
public:
	TBool()
	{}

	inline	explicit TBool( bool value )
		: m_value( value )
	{}
	
	// Casting to 'bool'.

	inline operator bool () const
	{
		return ( m_value != 0 );
	}

	// Assignment.

	inline TBool & operator = ( bool value )
	{
		m_value = value;
		return *this;
	}
	inline TBool & operator = ( const TBool other )
	{
		m_value = other.m_value;
		return *this;
	}

	// Comparison.

	inline TBool operator == ( bool value ) const
	{
		return (m_value == static_cast< STORAGE >( value ));
	}
	inline TBool operator != ( bool value ) const
	{
		return (m_value != static_cast< STORAGE >( value ));
	}

	inline TBool operator == ( const TBool other ) const
	{
		return m_value == other.m_value;
	}
	inline TBool operator != ( const TBool other ) const
	{
		return m_value != other.m_value;
	}

	// TODO: overload 'forbidden' operators like '+', '-', '/', '<<', etc
	// to catch programming errors early?
	//TBool operator + (const TBool& other) const
	//{ StaticAssert( false && "Invalid operation" ); }

private:
	STORAGE		m_value;
};

//
//	ETroolean
//
//	NOTE: don't modify the values of these constants!
//
enum ETroolean
{
	False	= 0,
	True	= 1,
	DontCare		// Whatever/don't care value
};

typedef TEnum<
	ETroolean,
	int
> mxTrool;

enum ThreeState_t
{
	TRS_FALSE = 0,
	TRS_TRUE,
	TRS_NONE,
};


template< typename BOOL_TYPE >
inline
BOOL_TYPE InvertBool( BOOL_TYPE & o )
{
	return (o = !o);
}

//--------------------------------------------------------

//
//	'Bitfield' type.
//
typedef unsigned	BITFIELD;

// ToDo: http://molecularmusings.wordpress.com/2011/08/23/flags-on-steroids/#more-125
//
//	TBits - is a simple class representing a bitmask.
//
template< typename TYPE, typename STORAGE = UINT32 >
struct TBits
{
	STORAGE	m_value;

	inline TBits()
	{}
	inline TBits( TYPE e )
	{
		m_value = static_cast< STORAGE >( e );
	}
	inline explicit TBits( STORAGE value )
		: m_value( value )
	{}

	inline operator TYPE () const
	{
		return static_cast< TYPE >( m_value );
	}
	inline void operator = ( TYPE e )
	{
		m_value = static_cast< STORAGE >( e );
	}

	inline bool operator == ( TYPE e ) const
	{
		return (m_value == static_cast< STORAGE >( e ));
	}
	inline bool operator != ( TYPE e ) const
	{
		return (m_value != static_cast< STORAGE >( e ));
	}

	inline TBits & operator &= ( TBits other )
	{
		m_value &= other.m_value;
		return *this;
	}
	inline TBits & operator ^= ( TBits other )
	{
		m_value ^= other.m_value;
		return *this;
	}
	inline TBits & operator |= ( TBits other )
	{
		m_value |= other.m_value;
		return *this;
	}
	inline TBits operator ~ () const
	{
		return TBits( ~m_value );
	}
	inline bool operator ! () const
	{
		return !m_value;
	}

	inline void Clear()
	{
		m_value = 0;
	}
	inline void SetAll( STORAGE s )
	{
		m_value = s;
	}
	inline void OrWith( STORAGE s )
	{
		m_value |= s;
	}
	inline void XorWith( STORAGE s )
	{
		m_value ^= s;
	}
	inline void AndWith( STORAGE s )
	{
		m_value &= s;
	}
	inline void SetWithMask( STORAGE s, STORAGE mask )
	{
		m_value = (m_value & ~mask) | (s & mask);
	}

	inline STORAGE Get() const
	{
		return m_value;
	}
	inline STORAGE Get( STORAGE mask ) const
	{
		return m_value & mask;
	}
	inline bool AnyIsSet( STORAGE mask ) const
	{
		return (m_value & mask) != 0;
	}
	inline bool AllAreSet( STORAGE mask ) const
	{
		return (m_value & mask) == mask;
	}
};

//---------------------------------------------------------------------------

//
//	The CPUTimer is a basic clock that measures accurate time in seconds, used for profiling.
//
class CPUTimer {
public:
	CPUTimer()
	{
		Initialize();
		Reset();
	}
	~CPUTimer()
	{
	}

	/// Resets the initial reference time.
	void Reset()
	{
		::QueryPerformanceCounter(&mStartTime);
		mStartTick = ::GetTickCount();
		mPrevElapsedTime = 0;
	}

	/// Returns the time in ms since the last call to reset or since 
	/// the CPUTimer was created.
	unsigned long int GetTimeMilliseconds()
	{
		//MX_ASSERT(!MemoryIsZero(&mClockFrequency,sizeof(mClockFrequency)));

		LARGE_INTEGER currentTime;
		::QueryPerformanceCounter(&currentTime);
		LONGLONG elapsedTime = currentTime.QuadPart - 
			mStartTime.QuadPart;

		// Compute the number of millisecond ticks elapsed.
		unsigned long msecTicks = (unsigned long)(1000 * elapsedTime / 
			mClockFrequency.QuadPart);

		// Check for unexpected leaps in the Win32 performance counter.  
		// (This is caused by unexpected data across the PCI to ISA 
		// bridge, aka south bridge.  See Microsoft KB274323.)
		unsigned long elapsedTicks = ::GetTickCount() - mStartTick;
		signed long msecOff = (signed long)(msecTicks - elapsedTicks);
		if (msecOff < -100 || msecOff > 100)
		{
			// Adjust the starting time forwards.
			LONGLONG msecAdjustment = smallest(msecOff * 
				mClockFrequency.QuadPart / 1000, elapsedTime - 
				mPrevElapsedTime);
			mStartTime.QuadPart += msecAdjustment;
			elapsedTime -= msecAdjustment;

			// Recompute the number of millisecond ticks elapsed.
			msecTicks = (unsigned long)(1000 * elapsedTime / 
				mClockFrequency.QuadPart);
		}

		// Store the current elapsed time for adjustments next time.
		mPrevElapsedTime = elapsedTime;

		return msecTicks;
	}

	/// Returns the time in us since the last call to reset or since 
	/// the CPUTimer was created.
	unsigned long int GetTimeMicroseconds()
	{
		//MX_ASSERT(!MemoryIsZero(&mClockFrequency,sizeof(mClockFrequency)));

		LARGE_INTEGER currentTime;
		::QueryPerformanceCounter(&currentTime);
		LONGLONG elapsedTime = currentTime.QuadPart - 
			mStartTime.QuadPart;

		// Compute the number of millisecond ticks elapsed.
		unsigned long msecTicks = (unsigned long)(1000 * elapsedTime / 
			mClockFrequency.QuadPart);

		// Check for unexpected leaps in the Win32 performance counter.  
		// (This is caused by unexpected data across the PCI to ISA 
		// bridge, aka south bridge.  See Microsoft KB274323.)
		unsigned long elapsedTicks = ::GetTickCount() - mStartTick;
		signed long msecOff = (signed long)(msecTicks - elapsedTicks);
		if (msecOff < -100 || msecOff > 100)
		{
			// Adjust the starting time forwards.
			LONGLONG msecAdjustment = smallest(msecOff * 
				mClockFrequency.QuadPart / 1000, elapsedTime - 
				mPrevElapsedTime);
			mStartTime.QuadPart += msecAdjustment;
			elapsedTime -= msecAdjustment;
		}

		// Store the current elapsed time for adjustments next time.
		mPrevElapsedTime = elapsedTime;

		// Convert to microseconds.
		unsigned long usecTicks = (unsigned long)(1000000 * elapsedTime / 
			mClockFrequency.QuadPart);

		return usecTicks;
	}

	void Initialize()
	{
		::QueryPerformanceFrequency( &mClockFrequency );
	}

private:
	LARGE_INTEGER	mClockFrequency;
	DWORD			mStartTick;
	LONGLONG		mPrevElapsedTime;
	LARGE_INTEGER	mStartTime;
};

struct PtDateTime
{
	UINT8	year;	// years since 1900
	UINT8	month;	// months since January [0..11]
	UINT8	day;	// day of month [0..30]
	UINT8	hour;	// hours since midnight [0..23]
	UINT8	minute;	// [0..59]
	UINT8	second;	// [0..59]
public:
	PtDateTime( int year, int month, int day, int hour, int minute, int second )
	{
		this->year = year - 1900;
		this->month = month;
		this->day = day;
		this->hour = hour;
		this->minute = minute;
		this->second = second;
	}
};

inline
void ConvertMicrosecondsToMinutesSeconds(
	UINT64 microseconds,
	UINT32 &minutes, UINT32 &seconds
)
{
	const UINT64 totalSeconds = microseconds / (1000*1000);
	const UINT64 totalMinutes = totalSeconds / 60;

	minutes = totalMinutes;

	seconds = totalSeconds - totalMinutes * 60;
}
inline
void ConvertMicrosecondsToHoursMinutesSeconds(
	UINT64 microseconds,
	UINT32 &hours, UINT32 &minutes, UINT32 &seconds
)
{
	const UINT64 totalSeconds = microseconds / (1000*1000);
	const UINT64 totalMinutes = totalSeconds / 60;
	const UINT64 totalHours = totalMinutes / 60;

	hours = totalHours;
	minutes = totalMinutes - totalHours * 60;
	seconds = totalSeconds - totalMinutes * 60;
}

template< class STRING >
void GetTimeOfDayString( STRING &OutTimeOfDay, int hour, int minute, int second, char separator = '-' )
{
	mxASSERT( hour >= 0 && hour <= 23 );
	mxASSERT( minute >= 0 && minute <= 59 );
	mxASSERT( second >= 0 && second <= 59 );
	//mxASSERT( OutTimeOfDay.IsEmpty() );

	STRING	szHour;
	Str::SetInt( szHour, hour );
	
	STRING	szMinute;
	Str::SetInt( szMinute, minute );

	STRING	szSecond;
	Str::SetInt( szSecond, second );

	if( hour < 10 ) {
		Str::Append( OutTimeOfDay, '0' );
	}
	Str::Append( OutTimeOfDay, szHour );
	Str::Append( OutTimeOfDay, separator );

	if( minute < 10 ) {
		Str::Append( OutTimeOfDay, '0' );
	}
	Str::Append( OutTimeOfDay, szMinute );
	Str::Append( OutTimeOfDay, separator );

	if( second < 10 ) {
		Str::Append( OutTimeOfDay, '0' );
	}
	Str::Append( OutTimeOfDay, szSecond );
}

template< class STRING >
void GetDateString( STRING &OutCurrDate, int year, int month, int day, char separator = '-' )
{
	mxASSERT( year >= 0 && year <= 9999 );
	mxASSERT( month >= 1 && month <= 12 );
	mxASSERT( day >= 1 && day <= 31 );
	//mxASSERT( OutCurrDate.IsEmpty() );

	STRING	szYear;
	szYear.SetInt( year );

	STRING	szMonth;
	szMonth.SetInt( month );

	STRING	szDay;
	szDay.SetInt( day );

	OutCurrDate += szYear;
	OutCurrDate += separator;

	if( month < 10 ) {
		OutCurrDate += '0';
	}
	OutCurrDate += szMonth;
	OutCurrDate += separator;

	if( day < 10 ) {
		OutCurrDate += '0';
	}
	OutCurrDate += szDay;
}

template< class STRING >
void GetDateTimeOfDayString(
	STRING &OutDateTime,
	const CalendarTime& dateTime = CalendarTime::GetCurrentLocalTime(),
	char separator = '-' )
{
	GetTimeOfDayString( OutDateTime, dateTime.hour, dateTime.minute, dateTime.second, separator );
	OutDateTime += '_';
	GetDateString( OutDateTime, dateTime.year, dateTime.month, dateTime.day, separator );
}

template< class STRING >
void GetCurrentDateString( STRING &OutCurrDate )
{
	//mxASSERT( OutCurrDate.IsEmpty() );
	CalendarTime	currDateTime( CalendarTime::GetCurrentLocalTime() );
	GetDateString( OutCurrDate, currDateTime.year, currDateTime.month, currDateTime.day );
}

template< class STRING >
void GetCurrentTimeString( STRING &OutCurrDate, char separator = '-' )
{
	//mxASSERT( OutCurrDate.IsEmpty() );
	CalendarTime	currDateTime( CalendarTime::GetCurrentLocalTime() );
	GetTimeOfDayString( OutCurrDate, currDateTime.hour, currDateTime.minute, currDateTime.second, separator );
}

template< class STRING >
void GetCurrentDateTimeString( STRING &OutCurrDate )
{
	//mxASSERT( OutCurrDate.IsEmpty() );
	CalendarTime	currDateTime( CalendarTime::GetCurrentLocalTime() );
	GetDateTimeOfDayString( OutCurrDate, currDateTime );
}

inline
const char* GetNumberExtension( UINT num )
{
	return	( num == 1 ) ? "st" :
			( num == 2 ) ? "nd" :
			( num == 3 ) ? "rd" :
			"th";
}

inline
const char* GetWordEnding( UINT num )
{
	return ( num < 10 ) ? "s" : "";
}

//
//	FCallback
//
typedef void FCallback( void* pUserData );


struct EmptyClass {};


template< typename A, typename B >	// where A : B
inline
void CopyStruct( A& dest, const B &src )
{
	// compile-time inheritance test
	B* dummy = static_cast<B*>(&dest);
	(void)dummy;

	mxSTATIC_ASSERT( sizeof dest >= sizeof src );
	memcpy( &dest, &src, sizeof src );
}


// Type punning.
// Always cast any type into any other type.
// (This is evil.)
//	See:
//	http://mail-index.netbsd.org/tech-kern/2003/08/11/0001.html
//	http://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Optimize-Options.html#Optimize-Options
//
template<  class TargetType, class SourceType > 
inline TargetType always_cast( SourceType x )
{
	mxCOMPILE_TIME_WARNING( sizeof(TargetType) == sizeof(SourceType), "Type sizes are different" );
	union {
		SourceType S;
		TargetType T;
	} value;

	value.S = x;
	return value.T;
}

template< typename A, typename B >
A& TypePunning( B &v )
{
    union
    {
        A *a;
        B *b;
    } pun;
    pun.b = &v;
    return *pun.a;
}

template< typename A, typename B >
const A& TypePunning( const B &v )
{
    union
    {
        const A *a;
        const B *b;
    } pun;
    pun.b = &v;
    return *pun.a;
}

//
//	TUtil_Conversion< T, U >
//
template< typename T, typename U >
class TUtil_Conversion
{
	typedef char Small;
	class Big { char dummy[2]; };
	static Small test(U);
	static Big test(...);
	static T makeT();
public:
	enum {
		exists = sizeof(test(makeT()))==sizeof(Small),
		sameType = 0
	};
};

template< typename T >
class TUtil_Conversion< T, T > {
public:
	enum {
		exists = 1,
		sameType = 1
	};
};

#define MX_SUPERSUBCLASS( T,U ) \
	TUtil_Conversion< const U*, const T* >::exists && \
	!TUtil_Conversion< const T*, const void* >::sameType

template< class T, class U>
struct SameType {
    enum {
        Result = false
    };
};

template< class T>
struct SameType<T,T> {
    enum {
        Result = true
    };
};

// you can use __is_base_of in MSCV 2008
template< typename BaseT, typename DerivedT >
struct IsRelated
{
    static DerivedT derived();
    static char test(const BaseT&); // sizeof(test()) == sizeof(char)
    static char (&test(...))[2];    // sizeof(test()) == sizeof(char[2])

	// true if conversion exists
    enum { exists = (sizeof(test(derived())) == sizeof(char)) }; 
};

// compile-time inheritance test, can be used to catch common typing errors
template< typename BASE, typename DERIVED >
inline void T_Check_Hierarchy()
{
	BASE* base;
	DERIVED* derived = static_cast<DERIVED*>( base );
	(void)derived;
}

template
<
	bool B,
	class T,
	class U
>
struct ConditionalType {
    typedef T Type;
};

template
<
	class T,
	class U
>
struct ConditionalType<false,T,U> {
    typedef U Type;
};

template< typename TYPE >
struct IncompleteType;

// Pointers are pod types
template< typename T >
struct TypeTrait<T*>
{
	enum { IsPlainOldDataType = true };
};

// Arrays
template< typename T, unsigned long N >
struct TypeTrait< T[N] >
	: public TypeTrait< T >
{};

template< typename T >
inline const T& GetConstObjectReference( const T& o ) {
	return o;
}
template< typename T >
inline const T& GetConstObjectReference( const T* o ) {
	mxASSERT_PTR(o);
	return *o;
}
template< typename T >
inline const T* GetConstObjectPointer( const T& o ) {
	return &o;
}
template< typename T >
inline const T* GetConstObjectPointer( const T* o ) {
	return o;
}

template< typename T >
inline T& GetObjectReference( T& o ) {
	return o;
}
template< typename T >
inline T& GetObjectReference( T* o ) {
	mxASSERT_PTR(o);
	return *o;
}
template< typename T >
inline T* GetObjectPointer( T& o ) {
	return &o;
}
template< typename T >
inline T* GetObjectPointer( T* o ) {
	return o;
}

struct AObjectHeader {
	void *	pVTBL;
};
struct VoidPointer {
	void *	o;
};
class PointerSizedPadding {
	void *	pad;
public:
	inline PointerSizedPadding() {
		pad = nil;
	}
};

struct Memory
{
	void *	data;
	UINT32	size;
};

template< typename TYPE1, typename TYPE2 >
inline bool IsAlignedBy( TYPE1 x, TYPE2 alignment )
{
	const size_t remainder = (size_t)x % alignment;
	return remainder == 0;
}

template< UINT ALIGN, typename TYPE >
inline TYPE TAlignUp( TYPE x )
{
	mxSTATIC_ASSERT_ISPOW2(ALIGN);
	return (x + (ALIGN - 1)) & ~(ALIGN - 1);
}

template< UINT ALIGN, typename TYPE >
inline TYPE TAlignDown( TYPE x )
{
	mxSTATIC_ASSERT_ISPOW2(ALIGN);
	return x - (x & (ALIGN - 1));
}

template< typename TYPE1, typename TYPE2 >
inline size_t AlignUp( TYPE1 x, TYPE2 alignment )
{
	const size_t remainder = x % alignment;
	return remainder ? x + (alignment - remainder) : x;
}

// use for non-POD types that can be streamed via << and >> operators.
//
#define mxIMPLEMENT_SERIALIZE_FUNCTION( className, serializeFuncName )\
	inline mxArchive& serializeFuncName( mxArchive & serializer, className & o )\
	{\
		if( AStreamWriter* saver = serializer.IsWriter() )\
		{\
			*saver << o;\
		}\
		if( AStreamReader* load = serializer.IsReader() )\
		{\
			*load >> o;\
		}\
		return serializer;\
	}\
	inline mxArchive& operator && ( mxArchive & serializer, className & o )\
	{\
		return serializeFuncName( serializer, o );\
	}

// dummy function used for in-place saving/loading
#define mxNO_SERIALIZABLE_POINTERS\
	template< class S, class P  > inline void CollectPointers( S & s, P p ) const\
	{}

//
//	Hash traits.
//

//
//	THashTrait<T> - used for generating hash values.
//
//	Should be specialized for custom types.
//
template< typename key >
struct THashTrait
{
	static inline UINT GetHashCode( const key& key )
	{
		return mxGetHashCode( key );
	}
};

//
//	TEqualsTrait - used for comparing keys in hash tables.
//
//	Must be specialized for custom types.
//
template< typename key >
struct TEqualsTrait
{
	static inline bool Equals( const key& a, const key& b )
	{
		return (a == b);
	}
};
template< typename T >
struct THashTrait< T* >
{
	static inline UINT GetHashCode( const void* pointer )
	{
		return static_cast< UINT >( mxPointerHash( pointer ) );
	}
};
template<>
struct THashTrait< INT32 >
{
	static inline UINT GetHashCode( INT32 k )
	{
		return static_cast< UINT >( k );
	}
};
template<>
struct THashTrait< UINT32 >
{
	static inline UINT GetHashCode( UINT32 k )
	{
		return static_cast< UINT >( k );
	}
};
template<>
struct THashTrait< INT64 >
{
	static inline UINT GetHashCode( INT64 k )
	{
		return static_cast< UINT >( k );
	}
};
template<>
struct THashTrait< UINT64 >
{
	static inline UINT GetHashCode( UINT64 k )
	{
		return static_cast< UINT >( k );
	}
};

//
//	TStaticBlob<T>
//
template< typename TYPE >
class TStaticBlob
{
	BYTE	storage[ sizeof(TYPE) ];
public:
	inline TStaticBlob()			{}
	inline TStaticBlob(EInitZero)	{ mxZERO_OUT( storage ); }

	inline TYPE* Ptr()				{ return c_cast(TYPE*)&storage; }
	inline const TYPE* Ptr () const	{ return c_cast(TYPE*)&storage; }

	inline TYPE& Get()				{ return *Ptr(); }
	inline const TYPE& Get () const	{ return *Ptr(); }

	inline operator TYPE& ()				{ return Get(); }
	inline operator const TYPE& () const	{ return Get(); }

	inline TYPE* Construct()	{ return new (storage) TYPE(); }
	inline void Destruct()		{ Get().~TYPE(); }
};

//TAlignedBlob
// maybe, use something like std::aligned_storage< N * sizeof(T), alignof(T) > rawStorage; ?
// @todo: keep a bool flag, check for initialized on access via Get()/Ptr()?
//
template< typename TYPE >
class TStaticBlob16
{
	mxPREALIGN(16) BYTE storage[sizeof(TYPE)] mxPOSTALIGN(16);
public:
	inline TStaticBlob16()			{}
	inline TStaticBlob16(EInitZero)	{ mxZERO_OUT( storage ); }

	inline TYPE* Ptr()				{ return c_cast(TYPE*)&storage; }
	inline const TYPE* Ptr () const	{ return c_cast(TYPE*)&storage; }

	inline TYPE& Get()				{ return *Ptr(); }
	inline const TYPE& Get () const	{ return *Ptr(); }

	inline operator TYPE& ()				{ return Get(); }
	inline operator const TYPE& () const	{ return Get(); }

	inline TYPE* Construct()	{ return ::new (&storage) TYPE(); }
	inline void Destruct()		{ Get().~TYPE(); }
};

//== MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// supply class name and ctor arguments
// it uses placement new on a static array to avoid dynamic memory allocations
//
#define mxCONSTRUCT_IN_PLACE( TYPE, ... )\
	{\
		static TStaticBlob16< TYPE >	TYPE##storage;\
		new( &TYPE##storage ) TYPE( ## __VA_ARGS__ );\
	}

//== MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
#define mxCONSTRUCT_IN_PLACE_X( PTR, TYPE, ... )\
	{\
		mxASSERT( PTR == nil );\
		static TStaticBlob16<TYPE>	TYPE##storage;\
		PTR = new( &TYPE##storage ) TYPE( ## __VA_ARGS__ );\
	}

//== MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
#if MX_DEBUG
	#define mxDECLARE_PRIVATE_DATA( STRUCT, VAR )	static TPtr< STRUCT >	VAR
	#define mxINITIALIZE_PRIVATE_DATA( VAR )		VAR.ConstructInPlace()
	#define mxSHUTDOWN_PRIVATE_DATA( VAR )			VAR.Destruct()
	#define mxGET_PRIVATE_DATA( STRUCT, VAR )		(*VAR)
#else
	#define mxDECLARE_PRIVATE_DATA( STRUCT, VAR )	static TStaticBlob16< STRUCT >	VAR
	#define mxINITIALIZE_PRIVATE_DATA( VAR )		VAR.Construct()
	#define mxSHUTDOWN_PRIVATE_DATA( VAR )			VAR.Destruct()
	#define mxGET_PRIVATE_DATA( STRUCT, VAR )		(*(STRUCT*) &(VAR))
#endif

//---------------------------------------------------------------------------

//
//	Triple
// 
template< typename T >
class Triple {
public:
	union {
		struct { T   iA, iB, iC; };
		struct { T   Points[ 3 ]; };
	};

	inline Triple()
	{}
	inline Triple( const T & A, const T & B, const T & C )
		: iA( A ), iB( B ), iC( C )
	{}
	inline T & operator [] ( UINT index )
	{
		//mxASSERT( index >= 0 && index <= 3 );
		return Points[ index ];
	}
	inline const T & operator [] ( UINT index ) const
	{
		//mxASSERT( index >= 0 && index <= 3 );
		return Points[ index ];
	}
};

/*
-----------------------------------------------------------------------------
	NiftyCounter

	Intent:
		Ensure a non-local static object is initialized before its first use
		and destroyed only after last use of the object.

	Also Known As:
		Schwarz Counter.

	To do:
		Locking policies for thread safety.
-----------------------------------------------------------------------------
*/
class NiftyCounter
{
	// this tells how many times the base system has been requested to initialize
	// (think 'reference-counting')
	//
	volatile int	m_count;

public:
	NiftyCounter()
	{
		m_count = 0;
	}
	~NiftyCounter()
	{
		//mxASSERT( m_count == 0 );
	}

	//-----------------------------------------------------
	// returns true if one time initialization is needed
	bool IncRef()
	{
		m_count++;

		if( m_count > 1 )
		{
			// already initialized
			return false;
		}

		// has been initialized for the first time

		mxASSERT(m_count == 1);

		return true;
	}
	//-----------------------------------------------------
	// returns true if one time destruction is needed
	bool DecRef()
	{
		mxASSERT(m_count > 0);

		m_count--;

		if( m_count > 0 )
		{
			// still referenced, needs to stay initialized
			return false;
		}

		mxASSERT(m_count == 0);

		// no external references, can shut down

		return true;
	}
	//-----------------------------------------------------
	int GetCount() const
	{
		return m_count;
	}
	//-----------------------------------------------------
	bool IsOpen() const
	{
		return this->GetCount() > 0;
	}
};

template< typename TYPE >
static inline
void TMemCopyArray( TYPE *dest, const TYPE* src, const UINT numItems )
{
	mxASSERT_PTR( dest );
	mxASSERT_PTR( src );
	mxASSERT( dest != src );
	mxASSERT( numItems > 0 );
	memcpy( dest, src, numItems * sizeof src[0] );
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace Array_Util
{
	// Returns true if the index is within the boundaries of this array.
	inline bool IsValidIndex( UINT index, const UINT num )
	{
		return (index >= 0) && (index < num);
	}

	template< class ARRAY, typename TYPE >
	bool RemoveElement( ARRAY & theArray, const TYPE& item )
	{
		const INT index = theArray.FindIndexOf( item );
		if( INDEX_NONE == index ) {
			return false;
		}
		theArray.RemoveAt( index );
		return true;
	}

	// Slow!
	template< typename TYPE, typename SIZETYPE >
	void RemoveAt(
		TYPE* theData, SIZETYPE& theNum,
		const UINT startIndex, const UINT count = 1 )
	{
		const UINT oldNum = theNum;

		mxASSERT( IsValidIndex( startIndex, oldNum ) );
		mxASSERT( IsValidIndex( startIndex + count - 1, oldNum ) );


		TYPE* data = theData;
		TYPE* dest = data + startIndex;
		TYPE* src = dest + count;

		// Destroy the elements.
		TDestructN_IfNonPOD( dest, count );

		const UINT numMovedElements = oldNum - startIndex - count;
		TMoveArray( dest, src, numMovedElements );

		theNum -= count;
	}

	// this method is faster (uses the 'swap trick')
	// Doesn't preserve the relative order of elements.
	//
	template< typename TYPE, typename SIZETYPE >
	inline void RemoveAt_Fast(
		TYPE* theData, SIZETYPE& theNum,
		const UINT index )
	{
		mxASSERT( IsValidIndex( index, theNum ) );

		TYPE* data = theData;
		const UINT num = --theNum;

		// swap with last element, and destroy the last element
		data[ index ] = data[ num ];

		TDestructOne_IfNonPOD( data[ num ] );
	}

	// deletes the last element
	template< typename TYPE, typename SIZETYPE >
	inline void PopLast(
		TYPE* theData, SIZETYPE& theNum
		)
	{
		--theNum;

		// remove and destroy the last element
		TDestructOne_IfNonPOD( theData[ theNum ] );
	}
}

/*
-----------------------------------------------------------------------------
	TArrayBase< TYPE, DERIVED >

	this mixin implements common array functions,
	it doesn't contain any member variables.

	the derived class must have the following functions
	(you'll get a compile-time error or stack overflow if it does't):

	'UINT Num() const'- returns number of elements in the array
	'TYPE* ToPtr()' - returns a pointer to the contents
	'const TYPE* ToPtr() const' - returns a pointer to the contents

	@todo: check if it causes any code bloat
-----------------------------------------------------------------------------
*/
template< typename TYPE, class DERIVED >	// where DERIVED : TArrayBase< TYPE, DERIVED >
struct TArrayBase
{
	// Convenience function to get the number of elements in this array.
	// Returns the size (the number of elements in the array).
	inline UINT32 Num() const
	{
		return this->AsDerived()->Num();
	}
	inline TYPE* ToPtr()
	{
		return this->AsDerived()->ToPtr();
	}
	inline const TYPE* ToPtr() const
	{
		return this->AsDerived()->ToPtr();
	}

public:
	// Checks if the size is zero.
	inline bool IsEmpty() const
	{
		return !this->Num();
	}

	inline bool NonEmpty() const
	{
		return this->Num();
	}

	// Returns the size of a single element, in bytes.
	inline UINT32 GetItemSize() const
	{
		return sizeof(TYPE);
	}

	// Returns the total size of stored elements, in bytes.
	inline size_t GetDataSize() const
	{
		return this->Num() * sizeof(TYPE);
	}

	// Returns true if the index is within the boundaries of this array.
	inline bool IsValidIndex( UINT32 index ) const
	{
		return index < this->Num();
	}

	inline TYPE & GetFirst()
	{
		mxASSERT(this->Num()>0);
		return this->ToPtr()[ 0 ];
	}
	inline const TYPE& GetFirst() const
	{
		mxASSERT(this->Num()>0);
		return this->ToPtr()[ 0 ];
	}

	inline TYPE & GetLast()
	{
		mxASSERT(this->Num()>0);
		return this->ToPtr()[ this->Num()-1 ];
	}
	inline const TYPE& GetLast() const
	{
		mxASSERT(this->Num()>0);
		return this->ToPtr()[ this->Num()-1 ];
	}

	inline void SetLast( const TYPE& newValue )
	{
		mxASSERT(this->Num()>0);
		this->ToPtr()[ this->Num()-1 ] = newValue;
	}

	inline UINT32 GetLastIndex() const
	{
		mxASSERT(this->Num()>0);
		return this->Num()-1;
	}

	// Returns the index of the first occurrence of the given element, or INDEX_NONE if not found.
	// Slow! (uses linear search)
	UINT32 FindIndexOf( const TYPE& element ) const
	{
		const UINT32 num = this->Num();
		const TYPE* data = this->ToPtr();
		for( UINT32 i = 0; i < num; i++ ) {
			if( data[ i ] == element ) {
				return i;
			}
		}
		return INDEX_NONE;
	}

	bool Contains( const TYPE& element ) const
	{
		const UINT32 num = this->Num();
		const TYPE* data = this->ToPtr();
		for( UINT32 i = 0; i < num; i++ ) {
			if( data[ i ] == element ) {
				return true;
			}
		}
		return false;
	}

	// assumes that this array stores pointers
	template< typename U >
	UINT32 FindPtrIndex( const U* pointer ) const
	{
		const UINT32 num = this->Num();
		const TYPE* data = this->ToPtr();
		for( UINT32 i = 0; i < num; i++ ) {
			if( data[ i ] == pointer ) {
				return i;
			}
		}
		return INDEX_NONE;
	}

	// Returns index of the last occurrence of the given element, or INDEX_NONE if not found.
	UINT32 LastIndexOf( const TYPE& element ) const
	{
		const UINT32 num = this->Num();
		const TYPE* data = this->ToPtr();
		for(UINT32 i = num-1; i >= 0; i++) {
			if( data[ i ] == element ) {
				return i;
			}
		}
		return INDEX_NONE;
	}

	// Searches for the given element and returns a pointer to it, or NULL if not found.
	TYPE* Find( const TYPE& element )
	{
		const UINT32 num = this->Num();
		const TYPE* data = this->ToPtr();

		for( UINT32 i = 0; i < num; i++ ) {
			if( data[ i ] == element ) {
				return &(data[ i ]);
			}
		}
		return nil;
	}

	UINT32 CountOf( const TYPE& element ) const
	{
		UINT32 counter = 0;
		const UINT32 num = this->Num();
		const TYPE* data = this->ToPtr();
		for( UINT32 i = 0; i < num; i++ ) {
			if( data[ i ] == element ) {
				counter++;
			}
		}
		return counter;
	}

	// Read/write access to the i'th element.
	inline TYPE & operator [] ( UINT32 i )
	{
		mxASSERT( this->IsValidIndex( i ) );
		return this->ToPtr()[ i ];
	}
	// Read only access to the i'th element.
	inline const TYPE& operator [] ( UINT32 i ) const
	{
		mxASSERT( this->IsValidIndex( i ) );
		return this->ToPtr()[ i ];
	}

	// Returns null if the index is out of array bounds.
	inline TYPE* SafeGetItemPtr( UINT32 index )
	{
		return this->IsValidIndex( index ) ? this->ToPtr() + index : nil;
	}
	// use it with pointer types only
	inline TYPE SafeGetItemRef( UINT32 index )
	{
		return this->IsValidIndex( index ) ? this->ToPtr()[ index ] : (TYPE)nil;
	}
	inline TYPE* SafeGetFirstItemPtr()
	{
		return ( this->Num() > 0 ) ? this->ToPtr() : nil;
	}
	inline const TYPE* SafeGetFirstItemPtr() const
	{
		return ( this->Num() > 0 ) ? this->ToPtr() : nil;
	}

	inline TYPE & At( UINT32 index )
	{
		mxASSERT( this->IsValidIndex( index ) );
		return this->ToPtr()[ index ];
	}
	inline const TYPE& At( UINT32 index ) const
	{
		mxASSERT( this->IsValidIndex( index ) );
		return this->ToPtr()[ index ];
	}

	inline TYPE * GetItemPtr( UINT32 index )
	{
		mxASSERT( this->IsValidIndex( index ) );
		return this->ToPtr() + index;
	}
	inline const TYPE* GetItemPtr( UINT32 index ) const
	{
		mxASSERT( this->IsValidIndex( index ) );
		return this->ToPtr() + index;
	}

	inline UINT32 GetContainedItemIndex( const TYPE* o ) const
	{
		const TYPE* start = this->ToPtr();
		const UINT32 index = o - start;
		mxASSERT( this->IsValidIndex( index ) );
		// @fixme: ideally, it should be:
		// (ptrdiff_t)((size_t)o - (size_t)start) / sizeof(TYPE);
		return index;
	}

	inline void ZeroOut()
	{
		//mxASSERT(this->Num()>0);
		MemZero( this->ToPtr(), this->GetDataSize() );
	}

	//Use with care!
	//inline void CopyFrom( const void* src, size_t size )
	//{
	//	memcpy( this->ToPtr(), src, size );
	//}
	//template< typename U, size_t N >
	//inline void CopyFromArray( const U (&src)[N] )
	//{
	//	memcpy( this->ToPtr(), src, sizeof src );
	//}

	// Set all items of this array to the given value.
	void SetAll( const TYPE& theValue )
	{
		const UINT32 num = this->Num();
		TYPE* arr = this->ToPtr();

		for( size_t i = 0; i < num; ++i )
		{
			arr[ i ] = theValue;
		}
	}

	template< class OTHER_ARRAY >
	bool Equals( const OTHER_ARRAY& other ) const
	{
		mxASSERT(this->ToPtr() != other.ToPtr());
		if( this->Num() != other.Num() ) {
			return false;
		}
		const UINT32 num = this->Num();
		const TYPE* arr1 = this->ToPtr();
		const TYPE* arr2 = other.ToPtr();
		for( UINT32 i = 0; i < num; i++ ) {
			if( arr1[ i ] != arr2[ i ] ) {
				return false;
			}
		}
		return true;
	}

public:	// Iterators and Algorithms.

	typedef TYPE* Iterator;

	inline Iterator GetStart() const { return this->ToPtr(); }
	inline Iterator GetEnd() const { return this->ToPtr() + this->Num(); }

	// Algorithms.

	template< class FUNCTOR >
	void Do_ForEach( FUNCTOR& functor, UINT32 startIndex, UINT32 endIndex )
	{
		mxASSERT(this->IsValidIndex(startIndex));
		mxASSERT(this->IsValidIndex(endIndex));

		for( UINT32 i = startIndex; i < endIndex; i++ ) {
			functor( (this*)[ i ] );
		}
	}

	template< class FUNCTOR >
	void Do_ForAll( FUNCTOR& functor )
	{
		const UINT32 num = this->Num();
		TYPE* arr = this->ToPtr();

		for( UINT32 i = 0; i < num; i++ ) {
			functor( arr[ i ] );
		}
	}

	template< class FUNCTOR >
	TYPE* Do_FindFirst( FUNCTOR& functor )
	{
		const UINT32 num = this->Num();
		TYPE* arr = this->ToPtr();

		for( UINT32 i = 0; i < num; i++ ) {
			if( functor( arr[i] ) ) {
				return &arr[i];
			}
		}
		return nil;
	}

	template< class FUNCTOR >
	INT Do_FindFirstIndex( FUNCTOR& functor )
	{
		const UINT32 num = this->Num();
		TYPE* arr = this->ToPtr();

		for( UINT32 i = 0; i < num; i++ ) {
			if( functor( arr[i] ) ) {
				return i;
			}
		}
		return INDEX_NONE;
	}

	// Invokes objects' destructors.
	void DestroyContents()
	{
		const UINT32 num = this->Num();
		TYPE* arr = this->ToPtr();

		TDestructN_IfNonPOD( arr, num );
	}

	// assuming that the stored elements are pointers,
	// deletes them.
	// NOTE: don't forget to empty the array afterwards, if needed.
	//
	void DeleteContents()
	{
		const UINT32 num = this->Num();
		TYPE* arr = this->ToPtr();

		for( UINT32 i = 0; i < num; i++ ) {
			delete arr[i];
		}
	}

protected:
	inline DERIVED* AsDerived()
	{
		return static_cast< DERIVED* >( this );
	}
	inline const DERIVED* AsDerived() const
	{
		return static_cast< const DERIVED* >( this );
	}
};

template< typename TYPE >
struct TArrayHead : TArrayBase< typename TYPE, TArrayHead< TYPE > >
{
	TYPE *	items;
	UINT32	count;
public:
	// TArrayBase interface
	inline UINT Num() const { return count; }
	inline TYPE* ToPtr() { return items; }
	inline const TYPE* ToPtr() const { return items; }
};

//------------------------------------------------------------------------------

//
// dynamically-sized raw memory blob
//
mxDEPRECATED
class ByteBuffer
	: public TArrayBase< char, ByteBuffer >
{
	// data allocated from memory heap
	char* ptr;
	size_t size;
	size_t allocSize;

public:
	typedef BYTE ITEM_TYPE;

	/// default constructor
	explicit ByteBuffer();
	/// constructor
	explicit ByteBuffer(const void* ptr, size_t size);
	/// reserve N bytes
	explicit ByteBuffer(size_t size);
	/// copy constructor
	explicit ByteBuffer(const ByteBuffer& rhs);
	/// destructor
	~ByteBuffer();
	/// assignment operator
	void operator=(const ByteBuffer& rhs);

	/// equality operator
	bool operator==(const ByteBuffer& rhs) const;
	/// inequality operator
	bool operator!=(const ByteBuffer& rhs) const;
	/// greater operator
	bool operator>(const ByteBuffer& rhs) const;
	/// less operator
	bool operator<(const ByteBuffer& rhs) const;
	/// greater-equal operator
	bool operator>=(const ByteBuffer& rhs) const;
	/// less-eqial operator
	bool operator<=(const ByteBuffer& rhs) const;

	/// return true if the blob contains data
	bool IsValid() const;
	/// reserve N bytes
	void Reserve(size_t size);
	/// trim the size member (without re-allocating!)
	void Trim(size_t size);
	/// set blob contents
	void Set(const void* ptr, size_t size);

	/// get blob size
	size_t GetDataSize() const;
	/// get a hash code (compatible with Util::HashTable)
	UINT HashCode() const;

	void SetSize( size_t size );

public:	//=== TArrayBase

	inline UINT Num() const
	{
		return this->size;
	}
	inline UINT Capacity() const
	{
		return this->allocSize;
	}
	inline char* ToPtr()
	{
		mxASSERT(this->IsValid());
		return this->ptr;
	}
	inline const char* ToPtr() const
	{
		mxASSERT(this->IsValid());
		return this->ptr;
	}

public:
	friend AStreamWriter& operator << ( AStreamWriter& file, const ByteBuffer & o );
	friend AStreamReader& operator >> ( AStreamReader& file, ByteBuffer & o );
	friend mxArchive& operator && ( mxArchive& archive, ByteBuffer & o );

private:
	/// delete content
	void Delete();
	/// allocate internal buffer
	void Allocate(size_t size);
	// grow if needed
	void Reallocate( size_t newSize );
	/// copy content
	void Copy(const void* ptr, size_t size);
	/// do a binary comparison between this and other blob
	int BinaryCompare(const ByteBuffer& rhs) const;
};

/*
-----------------------------------------------------------------------------
	MemoryBlobWriter
-----------------------------------------------------------------------------
*/
mxDEPRECATED
class MemoryBlobWriter : public AStreamWriter
{
	ByteBuffer & m_memBlob;
	size_t m_currOffset;

public:
	MemoryBlobWriter( ByteBuffer & memBlob, UINT32 startOffset = 0 )
		: m_memBlob( memBlob )
	{
		this->DbgSetName( "MemoryBlobWriter" );
		m_currOffset = startOffset;
	}
	virtual ERet Write( const void *pSrc, size_t size ) override
	{
		const size_t totalBytes =  m_currOffset + size;
		m_memBlob.Reserve( totalBytes );
		memcpy( m_memBlob.ToPtr() + m_currOffset, pSrc, size );
		m_currOffset = totalBytes;
		return ALL_OK;
	}
	virtual void VPreallocate( UINT32 sizeHint ) override
	{
		m_memBlob.SetSize( sizeHint );
	}
	inline const char* ToPtr() const
	{
		return m_memBlob.ToPtr();
	}
	inline char* ToPtr()
	{
		return m_memBlob.ToPtr();
	}
	inline size_t Tell() const
	{
		return m_currOffset;
	}
	inline void Rewind()
	{
		m_currOffset = 0;
	}
	inline void Seek( size_t absOffset )
	{
		m_memBlob.Reserve( absOffset );
		m_currOffset = absOffset;
	}
private:	PREVENT_COPY( MemoryBlobWriter );
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


template< class TYPE >
struct TStaticCounter
{
	inline TStaticCounter()
	{
		ms__numInstances++;
		ms__maxInstances++;
	}
	inline ~TStaticCounter()
	{
		ms__numInstances--;
	}
	static inline UINT NumInstances() { return ms__numInstances; }
	static inline UINT MaxInstances() { return ms__maxInstances; }
private:
	static UINT	ms__numInstances;
	static UINT	ms__maxInstances;
};
template< typename TYPE >
UINT TStaticCounter< TYPE >::ms__numInstances = 0;
template< typename TYPE >
UINT TStaticCounter< TYPE >::ms__maxInstances = 0;


template< class TYPE >
struct TCountedObject
{
	inline TCountedObject()
		: m__serialNumber( ms__numInstances++ )
	{
	}

	const UINT	m__serialNumber;
private:
	static UINT	ms__numInstances;
};
template< typename TYPE >
UINT TCountedObject< TYPE >::ms__numInstances = 0;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

mxSWIPED("Valve's Source Engine/ L4D 2 SDK");
//-----------------------------------------------------------------------------
// Declares a type-safe handle type; you can't assign one handle to the next
//-----------------------------------------------------------------------------

// 32-bit pointer handles.

// Typesafe 8-bit and 16-bit handles.
template< class HandleType >
class CBaseIntHandle
{
public:
	inline bool	operator==( const CBaseIntHandle &other )	{ return m_Handle == other.m_Handle; }
	inline bool	operator!=( const CBaseIntHandle &other )	{ return m_Handle != other.m_Handle; }

	// Only the code that doles out these handles should use these functions.
	// Everyone else should treat them as a transparent type.
	inline HandleType	GetHandleValue() const				{ return m_Handle; }
	inline void		SetHandleValue( HandleType val )	{ m_Handle = val; }

	typedef HandleType	HANDLE_TYPE;

protected:
	HandleType	m_Handle;
};

template< class DummyType >
class CIntHandle16 : public CBaseIntHandle< UINT16 >
{
public:
	inline			CIntHandle16() {}

	static inline	CIntHandle16<DummyType> MakeHandle( HANDLE_TYPE val )
	{
		return CIntHandle16<DummyType>( val );
	}

protected:
	inline			CIntHandle16( HANDLE_TYPE val )
	{
		m_Handle = val;
	}
};


template< class DummyType >
class CIntHandle32 : public CBaseIntHandle< UINT32 >
{
public:
	inline			CIntHandle32() {}

	static inline	CIntHandle32<DummyType> MakeHandle( HANDLE_TYPE val )
	{
		return CIntHandle32<DummyType>( val );
	}

protected:
	inline			CIntHandle32( HANDLE_TYPE val )
	{
		m_Handle = val;
	}
};


// NOTE: This macro is the same as windows uses; so don't change the guts of it
#define DECLARE_HANDLE_16BIT(name)	typedef CIntHandle16< struct name##__handle * > name;
#define DECLARE_HANDLE_32BIT(name)	typedef CIntHandle32< struct name##__handle * > name;

#define DECLARE_POINTER_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#define FORWARD_DECLARE_HANDLE(name) typedef struct name##__ *name


// (-1) represents a null value
// (0) (usually) represents the default value
#define mxIMPLEMENT_HANDLE( HANDLE_NAME, STORAGE )\
	struct HANDLE_NAME\
	{\
		STORAGE		id;\
	public:\
		inline void SetNil()	{ this->id = (STORAGE)~0; }\
		inline void SetDefault(){ this->id = (STORAGE) 0; }\
		inline bool IsNull() const	{ return this->id == (STORAGE)~0; }\
		inline bool IsValid() const	{ return this->id != (STORAGE)~0; }\
		inline bool operator == ( const HANDLE_NAME& other ) const { return this->id == other.id; }\
		inline bool operator != ( const HANDLE_NAME& other ) const { return this->id != other.id; }\
	};


#define mxDECLARE_8BIT_HANDLE( HANDLE_NAME )	mxIMPLEMENT_HANDLE( HANDLE_NAME, UINT8 )
#define mxINVALID_8BIT_HANDLE					{ MAX_UINT8 }

#define mxDECLARE_16BIT_HANDLE( HANDLE_NAME )	mxIMPLEMENT_HANDLE( HANDLE_NAME, UINT16 )
#define mxINVALID_16BIT_HANDLE					{ MAX_UINT16 }

#define mxDECLARE_32BIT_HANDLE( HANDLE_NAME )	mxIMPLEMENT_HANDLE( HANDLE_NAME, UINT32 )
#define mxINVALID_32BIT_HANDLE					{ MAX_UINT32 }

#define mxDECLARE_POINTER_HANDLE( HANDLE_NAME )	struct HANDLE_NAME { void* ptr; }
#define mxINVALID_POINTER_HANDLE				{ nil }

// The runtime unique identifier assigned to each object by the corresponding Object System.
// ObjectHandle may not be the same when saving/loading.
// ObjectHandle is mostly used in runtime for fast and unique identification of objects.
//
typedef UINT32 ObjectHandle;

// Use INDEX_NONE to indicate invalid ObjectHandle's.

// Object ID
class OID
{
	UINT32	m_handleValue;

public:
	inline OID()
	{}
	inline explicit OID( UINT i )
	{ m_handleValue = i; }

	inline bool operator == ( UINT i ) const
	{ return m_handleValue == i; }

	inline bool operator != ( UINT i ) const
	{ return m_handleValue != i; }

	// Only the code that doles out these handles should use these functions.
	// Everyone else should treat them as a transparent type.
	inline UINT32		GetHandleValue() const			{ return m_handleValue; }
	inline void	SetHandleValue( UINT32 newValue )	{ m_handleValue = newValue; }
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



// type of object is stored in 8 bits
struct TypedHandleBase
{
	union	// arrrgh... union cannot be used as a base class so we wrap it in a struct.
	{
		struct
		{
			BITFIELD	index : 24;
			BITFIELD	type : 8;
		};
		UINT32		value;
	};
};

ASSERT_SIZEOF(TypedHandleBase, sizeof(UINT32));



template< typename TYPE, typename ENUM, int INVALID_VALUE = -1 >
struct TypedHandle : TypedHandleBase
{
	typedef TypedHandle<TYPE,ENUM,INVALID_VALUE>	THIS_TYPE;

	inline TypedHandle()
	{
	}
	inline TypedHandle( ENUM eType, UINT nIndex )
	{
		this->index = nIndex;
		this->type = eType;
	}

	// NOTE: implicit conversions are intentionally disabled
	//
#if 0
	T& operator*();
	T* operator->();
	const T& operator*() const;
	const T* operator->() const;
#endif

	// Dereferencing.

	inline TYPE* ToPtr() const
	{
		return TYPE::Static_GetPointerByHandle( *this );
	}

	inline ENUM GetType() const
	{
		return static_cast<ENUM>( this->type );
	}

	inline void operator = ( const THIS_TYPE& other )
	{
		this->value = other.value;
	}
	inline bool operator == ( const THIS_TYPE& other ) const
	{
		this->value == other.value;
	}
	inline bool operator != ( const THIS_TYPE& other ) const
	{
		this->value != other.value;
	}

	inline bool IsValid() const
	{
		return this->value != INVALID_VALUE;
	}
	inline void SetInvalid()
	{
		this->value = INVALID_VALUE;
	}
};








/*
-----------------------------------------------------------------------------
	Data structure used to do the lookup from ID to system object.
	Requirements that need to be fulfilled:
	- There should be a 1-1 mapping between live objects and IDs.
	- If the system is supplied with an ID to an old object, it should be able to detect that the object is no longer alive.
	- Lookup from ID to object should be very fast (this is the most common operation).
	- Adding and removing objects should be fast.
-----------------------------------------------------------------------------
*/
#if 0
mxSWIPED("http://bitsquid.blogspot.com/2011/09/managing-decoupling-part-4-id-lookup.html");

enum { INDEX_MASK = 0xffff };
enum { NEW_OBJECT_ID_ADD = 0x10000 };

struct SObjIndex
{
	ObjectHandle id;
	UINT16 index;
	UINT16 next;
};
mxDECLARE_POD_TYPE(SObjIndex);

/*
 The ID Lookup Table with a fixed array size, a 32 bit ID and a FIFO free list.
*/
template< class OBJECT, UINT MAX_OBJECTS = 64*1024 >
struct TObjectSystem
{
	typedef TStaticArray<OBJECT,MAX_OBJECTS>	ObjectsArray;
	typedef TStaticArray<SObjIndex,MAX_OBJECTS>	IndicesArray;

	UINT			m_num_objects;
	ObjectsArray	m_objects;
	IndicesArray	m_indices;
	UINT			m_freelist_enqueue;
	UINT			m_freelist_dequeue;

public:
	typedef TObjectSystem THIS_TYPE;

	TObjectSystem()
	{
		m_num_objects = 0;
		for( UINT i=0; i<MAX_OBJECTS; ++i )
		{
			m_indices[i].id = i;
			m_indices[i].next = i+1;
		}
		m_freelist_dequeue = 0;
		m_freelist_enqueue = MAX_OBJECTS-1;
	}

	inline bool has( ObjectHandle id )
	{
		//mxASSERT(m_indices.IsValidIndex(id));
		SObjIndex &in = m_indices[id & INDEX_MASK];
		return in.id == id && in.index != USHRT_MAX;
	}

	inline OBJECT& lookup( ObjectHandle id )
	{
		return m_objects[ m_indices[ id & INDEX_MASK ].index ];
	}
	inline const OBJECT& lookup_const( ObjectHandle id ) const
	{
		return m_objects[ m_indices[ id & INDEX_MASK ].index ];
	}

	inline ObjectHandle Add()
	{
		mxASSERT(m_num_objects < m_objects.Capacity());

		SObjIndex & in = m_indices[ m_freelist_dequeue ];
		m_freelist_dequeue = in.next;
		in.id += NEW_OBJECT_ID_ADD;
		in.index = m_num_objects++;

		OBJECT &o = m_objects[ in.index ];
		o.id = in.id;
		return o.id;
	}

	inline UINT Num() const
	{
		return m_num_objects;
	}
	inline bool IsFull() const
	{
		//return m_num_objects == MAX_OBJECTS-1;
		return m_num_objects == MAX_OBJECTS;
	}
	inline OBJECT & GetObjAt( UINT index )
	{
		return m_objects[ index ];
	}
	inline const OBJECT & GetObjAt( UINT index ) const
	{
		return m_objects[ index ];
	}

	inline void remove( ObjectHandle id )
	{
		//mxASSERT(m_indices.IsValidIndex(id));

		SObjIndex &in = m_indices[id & INDEX_MASK];

		OBJECT &o = m_objects[in.index];
		o = m_objects[--m_num_objects];
		m_indices[o.id & INDEX_MASK].index = in.index;

		in.index = USHRT_MAX;
		m_indices[m_freelist_enqueue].next = id & INDEX_MASK;
		m_freelist_enqueue = id & INDEX_MASK;
	}

public:
	friend inline mxArchive& operator && ( mxArchive & archive, THIS_TYPE & o )
	{
		archive && o.m_num_objects;
		archive && o.m_objects;
		archive && o.m_indices;
		archive && o.m_freelist_enqueue;
		archive && o.m_freelist_dequeue;
		return archive;
	}
};
#endif

//
// Handle manager which always keeps objects in a fixed-size contiguous block of memory
// and supports removal of elements.
// NOTE: objects must be POD types!
//
// code based on:
// "http://bitsquid.blogspot.com/2011/09/managing-decoupling-part-4-id-lookup.html");
//

// See: http://c0de517e.blogspot.com/2011/03/alternatives-to-object-handles.html
// Permutation array.
// We add a second indirection level, to be able to sort resources to cause less cache misses.
// An handle in an index into an array of indices (permutation) of the array that stores the resources.

// this structure introduces another level of indirection
// so that object data stays contiguous after deleting elements at random places
//
struct SRemapObjIndex
{
	UINT16	objectIndex;	// index into array of objects
	UINT16	nextFreeSlot;	// index of the next free slot (in the indices array)

public:
	inline bool IsFree() const { return objectIndex != -1; }
};
mxDECLARE_POD_TYPE(SRemapObjIndex);

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


// 'Tagged' pointer which uses the lowest bit for storing user-defined flag.
// NOTE: the pointer must be at least 2-byte aligned!
//
template< typename TYPE >
class TypedPtr2
{
	ULONG		m_ptr;	// LSB stores user tag

public:
	enum { PTR_MASK = (ULONG)((-1)<<1) };
	enum { TAG_MASK = ~PTR_MASK };

	inline void Set( TYPE* ptr, UINT tag = 0 )
	{
		mxASSERT(IS_8_BYTE_ALIGNED( ptr ));
		mxASSERT( tag < (1<<1) );	// must fit into 1 bit
		const ULONG ptrAsInt = (ULONG)ptr;
		m_ptr = ptrAsInt | tag;
	}
	inline TYPE* GetPtr() const
	{
		return c_cast(TYPE*)(m_ptr & PTR_MASK);
	}
	inline UINT GetTag() const
	{
		return (m_ptr & TAG_MASK);
	}
};

// 'Tagged' pointer which uses lower 4 bits for storing additional data.
// NOTE: the pointer must be at least 16-byte aligned!
//
template< typename TYPE >
class TypedPtr16
{
	ULONG		m_ptr;	// 4 LSBs store user tag

public:
	enum { PTR_MASK = (ULONG)((-1)<<4) };
	enum { TAG_MASK = ~PTR_MASK };

	inline void Set( TYPE* ptr, UINT tag = 0 )
	{
		mxASSERT(IS_16_BYTE_ALIGNED( ptr ));
		mxASSERT( tag < (1<<4) );	// must fit into 4 bits

		const ULONG ptrAsInt = (ULONG)ptr;
		m_ptr = ptrAsInt | tag;
	}
	inline TYPE* GetPtr() const
	{
		return c_cast(TYPE*)(m_ptr & PTR_MASK);
	}
	inline UINT GetTag() const
	{
		return (m_ptr & TAG_MASK);
	}
};

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// represents a relative offset of some structure in memory
template
<
	typename OBJECT_TYPE,	// type of the pointed object
	typename OFFSET_TYPE = INT32	//<= NOTE: offset must be signed so that we can go in both directions!
>
struct TOffset
{
	OFFSET_TYPE		offset;	// (signed) byte offset of the structure relative to 'this'
	// null pointer is represented by a zero offset ('offset to self')
public:
	inline TOffset()
	{
		this->offset = 0;
	}

	inline bool		IsValid() const		{ return this->offset != 0; }
	inline bool		IsNull() const		{ return this->offset == 0; }

	inline const OBJECT_TYPE*	ToPtr() const	{ return (OBJECT_TYPE*)(((BYTE*)this) + offset); }
	inline OBJECT_TYPE*			ToPtr()			{ return (OBJECT_TYPE*)(((BYTE*)this) + offset); }

	inline const OBJECT_TYPE*	operator->() const	{ return this->ToPtr(); }
	inline OBJECT_TYPE*			operator->()		{ return this->ToPtr(); }
	inline const OBJECT_TYPE&	operator *() const	{ return *this->ToPtr(); }
	inline OBJECT_TYPE&			operator *()		{ return *this->ToPtr(); }

	mxUNDONE;
	//inline bool		operator ! () const	{ return !offset; }


	inline bool operator == ( const TOffset& other ) const
	{
		return this->offset == other->offset;
	}
	inline bool operator != ( const TOffset& other ) const
	{
		return this->offset != other->offset;
	}

	TOffset& operator = ( OBJECT_TYPE* o )
	{
		this->offset = PtrToBool(o) ? ((BYTE*)o - (BYTE*)this) : 0;
		return *this;
	}
};

/*
-----------------------------------------------------------------------------
	A very simple singleton template.
	Don't (mis)use singletons!
-----------------------------------------------------------------------------
*/
//
//	TGlobal< TYPE >
//
// If you want to make object of some class singleton
// you have to derive this class from TGlobal class.
//
template< class TYPE >
class TGlobal {
public:
	TGlobal()
	{
		mxASSERT2(!HasInstance(),"Singleton class has already been instantiated.");
		gInstance = static_cast< TYPE* >( this );
	}
	~TGlobal()
	{
		mxASSERT(HasInstance());
		gInstance = nil;
	}

	static bool HasInstance()
	{
		return (gInstance != nil);
	}

	static TYPE & Get()
	{
		mxASSERT(HasInstance());
		return (*gInstance);
	}

	//NOTE: doesn't perform any checks!
	static TYPE* Ptr()
	{
		return gInstance;
	}

	typedef TGlobal<TYPE> THIS_TYPE;

private:PREVENT_COPY( THIS_TYPE );
	// A static pointer to an object of T type.
	// This is the core member of the singleton. 
	// As it is declared as static no more than
	// one object of this type can exist at the time.
	//
	mxTHREAD_LOCAL static TYPE *	gInstance; // the one and only instance
};

template< typename TYPE >
mxTHREAD_LOCAL TYPE * TGlobal< TYPE >::gInstance = nil;

/*
==========================================================
	This can be used to ensure that a particular function
	only gets called one time.
==========================================================
*/
#if MX_DEBUG

	#define DBG_ENSURE_ONLY_ONE_CALL					\
	{													\
		static bool Has_Already_Been_Called = false;	\
		if ( Has_Already_Been_Called )					\
		{												\
			mxUNREACHABLE2( "singleton error" );		\
		}												\
		Has_Already_Been_Called = true;					\
	}

#else

	#define DBG_ENSURE_ONLY_ONE_CALL

#endif //MX_DEBUG

/*
==========================================================
	Helper macros to implement singleton objects:
    
    - DECLARE_SINGLETON      put this into class declaration
    - IMPLEMENT_SINGLETON    put this into the implemention file
    - CONSTRUCT_SINGLETON    put this into the constructor
    - DESTRUCT_SINGLETON     put this into the destructor

    Get a pointer to a singleton object using the static Instance() method:

    Core::Server* coreServer = Core::Server::Instance();
==========================================================
*/
#define DECLARE_SINGLETON(type) \
public: \
    mxTHREAD_LOCAL static type * TheSingleton; \
    static type * Instance() { mxASSERT(nil != TheSingleton); return TheSingleton; }; \
    static bool HasInstance() { return nil != TheSingleton; }; \
	static type & Get() { mxASSERT(nil != TheSingleton); return (*TheSingleton); }; \
private:	PREVENT_COPY(type);


#define DECLARE_INTERFACE_SINGLETON(type) \
public: \
    static type * TheSingleton; \
    static type * Instance() { mxASSERT(nil != TheSingleton); return TheSingleton; }; \
    static bool HasInstance() { return nil != TheSingleton; }; \
	static type & Get() { mxASSERT(nil != TheSingleton); return (*TheSingleton); }; \
private:


#define IMPLEMENT_SINGLETON(type) \
    mxTHREAD_LOCAL type * type::TheSingleton = nil;


#define IMPLEMENT_INTERFACE_SINGLETON(type) \
    type * type::TheSingleton = nil;


#define CONSTRUCT_SINGLETON \
    mxASSERT(nil == TheSingleton); TheSingleton = this;


#define CONSTRUCT_INTERFACE_SINGLETON \
    mxASSERT(nil == TheSingleton); TheSingleton = this;


#define DESTRUCT_SINGLETON \
    mxASSERT(TheSingleton); TheSingleton = nil;


#define DESTRUCT_INTERFACE_SINGLETON \
    mxASSERT(TheSingleton); TheSingleton = nil;

//===========================================================================

#if MX_DEBUG

	//@todo: make sure that TYPE derives from this class
	template< class TYPE >
	struct SingleInstance : NonCopyable
	{
		//static bool HasInstance() { return ms__hasBeenCreated; }
	protected:
		SingleInstance()
		{
			mxASSERT( !ms__hasBeenCreated );
			//typeid(__super) == typeid(TYPE);
			ms__hasBeenCreated = true;
		}
		~SingleInstance()
		{
			mxASSERT( ms__hasBeenCreated );
			ms__hasBeenCreated = false;
		}
	private:
		static bool ms__hasBeenCreated;
	};
	
	template< class TYPE >
	bool SingleInstance<TYPE>::ms__hasBeenCreated = false;

#else // MX_DEBUG

	template< class TYPE >
	struct SingleInstance
	{
	protected:
		SingleInstance()
		{
		}
		~SingleInstance()
		{
		}
	};

#endif // !MX_DEBUG

//===========================================================================

template< class TYPE >
struct InstanceCounter
{
	static UINT TotalNumInstances()
	{
		return msTotalNumInstances;
	}

protected:
	InstanceCounter()
	{
		++msTotalNumInstances;
	}
	~InstanceCounter()
	{
		--msTotalNumInstances;
	}
private:
	static UINT msTotalNumInstances;
};

template< typename TYPE >
UINT InstanceCounter<TYPE>::msTotalNumInstances = 0;

//===========================================================================

inline UINT BitsToBytes( UINT numBits )
{
	return (numBits >> 3) + ((numBits & 7) ? 1 : 0);
}
inline UINT BitsToUInts( UINT numBits )
{
	return (numBits >> 5) + ((numBits & 31) ? 1 : 0);
}

template< typename TYPE >
static inline void SetUpperBit( TYPE & o ) {
	o |= (1 << (sizeof(TYPE)*BITS_IN_BYTE - 1));
}
template< typename TYPE >
static inline bool GetUpperBit( const TYPE o ) {
	return o & (1 << (sizeof(TYPE)*BITS_IN_BYTE - 1));
}
template< typename TYPE >
static inline void ClearUpperBit( TYPE & o ) {
	o &= ~(1 << (sizeof(TYPE)*BITS_IN_BYTE - 1));
}

// Utility functions helpful when dealing with memory buffers and 
// pointers, especially when it is useful to go back and forth 
// between thinking of the buffer as bytes and as its type without
// a lot of casting.

// get the byte offset of B - A, as an int (64bit issues, so here for easy code checks)
inline UINT32 mxGetByteOffset32( const void* base, const void* pntr )
{
	const BYTE* from8 = (const BYTE*)base;
	const BYTE* to8 = (const BYTE*)pntr;
	const ptrdiff_t diff = to8 - from8;
	const UINT32 diff32 = (UINT32)diff;
#ifdef _M_X64
	mxASSERT( diff == (ptrdiff_t)diff32 );
#endif
	return diff32;
}

template< typename TYPE >
inline TYPE* mxAddByteOffset( TYPE* base, long offset )
{
	return reinterpret_cast<TYPE*>( reinterpret_cast<char*>(base) + offset );
}
template< typename TYPE >
inline const TYPE* mxAddByteOffset( const TYPE* base, long offset )
{
	return reinterpret_cast< const TYPE* >( reinterpret_cast< const char* >(base) + offset );
}

inline bool mxPointerInRange( const void* pointer, const void* start, const void* end )
{
	const BYTE* bytePtr = (const BYTE*) pointer;
	const BYTE* start8 = (const BYTE*) start;
	const BYTE* end8 = (const BYTE*) end;
	return (bytePtr >= start8) && (bytePtr < end8);
}
inline bool mxPointerInRange( const void* pointer, const void* start, size_t size )
{
	const BYTE* bytePtr = (const BYTE*) pointer;
	const BYTE* start8 = (const BYTE*) start;
	const BYTE* end8 = mxAddByteOffset( start8, size );
	return (bytePtr >= start8) && (bytePtr < end8);
}

//
// Test if two given memory areas are overlapping.
//
inline bool TestMemoryOverlap( const void* mem1, size_t size1, const void* mem2, size_t size2 )
{
	const BYTE* bytePtr1 = static_cast< const BYTE* >( mem1 );
	const BYTE* bytePtr2 = static_cast< const BYTE* >( mem2 );
	if ( bytePtr1 == bytePtr2 ) {
		return true;
	} else if ( bytePtr1 > bytePtr2 ) {
		return ( bytePtr2 + size2 ) > bytePtr1;
	} else {
		return ( bytePtr1 + size1 ) > bytePtr2;
	}
}

inline bool MemoryIsZero( const void* pMem, size_t size )
{
	const BYTE* ptr = (const BYTE*) pMem;
	while( size > 0 )
	{
		if( *ptr != 0 ) {
			return false ;
		}
		ptr++;
		size--;
	}
	return true;
}

//
//	IsValidAlignment
//
inline bool IsValidAlignment( size_t alignmentInBytes )
{
	return (alignmentInBytes >= MINIMUM_ALIGNMENT)
		&& (alignmentInBytes <= MAXIMUM_ALIGNMENT)
		&& (alignmentInBytes & (alignmentInBytes - 1)) == 0// alignment must be a power of two
		;
}

// Forces prefetch of memory.
inline void mxTouchMemory( void const* ptr )
{
	(void) *(char const volatile*) ptr;
}

#define IS_8_BYTE_ALIGNED( pointer )		(( (UINT_PTR)(pointer) & 7) == 0)
#define IS_16_BYTE_ALIGNED( pointer )		(( (UINT_PTR)(pointer) & 15) == 0)
#define IS_32_BYTE_ALIGNED( pointer )		(( (UINT_PTR)(pointer) & 31) == 0)
#define IS_64_BYTE_ALIGNED( pointer )		(( (UINT_PTR)(pointer) & 63) == 0)
#define IS_ALIGNED_BY( pointer, bytes )		(( (UINT_PTR)(pointer) & ((bytes) - 1)) == 0)

#define ALIGN_VALUE( val, alignment ) ( ( val + alignment - 1 ) & ~( alignment - 1 ) ) //  need macro for constant expression

#define ALIGN2( len ) (( len+1)&~1) // round up to 16 bits
#define ALIGN4( len ) (( len+3)&~3) // round up to 32 bits
#define ALIGN8( len ) (( len+7)&~7) // round up to 64 bits
#define ALIGN16( len ) (( len + 15 ) & ~15 ) // round up to 128 bits

namespace MemAlignUtil
{
	enum { alignment = 16 };	// 16 bytes

	inline size_t GetAlignedMemSize( size_t nBytes )
	{
		// we need to store a pointer to original (unaligned) memory block
		// so that we can free() it later
		// we can store it in the allocated memory block

		// Allocate a bigger buffer for alignment purpose,
		// stores the original allocated address just before the aligned buffer for a later call to free().
		const size_t expandSizeBytes = nBytes + (sizeof(void*) + (alignment-1));
		return expandSizeBytes;
	}

	inline void* GetAlignedPtr( void* allocatedMem )
	{
		BYTE* rawAddress = c_cast(BYTE*) allocatedMem;
		BYTE* alignedAddress = c_cast(BYTE*) (size_t(rawAddress + alignment + sizeof(void*)) & ~(alignment-1));

		(c_cast(void**)alignedAddress)[-1] = rawAddress;	// store pointer to original (allocated) memory block

		return alignedAddress;	// return aligned pointer
	}

	inline void* GetUnalignedPtr( void* alignedPtr )
	{
		// We need a way to go from our aligned pointer to the original pointer.
		BYTE* alignedAddress = c_cast(BYTE*) alignedPtr;

		// To accomplish this, we store a pointer to the memory returned by malloc
		// immediately preceding the start of our aligned memory block.
		BYTE* rawAddress = c_cast(BYTE*) (c_cast(void**)alignedAddress)[-1];

		return rawAddress;
	}
}

//---------------------------------------------------------------------------
// Static string that caches the length of the string.
//---------------------------------------------------------------------------

// provides array interface around raw string pointers
template< typename CHAR_TYPE, class STRING_TYPE >	// where STRING_TYPE : TStringBase< CHAR_TYPE, STRING_TYPE >
struct TStringBase : public TArrayBase< CHAR_TYPE, STRING_TYPE >
{
	// returns a *read-only* null-terminated string
	inline const CHAR_TYPE* c_str() const
	{ return this->SafeGetPtr(); }

	// returns "" if the string is empty
	inline const CHAR_TYPE* SafeGetPtr() const
	{
		return this->IsEmpty() ? mxEMPTY_STRING : this->ToPtr();
	}

	// case sensitive comparison
	template< UINT N >
	bool operator == ( const CHAR_TYPE (&_str)[N] ) const {
		return strncmp(c_str(), _str, Min(Num(), N-1)) == 0;
	}
	template< UINT N >
	bool operator != ( const CHAR_TYPE (&_str)[N] ) const {
		return !(*this == _str);
	}

	bool operator == ( const CHAR_TYPE c ) const {
		return Num() == 1 && c_str()[0] == c;
	}
	bool operator != ( const CHAR_TYPE c ) const {
		return !(*this == c);
	}
};

struct Chars : public TStringBase< char, Chars >
{
	const char *	buffer;	// pointer to the null-terminated string
	const UINT32	length;	// length (NOT including the terminator)

public:
	template< UINT N >
	inline Chars( const char (&_str)[N] )
		: buffer( _str )
		, length( N-1 )
	{}
	inline Chars( const char* text_, UINT length_ )
		: buffer( text_ ), length( length_ )
	{}
	inline explicit Chars( const char* _str )
		: buffer( _str ), length( strlen(_str) )
	{}
	inline Chars()
		: buffer( nil ), length( nil )
	{}
	inline Chars( const Chars& other )
		: buffer( other.buffer ), length( other.length )
	{}
	//=== TArrayBase
	inline UINT Num() const				{ return length; }
	inline char* ToPtr()				{ return const_cast< char* >( buffer ); }
	inline const char* ToPtr() const	{ return buffer; }
};

//--------------------------------------------------------------//
//	String constants
//--------------------------------------------------------------//

// "Unknown"
extern const char* mxSTRING_Unknown;

// string denoting unknown error
extern const char* mxSTRING_UNKNOWN_ERROR;

// "$"
extern const char* mxSTRING_DOLLAR_SIGN;
extern const char* mxSTRING_QUESTION_MARK;

// ""
extern const char* mxEMPTY_STRING;


enum { MAX_PRINTF_CHARS = 1024 };


//--------------------------------------------------------------//
//	String functions
//--------------------------------------------------------------//

/*
 * Copyright 2010-2013 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

/// Cross platform implementation of vsnprintf that returns number of
/// characters which would have been written to the final string if
/// enough space had been available.
inline int My_vsnprintf(char* _str, size_t _count, const char* _format, va_list _argList)
{
#if mxCOMPILER == mxCOMPILER_MSVC
	int len = ::vsnprintf_s(_str, _count, _TRUNCATE, _format, _argList);
	return (len < 0) ? ::_vscprintf(_format, _argList) : len;
#else
	return ::vsnprintf(_str, _count, _format, _argList);
#endif // mxCOMPILER == mxCOMPILER_MSVC
}

/// Cross platform implementation of My_vsnwprintf that returns number of
/// characters which would have been written to the final string if
/// enough space had been available.
inline int My_vsnwprintf(wchar_t* _str, size_t _count, const wchar_t* _format, va_list _argList)
{
#if BX_COMPILER_MSVC
	int len = ::_vsnwprintf_s(_str, _count, _count, _format, _argList);
	return -1 == len ? ::_vscwprintf(_format, _argList) : len;
#elif defined(__MINGW32__)
	return ::My_vsnwprintf(_str, _count, _format, _argList);
#else
	return ::vswprintf(_str, _count, _format, _argList);
#endif // BX_COMPILER_MSVC
}

inline int snprintf(char* _str, size_t _count, const char* _format, ...) // BX_PRINTF_ARGS(3, 4)
{
	va_list argList;
	va_start(argList, _format);
	int len = My_vsnprintf(_str, _count, _format, argList);
	va_end(argList);
	return len;
}

inline int swnprintf(wchar_t* _out, size_t _count, const wchar_t* _format, ...)
{
	va_list argList;
	va_start(argList, _format);
	int len = My_vsnwprintf(_out, _count, _format, argList);
	va_end(argList);
	return len;
}

//
//	mxSafeGetVarArgsANSI - returns the number of characters written or 0 if truncated.
//	appends NULL to the output buffer.
//
inline int SafeGetVarArgsANSI( ANSICHAR *outputBuffer, size_t maxChars, const ANSICHAR* formatString, va_list argList )
{
	mxASSERT( outputBuffer );
	mxASSERT( maxChars > 0 );
	mxASSERT( formatString );
	mxASSERT( argList );

	// returns the number of characters written, not including the terminating null, or a negative value if an output error occurs.
	// causes a crash in case of buffer overflow
	int result = _vsnprintf_s(
		outputBuffer,
		maxChars * sizeof(outputBuffer[0]),
		maxChars,
		formatString,
		argList
		);

#if MX_BOUNDS_CHECKS
	if( result < 0 ) {
		::DebugBreak();
		return 0;
	}
#endif

	return result;
}

template< int SIZE >
int tMy_sprintfA( char (&buf)[SIZE], const char* fmt, ... )
{
	va_list	 argPtr;
	va_start( argPtr, fmt );
	int len = My_vsnprintf( buf, mxCOUNT_OF(buf), fmt, argPtr );
	va_end( argPtr );
	return len;
}

template< int SIZE >
int tMy_sprintfW( wchar_t (&buf)[SIZE], const wchar_t* fmt, ... )
{
	va_list	 argPtr;
	va_start( argPtr, fmt );
	int len = My_vsnwprintf( buf, mxCOUNT_OF(buf), fmt, argPtr );
	va_end( argPtr );
	return len;
}

// Parameters:
//    FMT - const char*
//    ARGS - va_list
//    XXX - user statement
#define ptPRINT_VARARGS_BODY( FMT, ARGS, XXX )\
	{\
		char	tmp_[2048];\
		char *	ptr_ = tmp_;\
		int		len_ = My_vsnprintf( ptr_, mxCOUNT_OF(tmp_), FMT, ARGS );\
		/* len_ is the number of characters that would have been written,*/\
		/* not counting the terminating null character.*/\
		if( len_+1 > mxCOUNT_OF(tmp_) )\
		{\
			ptr_ = (char*) alloca(len_+1);\
			len_ = My_vsnprintf( ptr_, len_+1, FMT, ARGS );\
		}\
		ptr_[len_] = '\0';\
		XXX;\
	}\

//
//	mxGET_VARARGS_A where fmt is (char* , ...)
//
#define mxGET_VARARGS_A( buffer, fmt )\
	{\
		va_list	 argPtr;\
		va_start( argPtr, fmt );\
		SafeGetVarArgsANSI( buffer, mxCOUNT_OF(buffer), fmt, argPtr );\
		va_end( argPtr );\
	}


#if mxPLATFORM == mxPLATFORM_WINDOWS

//
//	PtUnicodeToAnsi
//
//	Converts UNICODE string to ANSI string.
//
//	pSrc [in] : Pointer to the wide character string to convert.
//	pDest [out] : Pointer to a buffer that receives the converted string.
//	destSize [in] : Size, in bytes, of the buffer indicated by pDest.
//
inline ANSICHAR* PtUnicodeToAnsi( ANSICHAR* pDest, const UNICODECHAR* pSrc, int destChars )
{
	::WideCharToMultiByte(
		CP_ACP,				// UINT     CodePage, CP_ACP - The system default Windows ANSI code page.
		0,					// DWORD    dwFlags
		pSrc,				// LPCWSTR  lpWideCharStr
		-1,					// int      cchWideChar
		pDest,				// LPSTR   lpMultiByteStr
		destChars,			// int      cbMultiByte
		NULL,				// LPCSTR   lpDefaultChar
		NULL				// LPBOOL  lpUsedDefaultChar
	);
	return pDest;
}

//
//	PtAnsiToUnicode
//
//	Converts ANSI string to UNICODE string.
//
inline UNICODECHAR* PtAnsiToUnicode( UNICODECHAR* pDest, const ANSICHAR* pSrc, int destChars )
{
	::MultiByteToWideChar(
		CP_ACP,				// UINT     CodePage, CP_ACP - The system default Windows ANSI code page.
		0,					// DWORD    dwFlags
		pSrc,				// LPCSTR   lpMultiByteStr
		-1,					// int      cbMultiByte, -1 means null-terminated source string
		pDest,				// LPWSTR  lpWideCharStr
		destChars			// int      cchWideChar
	);
	return pDest;
}

inline int GetLengthUnicode( const UNICODECHAR* pStr )
{
	return ::WideCharToMultiByte( CP_ACP, 0, pStr, -1, NULL, 0, NULL, NULL );
}

// converts ANSI string to UNICODE string; don't use for big strings or else it can crash;
// FIXME: the passed string's length is evaluated twice
//
#define mxTO_UNICODE( pSrc )\
	PtAnsiToUnicode(\
		(UNICODECHAR*)_alloca(sizeof(UNICODECHAR) * strlen(pSrc)),\
		(const ANSICHAR*)pSrc,\
		strlen(pSrc) )

// converts UNICODE string to ANSI string; don't use for big strings or else it can crash;
// FIXME: the passed string's length is evaluated twice
//
#define mxTO_ANSI( pSrc )\
	PtUnicodeToAnsi(\
		(ANSICHAR*)_alloca(sizeof(ANSICHAR) * GetLengthUnicode(pSrc)),\
		(const UNICODECHAR*)pSrc,\
		GetLengthUnicode(pSrc) )

#if UNICODE
	#define mxCHARS_AS_ANSI( pSrc )		mxTO_ANSI( pSrc )
	#define mxCHARS_AS_UNICODE( pSrc )	pSrc
#else
	#define mxCHARS_AS_ANSI( pSrc )		pSrc
	#define mxCHARS_AS_UNICODE( pSrc )	mxTO_UNICODE( pSrc )
#endif

#endif // mxPLATFORM == mxPLATFORM_WINDOWS

struct FileLine
{
	const char*	file;
	int			line;
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
