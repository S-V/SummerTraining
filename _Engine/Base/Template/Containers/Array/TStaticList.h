/*
=============================================================================
	File:	TStaticList.
	Desc:	Static list template.
			A non-growing, memset-able list using no memory allocation.
	ToDo:	Stop reinventing the wheel.
=============================================================================
*/
#pragma once

#include <Base/Object/ArrayDescriptor.h>

/*
--------------------------------------------------------------
	TStaticList< typename TYPE, const UINT32 SIZE >
	- 32-bit non-growable array
--------------------------------------------------------------
*/
template< typename TYPE, const UINT32 SIZE >
class TStaticList
	: public TArrayBase< TYPE, TStaticList<TYPE,SIZE> >
{
	UINT32	mNum;
	TYPE	mData[ SIZE ];

public:
	typedef TStaticList<TYPE,SIZE> THIS_TYPE;

	typedef
	TYPE
	ITEM_TYPE;

public:
	inline TStaticList()
	{
		mNum = 0;
	}
	inline explicit TStaticList(EInitZero)
	{
		mNum = 0;
		mxZERO_OUT(mData);
	}
	inline explicit TStaticList( const THIS_TYPE& other )
	{
		mNum = other.mNum;
		TCopyConstructArray( mData, other.mData, other.mNum );
	}
	inline ~TStaticList()
	{
	}

	// Returns the number of elements currently contained in the list.
	inline UINT32 Num() const
	{
		return mNum;
	}

	inline void SetNum( UINT32 newNum )
	{
		mxASSERT(newNum <= Max());
		mNum = newNum;
	}

	// Returns the maximum number of elements in the list.
	inline UINT32 Max() const
	{
		return SIZE;
	}
	inline UINT32 Capacity() const
	{
		return SIZE;
	}

	inline void Empty()
	{
		mNum = 0;
	}
	inline void Clear()
	{
		mNum = 0;
	}

	inline bool IsFull() const
	{
		return mNum == SIZE;
	}

	// Returns a pointer to the beginning of the array.  Useful for iterating through the list in loops.
	inline TYPE * ToPtr()
	{
		return mData;
	}
	inline const TYPE * ToPtr() const
	{
		return mData;
	}

	// Implicit type conversion - intentionally disabled:
	//inline operator TYPE* ()
	//{ return mData; }
	//inline operator const TYPE* () const
	//{ return mData; }

	inline void Add( const TYPE& newItem )
	{
		mxASSERT( mNum < SIZE );
		mData[ mNum++ ] = newItem;
	}
	inline TYPE & Add()
	{
		mxASSERT( mNum < SIZE );
		return mData[ mNum++ ];
	}

	void AddBytes( const void* src, size_t numBytes )
	{
		size_t oldNum = mNum;
		size_t newNum = oldNum + numBytes;
		SetNum( newNum );

		memcpy( mData + oldNum, src, numBytes );
	}

	// Slow!
	bool AddUnique( const TYPE& item )
	{
		for( UINT32 i = 0; i < mNum; i++ )
		{
			if( mData[i] == item ) {
				return false;
			}
		}
		Add( item );
		return true;
	}

	// Slow!
	inline bool Remove( const TYPE& item )
	{
		const UINT32 index = this->FindIndexOf( item );
		if( INDEX_NONE == index ) {
			return false;
		}
		this->RemoveAt( index );
		return true;
	}

	// Slow!
	inline void RemoveAt( UINT32 index )
	{
		mxASSERT( IsValidIndex( index ) );
		//TDestructOne_IfNonPOD( &mData[index] );
		--mNum;
		for( UINT32 i = index; i < mNum; i++ )
		{
			mData[ i ] = mData[ i + 1 ];
		}
	}

	// Doesn't preserve the relative order of elements.
	inline void RemoveAt_Fast( UINT32 index )
	{
		mxASSERT( IsValidIndex( index ) );
		//TDestructOne_IfNonPOD( &mData[index] );
		--mNum;
		if ( index != mNum ) {
			mData[ index ] = mData[ mNum ];
		}
	}

	// deletes the last element
	inline TYPE Pop()
	{
		mxASSERT(mNum > 0);
		--mNum;
		TYPE result = mData[ mNum ];
		// remove and destroy the last element
		TDestructOne_IfNonPOD( mData[ mNum ] );
		return result;
	}

	// Use it only if you know what you're doing.
	// This only works if array capacity is a power of two.
	inline TYPE & AddFast_Unsafe()
	{
		mxSTATIC_ASSERT( TIsPowerOfTwo< SIZE >::value );
		mxASSERT( mNum < SIZE );
		const UINT32 newIndex = (mNum++) & (SIZE-1);//avoid checking for overflow
		return mData[ newIndex ];
	}
	// only valid for lists with power-of-two capacities
	inline void AddFast_Unsafe( const TYPE& newItem )
	{
		mxSTATIC_ASSERT( TIsPowerOfTwo< SIZE >::value );
		mxASSERT( mNum < SIZE );
		const UINT32 newIndex = (mNum++) & (SIZE-1);//avoid checking for overflow
		mData[ newIndex ] = newItem;
	}

	inline void Prepend( const TYPE& newItem )
	{
		this->Insert( newItem, 0 );
	}

	// Insert the element at the given index.
	// Returns the index of the new element.
	inline UINT32 Insert( const TYPE& newItem, UINT32 index )
	{
		mxASSERT(!this->IsFull());
		mxASSERT(index < this->Capacity());
		index = smallest(index, mNum);
		for( UINT32 i = mNum; i > index; i-- )
		{
			mData[ i ] = mData[ i - 1 ];
		}
		++mNum;
		mData[ index ] = newItem;
		return index;
	}

	// Returns the total amount of occupied memory in bytes.
	inline UINT32 GetMemoryUsed() const
	{
		return sizeof(*this);
	}

	friend AStreamWriter& operator << ( AStreamWriter& file, const THIS_TYPE &o )
	{
		file << o.mNum;
		file.SerializeArray( o.mData, o.mNum );
		return file;
	}
	friend AStreamReader& operator >> ( AStreamReader& file, THIS_TYPE &o )
	{
		file >> o.mNum;
		mxASSERT( o.mNum <= SIZE );
		file.SerializeArray( o.mData, o.mNum );
		return file;
	}
	friend mxArchive& operator && ( mxArchive & archive, THIS_TYPE & o )
	{
		archive && o.mNum;
		TSerializeArray( archive, o.mData, o.mNum );
		return archive;
	}

public:
	class OStream : public AStreamWriter
	{
		THIS_TYPE &	mArray;
	public:
		OStream( THIS_TYPE & dest )
			: mArray( dest )
		{}
		virtual ERet Write( const void* pBuffer, size_t numBytes ) override
		{
			const UINT32 oldSize = mArray.Num();
			const UINT32 newSize = oldSize + numBytes;
			mArray.SetNum( newSize );
			memcpy( mArray.ToPtr() + oldSize, pBuffer, numBytes );
			return (newSize == oldSize) ? ALL_OK : ERR_FAILED_TO_WRITE_FILE;
		}
	};
	inline OStream GetOStream()
	{
		return OStream( *this );
	}

public_internal:

	class Descriptor : public mxArray
	{
	public:
		inline Descriptor( const Chars& typeName )
			: mxArray( typeName, STypeDescription::For_Type<THIS_TYPE>(), T_DeduceTypeInfo<ITEM_TYPE>(), sizeof(ITEM_TYPE) )
		{}
		//=-- mxArray
		virtual bool IsDynamic() const override
		{
			return false;
		}
		virtual void* Get_Array_Pointer_Address( const void* pArrayObject ) const override
		{
			const THIS_TYPE* theArray = static_cast< const THIS_TYPE* >( pArrayObject );
			return c_cast(void*) &theArray->mData;
		}
		virtual UINT32 Generic_Get_Count( const void* pArrayObject ) const override
		{
			const THIS_TYPE* theArray = static_cast< const THIS_TYPE* >( pArrayObject );
			return theArray->Num();
		}
		virtual ERet Generic_Set_Count( void* pArrayObject, UINT32 newNum ) const override
		{
			THIS_TYPE* theArray = static_cast< THIS_TYPE* >( pArrayObject );
			theArray->SetNum( newNum );
			return ALL_OK;
		}
		virtual UINT32 Generic_Get_Capacity( const void* pArrayObject ) const override
		{
			const THIS_TYPE* theArray = static_cast< const THIS_TYPE* >( pArrayObject );
			return theArray->Capacity();
		}
		virtual ERet Generic_Set_Capacity( void* pArrayObject, UINT32 newNum ) const override
		{
			THIS_TYPE* theArray = static_cast< THIS_TYPE* >( pArrayObject );
			mxASSERT(newNum <= theArray->Capacity());
			return ALL_OK;
		}
		virtual void* Generic_Get_Data( void* pArrayObject ) const override
		{
			THIS_TYPE* theArray = static_cast< THIS_TYPE* >( pArrayObject );
			return theArray->ToPtr();
		}
		virtual const void* Generic_Get_Data( const void* pArrayObject ) const override
		{
			const THIS_TYPE* theArray = static_cast< const THIS_TYPE* >( pArrayObject );
			return theArray->ToPtr();
		}
		virtual void SetDontFreeMemory( void* pArrayObject ) const override
		{
			mxUNUSED(pArrayObject);
		}
	};

	/// For serialization, we want to initialize the vtables
	/// in classes post data load, and NOT call the default constructor
	/// for the arrays (as the data has already been set).
	inline explicit TStaticList( _FinishedLoadingFlag )
	{
	}

private:
	//NO_ASSIGNMENT(THIS_TYPE);
	//NO_COPY_CONSTRUCTOR(THIS_TYPE);
	NO_COMPARES(THIS_TYPE);
};

//
//	mxStaticList_InitZeroed
//
template< typename TYPE, const UINT32 SIZE >
class TStaticList_InitZeroed : public TStaticList< TYPE, SIZE >
{
public:
	TStaticList_InitZeroed()
		: TStaticList(_InitZero)
	{}
};


template< typename TYPE, const UINT32 SIZE >
struct TypeDeducer< TStaticList< TYPE, SIZE > >
{
	static inline const mxType& GetType()
	{
		static TStaticList< TYPE, SIZE >::Descriptor typeInfo(mxEXTRACT_TYPE_NAME(TStaticList));
		return typeInfo;
	}
	static inline ETypeKind GetTypeKind()
	{
		return ETypeKind::Type_Array;
	}
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
