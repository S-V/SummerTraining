/*
=============================================================================
	File:	HashMap.cpp
	Desc:
=============================================================================
*/
#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

namespace HashMapUtil
{
	void* AllocateMemory( UINT bytes )
	{
		return mxAlloc( bytes );
	}

	void ReleaseMemory( void* ptr )
	{
		mxFree( ptr );
	}

	// hashMod = tableSize - 1
	UINT GetHashTableSize( const void* buckets, UINT hashMod )
	{
		return buckets ? (hashMod + 1) : 0;
	}

	UINT CalcHashTableSize( UINT numItems, UINT defaultTableSize )
	{
		return (numItems > 0) ? NextPowerOfTwo(defaultTableSize + numItems*2) : defaultTableSize;
	}

}//namespace HashMapUtil

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
