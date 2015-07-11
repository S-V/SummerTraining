/*
=============================================================================
	File:	TPool.h
	Desc:	Index-based growing free list.
=============================================================================
*/
#pragma once

#include <Base/Template/Containers/Array/TBuffer.h>

template< class TYPE, typename INDEX >
void TBuildFreeList(
					TYPE * array_start,	// the start of the whole array
					UINT32 first_index,	// number of 'live' elements (>=0)
					UINT32 array_count,	// the size of the array
					INDEX null_handle = -1
					)
{
	mxSTATIC_ASSERT(sizeof(array_start[0]) >= sizeof(INDEX));
	struct UPtr {
		INDEX	next;
	};
	UINT32 i = first_index;
	while( i < array_count )
	{
		((UPtr&)array_start[i]).next = INDEX( i + 1 );
		i++;
	}
	// The tail of the free list points to NULL.
	((UPtr&)array_start[i-1]).next = null_handle;
}

template< class TYPE >	// where TYPE is bitwise copyable
struct TPool
	: public TArrayBase< TYPE, TPool< TYPE > >
{
	typedef TBuffer< TYPE, UINT32 > BufferT;	// 'Storage' type
	typedef UINT32 IndexT;	// 'Handle' type

	BufferT		m_items;		// memory for storing items
	UINT32		m_liveCount;	// number of active objects
	IndexT		m_firstFree;	// index of the first free item (head of the linked list)

	//static const IndexT NIL_HANDLE = (IndexT)~0UL;
	enum { NIL_HANDLE = mxMiB(1) };

	struct UPtr {
		IndexT	nextFree;
	};
	static inline UPtr& AsUPtr( TYPE& o ) {
		return *c_cast(UPtr*) &o;
	}
	mxSTATIC_ASSERT(sizeof(TYPE) >= sizeof(UPtr));
public:
	TPool()
	{
		m_liveCount = 0;
		m_firstFree = NIL_HANDLE;
	}
	~TPool()
	{
		mxASSERT2( m_liveCount == 0, "Some handles haven't been released!" );
	}

	//=== TArrayBase
	// NOTE: returns the array capacity, not the number of live objects!
	inline UINT Num() const				{ return m_items.Num(); }
	inline TYPE* ToPtr()				{ return m_items.ToPtr(); }
	inline const TYPE* ToPtr() const	{ return m_items.ToPtr(); }

	inline UINT NumValidItems() const	{ return m_liveCount; }

	void Empty()
	{
		m_liveCount = 0;
		if( m_items.Num() ) {
			TBuildFreeList( m_items.ToPtr(), 0, m_items.Num(), NIL_HANDLE );
			m_firstFree = 0;
		} else {
			mxASSERT( m_firstFree == NIL_HANDLE );
		}
	}

	// Tries to find a free slot, doesn't allocate memory.
	inline IndexT FindFree()
	{
		if( m_firstFree != NIL_HANDLE )
		{
			const IndexT freeIndex = m_firstFree;
			const UPtr& listNode = AsUPtr( m_items[ freeIndex ] );
			m_firstFree = listNode.nextFree;
			m_liveCount++;
			return freeIndex;
		}
		return NIL_HANDLE;
	}
	inline IndexT GetFree()
	{
		const IndexT freeIndex = this->FindFree();
		mxASSERT( freeIndex != NIL_HANDLE );
		return freeIndex;
	}
	// Tries to find a free slot, allocates memory if needed.
	inline IndexT Alloc()
	{
		// First check if the array needs to grow.
		const UINT32 oldCount = m_items.Num();
		if(mxUNLIKELY( m_liveCount >= oldCount ))
		{
			mxASSERT2( m_liveCount < NIL_HANDLE, "Too many objects" );
			mxASSERT2( m_firstFree == NIL_HANDLE, "Tail free list must be NULL" );

			// Grow the array by doubling its capacity.
			const UINT32 newCount = NextPowerOfTwo( oldCount );
			m_items.SetNum( newCount );

			TYPE* a = m_items.ToPtr();

			TBuildFreeList( a, oldCount, newCount, NIL_HANDLE );

			// Free list head now points to the first added item.
			m_firstFree = oldCount;
		}
		// Now that we have ensured enough space, there must be an available slot.
		return this->GetFree();
	}
	inline void Free( IndexT id )
	{
		mxASSERT2(m_liveCount > 0, "Cannot delete items from empty pool");

		TYPE* a = m_items.ToPtr();

		TYPE& o = a[ id ];
		//o.~TYPE();
		AsUPtr( o ).nextFree = m_firstFree;

		m_firstFree = id;

		--m_liveCount;
	}

public:	// Binary Serialization.
	friend AStreamWriter& operator << ( AStreamWriter& file, const TPool< TYPE >& o )
	{
		//const UINT32 numAllocated = o.m_items.Num();
		const UINT32 numLiveItems = o.m_liveCount;
		const UINT32 firstFreeIdx = o.m_firstFree;
		file << numLiveItems;
		file << firstFreeIdx;
		file << o.m_items;
		return file;
	}
	friend AStreamReader& operator >> ( AStreamReader& file, TPool< TYPE >& o )
	{
		UINT32 numLiveItems;
		file >> numLiveItems;
		o.m_liveCount = numLiveItems;

		UINT32 firstFreeIdx;
		file >> firstFreeIdx;
		o.m_firstFree = firstFreeIdx;

		file >> o.m_items;

		return file;
	}
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
