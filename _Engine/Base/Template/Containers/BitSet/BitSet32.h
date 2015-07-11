/*
=============================================================================
	File:	BitSet32.h
	Desc:
=============================================================================
*/
#pragma once

struct BitSet32
{
	UINT32		v;

public:
	mxFORCEINLINE BitSet32()
	{}

	mxFORCEINLINE BitSet32( UINT32 i )
		: v( i )
	{}

	// Returns (in the lowest bit position) the bit at the given index.
	mxFORCEINLINE int	get( int index )
	{
		return v & (1 << index);
		//_bittest( v, index );
	}

	// Set the bit at the given index to 1.
	mxFORCEINLINE void set( int index )
	{
		v |= (1 << index);
	}

	// Clear the bit at the given index to 0.
	mxFORCEINLINE void clear( int index )
	{
		v &= ~(1 << index);
	}

	// Flips the bit at the given index.
	mxFORCEINLINE void flip( int index )
	{
		v ^= (1 << index);
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
		v = value;
	}
	
	mxFORCEINLINE void clearAll()
	{
		v = 0;
	}

	mxFORCEINLINE UINT Capacity() const
	{
		return BYTES_TO_BITS(sizeof(*this));
	}
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
