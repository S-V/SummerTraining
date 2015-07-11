/*
=============================================================================
	File:	TPtr.h
	Desc:	Weak pointer - doesn't take ownership over the object,
			doesn't know about reference counting,
			used to catch programming errors (automatic null pointer checks).
=============================================================================
*/
#pragma once

#include <Base/Object/PointerType.h>

//
//	TPtr< T > - is a weak pointer.
//
template< typename T >
class TPtr {
public:
	mxFORCEINLINE TPtr( T *pointer = NULL )
		: Ptr( pointer )
	{}
	mxFORCEINLINE explicit TPtr( const TPtr<T>& other )
	{
		*this = other;
	}
	mxFORCEINLINE ~TPtr()
	{
		this->Ptr = NULL;
	}

	mxFORCEINLINE T * operator -> () const
	{
		mxASSERT_PTR( this->Ptr );
		return this->Ptr;
	}
	mxFORCEINLINE T & operator * () const
	{
		mxASSERT_PTR( this->Ptr );
		return *this->Ptr;
	}
	mxFORCEINLINE operator T* () const
	{
		return this->Ptr;
	}

	mxFORCEINLINE T* ToPtr() const
	{
		mxASSERT_PTR( this->Ptr );
		return this->Ptr;
	}

	mxFORCEINLINE void operator = ( T* newPointer )
	{
		this->Ptr = newPointer;
	}
	mxFORCEINLINE void operator = ( const TPtr<T>& other )
	{
		this->Ptr = other.Ptr;
	}

	mxFORCEINLINE bool operator ! () const
	{
		return ( this->Ptr == NULL );
	}
	mxFORCEINLINE bool operator == ( T* pObject ) const
	{
		return ( this->Ptr == pObject );
	}
	mxFORCEINLINE bool operator != ( T* pObject ) const
	{
		return ( this->Ptr != pObject );
	}
	mxFORCEINLINE bool operator == ( const TPtr<T>& other ) const
	{
		return ( this->Ptr == other.Ptr );
	}
	mxFORCEINLINE bool operator != ( const TPtr<T>& other ) const
	{
		return ( this->Ptr != other.Ptr );
	}

	mxFORCEINLINE bool IsNull() const
	{
		return ( this->Ptr == NULL );
	}
	mxFORCEINLINE bool IsValid() const
	{
		return ( this->Ptr != NULL );
	}

	mxFORCEINLINE T& get_ref()
	{
		mxASSERT_PTR( this->Ptr );
		return *this->Ptr;
	}

	// Testing & Debugging.

	// An unsafe way to get the pointer.
	mxFORCEINLINE T*& get_ptr_ref()
	{
		return this->Ptr;
	}
	mxFORCEINLINE T* get_ptr()
	{
		return this->Ptr;
	}
	mxFORCEINLINE const T* get_ptr() const
	{
		return this->Ptr;
	}

public:

	//@todo: make a special smart pointer with ConstructInPlace() and Destruct() (and check for double construct/delete)

	//NOTE: use for singletons only!
	T* ConstructInPlace()
	{
		mxASSERT( this->Ptr == NULL );
		if( this->Ptr == NULL )
		{
			mxCONSTRUCT_IN_PLACE_X( this->Ptr, T );
			return this->Ptr;
		}
		return NULL;
	}

	void Destruct()
	{
		if( this->Ptr != NULL )
		{
			this->Ptr->~T();
			this->Ptr = NULL;
		}
	}
	void Delete()
	{
		if( this->Ptr != NULL )
		{
			delete this->Ptr;
			this->Ptr = NULL;
		}
	}

	T& ReConstructInPlace()
	{
		if( this->Ptr != NULL )
		{
			this->Destruct();
			this->ConstructInPlace();
		}
		return *this->Ptr;
	}

public_internal:

	T *		Ptr;
};

template< class CLASS >
struct TypeDeducer< TPtr<CLASS> >
{
	static inline const mxType& GetType()
	{
		static mxPointerType staticTypeInfo(
			mxEXTRACT_TYPE_NAME(TPtr),
			STypeDescription::For_Type<CLASS*>(),
			T_DeduceTypeInfo<CLASS>()
		);
		return staticTypeInfo;
	}
	static inline ETypeKind GetTypeKind()
	{
		return ETypeKind::Type_Pointer;
	}
};


//---------------------------------------------------------------------------

//
//	TValidPtr< TYPE > - is a weak pointer
//	that always stores a non-null pointer
//	which cannot be reassigned.
//
template< typename TYPE >
class TValidPtr
{
	TYPE *	m_pointer;

public:
	//mxFORCEINLINE TValidPtr()
	//{
	//	m_pointer = NULL;
	//}
	mxFORCEINLINE TValidPtr( TYPE* pointer )
	{
		mxASSERT_PTR( pointer );
		m_pointer = pointer;
	}
	mxFORCEINLINE ~TValidPtr()
	{
	}

	mxFORCEINLINE TYPE * operator -> () const
	{
		mxASSERT_PTR( m_pointer );
		return m_pointer;
	}
	mxFORCEINLINE TYPE & operator * () const
	{
		mxASSERT_PTR( m_pointer );
		return *m_pointer;
	}
	mxFORCEINLINE operator TYPE* () const
	{
		mxASSERT_PTR( m_pointer );
		return m_pointer;
	}
	mxFORCEINLINE operator const TYPE* () const
	{
		mxASSERT_PTR( m_pointer );
		return m_pointer;
	}

