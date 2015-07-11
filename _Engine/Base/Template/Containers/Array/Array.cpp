/*
=============================================================================
	File:	Array.cpp
	Desc:
=============================================================================
*/
#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>




namespace Array_Util
{
	// figure out the size for allocating a new buffer
	UINT CalculateNewCapacity( UINT numElements )
	{
		// if number is small then default to 8 elements
		enum { DEFAULT_CAPACITY = 8 };

		UINT newCapacity = (numElements >= DEFAULT_CAPACITY)
			? CeilPowerOfTwo( numElements )
			: DEFAULT_CAPACITY
			;

		return newCapacity;
	}

}//namespace Array_Util



//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
