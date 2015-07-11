/*
=============================================================================
	File:	ObjectModel.h
	Desc:	Object system.
			Stolen from PhyreENGINE.
=============================================================================
*/
#pragma once

#include <Base/Memory/FreeList/FreeList.h>

#include <Core/Core.h>
#include <Core/Asset.h>
#include <Core/Editor.h>

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*
-----------------------------------------------------------------------------
	Object buffers store objects of a certain type (derived from CStruct).

	Storing related data together improves locality of reference
	and allows for more efficient cache usage.

	Note:
	very good for storing fixed-size POD structs;
	doesn't support polymorphic types.

	Implementation:
		Once created, an object list cannot be resized.
		Dead (deleted) objects are kept in a free list sorted by objects' addresses.
	We sacrifice element contiguity in favor of stability; references and iterators
	to a given element remain valid as long as the element is not erased.
-----------------------------------------------------------------------------
*/
struct ObjectList : public CStruct, public TSinglyLinkedList< ObjectList >
{
	TPtr< const mxClass >	m_type;	// the class of objects stored in this list

	FixedListAllocator	m_freeList;	// (sorted) free list for reusing dead objects

	UINT32	m_count;	// number of valid (live) objects in this list
	UINT32	m_capacity;	// maximum number of objects that can be stored in this list
	UINT32	m_flags;

	enum Flags
	{
		CanFreeMemory = BIT(0),
	};

public:
	mxDECLARE_CLASS(ObjectList,CStruct);
	mxDECLARE_REFLECTION;
	ObjectList( const mxClass& type, UINT32 capacity );
	~ObjectList();

	const mxClass& GetType() const;
	UINT32 GetStride() const;

	bool HasAddress( const void* ptr ) const;

	// returns true if the given item is live in this object list
	bool HasValidItem( const void* item ) const;

	// returns true if this list owns memory block for the given object
	bool ContainsItem( const void* item ) const;

	UINT32 IndexOfContainedItem( const void* item ) const;

	// destroys the object and adds it to the free list
	ERet DeleteItem( void* item );

	// tries to allocate a new item, doesn't call a constructor
	CStruct* Allocate();

	// returns the number of valid objects
	UINT32 Num() const;

	// returns the maximum amount of objects which can be stored in this list
	UINT32 Capacity() const;

	// returns a pointer to the array of stored elements
	CStruct* GetArrayPtr() const;

	CStruct* GetItemAtIndex( UINT32 itemIndex ) const;

	CStruct* GetFirstFreeItem() const;

	// doesn't release allocated memory
	void Empty();

	// destroys all live objects and frees allocated memory
	void Clear();

	void IterateItems( Reflection::AVisitor* visitor, void* userData ) const;
	void IterateItems( Reflection::AVisitor2* visitor, void* userData ) const;

	void DbgCheckPointers();

public:
	// Base class for iterating over live objects inside an ObjectList.
	class IteratorBase
	{
	protected:
		void *	m_current;		// pointer to the current element
		void *	m_nextFree;		// next free object in the sorted free list
		UINT32	m_stride;		// byte distance between objects, in bytes
		UINT32	m_remaining;	// number of remaining (live) objects to iterate over

	public:
		IteratorBase();
		IteratorBase( const ObjectList& objectList );

		bool IsValid() const;
		void MoveToNext();
		void Skip( UINT32 count );
		UINT32 NumRemaining() const;
		UINT32 NumContiguousObjects() const;

		void Initialize( const ObjectList& objectList );
		void Reset();

		void* ToVoidPtr() const;

		template< class CLASS >
		inline CLASS* ToPtr() const
		{
			return static_cast< CLASS* >( m_current );
		}
		template< class CLASS >
		inline CLASS& ToValue() const
		{
			return *this->ToPtr< CLASS >();
		}

	private:PREVENT_COPY(IteratorBase);
	};

private:
	PREVENT_COPY(ObjectList);
};

// a type-safe wrapper around ObjectList
template< class CLASS >
class TObjectList
{
	ObjectList & m_list;
public:
	TObjectList( ObjectList & _list ) : m_list( _list )
	{
		mxASSERT(_list.GetType() == CLASS::MetaClass());
	}
	ERet New( CLASS *& _o )
	{
		void* mem = m_list.Allocate();
		if( mem ) {
			_o = new(mem) CLASS();
			return ALL_OK;
		}
		return ERR_OUT_OF_MEMORY;
	}
};

