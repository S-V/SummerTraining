/*
=============================================================================
	File:	Array.h
	Desc:	Dynamic (variable sized) templated array.
	The array is always stored in a contiguous chunk.
	The array can be resized.
	A size increase will cause more memory to be allocated,
	and may result in relocation of the array memory.
	A size decrease has no effect on the memory allocation.

	ToDo:	Stop reinventing the wheel.
	Idea:	store size, capacity and memory manager index
			(and other flags) in a single integer (bit mask);
			store size and capacity at the beginning of allocated memory;
			can store additional info in upper 4 bits
			of a (16-byte aligned) pointer to allocated memory.
=============================================================================
*/
#pragma once

#include <Base/Object/ArrayDescriptor.h>

class mxType;
class mxClass;

namespace Array_Util
{
	// figure out the size for allocating a new buffer
	UINT32 CalculateNewCapacity( UINT32 numElements );

}//namespace Array_Util

/*
-----------------------------------------------------------------------------
	TArray< TYPE >

	- is a resizable array which doubles in size by default.
	Does not allocate memory until the first item is added.
-----------------------------------------------------------------------------
*/
template
<
	typename TYPE	// The type of stored elements
>
class TArray
	: public TArrayBase< TYPE, TArray< TYPE > >
{
	TYPE *		mData;	// The pointer to the allocated memory
	UINT32		mNum;	// The number of elements, this should be inside the range [0..mCapacity)
	UINT32		mCapacity;	// The number of allocated entries + highest bit is set if we cannot deallocate the memory

public:
	typedef	TArray< TYPE >	THIS_TYPE;

	typedef	TYPE	ITEM_TYPE;

	typedef	UINT32	UINT32;

	// 1 bit is used for indicating if the memory was externally allocated (and the array cannot delete it)
	enum { CAPACITY_BITS = (sizeof(UINT32) * BITS_IN_BYTE) - 1 };

	// maximum allowed array size
	//enum { MAX_CAPACITY = TPow2< CAPACITY_BITS >::value };
	enum { MAX_CAPACITY = (1<<24) };

	enum We_Store_Capacity_And_Bit_Flags_In_One_Int
	{
		DONT_FREE_MEMORY_MASK = UINT32(1 << CAPACITY_BITS),	// Indicates that the storage is not the array's to delete
		EXTRACT_CAPACITY_MASK = UINT32(~DONT_FREE_MEMORY_MASK),
	};

public:

	// Creates a zero length array.
	inline TArray()
	{
		mData = nil;
		mNum = 0;
		mCapacity = 0;
	}

	inline explicit TArray( const THIS_TYPE& other )
	{
		mData = nil;
		mNum = 0;
		mCapacity = 0;
		this->Copy( other );
	}

	inline TArray( TYPE* externalStorage, UINT32 maxCount )
	{
		mData = nil;
		mNum = 0;
		mCapacity = 0;
		this->SetExternalStorage( externalStorage, maxCount );
	}

	// Use it only if you know what you're doing.
	inline explicit TArray(ENoInit)
	{}

	// Deallocates array memory.
	inline ~TArray()
	{
		this->Clear();
	}

	void SetExternalStorage( TYPE* externalMemory, UINT32 maxCount )
	{
		mxASSERT(mData == nil && mCapacity == 0);
		mData = externalMemory;
		mNum = 0;
		mCapacity = maxCount;
		mCapacity |= DONT_FREE_MEMORY_MASK;
	}

	// Returns the current capacity of this array.
	inline UINT32 Capacity() const
	{
		UINT32 capacity = (mCapacity & EXTRACT_CAPACITY_MASK);
		//mxASSERT(tmp < MAX_CAPACITY);
		return capacity;
	}

	// Convenience function to get the number of elements in this array.
	// Returns the size (the number of elements in the array).
	inline UINT32 Num() const {
		return mNum;
	}

	inline TYPE * ToPtr() {
		return mData;
	}
	inline const TYPE* ToPtr() const {
		return mData;
	}

	// Convenience function to empty the array. Doesn't release allocated memory.
	// Doesn't call objects' destructors.
	inline void Empty()
	{
		mNum = 0;
	}

	// Convenience function to empty the array. Doesn't release allocated memory.
	// Invokes objects' destructors.
	inline void DestroyAndEmpty()
	{
		TDestructN_IfNonPOD( mData, this->Capacity() );
		this->Empty();
	}

	// Releases allocated memory (calling destructors of elements) and empties the array.
	void Clear()
	{
		if(PtrToBool( mData ))
		{
			TDestructN_IfNonPOD( mData, this->Capacity() );
			this->ReleaseMemory();
			mData = nil;
		}
		mNum = 0;
		mCapacity = 0;
	}

	// Resizes the array to exactly the number of elements it contains or frees up memory if empty.
	void Shrink()
	{
		// Condense the array.
		if( mNum > 0 ) {
			this->Resize( mNum );
		} else {
			this->Clear();
		}
	}

	// See: http://www.codercorner.com/blog/?p=494
	void EmptyOrClear()
	{
		const UINT32 capacity = this->Capacity();
		const UINT32 num = this->Num();
		if( num > capacity/2 ) {
			this->Empty();
		} else {
			this->Clear();
		}
	}

	// Adds an element to the end.
	inline TYPE & Add( const TYPE& newOne )
	{
		this->Reserve( mNum + 1 );
		mData[ mNum ] = newOne;
		return mData[ mNum++ ];
	}

	// Increments the size by 1 and returns a reference to the first element created.
	inline TYPE & Add()
	{
		this->Reserve( mNum + 1 );
		return mData[ mNum++ ];
	}

	inline ERet Add( const TYPE* items, UINT32 numItems )
	{
		const UINT32 oldNum = mNum;
		const UINT32 newNum = oldNum + numItems;
		mxDO(this->SetNum( newNum ));
		TCopyArray( mData + oldNum, items, numItems );
		return ALL_OK;
	}

	// Slow!
	bool AddUnique( const TYPE& item )
	{
		const UINT32 num = mNum;
		for( UINT32 i = 0; i < num; i++ )
		{
			if( mData[i] == item ) {
				return false;
			}
		}
		this->Add( item );
		return true;
	}

	inline ERet AddZeroed( UINT32 numElements )
	{
		const UINT32 newNum = mNum + numElements;
		mxDO(this->Reserve( newNum ));
		MemZero( (BYTE*)mData + mNum*sizeof(TYPE), numElements*sizeof(TYPE) );
		mNum = newNum;
		return ALL_OK;
	}

	inline TYPE* AddManyUninitialized( UINT32 numElements )
	{
		const UINT32 oldNum = mNum;
		const UINT32 newNum = oldNum + numElements;
		if(mxFAILED(this->Reserve( newNum ))) {return nil;}
		mNum = newNum;
		return mData + oldNum;
	}

	// Use it only if you know what you're doing.
	// This only works if 'capacity' is a power of two.
	inline TYPE& AddFast_Unsafe()
	{
		mxASSERT(IsPowerOfTwo( this->Capacity() ));
		const UINT32 newIndex = (mNum++) & (this->Capacity()-1);//avoid checking for overflow
		return mData[ newIndex ];
	}
	inline void AddFast_Unsafe( const TYPE& newOne )
	{
		mxASSERT(IsPowerOfTwo( this->Capacity() ));
		const UINT32 newIndex = (mNum++) & (this->Capacity()-1);//avoid checking for overflow
		mData[ newIndex ] = newOne;
	}

	// Slow!
	inline bool Remove( const TYPE& item )
	{
		return Array_Util::RemoveElement( *this, item );
	}

	// Slow!
	mxINLINE void RemoveAt( UINT32 index, UINT32 count = 1 )
	{
		Array_Util::RemoveAt( mData, mNum, index, count );
	}

	// this method is faster (uses the 'swap trick')
	// Doesn't preserve the relative order of elements.
	inline void RemoveAt_Fast( UINT32 index )
	{
		Array_Util::RemoveAt_Fast( mData, mNum, index );
	}

	// Removes all occurrences of value in the array
	// and returns the number of entries removed.
	//
	UINT32 RemoveAll( const TYPE& theValue )
	{
		UINT32 numRemoved = 0;
		for( UINT32 i = 0; i < mNum; ++i )
		{
			if( mData[i] == theValue ) {
				this->RemoveAt( i );
				numRemoved++;
			}
		}
		return numRemoved;
	}

	// deletes the last element
	inline void PopLast()
	{
		mxASSERT(this->NonEmpty());
		Array_Util::PopLast( mData, mNum );
	}

	// Slow!
	// inserts a new element at the given index and keeps the relative order of elements.
	TYPE & InsertAt( UINT32 index )
	{
		mxASSERT( this->IsValidIndex( index ) );
		const UINT32 oldNum = mNum;
		const UINT32 newNum = oldNum + 1;
		TYPE* data = mData;
		this->Reserve( newNum );
		for ( UINT32 i = oldNum; i > index; --i )
		{
			data[i] = data[i-1];
		}
		mNum = newNum;
		return data[ index ];
	}

	inline UINT32 RemoveContainedItem( const TYPE* o )
	{
		const UINT32 itemIndex = this->GetContainedItemIndex( o );
		chkRET_X_IF_NOT( this->IsValidIndex( itemIndex ), INDEX_NONE );
		this->RemoveAt( itemIndex );
		return itemIndex;
	}

	// Ensures no reallocation occurs until at least size 'numElements'.
	ERet Reserve( UINT32 numElements )
	{
	//	mxASSERT( numElements > 0 );//<- this helped me to catch errors
		mxASSERT( numElements <= MAX_CAPACITY );
		// resize if necessary
		if( numElements > this->Capacity() )
		{
			const UINT32 newCapacity = Array_Util::CalculateNewCapacity( numElements );
			mxDO(this->Resize( newCapacity ));
		}
		return ALL_OK;
	}

	// Ensures that there's a space for at least the given number of elements.
	inline ERet ReserveMore( UINT32 numElements )
	{
		const UINT32 oldCapacity = this->Capacity();
		const UINT32 newCapacity = oldCapacity + numElements;
		return this->Reserve( newCapacity );
	}

	// Sets the new number of elements. Resizes the array if necessary.
	inline ERet SetNum( UINT32 numElements )
	{
		// Resize to the exact size specified irregardless of granularity.
		if( numElements > this->Capacity() )
		{
			const UINT32 newCapacity = numElements;
			mxDO(this->Resize( newCapacity ));
		}
		mNum = numElements;
		return ALL_OK;
	}

	// Sets the new number of elements directly.
	// Use it only if you know what you're doing.
	inline void SetNum_Unsafe( UINT32 numElements )
	{
		mNum = numElements;
	}
	inline void DecNum_Unsafe()
	{
		--mNum;
	}
	inline void IncNum_Unsafe()
	{
		++mNum;
	}

	inline bool OwnsMemory() const
	{
		return (mCapacity & DONT_FREE_MEMORY_MASK) == 0;
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

	inline friend void F_UpdateMemoryStats( MemStatsCollector& stats, const THIS_TYPE& o )
	{
		stats.staticMem += sizeof o;
		stats.dynamicMem += o.GetAllocatedMemory();
	}

public:	// Reflection.

	typedef
	THIS_TYPE
	ARRAY_TYPE;

	class ArrayDescriptor : public mxArray
	{
	public:
		inline ArrayDescriptor( const Chars& typeName )
			: mxArray( typeName, STypeDescription::For_Type<ARRAY_TYPE>(), T_DeduceTypeInfo<ITEM_TYPE>(), sizeof(ITEM_TYPE) )
		{}
		//=-- mxArray
		virtual bool IsDynamic() const override
		{
			return true;
		}
		virtual void* Get_Array_Pointer_Address( const void* pArrayObject ) const override
		{
			const ARRAY_TYPE* theArray = static_cast< const ARRAY_TYPE* >( pArrayObject );
			return c_cast(void*) &theArray->mData;
		}
		virtual UINT32 Generic_Get_Count( const void* pArrayObject ) const override
		{
			const ARRAY_TYPE* theArray = static_cast< const ARRAY_TYPE* >( pArrayObject );
			return theArray->Num();
		}
		virtual ERet Generic_Set_Count( void* pArrayObject, UINT32 newNum ) const override
		{
			ARRAY_TYPE* theArray = static_cast< ARRAY_TYPE* >( pArrayObject );
			mxDO(theArray->SetNum( newNum ));
			return ALL_OK;
		}
		virtual UINT32 Generic_Get_Capacity( const void* pArrayObject ) const override
		{
			const ARRAY_TYPE* theArray = static_cast< const ARRAY_TYPE* >( pArrayObject );
			return theArray->Capacity();
		}
		virtual ERet Generic_Set_Capacity( void* pArrayObject, UINT32 newNum ) const override
		{
			ARRAY_TYPE* theArray = static_cast< ARRAY_TYPE* >( pArrayObject );
			mxDO(theArray->Reserve( newNum ));
			return ALL_OK;
		}
		virtual void* Generic_Get_Data( void* pArrayObject ) const override
		{
			ARRAY_TYPE* theArray = static_cast< ARRAY_TYPE* >( pArrayObject );
			return theArray->ToPtr();
		}
		virtual const void* Generic_Get_Data( const void* pArrayObject ) const override
		{
			const ARRAY_TYPE* theArray = static_cast< const ARRAY_TYPE* >( pArrayObject );
			return theArray->ToPtr();
		}
		virtual void SetDontFreeMemory( void* pArrayObject ) const override
		{
			ARRAY_TYPE* theArray = static_cast< ARRAY_TYPE* >( pArrayObject );
			theArray->mCapacity |= DONT_FREE_MEMORY_MASK;
		}
	};

public:	// Binary Serialization.

	friend AStreamWriter& operator << ( AStreamWriter& file, const THIS_TYPE& o )
	{
		const UINT32 number = o.mNum;
		file << number;

		file.SerializeArray( o.mData, number );

		return file;
	}
	friend AStreamReader& operator >> ( AStreamReader& file, THIS_TYPE& o )
	{
		UINT32 number;
		file >> number;
		o.SetNum( number );

		file.SerializeArray( o.mData, number );

		return file;
	}
	friend mxArchive& operator && ( mxArchive & archive, THIS_TYPE & o )
	{
		UINT32 num = o.Num();
		archive && num;

		if( archive.IsReading() )
		{
			o.SetNum( num );
		}

		TSerializeArray( archive, o.mData, num );

		return archive;
	}

public:
	//TODO: make special Shrink(), ReserveAndGrowByHalf(newCapacity) and ReserveAndGrowByNumber(newCapacity,granularity) ?

	//TODO: sorting, binary search, algorithms & iterators

	// Deep copy. Slow!
	THIS_TYPE & operator = ( const THIS_TYPE& other )
	{
		return this->Copy( other );
	}

	template< class OTHER_ARRAY >
	THIS_TYPE& Copy( const OTHER_ARRAY& other )
	{
		const UINT32 newNum = other.Num();
		mxASSERT(newNum < MAX_CAPACITY);
		//@todo: copy memory allocator?
		this->SetNum( newNum );
		if( newNum )
		{
			mxWARNING_NOTE("untested: CopyConstruct should be faster then Assignment");
			//TCopyArray( mData, other.ToPtr(), newNum );
			TCopyConstructArray( mData, other.ToPtr(), newNum );
		}
		return *this;
	}

	void AddBytes( const void* src, size_t numBytes )
	{
		mxSTATIC_ASSERT( sizeof TYPE == sizeof BYTE );
		const size_t oldNum = mNum;
		const size_t newNum = oldNum + numBytes;
		this->SetNum( newNum );
		memcpy( (BYTE*)mData + oldNum, src, numBytes );
	}

	template< typename U >
	void CopyFromArray( const U* src, UINT32 num )
	{
		this->SetNum( num );
		for( UINT32 i = 0; i < num; i++ )
		{
			mData[ i ] = src[ i ];
		}
	}
	template< typename U, size_t N >
	inline void CopyFromArray( const U (&src)[N] )
	{
		this->CopyFromArray( src, N );
	}

	THIS_TYPE& AppendArray( const THIS_TYPE& other )
	{
		const size_t oldNum = mNum;
		const size_t newNum = oldNum + other.Num();
		this->SetNum( newNum );
		TCopyArray( mData + oldNum, other.mData, other.Num() );
		return *this;
	}

	//NOTE:extremely slow!
	THIS_TYPE& AppendUniqueElements( const THIS_TYPE& other )
	{
		for( UINT32 i = 0; i < other.Num(); i++ )
		{
			if(!this->Contains( other[i] ))
			{
				this->Add( other[i] );
			}
		}
		return *this;
	}

	// works only with types that can be copied via assignment
	// returns the number of removed elements
	//
	template< class FUNCTOR >
	UINT32 Do_RemoveIf( FUNCTOR& functor )
	{
		const UINT32 oldNum = mNum;
		UINT32 newNum = 0;
		for( UINT32 i = 0; i < oldNum; i++ )
		{
			// if no need to remove this element
			if( !functor( mData[i] ) )
			{
				// then copy it
				mData[ newNum++ ] = mData[ i ];
			}
			// otherwise, skip it
		}
		mNum = newNum;
		return (oldNum - newNum);
	}

public:
	class Iterator
	{
		THIS_TYPE &	m_array;
		UINT32		m_currentIndex;

	public:
		mxINLINE Iterator( THIS_TYPE& rArray )
			: m_array( rArray )
			, m_currentIndex( 0 )
		{}

		// Functions.

		// returns 'true' if this iterator is valid (there are other elements after it)
		inline bool IsValid() const
		{
			return m_currentIndex < m_array.Num();
		}
		inline ITEM_TYPE& Value() const
		{
			return m_array[ m_currentIndex ];
		}
		inline void MoveToNext()
		{
			++m_currentIndex;
		}
		inline void Skip( UINT32 count )
		{
			m_currentIndex += count;
		}
		inline void Reset()
		{
			m_currentIndex = 0;
		}

		// Overloaded operators.

		// Pre-increment.
		inline const Iterator& operator ++ ()
		{
			++m_currentIndex;
			return *this;
		}
		// returns 'true' if this iterator is valid (there are other elements after it)
		inline operator bool () const
		{
			return this->IsValid();
		}
		inline ITEM_TYPE* operator -> () const
		{
			return &this->Value();
		}
		inline ITEM_TYPE& operator * () const
		{
			return this->Value();
		}
	};
	class ConstIterator
	{
		const THIS_TYPE &	m_array;
		UINT32				m_currentIndex;

	public:
		mxINLINE ConstIterator( const THIS_TYPE& rArray )
			: m_array( rArray )
			, m_currentIndex( 0 )
		{}

		// Functions.

		// returns 'true' if this iterator is valid (there are other elements after it)
		inline bool IsValid() const
		{
			return m_currentIndex < m_array.Num();
		}
		inline const ITEM_TYPE& Value() const
		{
			return m_array[ m_currentIndex ];
		}
		inline void MoveToNext()
		{
			++m_currentIndex;
		}
		inline void Skip( UINT32 count )
		{
			m_currentIndex += count;
		}
		inline void Reset()
		{
			m_currentIndex = 0;
		}

		// Overloaded operators.

		// Pre-increment.
		inline const ConstIterator& operator ++ ()
		{
			++m_currentIndex;
			return *this;
		}
		// returns 'true' if this iterator is valid (there are other elements after it)
		inline operator bool () const
		{
			return this->IsValid();
		}
		inline const ITEM_TYPE* operator -> () const
		{
			return &this->Value();
		}
		inline const ITEM_TYPE& operator * () const
		{
			return this->Value();
		}
	};

public:
	class OStream : public AStreamWriter
	{
		THIS_TYPE &	mArray;

	public:
		OStream( THIS_TYPE & dest )
			: mArray( dest )
		{}
		virtual size_t Write( const void* pBuffer, size_t numBytes ) override
		{
			const UINT32 oldSize = mArray.Num();
			const UINT32 newSize = oldSize + numBytes;

			mArray.SetNum( newSize );

			memcpy( (BYTE*)mArray.ToPtr() + oldSize, pBuffer, numBytes );

			return newSize - oldSize;
		}
	};
	OStream GetOStream()
	{
		return OStream( *this );
	}

public:
	// Testing, Checking & Debugging.

	// checks some invariants
	bool DbgCheckSelf() const
	{
		//chkRET_FALSE_IF_NOT( IsPowerOfTwo( this->Capacity() ) );
		chkRET_FALSE_IF_NOT( this->Capacity() <= MAX_CAPACITY );
		chkRET_FALSE_IF_NOT( mNum <= this->Capacity() );//this can happen after AddFast_Unsafe()
		chkRET_FALSE_IF_NOT( mData );
		chkRET_FALSE_IF_NOT( sizeof(UINT32) <= sizeof(UINT32) );//need to impl size_t for 64-bit systems
		return true;
	}

private:
	inline void ReleaseMemory()
	{
		if( this->OwnsMemory() )
		{
			mxFree( mData );
		}
	}

	ERet Resize( UINT32 newCapacity )
	{
		mxASSERT( newCapacity > 0 && newCapacity < MAX_CAPACITY );

		TYPE * const oldArray = this->ToPtr();
		const UINT32 oldCapacity = this->Capacity();

		if( newCapacity == oldCapacity ) {
			return ALL_OK;
		}

		// Allocate a new memory buffer
		TYPE * newArray = c_cast(TYPE*) mxAlloc( newCapacity * sizeof(TYPE) );
		if( !newArray ) {
			return ERR_OUT_OF_MEMORY;
		}

		if( PtrToBool( oldArray ) && oldCapacity )
		{
			// copy-construct the new elements
			const UINT32 overlap = Min( oldCapacity, newCapacity );
			TCopyConstructArray( newArray, oldArray, overlap );
			// destroy the old contents
			TDestructN_IfNonPOD( oldArray, oldCapacity );
			// deallocate old memory buffer
			this->ReleaseMemory();
		}

		// call default constructors for the rest
		const UINT32 numNewItems = (newCapacity > oldCapacity) ? (newCapacity - oldCapacity) : 0;
		TConstructN_IfNonPOD( newArray + oldCapacity, numNewItems );

		mData = newArray;
		mCapacity = newCapacity;

		mxDEBUG_CODE(this->DbgCheckSelf());

		return ALL_OK;
	}

public_internal:

	/// For serialization, we want to initialize the vtables
	/// in classes post data load, and NOT call the default constructor
	/// for the arrays (as the data has already been set).
	inline explicit TArray( _FinishedLoadingFlag )
	{
	}

private:
	NO_COMPARES(THIS_TYPE);
};

//---------------------------------------------------------------------------
// Reflection.
//
template< typename TYPE >
struct TypeDeducer< TArray< TYPE > >
{
	static inline const mxType& GetType()
	{
		static TArray< TYPE >::ArrayDescriptor staticTypeInfo(mxEXTRACT_TYPE_NAME(TArray));
		return staticTypeInfo;
	}
	static inline ETypeKind GetTypeKind()
	{
		return ETypeKind::Type_Array;
	}
};

//---------------------------------------------------------------------------

namespace Array_Util
{
	template< class CLASS >	// where CLASS is reference counted via Grab()/Drop()
	inline void DropReferences( TArray<CLASS*> & a )
	{
		struct {
			inline void operator () ( CLASS* o )
			{
				o->Drop();
			}
		} dropThem;

		a.Do_ForAll( dropThem );
	}

}//namespace Array_Util


template< class TYPE >	// where TYPE has member 'name' of type 'String'
TYPE* FindByName( TArray< TYPE >& items, const char* name )
{
	for( UINT32 i = 0; i < items.Num(); i++ )
	{
		TYPE& item = items[ i ];
		if( Str::EqualS( item.name, name ) ) {
			return &item;
		}
	}
	return nil;
}
template< class TYPE >	// where TYPE has member 'name' of type 'String'
const TYPE* FindByName( const TArray< TYPE >& items, const char* name )
{
	for( UINT32 i = 0; i < items.Num(); i++ )
	{
		const TYPE& item = items[ i ];
		if( Str::EqualS( item.name, name ) ) {
			return &item;
		}
	}
	return nil;
}

template< class TYPE >	// where TYPE has member 'name' of type 'String'
int FindIndexByName( TArray< TYPE >& items, const char* name )
{
	for( int i = 0; i < items.Num(); i++ )
	{
		const TYPE& item = items[ i ];
		if( Str::EqualS( item.name, name ) ) {
			return i;
		}
	}
	return -1;
}


typedef TArray< char >	ByteArrayT;

class ByteArrayWriter : public AStreamWriter
{
	ByteArrayT & m_buffer;
	size_t m_writePointer;

public:
	ByteArrayWriter( ByteArrayT & buffer, UINT32 startOffset = 0 )
		: m_buffer( buffer )
	{
		this->DbgSetName( "ByteArrayWriter" );
		m_writePointer = startOffset;
	}
	virtual ERet Write( const void* source, size_t numBytes ) override
	{
		const size_t totalBytes =  m_writePointer + numBytes;
		m_buffer.SetNum( totalBytes );
		memcpy( m_buffer.ToPtr() + m_writePointer, source, numBytes );
		m_writePointer = totalBytes;
		return ALL_OK;
	}
	inline const char* ToPtr() const
	{
		return m_buffer.ToPtr();
	}
	inline char* ToPtr()
	{
		return m_buffer.ToPtr();
	}
	inline size_t Tell() const
	{
		return m_writePointer;
	}
	inline void Rewind()
	{
		m_writePointer = 0;
	}
	inline void Seek( size_t absOffset )
	{
		m_buffer.Reserve( absOffset );
		m_writePointer = absOffset;
	}
private:	PREVENT_COPY( ByteArrayWriter );
};

struct ByteWriter : public AStreamWriter
{
	TArray< BYTE >	m_data;
public:
	ByteWriter()
	{}
	virtual ERet Write( const void* data, size_t size ) override;
	PREVENT_COPY( ByteWriter );
};

// contains an embedded buffer to avoid initial memory allocations
template< typename TYPE, UINT32 RESERVED >
class TLocalArray : public TArray< TYPE >
{
	TYPE	m__storage[RESERVED];
public:
	TLocalArray()
	{
		this->SetExternalStorage( m__storage, mxCOUNT_OF(m__storage) );
	}
	PREVENT_COPY( TLocalArray );
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
