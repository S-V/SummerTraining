/*
=============================================================================
	File:	ReferenceCounted.h
	Desc:	A very simple basic class for reference counted objects (analog of Boost's 'intrusive_ptr').
	Note:	Reference counted objects MUST be accessed through smart pointers
			at all times to make sure reference counting scheme operates correctly.
			Failure to do so will result in strange and unpredictable behaviour and crashes.
=============================================================================
*/
#pragma once

/*
=======================================================

	ReferenceCounted

=======================================================
*/

//
//	ReferenceCounted - calls 'delete' on the object when the reference count reaches zero.
//
struct ReferenceCounted
{
	void	Grab() const;
	bool	Drop() const;	// ( Returns true if the object has been deleted. )

	INT	GetReferenceCount() const;

	ReferenceCounted& operator = ( const ReferenceCounted& other );

protected:
			ReferenceCounted();
	virtual	~ReferenceCounted();

private:
	volatile mutable INT	referenceCounter;	// Number of references to this object.
};

/*================================
		ReferenceCounted
================================*/

mxFORCEINLINE ReferenceCounted::ReferenceCounted()
	: referenceCounter( 0 )
{}

mxFORCEINLINE ReferenceCounted::~ReferenceCounted()
{
	mxASSERT( 0 == referenceCounter );
}

mxFORCEINLINE void ReferenceCounted::Grab() const {
	++referenceCounter;
}

mxFORCEINLINE bool ReferenceCounted::Drop() const
{	mxASSERT( referenceCounter > 0 );
	--referenceCounter;
	if ( referenceCounter == 0 )
	{
		// 'delete this' is evil (?)
		delete( this );
		return true;
	}
	return false;
}

mxFORCEINLINE INT ReferenceCounted::GetReferenceCount() const
{
	return referenceCounter;
}

mxFORCEINLINE ReferenceCounted& ReferenceCounted::operator = ( const ReferenceCounted& other )
{
	return *this;
}

//
//	ReferenceCountedX - the same as ReferenceCounted, but calls Destroy() when the reference count reaches zero.
//
struct ReferenceCountedX
{
	// 'delete this' is evil (?)
	virtual void Destroy() { delete( this ); }

	void	Grab() const;
	bool	Drop() const;	// ( Returns true if the object has been deleted. )

	INT	GetReferenceCount() const;

	ReferenceCountedX& operator = ( const ReferenceCountedX& other );

	// so that property system works
	mxFORCEINLINE UINT32 GetNumRefs() const
	{
		return static_cast<UINT32>(this->GetReferenceCount());
	}

protected:
			ReferenceCountedX();
	virtual	~ReferenceCountedX();

private:
	volatile mutable INT	referenceCounter;	// Number of references to this object.
};

/*================================
		ReferenceCountedX
================================*/

mxFORCEINLINE ReferenceCountedX::ReferenceCountedX()
	: referenceCounter( 0 )
{}

mxFORCEINLINE ReferenceCountedX::~ReferenceCountedX()
{
	mxASSERT( 0 == referenceCounter );
}

mxFORCEINLINE void ReferenceCountedX::Grab() const {
	++referenceCounter;
}

mxFORCEINLINE bool ReferenceCountedX::Drop() const
{	mxASSERT( referenceCounter > 0 );
	--referenceCounter;
	if ( referenceCounter == 0 )
	{
		ReferenceCountedX* nonConstPtr = const_cast<ReferenceCountedX*>(this);
		nonConstPtr->Destroy();
		return true;
	}
	return false;
}

mxFORCEINLINE INT ReferenceCountedX::GetReferenceCount() const
{
	return referenceCounter;
}

mxFORCEINLINE ReferenceCountedX& ReferenceCountedX::operator = ( const ReferenceCountedX& other )
{
	return *this;
}

//----------------------------------------------------------------------------------------------------------------------

//
//	Grab( ReferenceCounted* ) - increments the reference count of the given object.
//
mxFORCEINLINE void GRAB( ReferenceCounted* p )
{
	if (PtrToBool( p ))
	{
		p->Grab();
	}
}

//
//	Drop( ReferenceCounted * ) - decrements the reference count of the given object and sets the pointer to null.
//
mxFORCEINLINE void DROP( ReferenceCounted *& p )
{
	if (PtrToBool( p ))
	{
		p->Drop();
		p = nil;
	}
}

//----------------------------------------------------------------------------------------------------------------------

//
//	TRefPtr< T > - a pointer to a reference-counted object (aka 'instrusive pointer').
//
template< typename T >	// where T : ReferenceCounted
class TRefPtr
{
	T *		m_pObject;	// The shared reference counted object.

public:
			TRefPtr();
			TRefPtr( T* refCounted );
			TRefPtr( T* refCounted, EDontAddRef );
			TRefPtr( const TRefPtr& other );
			~TRefPtr();

	// Implicit conversions.

			operator T*	() const;
	T &		operator *	() const;
	T *		operator ->	() const;

	bool	operator !	() const;

	// Assignment.

	TRefPtr &	operator = ( T* pObject );
	TRefPtr &	operator = ( const TRefPtr& other );

	// Comparisons.

