/*
=============================================================================
	File:	StackAlloc.h
	Desc:	Stack- and scope-based memory allocators.
	Note:	inspired by Brooke Hodgman (author of the Eighth Engine), see:
http://eighthengine.com/2013/08/30/a-memory-management-adventure-2/
https://code.google.com/p/eight/source/browse/include/eight/core/alloc/scope.h
=============================================================================
*/
#pragma once

/*
--------------------------------------------------------------
	StackAllocator
--------------------------------------------------------------
*/
class StackAllocator
{
	void *	m_memory;	
	UINT32	m_marker;
	UINT32	m_capacity;
	const void *m_owner;

public:
	StackAllocator();

	ERet Initialize( void* memory, UINT32 capacity );
	void Shutdown();

	char* Alloc( UINT32 size );
	char* AllocA( UINT32 size, UINT32 alignment = EFFICIENT_ALIGNMENT );

	ERet AlignTo( UINT32 alignment );
	void FreeTo( UINT32 marker );
	void Reset();

	UINT32 Capacity() const;
	UINT32 Position() const;

	void* GetBufferPtr() { return m_memory; }

	void TransferOwnership( const void* from, const void* to )
	{
		mxASSERT(from == m_owner);
		m_owner = to;
	}
	const void* Owner() const
	{
		return m_owner;
	}

	template< typename TYPE >
	inline TYPE* AllocMany( UINT32 numObjects, UINT32 alignment = EFFICIENT_ALIGNMENT )
	{
		return (TYPE*) AllocA( numObjects * sizeof(TYPE), alignment );
	}
};

/*
--------------------------------------------------------------
	ScopedStackAlloc
	frees the memory upon leaving the scope
--------------------------------------------------------------
*/
class ScopedStackAlloc
{
	StackAllocator &	allocator;
	const UINT32		marker;

public:
	inline ScopedStackAlloc( StackAllocator & alloc )
		: allocator( alloc )
		, marker( alloc.Position() )
	{}
	inline ~ScopedStackAlloc()
	{
		allocator.FreeTo( marker );
	}
	inline char* Alloc( UINT32 size )
	{
		return allocator.Alloc( size );
	}
	inline char* AllocA( UINT32 size, UINT32 alignment = EFFICIENT_ALIGNMENT )
	{
		return allocator.AllocA( size, alignment );
	}

	template< typename TYPE >
	inline TYPE* AllocMany( UINT32 count, UINT32 alignment = EFFICIENT_ALIGNMENT )
	{
		return (TYPE*) AllocA( count * sizeof(TYPE), alignment );
	}
};

/*
--------------------------------------------------------------
	MemoryScope - is a trashable heap.
--------------------------------------------------------------
*/
class MemoryScope {
public:
	enum { DEFAULT_ALIGNMENT = 16 };

	typedef FCallback FDestruct;

	MemoryScope( StackAllocator & alloc );
	MemoryScope( MemoryScope * parent );
	~MemoryScope();

	void* Alloc( UINT32 size );
	void* AllocA( UINT32 size, UINT32 alignment = EFFICIENT_ALIGNMENT );

	void Unwind();
	void Seal();
	bool IsSealed() const;

	ERet AddDestructor( void* object, FDestruct fn );

	UINT32 Capacity() const;
	UINT32 Position() const;

	template< typename TYPE >
	inline TYPE* New()
	{
		FDestruct* dtor = &FDestructorCallback< TYPE >;
		return (TYPE*) this->AllocMany( 1, sizeof(TYPE), dtor );
	}
	template< typename TYPE >
	inline TYPE* NewArray( UINT32 count )
	{
		FDestruct* dtor = &FDestructorCallback< TYPE >;
		return (TYPE*) this->AllocMany( count, sizeof(TYPE), dtor );
	}

private:
	struct Destructor
	{
		FDestruct *		fun;
		Destructor *	next;
		void *			object;
		void *			_pad16;
	};

	void* AllocMany( UINT32 count, UINT32 stride, FDestruct* fun );

	void AddDestructor( Destructor* dtor, void* object, FDestruct* fun );
	void AddDestructors( Destructor* dtors, UINT32 count, void* objects, UINT32 stride, FDestruct* fun );

	StackAllocator &	m_alloc;
	MemoryScope *		m_parent;	//<= optional
	Destructor *		m_destructors;
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
