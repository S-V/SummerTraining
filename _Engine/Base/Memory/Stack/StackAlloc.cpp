/*
=============================================================================
	File:	StackAlloc.cpp
	Desc:	Stack-based memory allocator.
=============================================================================
*/
#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#include <Base/Memory/Stack/StackAlloc.h>

/*
--------------------------------------------------------------
	StackAllocator
--------------------------------------------------------------
*/
StackAllocator::StackAllocator()
{
	m_marker = 0;
	m_capacity = 0;
	m_memory = nil;
}

ERet StackAllocator::Initialize( void* memory, UINT32 capacity )
{
	chkRET_X_IF_NIL(memory, ERR_NULL_POINTER_PASSED);
	chkRET_X_IF_NOT(capacity > 0, ERR_INVALID_PARAMETER);
	chkRET_X_IF_NOT(IS_16_BYTE_ALIGNED(memory), ERR_INVALID_PARAMETER);

	m_memory = memory;
	m_marker = 0;
	m_capacity = capacity;

	return ALL_OK;
}

void StackAllocator::Shutdown()
{
	m_memory = nil;
	m_marker = 0;
	m_capacity = 0;
}

char* StackAllocator::Alloc( UINT32 size )
{
	//mxASSERT(size >= MINIMUM_ALIGNMENT);
	const UINT32 alignedSize = TAlignUp< MINIMUM_ALIGNMENT >( size );

	// check if there's enough space
	chkRET_NIL_IF_NOT( m_marker + alignedSize <= m_capacity );

	void* ptr = mxAddByteOffset( m_memory, m_marker );
	m_marker += alignedSize;

#if MX_DEBUG
	memset(ptr, 0xCDCDCDCD, size);	// 0xCDCDCDCD is MVC++-specific (uninitialized heap memory)
#endif

	return (char*) ptr;
}

char* StackAllocator::AllocA( UINT32 size, UINT32 alignment )
{
	//mxASSERT(size >= MINIMUM_ALIGNMENT);
	//mxASSERT(IsValidAlignment(alignment));
	if( mxFAILED(this->AlignTo(alignment)) ) {
		return nil;
	}
	char* alignedAddress = this->Alloc( size );
	return alignedAddress;
}

ERet StackAllocator::AlignTo( UINT32 alignment )
{
	mxASSERT(IsValidAlignment(alignment));

	const UINT32 offset = ((m_marker + alignment-1) / alignment) * alignment;
	if( offset >= m_capacity ) {
		return ERR_OUT_OF_MEMORY;
	}

	return ALL_OK;
}

void StackAllocator::FreeTo( UINT32 marker )
{
	mxASSERT( marker < m_capacity );
	UINT32 size = m_marker - marker;
	// simply reset the pointer to the old position
	m_marker = marker;
#if MX_DEBUG
	void* start = mxAddByteOffset(m_memory, marker);
	memset(start, 0xFEEEFEEE, size);	// 0xFEEEFEEE is MVC++-specific (freed memory)
#endif
}

void StackAllocator::Reset()
{
	this->FreeTo( 0 );
}

UINT32 StackAllocator::Capacity() const
{
	return m_capacity;
}

UINT32 StackAllocator::Position() const
{
	return m_marker;
}

/*
--------------------------------------------------------------
	MemoryScope
--------------------------------------------------------------
*/
MemoryScope::MemoryScope( StackAllocator & alloc )
	: m_alloc( alloc )
	, m_parent( nil )
{
}

MemoryScope::MemoryScope( MemoryScope * parent )
	: m_alloc( parent->m_alloc )
	, m_parent( parent )
{
}

MemoryScope::~MemoryScope()
{
	Destructor* current = m_destructors;
	while( current )
	{
		(*current->fun)(current->object);
		current = current->next;
	}
	if( m_alloc.Owner() == this )
	{
		m_alloc.Reset();
		m_alloc.TransferOwnership( this, m_parent );
	}
}

void* MemoryScope::Alloc( UINT32 size )
{
	mxASSERT( !this->IsSealed() );
	return m_alloc.Alloc( size );
}

void* MemoryScope::AllocA( UINT32 size, UINT32 alignment )
{
	mxASSERT( !this->IsSealed() );
	return m_alloc.AllocA( size, alignment );
}

void MemoryScope::Unwind()
{
	mxASSERT( m_alloc.Owner() == this );
	Destructor* current = m_destructors;
	while( current )
	{
		(*current->fun)(current->object);
		current = current->next;
	}
	m_destructors = nil;
	m_alloc.Reset();
}

void MemoryScope::Seal()
{
	mxASSERT( !this->IsSealed() );
	m_alloc.TransferOwnership( this, m_parent );
}

bool MemoryScope::IsSealed() const
{
	return m_alloc.Owner() != this;
}

ERet MemoryScope::AddDestructor( void* object, FDestruct fn )
{
	mxASSERT( object );
	mxASSERT_PTR( fn );
	const UINT32 top = m_alloc.Position();
	Destructor* dtor = m_alloc.AllocMany< Destructor >( 1 );
	if( dtor )
	{
		this->AddDestructor( dtor, object, fn );
		return ALL_OK;
	}
	m_alloc.FreeTo(top);
	return ERR_OUT_OF_MEMORY;
}

UINT32 MemoryScope::Capacity() const
{
	return m_alloc.Capacity();
}

UINT32 MemoryScope::Position() const
{
	return m_alloc.Position();
}

void* MemoryScope::AllocMany( UINT32 count, UINT32 stride, FDestruct* fun )
{
	mxASSERT( m_alloc.Owner() == this );
	const UINT32 top = m_alloc.Position();
	m_alloc.AlignTo(DEFAULT_ALIGNMENT);

	Destructor* dtors = nil;
	if( fun )
	{
		dtors = m_alloc.AllocMany< Destructor >( count );
		if( !dtors ) {
			goto L_Error;
		}
	}

	void* objects = m_alloc.Alloc( count * stride );
	if( !objects ) {
		goto L_Error;
	}

	if( fun )
	{
		this->AddDestructors( dtors, count, objects, stride, fun );
	}

	return objects;

L_Error:
	m_alloc.FreeTo(top);
	return nil;
}

void MemoryScope::AddDestructor( Destructor* dtor, void* object, FDestruct* fun )
{
	dtor->fun = fun;
	dtor->next = m_destructors;
	dtor->object = object;
	m_destructors = dtor;
}

void MemoryScope::AddDestructors( Destructor* dtors, UINT32 count, void* objects, UINT32 stride, FDestruct* fun )
{
	Destructor* current = m_destructors;
	for( UINT32 iDtor = 0; iDtor < count; iDtor++ )
	{
		dtors[iDtor].fun = fun;
		dtors[iDtor].next = current;
		dtors[iDtor].object = objects;
		current = m_destructors + iDtor;
		objects = mxAddByteOffset(objects, stride);
	}
	m_destructors = current;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
