/*=============================================================================
//	File:	TDoublyLinkedList.h
//	Desc:	Doubly linked list template.
//	ToDo:	rewrite optimally
=============================================================================*/
#pragma once

/*
--------------------------------------------------------------
	Doubly linked list template
--------------------------------------------------------------
*/
template< class CLASS >	// where CLASS : TDoublyLinkedList< CLASS >
struct TDoublyLinkedList
{
	CLASS *		next;	// the next item in a singly linked list, null if this is the last item
	CLASS *		prev;

public:
	typedef CLASS* Head;

	TDoublyLinkedList()
	{
		next = nil;
		prev = nil;
	}

	bool IsLoose() const
	{
		return next == nil
			&& prev == nil
			;
	}

	bool FindSelfInList( const CLASS* head ) const
	{
		const CLASS* curr = head;
		while(PtrToBool( curr ) && curr != this )
		{
			curr = curr->next;
		}
		return PtrToBool(curr);
	}

	// insert 'this' between 'node.prev' and 'node'
	void InsertBefore( CLASS * node )
	{
		mxASSERT_PTR(node);
		this->RemoveSelfFromList();

		CLASS* me = static_cast< CLASS* >( this );

		CLASS* hisPrev = node->prev;

		this->next = node;
		this->prev = hisPrev;

		if( hisPrev ) {
			hisPrev->next = me;
		}
		node->prev = me;
	}

	// insert 'this' between 'node' and 'node.next'
	void InsertAfter( CLASS * node )
	{
		mxASSERT_PTR(node);
		this->RemoveSelfFromList();

		CLASS* me = static_cast< CLASS* >( this );

		CLASS* hisNext = node->next;

		this->next = hisNext;
		this->prev = node;

		if( hisNext ) {
			hisNext->prev = me;
		}
		node->next = me;
	}

	// unlinks this node from the containing list
	void RemoveSelfFromList()
	{
		if( prev ) {
			prev->next = next;
		}
		if( next ) {
			next->prev = prev;
		}
		prev = nil;
		next = nil;
	}

	// prepends this node to the given list
	// (will make this the head of the list if the list is nil)
	// NOTE: the head of the list can be a null pointer (if empty list)
	void PrependSelfToList( CLASS ** head )
	{
		mxASSERT(this->IsLoose());
		mxASSERT_PTR(head);

		CLASS* me = static_cast< CLASS* >( this );

		if( *head )
		{
			CLASS* herPrev = (*head)->prev;
			//CLASS* herNext = (*head)->next;

			// insert this node between head->prev and head :
			//
			// ... herPrev <-> head <-> herNext ...
			// =>
			// ... herPrev <-> this <-> head <-> herNext ...

			this->next = *head;
			(*head)->prev = me;

			this->prev = herPrev;
			if( herPrev ) {
				herPrev->next = me;
			}
		}
		else
		{
			*head = me;
		}
	}

	// appends this node to the given list
	// (will make this the head of the list if the list is nil)
	// NOTE: the head of the list can be a null pointer (if empty list)
	void AppendSelfToList( CLASS ** head )
	{
		mxASSERT(this->IsLoose());
		mxASSERT_PTR(head);

		CLASS* me = static_cast< CLASS* >( this );

		if( *head )
		{
			//CLASS* herPrev = (*head)->prev;
			CLASS* herNext = (*head)->next;

			// insert this node after head and before head->next :
			//
			// ... herPrev <-> head <-> herNext ...
			// =>
			// ... herPrev <-> head <-> this <-> herNext ...

			this->prev = *head;
			(*head)->next = me;

			this->next = herNext;
			if( herNext ) {
				herNext->prev = me;
			}
		}
		else
		{
			*head = me;
		}
	}

public:
	static UINT CountItemsInList( const CLASS* head )
	{
		UINT result = 0;
		const CLASS* current = head;
		while(PtrToBool( current ))
		{
			result++;
			current = current->next;
		}
		return result;
	}
	static CLASS* FindTail( CLASS* head, UINT *count = nil )
	{
		CLASS* result = head;
		CLASS* current = head;
		UINT itemCount = 0;
		while(PtrToBool( current ))
		{
			itemCount++;
			result = current;
			current = current->next;
		}
		if( count != nil ) { *count = itemCount; }
		return result;
	}
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
