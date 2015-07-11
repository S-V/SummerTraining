/*
=============================================================================
	File:	TBuffer.h
	Desc:	Dynamic (variable sized) templated array
			that occupies relatively little space.
	Note:	number of allocated elements is always equal to the array size
			(which leads to excessive memory fragmentation).
			This array is best used with in-place loading system.
=============================================================================
*/
#pragma once

//
//	TBuffer< TYPE, SIZETYPE >
//
template< typename TYPE, typename SIZETYPE = UINT16 >
class TBuffer : public TArrayBase< TYPE, TBuffer< TYPE, SIZETYPE > >
{
	TYPE *		mData;	// pointer to the allocated memory
	SIZETYPE	mNum;	// number of elements/allocated entries, the highest bit tells if the array owns memory
public:
	typedef TYPE ITEM_TYPE;
	typedef TBuffer< TYPE, SIZETYPE > THIS_TYPE;

	enum { ITEMS_COUNT_SHIFT = (sizeof(SIZETYPE) * BITS_IN_BYTE) - 1 };

	enum We_Store_Capacity_And_Bit_Flags_In_One_Int
	{
		DONT_FREE_MEMORY_MASK = SIZETYPE(1 << ITEMS_COUNT_SHIFT),	// Indicates that the storage is not the array's to delete
		GET_NUM_OF_ITEMS_MASK = SIZETYPE(~DONT_FREE_MEMORY_MASK),
	};

public:
	inline TBuffer()
	{
		mData = nil;
		mNum = 0;
	}
	inline explicit TBuffer( const THIS_TYPE& other )
	{
		mData = nil;
		mNum = 0;
		this->Copy( other );
	}
	inline TBuffer( TYPE* externalStorage, UINT maxCount )
	{
		mData = nil;
		mNum = 0;
		this->SetExternalStorage( externalStorage, maxCount );
	}

	// Deallocates array memory.
	inline ~TBuffer()
	{
		this->Clear();
	}

	void SetExternalStorage( TYPE* externalMemory, UINT count )
	{
		mxASSERT(mData == nil && mNum == 0);
		mData = externalMemory;
		mNum = count;
		mNum |= DONT_FREE_MEMORY_MASK;
	}

	inline bool OwnsMemory() const
	{
		return (mNum & DONT_FREE_MEMORY_MASK) == 0;
	}

	// Convenience function to get the number of elements in this array.
	// Returns the size (the number of elements in the array).
	inline UINT Num() const {
		return (mNum & GET_NUM_OF_ITEMS_MASK);
	}
	inline TYPE * ToPtr() {
		return mData;
	}
	inline const TYPE * ToPtr() const {
		return mData;
	}

	// Returns the current capacity of this array.
	inline UINT Capacity() const {
		return this->Num();
	}

	// Convenience function to empty the array. Doesn't release allocated memory.
	// Doesn't call objects' destructors.
	inline void Empty()
	{
		mNum = (mNum & DONT_FREE_MEMORY_MASK);	// set to zero all bits below the highest one
	}

	// Convenience function to empty the array. Doesn't release allocated memory.
	// Invokes objects' destructors.
	inline void DestroyAndEmpty()
	{
		TDestructN_IfNonPOD( mData, this->Num() );
		this->Empty();
	}

	// One-time init.
	// Sets the number of elements.
	inline ERet SetNum( UINT numElements )
	{
		if( numElements > 0 )
		{
			return this->Resize( numElements );
		}
		return ALL_OK;
	}

	// Releases allocated memory (calling destructors of elements) and empties the array.
	inline void Clear()
	{
		if( mData != nil )
		{
			TDestructN_IfNonPOD( mData, this->Num() );
			this->ReleaseMemory();
			mData = nil;
		}
		mNum = 0;
	}


	// Adds an element to the end. NOTE: can cause excessive memory fragmentation!
	TYPE & Add( const TYPE& newOne )
	{
		const UINT oldNum = this->Num();
		this->SetNum( oldNum + 1 );
		mData[ oldNum ] = newOne;
		return mData[ oldNum ];
	}
	// Increments the size by 1 and returns a reference to the first element created.
	// NOTE: can cause excessive memory fragmentation!
	TYPE & Add()
	{
		const UINT oldNum = this->Num();
		this->SetNum( oldNum + 1 );
		return mData[ oldNum ];
	}

	// Deep copy. Slow!
	template< class OTHER_ARRAY >
	THIS_TYPE& Copy( const OTHER_ARRAY& other )
	{
		const UINT newNum = other.Num();
		this->SetNum( newNum );
		if( newNum ) {
			TCopyConstructArray( mData, other.ToPtr(), newNum );
		}
		return *this;
	}

