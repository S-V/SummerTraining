/*
=============================================================================
	File:	BitArray.cpp
	Desc:
=============================================================================
*/

#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#include <Base/Template/Containers/BitSet/BitArray.h>

/*================================
			BitArray
================================*/

BitArray::BitArray()
	: mSize( 0 )
	, mBits( nil )
{
}

BitArray::BitArray( UINT32 numBits )
	: mSize( 0 )
	, mBits( nil )
{
	Init( numBits );
}

BitArray::~BitArray()
{
	mxFree( mBits );
	mSize = 0;
}

/*
==============================================================
	BitArray::Init

	Initializes the bit array for a given number of entries.

	\param		numBits		[in] max number of entries in the array
	\return		true if success
==============================================================
*/
bool BitArray::Init( UINT32 numBits )
{
	mSize = BitsToUInts( numBits );
	// Get ram for n bits.
	mxFree( mBits );
	mBits = c_cast(UINT32*)mxAlloc( sizeof(UINT32) * mSize );
	// Set all bits to 0.
	this->ClearAll();
	return true;
}

void BitArray::SetData( UINT32* _dwords, UINT32 _count )
{
	mxFree( mBits );
	mBits = _dwords;
	mSize = _count;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