/*
-----------------------------------------------------------------------------
	A 'Clump' groups together related objects.
	This is mainly used for easy & efficient serialization and fast iteration.

	Each 'Clump' stores objects in object lists - contiguous arrays of objects.
	There can be several object lists for the same type (class) of objects
	(except for serialized/optimized clumps).
-----------------------------------------------------------------------------
*/
struct Clump : public CStruct, public TDoublyLinkedList< Clump >//, public ReferenceCounted
{
	ObjectList::Head	m_objectLists;	// head of the linked list of object lists
	FreeListAllocator	m_objectListsStorage;	// used for recycling object lists
	//UINT32				m_referenceCount;
	//UINT32				m_flags;

public:
	mxDECLARE_CLASS(Clump,CStruct);
	mxDECLARE_REFLECTION;

	Clump();
	~Clump();

	//CStruct* Allocate( const mxClass& type, UINT32 count );

	// simply allocates storage, doesn't call ctor
	// NOTE: the returned memory block is uninitialized!
	CStruct* Allocate( const mxClass& type, UINT32 granularity = 1 );

	// allocates storage and calls default constructor
	//CStruct* AllocateAndConstruct( const mxClass& type );

	ERet DeleteOne( const mxClass& type, void* o );
	void DeleteAll( const mxClass& type );
	void RemoveAll( const mxClass& type );

	bool HasAddress( const void* pointer ) const;

	ObjectList* CreateObjectList( const mxClass& type, UINT32 capacity );

	void Clear();	// remove all items and release allocated memory
	void Empty();	// doesn't free allocated memory

	ObjectList::Head GetObjectLists() const;

	bool IsEmpty() const;

	// walk over all object lists
	void IterateObjects( Reflection::AVisitor* visitor, void* userData ) const;
	void IterateObjects( Reflection::AVisitor2* visitor, void* userData ) const;

public:
	// creates a new object (may reuse existing free-listed items)
	mxOBSOLETE
	template< class CLASS >
	CLASS* New( UINT32 granularity = 1 )
	{
		const mxClass& type = T_DeduceClass< CLASS >();
		void* storage = this->Allocate( type, granularity );
		chkRET_NIL_IF_NIL(storage);
		return new (storage) CLASS();
	}

	template< class CLASS >
	ERet New( CLASS *&_o, UINT32 granularity = 1 )
	{
		_o = NULL;
		const mxClass& type = T_DeduceClass< CLASS >();
		void* storage = this->Allocate( type, granularity );
		if( storage ) {
			_o = new (storage) CLASS();
		}
		return _o ? ALL_OK : ERR_OUT_OF_MEMORY;
	}

	// calls the object's destructor and adds it to the free list
	template< class CLASS >
	ERet Free( CLASS* o )
	{
		return this->DeleteOne( T_DeduceClass< CLASS >(), o );
	}

	// NOTE: the returned memory block is uninitialized!
	template< class CLASS >
	CLASS* CreateObjectList( UINT32 capacity )
	{
		ObjectList* objectList = this->CreateObjectList( T_DeduceClass< CLASS >(), capacity );
		if( objectList ) {
			return static_cast< CLASS* >( objectList->GetArrayPtr() );
		}
		return NULL;
	}

public:
	class ObjectListIterator
	{
		ObjectList *	m_currentList;	// current object list being iterated
		const mxClass &	m_baseClass;	// base type of objects that should be iterated

	public:
		ObjectListIterator( const Clump& clump, const mxClass& type );

		bool IsValid() const;
		void MoveToNext();

		UINT32 NumObjects() const;

		ObjectList* ToPtr() const;
	};
	class IteratorBase
	{
	protected:
		ObjectList::IteratorBase	m_objectIterator;	// Iterator for the objects in the current object list.
		ObjectList *				m_currentList;
		const mxClass &				m_type;	// for iterating over objects of a specific type

	public:
		IteratorBase( const Clump& clump, const mxClass& type );

		bool IsValid() const;
		void MoveToNext();
		void Skip( UINT32 count );
		UINT32 NumContiguousObjects() const;

		void* ToVoidPtr() const;

	protected:
		void SetFromCurrentList();
		bool IsCurrentListValid() const;
		void MoveToFirstObject();
		void MoveToNextList();
	};

public:
	// this function must be called after loading
	// to fixup references to external assets
	// (create 'live' resource objects from external data)
	ERet LoadAssets( UINT32 timeOutMilliseconds = INFINITE );

	ERet LoadAssets2();

	void DbgCheckPointers();

	//struct Header_d
	//{
	//	UINT32	fourCC;
	//	UINT32	bufferSize;
	//	UINT32	_pad[2];
	//	enum { ID = MCHAR4('C','L','M','P') };
	//};
	static ERet Load( Assets::LoadContext2 & context );
	static ERet Online( Assets::LoadContext2 & context );
	static void Offline( Assets::LoadContext2 & context );
	static void Destruct( Assets::LoadContext2 & context );

