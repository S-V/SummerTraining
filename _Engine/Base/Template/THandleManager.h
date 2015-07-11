// a resizable array-based handle manager
//
// can store only bitwise copyable objects (they are copied when the array grows);
// (this could easily be avoided by hardcoding array size or just use FindFree());
// could also embed a small counter into objects to track dead items (tombstones);
// sort free list of deleted items in order of increasing addresses for iteration?
// @todo: make it non-template to prevent code bloat, pass element stride in ctor?
//
#pragma once

template< class TYPE >	// where TYPE is bitwise copyable, HANDLE
struct THandleManager
	: public TArrayBase< TYPE, THandleManager< TYPE > >
{
	TYPE *	m_items;		// memory for storing items
	UINT16	m_allocated;	// count of allocated items
	UINT16	m_liveCount;	// number of active objects
	UINT16	m_firstFree;	// index of the first free item (head of the linked list)

	enum { NULL_HANDLE = (UINT16)~0UL };
	struct UPtr {
		UINT16	nextFree;
	};
	static inline UPtr& AsUPtr( TYPE& o ) {
		return *c_cast(UPtr*) &o;
	}
	mxSTATIC_ASSERT(sizeof(TYPE) >= sizeof(UINT16));

public:
	THandleManager()
	{
		m_items = nil;
		m_allocated = 0;
		m_liveCount = 0;
		m_firstFree = NULL_HANDLE;
	}
	~THandleManager()
	{
		mxASSERT2( m_liveCount == 0, "Some handles haven't been released!" );
		if( m_items ) {
			TDestructN_IfNonPOD( m_items, m_allocated );
			mxFree(m_items);
			m_items = nil;
			m_allocated = 0;
		}
	}

	//=== TArrayBase
	// NOTE: returns the array capacity, not the number of live objects!
	inline UINT Num() const				{ return m_allocated; }
	inline TYPE* ToPtr()				{ return m_items; }
	inline const TYPE* ToPtr() const	{ return m_items; }

	inline UINT NumValidItems() const	{ return m_liveCount; }

	void Empty()
	{
		m_liveCount = 0;
	}

	// Tries to find a free slot, doesn't allocate memory.
	inline UINT16 FindFree()
	{
		if( m_firstFree != NULL_HANDLE )
		{
			const UINT firstFree = m_firstFree;
			const UPtr& listNode = AsUPtr( m_items[ firstFree ] );
			m_firstFree = listNode.nextFree;
			m_liveCount++;
			return firstFree;
		}
		return NULL_HANDLE;
	}
	inline UINT16 GetFree()
	{
		const UINT firstFree = this->FindFree();
		mxASSERT( firstFree != NULL_HANDLE );
		return firstFree;
	}
	// Tries to find a free slot, allocates memory if needed.
	inline UINT16 Alloc()
	{
		// First check if the array needs to grow.
		const UINT oldCount = m_allocated;
		if(mxUNLIKELY( m_liveCount >= oldCount ))
		{
			mxASSERT2( m_liveCount < MAX_UINT16, "Too many objects" );
			mxASSERT2( m_firstFree == NULL_HANDLE, "Tail free list must be NULL" );

			const UINT newCount = NextPowerOfTwo( oldCount );

			TYPE* newArray = c_cast(TYPE*) mxAlloc( newCount * sizeof(TYPE) );
			if( !newArray ) {
				return NULL_HANDLE;
			}
			TYPE* oldArray = m_items;
			if( PtrToBool(oldArray) && oldCount )
			{
				TCopyConstructArray( newArray, oldArray, oldCount );
				TDestructN_IfNonPOD( oldArray, oldCount );
				mxFree(m_items);
			}

			const UINT numNewItems = newCount - oldCount;
			TConstructN_IfNonPOD( newArray + oldCount, numNewItems );

			m_items = newArray;
			m_allocated = newCount;

			// Build the free list out of newly created elements.
			for( UINT i = oldCount; i < newCount; i++ )
			{
				AsUPtr( newArray[i] ).nextFree = i + 1;
			}
			AsUPtr( newArray[newCount - 1] ).nextFree = NULL_HANDLE;
			// Free list head now points to the first added item.
			m_firstFree = oldCount;
		}
		// Now that we have ensured enough space, there must be an available slot.
		return this->GetFree();
	}

	inline void Free( UINT16 id )
	{
		mxASSERT2(m_liveCount > 0, "Cannot delete items from empty pool");

		TYPE* a = m_items;

		TYPE& o = a[ id ];
		o.~TYPE();
		mxUNUSED(o);

		// The free list must be sorted by object indices (== addresses in our case).
		// Iterate over the free list and find the correct position for insertion.

		UINT16 prev = NULL_HANDLE;
		UINT16 curr = m_firstFree;
		while( curr != NULL_HANDLE )
		{
			mxASSERT2(curr != id, "The object being deleted has already been deleted");
			if( curr > id ) {
				break;	// 'id' will be inserted right before 'curr' (between 'prev' and 'curr')
			}
			prev = curr;
			curr = AsUPtr(a[curr]).nextFree;
		}

		if( prev != NULL_HANDLE )
		{
			AsUPtr(a[id]).nextFree = AsUPtr(a[prev]).nextFree;	// item->next = prev->next
			AsUPtr(a[prev]).nextFree = id;						// prev->next = item
		}
		else
		{
			AsUPtr(a[id]).nextFree = m_firstFree;
			m_firstFree = id;
		}

		--m_liveCount;
	}

public:
	struct Iterator
	{
		TYPE *	m_objects;
		UINT16	m_current;		// pointer to the current element
		UINT16	m_nextFree;		// next free object in the sorted free list
		UINT16	m_remaining;	// number of remaining valid objects to iterate over
	public:
		Iterator( THandleManager& items )
		{
			m_objects		= items.ToPtr();
			m_current		= 0;
			m_nextFree		= items.m_firstFree;
			m_remaining		= items.m_liveCount;
			// skip freelisted elements and move to the first valid object
			while( m_remaining )
			{
				// move forward until we hit a live object
				if( m_current != m_nextFree ) {
					break;
				}
				m_current++;
				m_nextFree = AsUPtr(m_objects[m_nextFree]).nextFree;
				m_remaining--;
			}
		}
		void MoveToNext()
		{
			// move to the next live object
			while( m_remaining )
			{
				++m_current;
				--m_remaining;
				// move forward until we hit an active object
				if( m_current != m_nextFree ) {
					break;
				}
				m_nextFree = AsUPtr(m_objects[m_nextFree]).nextFree;
			}
		}
		bool IsValid() const	{ return m_remaining > 0; }
		TYPE& Value() const		{ return m_objects[m_current]; }
	};
};

template< class TYPE >	// where TYPE has "Destroy()".
void TDestroyLiveObjects( THandleManager< TYPE > & handles )
{
	if( handles.NumValidItems() )
	{
		ptWARN("%u live object(s) of type '%s'", handles.NumValidItems(), typeid(TYPE).name());

		THandleManager< TYPE >::Iterator	it( handles );
		while( it.IsValid() )
		{
			TYPE& o = it.Value();
			o.Destroy();
			it.MoveToNext();
		}

		handles.Empty();
	}
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
