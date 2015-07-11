/*
=============================================================================
	File:	BitSetN.h
	Desc:
=============================================================================
*/

#ifndef __MX_BIT_SET_N_H__
#define __MX_BIT_SET_N_H__

#if !mxPLATFORM_WIN32 || !mxCPU_X86
#	error Unsupported platform!
#endif

template< size_t NUMBITS >
struct BitSetN
{
	static const size_t size = ((NUMBITS + 31) / 32);

	UINT32	mBits[ size ];

	enum : UINT32 { BITS = BYTES_TO_BITS(sizeof(bits[0])) };

public:
	mxFORCEINLINE BitSetN()
	{}

	// Returns (in the lowest bit position) the bit at the given index.
	mxFORCEINLINE int	get( int iBit )
	{
		return mBits[ iBit >> 5 ] & (1 << (iBit & 31));
	}

	// Set the bit at the given index to 1.
	mxFORCEINLINE void set( int iBit )
	{
		mBits[iBit>>5] |= 1<<(iBit&31);
	}

	// Clear the bit at the given index to 0.
	mxFORCEINLINE void clear( int iBit )
	{
		mBits[iBit>>5] &= ~(1<<(iBit&31));
	}

	// Flips the bit at the given index.
	mxFORCEINLINE void flip( int iBit )
	{
		mBits[iBit>>5] ^= 1<<(iBit&31);
	}

	// Returns the index of the first set bit.
	mxFORCEINLINE unsigned firstOneBit()
	{
		DWORD index;
		// Search the mask data from least significant bit (LSB) to the most significant bit (MSB) for a set bit (1).
		_BitScanForward( &index, v );
		return index;
	}
	
	// Returns the index of the first zero bit. Slow.
	mxFORCEINLINE unsigned firstZeroBit()
	{
		DWORD inv = ~this->v;
		DWORD index;
		_BitScanForward( &index, inv );
		return index;
	}

	mxFORCEINLINE void setAll( int value )
	{
		mBits = value;
	}
	
	mxFORCEINLINE void clearAll()
	{
		mBits = 0;
	}

	mxFORCEINLINE UINT Capacity() const
	{
		return BYTES_TO_BITS(sizeof(*this));
	}
};

#endif // ! __MX_BIT_SET_N_H__

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