	mxFORCEINLINE bool operator == ( TYPE* pointer ) const
	{
		return ( m_pointer == pointer );
	}
	mxFORCEINLINE bool operator != ( TYPE* pointer ) const
	{
		return ( m_pointer != pointer );
	}
	mxFORCEINLINE bool operator == ( const TValidPtr<TYPE>& other ) const
	{
		return ( m_pointer == other.m_pointer );
	}
	mxFORCEINLINE bool operator != ( const TValidPtr<TYPE>& other ) const
	{
		return ( m_pointer != other.m_pointer );
	}

public:

	mxFORCEINLINE void Destruct()
	{
		mxASSERT_PTR( m_pointer );
		if( m_pointer != NULL )
		{
			m_pointer->~TYPE();
			m_pointer = NULL;
		}
	}
	mxFORCEINLINE void Delete()
	{
		mxASSERT_PTR( m_pointer );
		if( m_pointer != NULL )
		{
			delete m_pointer;
			m_pointer = NULL;
		}
	}

private:	PREVENT_COPY( TValidPtr<TYPE> );
};

//---------------------------------------------------------------------------

//
//	TValidRef< TYPE > - stores a reference
//	that cannot be reassigned.
//
template< typename TYPE >
class TValidRef
{
	TYPE &	m_reference;

public:
	mxFORCEINLINE TValidRef( TYPE& reference )
		: m_reference( reference )
	{	
	}

	mxFORCEINLINE ~TValidRef()
	{
	}

	mxFORCEINLINE TYPE & ToRef() const
	{
		return m_reference;
	}

	mxFORCEINLINE TYPE & operator & () const
	{
		return m_reference;
	}

	mxFORCEINLINE bool operator == ( TYPE* pointer ) const
	{
		return ( m_reference == pointer );
	}
	mxFORCEINLINE bool operator != ( TYPE* pointer ) const
	{
		return ( m_reference != pointer );
	}
	mxFORCEINLINE bool operator == ( const TValidRef<TYPE>& other ) const
	{
		return ( m_reference == other.m_reference );
	}
	mxFORCEINLINE bool operator != ( const TValidRef<TYPE>& other ) const
	{
		return ( m_reference != other.m_reference );
	}

private:	PREVENT_COPY( TValidRef<TYPE> );
};


//---------------------------------------------------------------------------

//@fixme: untested
//
//	TIndexPtr< TYPE > - is a handle-based smart pointer.
//
template< typename TYPE >
class TIndexPtr
{
	UINT	m_objectIndex;

	mxFORCEINLINE TYPE* Get_Pointer_By_Index( const UINT theIndex ) const
	{
		TYPE* thePointer = TYPE::GetPointerByIndex( theIndex );
		return thePointer;
	}
	mxFORCEINLINE UINT Get_Index_By_Pointer( const TYPE* thePointer ) const
	{
		return TYPE::GetIndexByPointer( thePointer );
	}

public:
	mxFORCEINLINE bool IsValid() const
	{
		return this->Get_Pointer_By_Index( m_objectIndex ) != NULL;
	}
	mxFORCEINLINE bool IsNull() const
	{
		return !this->IsValid();
	}

	mxFORCEINLINE TIndexPtr()
	{
		m_objectIndex = INDEX_NONE;
	}
	mxFORCEINLINE TIndexPtr( TYPE* thePointer )
	{
		*this = thePointer;
	}
	mxFORCEINLINE TIndexPtr( const TIndexPtr& other )
	{
		*this = other;
	}
	mxFORCEINLINE ~TIndexPtr()
	{
	}

	mxFORCEINLINE TYPE * operator -> () const
	{
		mxASSERT( this->IsValid() );
		return this->Get_Pointer_By_Index( m_objectIndex );
	}

	mxFORCEINLINE TYPE & operator * () const
	{
		mxASSERT( this->IsValid() );
		return *this->Get_Pointer_By_Index( m_objectIndex );
	}

	mxFORCEINLINE TYPE* ToPtr() const
	{
		return this->Get_Pointer_By_Index( m_objectIndex );
	}
	mxFORCEINLINE TYPE* SafeGetPtr() const
	{
		TYPE* thePointer = this->Get_Pointer_By_Index( m_objectIndex );
		return thePointer;
	}
	mxFORCEINLINE operator TYPE* () const
	{
		return this->SafeGetPtr();
	}
	mxFORCEINLINE operator const TYPE* () const
	{
		return this->SafeGetPtr();
	}

	mxFORCEINLINE void operator = ( TYPE* thePointer )
	{
		if(!PtrToBool( thePointer )) {
			m_objectIndex = INDEX_NONE;
		} else {
			m_objectIndex = this->Get_Index_By_Pointer( thePointer );
		}
	}
	mxFORCEINLINE void operator = ( const TIndexPtr<TYPE>& other )
	{
		m_objectIndex = other.m_objectIndex;
	}

	mxFORCEINLINE bool operator == ( TYPE* thePointer ) const
	{
		mxASSERT_PTR( thePointer );
		return m_objectIndex == TYPE::GetIndexByPointer( thePointer );
	}
	mxFORCEINLINE bool operator != ( TYPE* thePointer ) const
	{
		return !( *this == thePointer );
	}

	mxFORCEINLINE bool operator == ( const TIndexPtr<TYPE>& other ) const
	{
		return ( m_objectIndex == other.m_objectIndex );
	}
	mxFORCEINLINE bool operator != ( const TIndexPtr<TYPE>& other ) const
	{
		return ( m_objectIndex != other.m_objectIndex );
	}

public_internal:
	mxFORCEINLINE UINT& GetHandleRef()
	{
		return m_objectIndex;
	}
	bool UpdateHandle()
	{
		TYPE* thePointer = this->Get_Pointer_By_Index( m_objectIndex );
		if(!PtrToBool( thePointer )) {
			m_objectIndex = INDEX_NONE;
			return false;
		}
		return true;
	}
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
