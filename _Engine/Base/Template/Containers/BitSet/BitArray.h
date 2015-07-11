/*
=============================================================================
 File: BitArray.h
 Desc: Swiped from ICE (Pierre Terdiman)
=============================================================================
*/
#pragma once

mxSWIPED("ICE");

//
// BitArray - should be used instead of an array of bools.
//
struct BitArray
{
	UINT32 *	mBits;  //!< Array of bits
	UINT32		mSize;  //!< Size of the array in dwords

public:
	BitArray();
	BitArray( UINT32 numBits );
	~BitArray();

	bool Init( UINT32 numBits );
	void SetData( UINT32* _dwords, UINT32 _count );

	// Data management

	inline void SetBit( UINT32 iBit )
	{
		// the same as
		// mBits[i / 32] |= (1 << (i % 32));
		mBits[iBit>>5] |= 1<<(iBit&31);
	}

	inline void ClearBit( UINT32 iBit )
	{
		// the same as
		// mBits[i / 32] &= ~(1 << (i % 32));
		mBits[iBit>>5] &= ~(1<<(iBit&31));
	}

	inline void ToggleBit( UINT32 iBit )
	{
		mBits[iBit>>5] ^= 1<<(iBit&31);
	}

	inline void ClearAll()
	{
		ZeroMemory( mBits, mSize*4 );
	}
	inline void SetAll()
	{
		FillMemory( mBits, mSize*4, 0xFF );
	}

	// Data access

	inline BOOL IsSet( UINT32 iBit ) const
	{
		return mBits[iBit>>5] & (1<<(iBit&31));
	}

	inline const UINT32 * GetBits() const { return mBits; }
	inline UINT32   GetSize() const { return mSize; }
};

// - We consider square symmetric N*N matrices
// - A N*N symmetric matrix has N(N+1)/2 elements
// - A boolean version needs N(N+1)/16 bytes
//  N  NbBits NbBytes
//  4  10  -
//  8  36  4.5
//  16  136  17  <= the one we select
//  32  528  66
static const BYTE BitMasks[]	= { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
static const BYTE NegBitMasks[]	= { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F };

//
// BoolSquareSymmetricMatrix16
//
struct BoolSquareSymmetricMatrix16
{
	inline UINT32 Index( UINT32 x, UINT32 y ) const { if(x>y) TSwap(x,y); return x + (y ? ((y-1)*y)>>1 : 0);   }

	inline void Set( UINT32 x, UINT32 y )   { UINT32 i = Index(x, y); mBits[i>>3] |= BitMasks[i&7];    }
	inline void Clear( UINT32 x, UINT32 y )   { UINT32 i = Index(x, y); mBits[i>>3] &= NegBitMasks[i&7];   }
	inline void Toggle( UINT32 x, UINT32 y )  { UINT32 i = Index(x, y); mBits[i>>3] ^= BitMasks[i&7];    }
	inline bool IsSet( UINT32 x, UINT32 y ) const { UINT32 i = Index(x, y); return (mBits[i>>3] & BitMasks[i&7])!=0; }

	inline void ClearAll()	{ ZeroMemory(mBits, 17);  }
	inline void SetAll()	{ FillMemory(mBits, 17, 0xff); }

	BYTE mBits[17];
};

//--------------------------------------------------------------//
//    End Of File.         //
//--------------------------------------------------------------//