	bool	operator == ( T* pObject ) const;
	bool	operator != ( T* pObject ) const;
	bool	operator == ( const TRefPtr& other ) const;
	bool	operator != ( const TRefPtr& other ) const;

	mxFORCEINLINE		bool	isNull() const { return (nil == m_pObject); }
	mxFORCEINLINE		bool	isValid() const { return (nil != m_pObject); }

	// Unsafe...
	mxFORCEINLINE		T *		get_ptr()	{ return m_pObject; }
	mxFORCEINLINE		T *&	get_ref()	{ return m_pObject; }

private:
	void _dbgChecks() { mxCOMPILE_TIME_ASSERT(sizeof(*this) == sizeof(T*)); }
};

template< typename T >
mxFORCEINLINE TRefPtr< T >::TRefPtr()
	: m_pObject( nil )
{
	_dbgChecks();
}

template< typename T >
mxFORCEINLINE TRefPtr< T >::TRefPtr( T* refCounted )
	: m_pObject( refCounted )
{
	if (PtrToBool( m_pObject )) {
		m_pObject->Grab();
	}
	_dbgChecks();
}

template< typename T >
mxFORCEINLINE TRefPtr< T >::TRefPtr( T* refCounted, EDontAddRef )
	: m_pObject( refCounted )
{
	_dbgChecks();
}

template< typename T >
mxFORCEINLINE TRefPtr< T >::TRefPtr( const TRefPtr& other )
	: m_pObject( other.m_pObject )
{
	if (PtrToBool( m_pObject )) {
		m_pObject->Grab();
	}
	_dbgChecks();
}

template< typename T >
mxFORCEINLINE TRefPtr< T >::~TRefPtr()
{
	if (PtrToBool( m_pObject )) {
		m_pObject->Drop();
	}
	m_pObject = nil;
}

template< typename T >
mxFORCEINLINE TRefPtr< T >::operator T* () const
{
	return m_pObject;
}

template< typename T >
mxFORCEINLINE T & TRefPtr< T >::operator * () const
{	mxASSERT( m_pObject );
	return *m_pObject;
}

template< typename T >
mxFORCEINLINE T * TRefPtr< T >::operator -> () const
{
	return m_pObject;
}

template< typename T >
mxFORCEINLINE bool TRefPtr< T >::operator ! () const
{
	return m_pObject == nil;
}

template< typename T >
mxFORCEINLINE TRefPtr< T > & TRefPtr< T >::operator = ( T* pObject )
{
	if ( pObject == nil )
	{
		if (PtrToBool( m_pObject )) {
			m_pObject->Drop();
			m_pObject = nil;
		}
		return *this;
	}
	if ( m_pObject != pObject )
	{
		if (PtrToBool( m_pObject )) {
			m_pObject->Drop();
		}
		m_pObject = pObject;
		m_pObject->Grab();
		return *this;
	}
	return *this;
}

template< typename T >
mxFORCEINLINE TRefPtr< T > & TRefPtr< T >::operator = ( const TRefPtr& other )
{
	return ( *this = other.m_pObject );
}

template< typename T >
mxFORCEINLINE bool TRefPtr< T >::operator == ( T* pObject ) const
{
	return m_pObject == pObject;
}

template< typename T >
mxFORCEINLINE bool TRefPtr< T >::operator != ( T* pObject ) const
{
	return m_pObject != pObject;
}

template< typename T >
mxFORCEINLINE bool TRefPtr< T >::operator == ( const TRefPtr& other ) const
{
	return m_pObject == other.m_pObject;
}

template< typename T >
mxFORCEINLINE bool TRefPtr< T >::operator != ( const TRefPtr& other ) const
{
	return m_pObject != other.m_pObject;
}

/*
--------------------------------------------------------------
	TRefCounted< T >
--------------------------------------------------------------
*/
template< class TYPE >
class TRefCounted : public ReferenceCounted
{
public:
	typedef TRefPtr< TYPE >	Ref;
};

//----------------------------------------------------------------------------------------------------------------------

template< class TYPE >
class RefPtrDropUtil {
public:
	inline RefPtrDropUtil()
	{}

	inline void operator () ( TRefPtr<TYPE>& ptr ) {
		ptr = nil;
	}
	inline void operator () ( ReferenceCounted* ptr ) {
		ptr->Drop();
	}
	inline void operator () ( ReferenceCountedX* ptr ) {
		ptr->Drop();
	}
};

template< class TYPE >
class RefPtrDropUtilX
{
	UINT numDecrements;

public:
	inline RefPtrDropUtilX( UINT numDecrements = 1 )
		: numDecrements(numDecrements)
	{}

	inline void operator () ( TRefPtr<TYPE>& ptr ) {
		for( UINT i = 0; i < numDecrements; i++ ) {
			ptr = nil;
		}
	}
	inline void operator () ( ReferenceCounted* ptr ) {
		for( UINT i = 0; i < numDecrements; i++ ) {
			ptr->Drop();
		}
	}
	inline void operator () ( ReferenceCountedX* ptr ) {
		for( UINT i = 0; i < numDecrements; i++ ) {
			ptr->Drop();
		}
	}
};

template< typename T >
inline const T& GetConstObjectReference( const TRefPtr< T >& o )
{
	return *o;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
