/*
=============================================================================
	File:	TStaticArray.h
	Desc:	C-style array
	ToDo:	Stop reinventing the wheel.
=============================================================================
*/

#include <Base/Object/ArrayDescriptor.h>

//
//	TStaticArray - static, fixed-size array (the size is known at compile time).
//
//	It's recommended to use this class instead of bare C-style arrays.
//
template< typename TYPE, const UINT SIZE >
class TStaticArray
	: public TArrayBase< TYPE, TStaticArray<TYPE,SIZE> >
{
	TYPE	mData[ SIZE ];

public:
	typedef TStaticArray
	<
		TYPE,
		SIZE
	>
	THIS_TYPE;

	typedef
	TYPE
	ITEM_TYPE;

public:
	inline TStaticArray()
	{}

	inline explicit TStaticArray(EInitZero)
	{
		mxZERO_OUT(mData);
	}

	inline ~TStaticArray()
	{}

	inline UINT Num() const
	{
		return SIZE;
	}
	inline UINT Capacity() const
	{
		return SIZE;
	}

	inline void SetNum( UINT newNum )
	{
		mxASSERT(newNum <= SIZE);
	}
	inline void Empty()
	{
	}
	inline void Clear()
	{
	}

	THIS_TYPE& operator = ( const THIS_TYPE& other )
	{
		return this->Copy( other );
	}

	THIS_TYPE& Copy( const TStaticArray& other )
	{
		for( UINT i = 0; i < SIZE; i++ ) {
			mData[i] = other[i];
		}
		return *this;
	}

	inline TYPE * ToPtr()
	{ return mData; }

	inline const TYPE * ToPtr() const
	{ return mData; }

public:	// Reflection.

	typedef	THIS_TYPE	ARRAY_TYPE;

	class ArrayDescriptor : public mxArray
	{
	public:
		inline ArrayDescriptor( const Chars& typeName )
			: mxArray( typeName, STypeDescription::For_Type<ARRAY_TYPE>(), T_DeduceTypeInfo<ITEM_TYPE>(), sizeof(ITEM_TYPE) )
		{}

		//=-- mxArray
		virtual bool IsDynamic() const override
		{
			return false;
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
			theArray->SetNum( newNum );
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
			mxASSERT(newNum <= theArray->Capacity());
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
			mxUNUSED(pArrayObject);
		}
	};

public:	// Serialization.

	friend AStreamWriter& operator << ( AStreamWriter& file, const THIS_TYPE &o )
	{
		file.SerializeArray( o.mData, SIZE );
		return file;
	}
	friend AStreamReader& operator >> ( AStreamReader& file, THIS_TYPE &o )
	{
		file.SerializeArray( o.mData, SIZE );
		return file;
	}
	friend mxArchive& operator && ( mxArchive & archive, THIS_TYPE & o )
	{
		TSerializeArray( archive, o.mData, SIZE );
		return archive;
	}

public_internal:

	/// For serialization, we want to initialize the vtables
	/// in classes post data load, and NOT call the default constructor
	/// for the arrays (as the data has already been set).
	inline explicit TStaticArray( _FinishedLoadingFlag )
	{
	}

	NO_COMPARES(THIS_TYPE);
};

//
//	TStaticArray_InitZeroed
//
template< typename TYPE, const size_t SIZE >
class TStaticArray_InitZeroed : public TStaticArray< TYPE, SIZE >
{
public:
	TStaticArray_InitZeroed()
		: TStaticArray(_InitZero)
	{}
};

template< typename TYPE, const UINT SIZE >
struct TypeDeducer< TStaticArray< TYPE, SIZE > >
{
	static inline const mxType& GetType()
	{
		static TStaticArray< TYPE, SIZE >::ArrayDescriptor staticTypeInfo(mxEXTRACT_TYPE_NAME(TStaticArray));
		return staticTypeInfo;
	}
	static inline ETypeKind GetTypeKind()
	{
		return ETypeKind::Type_Array;
	}
};

// specialization for embedded one-dimensional static arrays of arbitrary values
template< typename TYPE, UINT32 SIZE >
inline const mxType& T_DeduceTypeInfo( const TYPE (&) [SIZE] )
{
	static TStaticArray< TYPE, SIZE >::ArrayDescriptor staticTypeInfo(mxEXTRACT_TYPE_NAME(TStaticArray));
	return staticTypeInfo;
}

// ... could be extended to handle 2D arrays [][] and so on...

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
