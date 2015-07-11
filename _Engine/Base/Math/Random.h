/*
=============================================================================
	File:	Random.h
	Desc:	Random number generator.
			Originally written by Id Software ( Random.h ) -
			- Copyright (C) 2004 Id Software, Inc. ( idRandom )
=============================================================================
*/

#ifndef __MATH_RANDOM_H__
#define __MATH_RANDOM_H__
mxSWIPED("idSoftware, Doom3/Prey/Quake4 SDKs");


//
//	mxRandom
//

class mxRandom {
public:
						mxRandom( INT seed = 0 );

	void				SetSeed( INT seed );
	INT					GetSeed( void ) const;

	INT					RandomInt( void );			// random integer in the range [0, MAX_RAND]
	INT					RandomInt( INT max );		// random integer in the range [0, max]
	FLOAT				RandomFloat( void );		// random number in the range [0.0f, 1.0f]
	FLOAT				CRandomFloat( void );		// random number in the range [-1.0f, 1.0f]

	FLOAT				RandomFloat( FLOAT low, FLOAT high );	// random number in the range [low, high]

	static const INT	MAX_RAND = 0x7fff;

private:
	INT					seed;
};

mxFORCEINLINE mxRandom::mxRandom( INT seed ) {
	this->seed = seed;
}

mxFORCEINLINE void mxRandom::SetSeed( INT seed ) {
	this->seed = seed;
}

mxFORCEINLINE INT mxRandom::GetSeed( void ) const {
	return seed;
}

mxFORCEINLINE INT mxRandom::RandomInt( void ) {
	seed = 69069 * seed + 1;
	return ( seed & mxRandom::MAX_RAND );
}

mxFORCEINLINE INT mxRandom::RandomInt( INT max ) {
	if ( max == 0 ) {
		return 0;			// avoid divide by zero error
	}
	return RandomInt() % max;
}

mxFORCEINLINE FLOAT mxRandom::RandomFloat( void ) {
	return ( RandomInt() / ( FLOAT )( mxRandom::MAX_RAND + 1 ) );
}

mxFORCEINLINE FLOAT mxRandom::CRandomFloat( void ) {
	return ( 2.0f * ( RandomFloat() - 0.5f ) );
}

mxFORCEINLINE FLOAT mxRandom::RandomFloat( FLOAT low, FLOAT high ) {
	return low + RandomFloat() * ( high - low );
}

/*
=============================================================================

	Random number generator

=============================================================================
*/

//
//	mxRandom2
//
class mxRandom2 {
public:
							mxRandom2( unsigned long seed = 0 );

	void					SetSeed( unsigned long seed );
	unsigned long			GetSeed( void ) const;

	INT						RandomInt( void );			// random integer in the range [0, MAX_RAND]
	INT						RandomInt( INT max );		// random integer in the range [0, max]
	FLOAT					RandomFloat( void );		// random number in the range [0.0f, 1.0f]
	FLOAT					CRandomFloat( void );		// random number in the range [-1.0f, 1.0f]

	static const INT		MAX_RAND = 0x7fff;

private:
	unsigned long			seed;

	static const unsigned long	IEEE_ONE = 0x3f800000;
	static const unsigned long	IEEE_MASK = 0x007fffff;
};

mxFORCEINLINE mxRandom2::mxRandom2( unsigned long seed ) {
	this->seed = seed;
}

mxFORCEINLINE void mxRandom2::SetSeed( unsigned long seed ) {
	this->seed = seed;
}

mxFORCEINLINE unsigned long mxRandom2::GetSeed( void ) const {
	return seed;
}

mxFORCEINLINE INT mxRandom2::RandomInt( void ) {
	seed = 1664525L * seed + 1013904223L;
	return ( (INT) seed & mxRandom2::MAX_RAND );
}

mxFORCEINLINE INT mxRandom2::RandomInt( INT max ) {
	if ( max == 0 ) {
		return 0;		// avoid divide by zero error
	}
	return ( RandomInt() >> ( 16 - Math::BitsForInteger( max ) ) ) % max;
}

mxFORCEINLINE FLOAT mxRandom2::RandomFloat( void ) {
	unsigned long i;
	seed = 1664525L * seed + 1013904223L;
	i = mxRandom2::IEEE_ONE | ( seed & mxRandom2::IEEE_MASK );
	return ( ( *(FLOAT *)&i ) - 1.0f );
}

mxFORCEINLINE FLOAT mxRandom2::CRandomFloat( void ) {
	unsigned long i;
	seed = 1664525L * seed + 1013904223L;
	i = mxRandom2::IEEE_ONE | ( seed & mxRandom2::IEEE_MASK );
	return ( 2.0f * ( *(FLOAT *)&i ) - 3.0f );
}


/*
Here's a small random number generator developed by George Marsaglia.
He's an expert in the field, so you can be confident the generator has good statistical properties.
Here u and v are unsigned ints. Initialize them to any non-zero values.
Each time you generate a random number, store u and v somewhere.
*/
mxFORCEINLINE UINT32 RandomUInt( UINT32 v, UINT32 u )
{
	v = 36969*(v & 65535) + (v >> 16);
	u = 18000*(u & 65535) + (u >> 16);
	return (v << 16) + u;
}



#endif /* !__MATH_RANDOM_H__ */

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
