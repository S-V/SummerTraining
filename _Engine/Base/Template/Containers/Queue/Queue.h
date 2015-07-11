/*
=============================================================================
	File:	Queue.h
	Desc:	A FIFO circular queue.
	ToDo:	Stop reinventing the wheel.
=============================================================================
*/

#ifndef __MX_CONTAINTERS_FIFO_QUEUE_H__
#define __MX_CONTAINTERS_FIFO_QUEUE_H__



//
//	TQueue< TYPE >
//
template< typename TYPE >
class TQueue
	: public TArrayBase< TYPE, TQueue<TYPE> >
{
	// queue memory management
	TArray< TYPE >	mData;

public:
	typedef TQueue
	<
		TYPE
	>
	THIS_TYPE;

	typedef
	TYPE
	ITEM_TYPE;

	typedef TypeTrait
	<
		TYPE
	>
	TYPE_TRAIT;

public:
	// Creates a zero length queue.
	mxFORCEINLINE TQueue()
	{}

	// Creates a zero length queue and sets the memory manager.
	mxFORCEINLINE explicit TQueue( HMemory hMemoryMgr )
		: mData( hMemoryMgr )
	{}

	// Use it only if you know what you're doing.
	mxFORCEINLINE explicit TQueue(ENoInit)
		: mData( EMemHeap::DefaultHeap )
	{}

	// Deallocates queue memory.
	mxFORCEINLINE ~TQueue()
	{
		this->Clear();
	}

	// Returns the total capacity of the queue storage.
	mxFORCEINLINE UINT Capacity() const
	{
		return mData.Capacity();
	}

	// Convenience function to get the number of elements in this queue.
	mxFORCEINLINE UINT Num() const
	{
		return mData.Num();
	}

	mxFORCEINLINE void Reserve( UINT numElements )
	{
		mData.Reserve( numElements );
	}

	// Convenience function to empty the queue. Doesn't release allocated memory.
	// Invokes objects' destructors.
	mxFORCEINLINE void Empty()
	{
		mData.Empty();
	}

	// Releases allocated memory (calling destructors of elements) and empties the queue.
	mxFORCEINLINE void Clear()
	{
		mData.Clear();
	}

	// Accesses the element at the front of the queue but does not remove it.
	mxFORCEINLINE TYPE& Peek()
	{
		return mData.GetFirst();
	}
	mxFORCEINLINE const TYPE& Peek() const
	{
		return mData.GetFirst();
	}

	// Places a new element to the back of the queue and expand storage if necessary.
	mxFORCEINLINE void Enqueue( const TYPE& newOne )
	{
		mData.Add( newOne );
	}

	// Fills in the data with the element at the front of the queue
	// and removes the element from the front of the queue.
	mxFORCEINLINE void Deque( TYPE &element )
	{
		element = mData.GetFirst();
		mData.RemoveAt( 0 );
	}
	mxFORCEINLINE void Deque()
	{
		mData.RemoveAt( 0 );
	}

	mxFORCEINLINE void Add( const TYPE& newOne )
	{
		Enqueue( newOne );
	}
	mxFORCEINLINE TYPE& Add()
	{
		return mData.Add();
	}

	mxFORCEINLINE TYPE* ToPtr() {
		return mData.ToPtr();
	}
	mxFORCEINLINE const TYPE* ToPtr() const {
		return mData.ToPtr();
	}


public_internal:
	void SetNum_Unsafe( UINT newNum )
	{
		mData.SetNum_Unsafe( newNum );
	}

private:
	NO_ASSIGNMENT(THIS_TYPE);
	NO_COPY_CONSTRUCTOR(THIS_TYPE);
	NO_COMPARES(THIS_TYPE);
};



#endif // ! __MX_CONTAINTERS_QUEUE_H__

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