	static void* Alloc( UINT32 size );
	static void Free( void* pointer );

private:
	void UnloadAssets();
};

//
//	TObjectIterator< CLASS >
//
template< class CLASS >
class TObjectIterator : public Clump::IteratorBase
{
public:
	TObjectIterator( const Clump& clump )
		: Clump::IteratorBase( clump, CLASS::MetaClass() )
	{}
	inline CLASS* ToPtr() const
	{
		mxASSERT(this->IsValid());
		return m_objectIterator.ToPtr< CLASS >();
	}
	inline CLASS& Value() const
	{
		mxASSERT(this->IsValid());
		return m_objectIterator.ToValue< CLASS >();
	}
};

struct ClumpList
{
	Clump::Head		m_clumps;	// head of the linked list of clumps
};

ObjectList* FindFirstObjectListOfType( const Clump& clump, const mxClass& type );
AssetExport* FindAssetExportByPointer( const Clump& clump, const void* assetInstance );

template< class CLASS >
CLASS* FindSingleInstance( const Clump& _clump )
{
	const mxClass& type = CLASS::MetaClass();
	ObjectList* objectList = FindFirstObjectListOfType( _clump, type );
	if( PtrToBool(objectList) )
	{
		mxASSERT(objectList->Num() == 1);
		return static_cast< CLASS* >( objectList->GetItemAtIndex( 0 ) );
	}
	return NULL;
}

template< class CLASS >
CLASS* GoC_SingleInstance( Clump & clump )
{
	const mxClass& type = CLASS::MetaClass();
	ObjectList* objectList = FindFirstObjectListOfType( clump, type );
	if( !PtrToBool(objectList) )
	{
		objectList = clump.CreateObjectList( type, 1 );
		return new(objectList->Allocate()) CLASS();
	}
	return static_cast< CLASS* >( objectList->GetItemAtIndex( 0 ) );
}

template< class CLASS >
ERet GoC_SingleInstance( Clump & clump, CLASS *& o )
{
	o = GoC_SingleInstance< CLASS >( clump );
	if( !PtrToBool(o) ) {
		return ERR_OUT_OF_MEMORY;
	}
	return ALL_OK;
}

template< class CLASS >
ERet PutSingleInstance( const CLASS& _o, Clump &_clump )
{
	CLASS* p = GoC_SingleInstance< CLASS >( _clump );
	if( !PtrToBool(p) ) {
		return ERR_OUT_OF_MEMORY;
	}
	*p = _o;
	return ALL_OK;
}

template< class CLASS >
ERet GetSingleInstance( const Clump& _clump, CLASS **_o )
{
	CLASS* existing = FindSingleInstance< CLASS >( _clump );
	if( !PtrToBool(existing) ) {
		return ERR_OBJECT_NOT_FOUND;
	}
	*_o = existing;
	return ALL_OK;
}

template< class TYPE >	// where TYPE has member 'name' of type 'String'
TYPE* FindByName( const Clump& _clump, const char* _name )
{
	TObjectIterator< TYPE >	it( _clump );
	while( it.IsValid() )
	{
		TYPE& item = it.Value();
		if( Str::EqualS( item.name, _name ) ) {
			return &item;
		}
		it.MoveToNext();
	}
	return NULL;
}
template< class TYPE >	// where TYPE has member 'hash' of type 'UINT32'
TYPE* FindByHash( const Clump& _clump, const UINT32 _hash )
{
	TObjectIterator< TYPE >	it( _clump );
	while( it.IsValid() )
	{
		TYPE& item = it.Value();
		if( item.hash == _hash ) {
			return &item;
		}
		it.MoveToNext();
	}
	return NULL;
}

template< class TYPE >	// where TYPE has member 'name' of type 'String'
ERet GetByName( const Clump& _clump, const char* _name, TYPE *&_o )
{
	_o = FindByName< TYPE >(_clump,_name);
	if( _o == NULL ) {
		return ERR_OBJECT_NOT_FOUND;
	}
	return ALL_OK;
}
template< class TYPE >	// where TYPE has member 'hash' of type 'UINT32'
ERet GetByHash( const Clump& _clump, const UINT32 _hash, TYPE *&_o )
{
	_o = FindByHash< TYPE >(_clump,_hash);
	if( _o == NULL ) {
		return ERR_OBJECT_NOT_FOUND;
	}
	return ALL_OK;
}

void DBG_DumpFields( const void* _memory, const mxType& _type, ATextStream &_log );

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