	THIS_TYPE& operator = ( const THIS_TYPE& other )
	{
		return this->Copy( other );
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

	inline void SetDontFreeMemory()
	{
		mNum |= DONT_FREE_MEMORY_MASK;
	}

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
		virtual UINT Generic_Get_Count( const void* pArrayObject ) const override
		{
			const ARRAY_TYPE* theArray = static_cast< const ARRAY_TYPE* >( pArrayObject );
			return theArray->Num();
		}
		virtual ERet Generic_Set_Count( void* pArrayObject, UINT newNum ) const override
		{
			ARRAY_TYPE* theArray = static_cast< ARRAY_TYPE* >( pArrayObject );
			mxDO(theArray->SetNum( newNum ));
			return ALL_OK;
		}
		virtual UINT Generic_Get_Capacity( const void* pArrayObject ) const override
		{
			const ARRAY_TYPE* theArray = static_cast< const ARRAY_TYPE* >( pArrayObject );
			return theArray->Capacity();
		}
		virtual ERet Generic_Set_Capacity( void* pArrayObject, UINT newNum ) const override
		{
			ARRAY_TYPE* theArray = static_cast< ARRAY_TYPE* >( pArrayObject );
			mxDO(theArray->SetNum( newNum ));
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
			theArray->SetDontFreeMemory();
		}
	};

public:	// Iterators.
	class Iterator
	{
		THIS_TYPE &	m_array;
		UINT		m_currentIndex;
	public:
		inline Iterator( THIS_TYPE& rArray )
			: m_array( rArray )
			, m_currentIndex( 0 )
		{}
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
		inline void Skip( UINT count )
		{
			m_currentIndex += count;
		}
		inline void Reset()
		{
			m_currentIndex = 0;
		}
		inline const Iterator& operator ++ ()
		{
			++m_currentIndex;
			return *this;
		}
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
		UINT				m_currentIndex;
	public:
		inline ConstIterator( const THIS_TYPE& rArray )
			: m_array( rArray )
			, m_currentIndex( 0 )
		{}
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
		inline void Skip( UINT count )
		{
			m_currentIndex += count;
		}
		inline void Reset()
		{
			m_currentIndex = 0;
		}
		inline const ConstIterator& operator ++ ()
		{
			++m_currentIndex;
			return *this;
		}
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

private:
	void ReleaseMemory()
	{
		if( this->OwnsMemory() )
		{
			mxFree( mData );
		}		
	}

	ERet Resize( UINT32 newCount )
	{
		mxASSERT( newCount > 0 );

		const UINT32 oldCount = this->Num();
		TYPE * const oldArray = this->ToPtr();		

		if( oldCount == newCount ) {
			return ALL_OK;
		}

		// Allocate a new memory buffer
		TYPE * newArray = c_cast(TYPE*) mxAlloc( newCount * sizeof(TYPE) );
		if( !newArray ) {
			return ERR_OUT_OF_MEMORY;
		}

		if( PtrToBool( oldArray ) && oldCount )
		{
			// copy-construct the new elements
			const UINT32 overlap = Min( oldCount, newCount );
			TCopyConstructArray( newArray, oldArray, overlap );
			// destroy the old contents
			TDestructN_IfNonPOD( oldArray, oldCount );
			// deallocate old memory buffer
			this->ReleaseMemory();
		}

		// call default constructors for the rest
		const UINT32 numNewItems = (newCount > oldCount) ? (newCount - oldCount) : 0;
		TConstructN_IfNonPOD( newArray + oldCount, numNewItems );

		mData = newArray;
		mNum = newCount;

		return ALL_OK;
	}

private:
	NO_COMPARES(THIS_TYPE);
};

//---------------------------------------------------------------------------
// Reflection.
//
template< typename TYPE >
struct TypeDeducer< TBuffer< TYPE, UINT16 > >
{
	static inline const mxType& GetType()
	{
		static TBuffer< TYPE, UINT16 >::ArrayDescriptor staticTypeInfo(mxEXTRACT_TYPE_NAME(TBuffer));
		return staticTypeInfo;
	}
	static inline ETypeKind GetTypeKind()
	{
		return ETypeKind::Type_Array;
	}
};
template< typename TYPE >
struct TypeDeducer< TBuffer< TYPE, UINT32 > >
{
	static inline const mxType& GetType()
	{
		static TBuffer< TYPE, UINT32 >::ArrayDescriptor staticTypeInfo(mxEXTRACT_TYPE_NAME(TBuffer));
		return staticTypeInfo;
	}
	static inline ETypeKind GetTypeKind()
	{
		return ETypeKind::Type_Array;
	}
};



template< class TYPE >	// where TYPE has member 'name' of type 'String'
TYPE* FindByName( TBuffer< TYPE >& items, const char* name )
{
	for( UINT i = 0; i < items.Num(); i++ )
	{
		TYPE& item = items[ i ];
		if( Str::EqualS( item.name, name ) ) {
			return &item;
		}
	}
	return nil;
}
template< class TYPE >	// where TYPE has member 'name' of type 'String'
const TYPE* FindByName( const TBuffer< TYPE >& items, const char* name )
{
	for( UINT i = 0; i < items.Num(); i++ )
	{
		const TYPE& item = items[ i ];
		if( Str::EqualS( item.name, name ) ) {
			return &item;
		}
	}
	return nil;
}

template< class TYPE >	// where TYPE has member 'name' of type 'String'
int FindIndexByName( TBuffer< TYPE >& items, const char* name )
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

typedef TBuffer< BYTE, UINT32 > ByteBuffer32;

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
