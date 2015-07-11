/*
=============================================================================
	File:	TFixedArray.h
	Desc:	One dimensional fixed-size array template (cannot be resized).
	Note:	This array can never grow and reallocate memory
			so it should be safe to store pointers to its contents.
			Nice thing is that it doesn't move/copy its contents around.
	ToDo:	Stop reinventing the wheel.
=============================================================================
*/
#pragma once

//
//	TFixedArray< TYPE, SIZETYPE >
//
template< typename TYPE, typename SIZETYPE = UINT32 >
class TFixedArray
	: public TArrayBase< TYPE, TFixedArray<TYPE,SIZETYPE> >
{
	TYPE *			mData;	// pointer to the allocated memory
	SIZETYPE		mNum;	// number of elements, this should be inside the range [0..mCapacity)
	SIZETYPE		mCapacity;	// number of allocated entries

public:
	typedef TFixedArray
	<
		TYPE, SIZETYPE
	>
	THIS_TYPE;

	typedef
	SIZETYPE
	SIZETYPE;

	// maximum allowed array size
	enum { MAX_CAPACITY = TPow2< sizeof(SIZETYPE) * BITS_IN_BYTE >::value };

public:
	// Creates a zero length array and sets the memory manager.
	inline TFixedArray()
	{
		mData = nil;
		mNum = 0;
		mCapacity = 0;
	}

	// Use it only if you know what you're doing.
	inline explicit TFixedArray(ENoInit)
	{}

	// Deallocates array memory.
	inline ~TFixedArray()
	{
		this->Clear();
	}

	// Returns the current capacity of this array.
	inline UINT Capacity() const
	{
		return mCapacity;
	}

	// Convenience function to get the number of elements in this array.
	// Returns the size (the number of elements in the array).
	inline UINT Num() const
	{
		return mNum;
	}

	// One-time init.
	// Sets the number of elements.
	inline void SetNum( UINT numElements )
	{
		chkRET_IF_NOT( mData == nil );
		this->AllocateMemory( numElements );
		mNum = numElements;
	}

	// Convenience function to empty the array. Doesn't release allocated memory.
	// Invokes objects' destructors.
	inline void Empty()
	{
		TDestructN_IfNonPOD( mData, mCapacity );
		mNum = 0;
	}

	// Releases allocated memory (calling destructors of elements) and empties the array.
	inline void Clear()
	{
		if( mData != nil )
		{
			TDestructN_IfNonPOD( mData, mNum );
			this->ReleaseMemory( mData );
			mData = nil;
		}
		mNum = 0;
		mCapacity = 0;
	}

	inline TYPE * ToPtr() {
		return mData;
	}
	inline const TYPE * ToPtr() const {
		return mData;
	}

	// Returns the amount of reserved memory in bytes (memory allocated for storing the elements).
	inline size_t GetAllocatedMemory() const
	{
		return this->Capacity() * sizeof(TYPE);
	}

	// Returns the total amount of occupied memory in bytes.
	inline size_t GetMemoryUsed() const
	{
		return this->GetAllocatedMemory() + sizeof(*this);
	}

public:
	// Serialization.

	friend AStreamWriter& operator << ( AStreamWriter& file, const THIS_TYPE& o )
	{
		file << o.mNum;
		file.SerializeArray( o.mData, o.mNum );
		return file;
	}
	friend AStreamReader& operator >> ( AStreamReader& file, THIS_TYPE& o )
	{
		SIZETYPE number;
		file >> number;
		o.SetNum( number );
		file.SerializeArray( o.mData, o.mNum );
		return file;
	}
	friend mxArchive& operator && ( mxArchive & archive, THIS_TYPE & o )
	{
		SIZETYPE num = o.Num();
		archive && num;

		if( archive.IsReading() )
		{
			o.SetNum( num );
		}

		TSerializeArray( archive, o.ToPtr(), num );
		return archive;
	}

public:
	// Testing, Checking & Debugging.

	bool DbgCheckSelf()
	{
		//chkRET_FALSE_IF_NOT( IsPowerOfTwo( this->Capacity() ) );
		chkRET_FALSE_IF_NOT( this->Capacity() <= MAX_CAPACITY );
		chkRET_FALSE_IF_NOT( mNum <= this->Capacity() );//this can happen after AddFast_Unsafe()
		chkRET_FALSE_IF_NOT( mData );
		chkRET_FALSE_IF_NOT( sizeof(SIZETYPE) <= sizeof(UINT) );//need to impl size_t for 64-bit systems
		return true;
	}

private:
	inline void ReleaseMemory( TYPE* ptr )
	{
		mxFree( ptr );
	}
	void AllocateMemory( UINT capacity )
	{
		mxASSERT(capacity > 0);
		mxASSERT2( !mData && !mCapacity, "This function can only be called once.");

		// Allocate a new memory buffer
		mData = (TYPE*) mxAlloc( capacity * sizeof(TYPE) );

		// call default constructors
		TConstructN_IfNonPOD( mData, capacity );

		mCapacity = capacity;

		mxDEBUG_CODE(this->DbgCheckSelf());
	}

private:
	PREVENT_COPY(THIS_TYPE);
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
