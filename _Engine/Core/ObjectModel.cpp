/*
=============================================================================
	File:	ObjectModel.cpp
	Desc:
=============================================================================
*/
#include <Core/Core_PCH.h>
#pragma hdrstop
#include <Core/Core.h>

#include <Core/Asset.h>
#include <Core/ObjectModel.h>
#include <Core/VectorMath.h>
#include <Core/Serialization.h>

#define MX_DEBUG_OBJECT_SYSTEM	0

#define chkALIGN( PTR )		mxASSERT(IS_16_BYTE_ALIGNED( (PTR) ))

static inline UINT32 GetItemStride( const mxType& type )
{
	return ALIGN16( type.m_size );
}

/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS_NO_DEFAULT_CTOR( ObjectList );
mxBEGIN_REFLECTION( ObjectList )
	mxMEMBER_FIELD( _next ),
	mxMEMBER_FIELD( m_type ),
	mxMEMBER_FIELD( m_freeList ),
	mxMEMBER_FIELD( m_count ),
	mxMEMBER_FIELD( m_capacity ),
	mxMEMBER_FIELD( m_flags ),
mxEND_REFLECTION

ObjectList::ObjectList( const mxClass& type, UINT32 capacity )
{
	//DBGOUT("Creating object buffer (type: %s, num: %u)\n", type.GetTypeName(), capacity);
	{
		const UINT32 alignment = type.m_align;
		const UINT32 itemSize = type.GetInstanceSize();
		mxASSERT2( itemSize % alignment == 0, "Object size is not a multiple of its alignment!" );
		mxASSERT( itemSize == AlignUp( itemSize, alignment ) );
	}

	m_type = &type;

	const UINT32 stride = GetItemStride( type );

	// allocate a new memory buffer
	const size_t bufferSize = capacity * stride;
	void* allocatedBuffer = mxAlloc( bufferSize );
	mxASSERT(IS_16_BYTE_ALIGNED(allocatedBuffer));

	if(MX_DEBUG)
	{
		memset(allocatedBuffer, mxDBG_UNINITIALIZED_MEMORY_TAG, bufferSize);
	}

	// add all items to the free list
	m_freeList.Initialize( allocatedBuffer, stride, capacity );

	m_capacity = capacity;
	m_count = 0;

	m_flags = ObjectList::CanFreeMemory;
}

ObjectList::~ObjectList()
{
	this->Clear();

	m_flags = 0;
}

const mxClass& ObjectList::GetType() const
{
	return *m_type;
}

UINT32 ObjectList::GetStride() const
{
	return GetItemStride( *m_type );
}

bool ObjectList::HasAddress( const void* ptr ) const
{
	return m_freeList.HasAddress( ptr );
}

bool ObjectList::HasValidItem( const void* item ) const
{
	return this->ContainsItem( item ) && !m_freeList.ItemInFreeList( item );
}

bool ObjectList::ContainsItem( const void* item ) const
{
	return this->IndexOfContainedItem( item ) != INDEX_NONE;
}

UINT32 ObjectList::IndexOfContainedItem( const void* item ) const
{
	if( m_freeList.HasAddress( item ) )
	{
		const UINT32 itemSize = this->GetStride();
		const UINT32 byteOffset = mxGetByteOffset32( m_freeList.m_start, item );
		mxASSERT( byteOffset % itemSize == 0 );
		return byteOffset % itemSize;
	}
	return INDEX_NONE;
}

ERet ObjectList::DeleteItem( void* item )
{
	chkRET_X_IF_NOT(this->HasValidItem( item ), ERR_OBJECT_NOT_FOUND);
	mxASSERT(m_count > 0);

	DBGOUT("~ Destroying object of type '%s'\n", m_type->GetTypeName());

	m_type->DestroyInstance( item );

	if(MX_DEBUG) { memset(item, mxDBG_UNINITIALIZED_MEMORY_TAG, this->GetStride()); }

	m_count--;

	// insert the destroyed object into the free list
	// NOTE: the free list should be sorted by increasing addresses
	m_freeList.ReleaseSorted( item );

	return ALL_OK;
}

CStruct* ObjectList::Allocate()
{
	CStruct* newItem = c_cast(CStruct*) m_freeList.Allocate();
	if( newItem != NULL ) {
		m_count++;
		return newItem;
	}
	return NULL;
}

UINT32 ObjectList::Num() const
{
	return m_count;
}

UINT32 ObjectList::Capacity() const
{
	return m_capacity;
}

CStruct* ObjectList::GetArrayPtr() const
{
	return c_cast(CStruct*) m_freeList.m_start;
}

CStruct* ObjectList::GetItemAtIndex( UINT32 itemIndex ) const
{
	mxASSERT( itemIndex < m_capacity );
	void* o = mxAddByteOffset( this->GetArrayPtr(), itemIndex * this->GetStride() );
	return c_cast(CStruct*) o;
}

CStruct* ObjectList::GetFirstFreeItem() const
{
	return c_cast(CStruct*) m_freeList.m_firstFree;
}

void ObjectList::Empty()
{
	IteratorBase	it( *this );
	while( it.IsValid() )
	{
		void* o = it.ToVoidPtr();
		m_type->DestroyInstance( o );
		it.MoveToNext();
	}
	m_count = 0;

	// add all items to the free list
	m_freeList.Initialize( m_freeList.GetBuffer(), this->GetStride(), m_capacity );
}

void ObjectList::Clear()
{
	IteratorBase	it( *this );
	while( it.IsValid() )
	{
		void* o = it.ToVoidPtr();
		m_type->DestroyInstance( o );
		it.MoveToNext();
	}
	m_count = 0;

	// release allocated memory (if we own it)
	if( m_flags & ObjectList::CanFreeMemory )
	{
		void* allocatedBuffer = m_freeList.GetBuffer();
		mxFree(allocatedBuffer);
		m_freeList.Clear();
	}
	m_capacity = 0;

	m_flags = 0;
}

void ObjectList::IterateItems( Reflection::AVisitor* visitor, void* userData ) const
{
	const mxClass& itemType = this->GetType();

	IteratorBase	it( *this );
	while( it.IsValid() )
	{
		void* o = it.ToVoidPtr();
		Reflection::Walker::Visit( o, itemType, visitor, userData );
		it.MoveToNext();
	}
}
void ObjectList::IterateItems( Reflection::AVisitor2* visitor, void* userData ) const
{
	const mxClass& itemType = this->GetType();

	IteratorBase	it( *this );
	while( it.IsValid() )
	{
		void* o = it.ToVoidPtr();
		Reflection::Walker2::Visit( o, itemType, visitor, userData );
		it.MoveToNext();
	}
}

void ObjectList::DbgCheckPointers()
{
#if MX_DEBUG
	Reflection::PointerChecker	pointerChecker;
	this->IterateItems( &pointerChecker, NULL );
#endif // MX_DEBUG
}

//---------------------------------------------------------------------
//	ObjectList::IteratorBase
//---------------------------------------------------------------------
//
ObjectList::IteratorBase::IteratorBase()
{
	this->Reset();
}

ObjectList::IteratorBase::IteratorBase( const ObjectList& objectList )
{
	this->Initialize( objectList );
}

bool ObjectList::IteratorBase::IsValid() const
{
	return m_remaining > 0;
}

// Moves the iterator to the next valid object.
void ObjectList::IteratorBase::MoveToNext()
{
	mxASSERT(m_remaining > 0);
	// move to the next live object
	while( m_remaining )
	{
		m_current = mxAddByteOffset( m_current, m_stride );
		m_remaining--;
		// move forward until we hit an active object
		if( m_current != m_nextFree ) {
			break;
		}
		m_nextFree = *(void**)m_nextFree;
	}
}

// Skips over the given number of objects.
void ObjectList::IteratorBase::Skip( UINT32 count )
{
	mxASSERT(count <= m_remaining);
	while( count-- )
	{
		this->MoveToNext();
	}
}

UINT32 ObjectList::IteratorBase::NumRemaining() const
{
	return m_remaining;
}

UINT32 ObjectList::IteratorBase::NumContiguousObjects() const
{
	return ( m_current < m_nextFree ) ?
		mxGetByteOffset32( m_current, m_nextFree ) / m_stride
		:
		m_remaining
		;
}

void ObjectList::IteratorBase::Initialize( const ObjectList& objectList )
{
	m_current		= objectList.GetArrayPtr();
	m_nextFree		= objectList.GetFirstFreeItem();
	m_stride		= objectList.GetStride();
	m_remaining		= objectList.m_count;

	// skip freelisted elements and move to the first valid object
	while( m_remaining )
	{
		// move forward until we hit a live object
		if( m_current != m_nextFree ) {
			break;
		}
		m_current = mxAddByteOffset( m_current, m_stride );
		m_nextFree = *(void**)m_nextFree;
		m_remaining--;
	}
}

void ObjectList::IteratorBase::Reset()
{
	m_current	= NULL;
	m_nextFree	= NULL;
	m_stride	= 0;
	m_remaining	= 0;
}

void* ObjectList::IteratorBase::ToVoidPtr() const
{
	return m_current;
}

/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS_NO_DEFAULT_CTOR( Clump );
mxBEGIN_REFLECTION( Clump )
	//mxMEMBER_FIELD( prev ),
	//mxMEMBER_FIELD( next ),
	mxMEMBER_FIELD( m_objectLists ),
	//mxMEMBER_FIELD( m_objectListsStorage ),
mxEND_REFLECTION

Clump::Clump()
{
	m_objectLists = NULL;
	m_objectListsStorage.Initialize( sizeof(ObjectList), 16 );
	//m_referenceCount = 0;
	//m_flags = 0;
}

Clump::~Clump()
{
	this->Clear();
}

//CStruct* Clump::Allocate( const mxClass& type, UINT32 count )
//{
//	UNDONE;
//	{
//		ObjectList* current = m_objectLists;
//		while(PtrToBool( current ))
//		{
//			if( current->GetType() == type )
//			{
//				ObjectList::IteratorBase	it( *current );
//
//				while( it.IsValid() )
//				{
//					if( it.NumContiguousObjects() >= count )
//					{
//						void* objects = it.ToVoidPtr();
//						break;
//					}
//				}
//
//				CStruct* newItem = current->Allocate();
//				if(PtrToBool( newItem ))
//				{
//					return newItem;
//				}
//			}
//			current = current->_next;
//		}
//	}
//	return NULL;
//}

// NOTE: doesn't call constructor!
CStruct* Clump::Allocate( const mxClass& type, UINT32 granularity )
{
	// first try to reuse deleted item from an object list of the given type
	{
		ObjectList* current = m_objectLists;
		while(PtrToBool( current ))
		{
			if( current->GetType() == type )
			{
				CStruct* newItem = current->Allocate();
				if(PtrToBool( newItem ))
				{
					return newItem;
				}
			}
			current = current->_next;
		}
	}

	// failed to find a free slot in existing object lists - create a new object list

	ObjectList* newObjectList = this->CreateObjectList(
		type, granularity > 1 ? granularity : type.allocationGranularity
	);
	chkRET_NIL_IF_NIL(newObjectList);

	CStruct* newItem = newObjectList->Allocate();
	mxASSERT_PTR(newItem);

	return newItem;
}

//CStruct* Clump::AllocateAndConstruct( const mxClass& type )
//{
//	CStruct* o = this->Allocate( type );
//	chkRET_NIL_IF_NIL( o );
//
//	mxENSURE(type.ConstructInPlace( o ));
//
//	return o;
//}

ERet Clump::DeleteOne( const mxClass& type, void* o )
{
	chkRET_X_IF_NIL( o, ERR_NULL_POINTER_PASSED );
	chkALIGN(o);

	ObjectList* current = m_objectLists;
	while(PtrToBool( current ))
	{
		if( current->HasValidItem( o ) )
		{
			mxASSERT(current->GetType().IsDerivedFrom( type ));
			return current->DeleteItem( o );
		}
		current = current->_next;
	}

	return ALL_OK;
}

void Clump::DeleteAll( const mxClass& type )
{
	ObjectList* current = m_objectLists;
	while(PtrToBool( current ))
	{
		ObjectList* next = current->_next;

		if( current->GetType().IsDerivedFrom(type) )
		{
			current->RemoveSelfFromList( &m_objectLists );
			m_objectListsStorage.Destroy( current );
		}

		current = next;
	}
}

void Clump::RemoveAll( const mxClass& type )
{
	ObjectListIterator it( *this, type );
	while( it.IsValid() )
	{
		ObjectList* objectList = it.ToPtr();
		if( objectList->GetType().IsDerivedFrom( type ) )
		{
			DBGOUT("Removing object of type '%s'\n",type.GetTypeName());
			objectList->Empty();
		}
		it.MoveToNext();
	}
}

bool Clump::HasAddress( const void* pointer ) const
{
	ObjectList* current = m_objectLists;
	while(PtrToBool( current ))
	{
		if( current->HasAddress( pointer ) )
		{
			return true;
		}
		current = current->_next;
	}
	return false;
}

ObjectList* Clump::CreateObjectList( const mxClass& type, UINT32 capacity )
{
	mxASSERT(capacity > 0);
	void* storage = m_objectListsStorage.AllocateItem();
	chkRET_NIL_IF_NIL(storage);
	ObjectList* objectList = new (storage) ObjectList( type, capacity );
	objectList->PrependSelfToList( &m_objectLists );
	return objectList;
}

void Clump::Clear()
{
	this->UnloadAssets();

	ObjectList* current = m_objectLists;
	while(PtrToBool( current ))
	{
		ObjectList* next = current->_next;

		current->RemoveSelfFromList( &m_objectLists );
		m_objectListsStorage.Destroy( current );

		current = next;
	}
}

void Clump::Empty()
{
	this->UnloadAssets();

	ObjectList* current = m_objectLists;
	while(PtrToBool( current ))
	{
		ObjectList* next = current->_next;

		current->RemoveSelfFromList( &m_objectLists );
		current->Empty();
		m_objectListsStorage.ReleaseItem( current );

		current = next;
	}
}

ObjectList::Head Clump::GetObjectLists() const
{
	return m_objectLists;
}

bool Clump::IsEmpty() const
{
	return m_objectLists == NULL;
}

void Clump::IterateObjects( Reflection::AVisitor* visitor, void* userData ) const
{
	ObjectList* current = m_objectLists;
	while(PtrToBool( current ))
	{
		current->IterateItems( visitor, userData );
		current = current->_next;
	}
}
void Clump::IterateObjects( Reflection::AVisitor2* visitor, void* userData ) const
{
	ObjectList* current = m_objectLists;
	while(PtrToBool( current ))
	{
		current->IterateItems( visitor, userData );
		current = current->_next;
	}
}

/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/
Clump::IteratorBase::IteratorBase( const Clump& clump, const mxClass& type )
	: m_type( type )
{
	m_currentList = clump.m_objectLists;
	this->MoveToFirstObject();
}

bool Clump::IteratorBase::IsValid() const
{
	return m_objectIterator.IsValid();
}

void Clump::IteratorBase::MoveToNext()
{
	mxASSERT2(m_objectIterator.IsValid(), "Iterator has run off the end of the list");
	m_objectIterator.MoveToNext();
	if( !m_objectIterator.IsValid() )
	{
		this->MoveToNextList();
	}
}

UINT32 Clump::IteratorBase::NumContiguousObjects() const
{
	return m_objectIterator.NumContiguousObjects();
}

void* Clump::IteratorBase::ToVoidPtr() const
{
	return m_objectIterator.ToVoidPtr();
}

void Clump::IteratorBase::SetFromCurrentList()
{
	if(PtrToBool( m_currentList ))
	{
		m_objectIterator.Initialize( *m_currentList );
	}
	else
	{
		m_objectIterator.Reset();
	}
}

bool Clump::IteratorBase::IsCurrentListValid() const
{
#if 0
	if( m_currentList == NULL )
	{
		// this is a valid state, need to move on to the next list
		// or reset the object iterator to null
		return true;
	}
	return m_currentList->GetType().IsDerivedFrom(m_type) && (m_currentList->Num() > 0);
#else
	// rewritten for speed
	return !m_currentList
		|| (m_currentList->GetType().IsDerivedFrom(m_type) && m_currentList->Num() > 0)
		;
#endif
}

void Clump::IteratorBase::MoveToFirstObject()
{
	while( !this->IsCurrentListValid() )
	{
		m_currentList = m_currentList->_next;
	}
	this->SetFromCurrentList();
}

void Clump::IteratorBase::MoveToNextList()
{
	do
	{
		m_currentList = m_currentList->_next;
	}
	while( !this->IsCurrentListValid() );
	this->SetFromCurrentList();
}

void Clump::DbgCheckPointers()
{
	ObjectList* current = m_objectLists;
	while(PtrToBool( current ))
	{
		current->DbgCheckPointers();
		current = current->_next;
	}
}

ERet Clump::LoadAssets( UINT32 timeOutMilliseconds )
{
	UNDONE;
	// Load asset exports and register them in the hash table.
	{
		TObjectIterator< AssetExport >	it( *this );
		while( it.IsValid() )
		{
			AssetExport& assetExport = it.Value();
//			assetExport.owner = this;
UNDONE;
			//mxDO(Assets::LoadInstance( assetExport, assetExport.o ));

			//Assets::InsertInstance(assetExport, assetExport.o);

			it.MoveToNext();
		}
	}

	TObjectIterator< AssetImport >	it( *this );
	while( it.IsValid() )
	{
		AssetImport& assetRef = it.Value();
		mxUNUSED(assetRef);
UNDONE;
		//if( !AddAssetLoadRequest( assetRef ) ) {
		//	return false;
		//}

		it.MoveToNext();
	}

	mxUNDONE;
	//if( !gCore.loader->WaitForAll( timeOutMilliseconds ) )
	//{
	//	ptERROR("Failed to load resources\n");
	//	return false;
	//}

	//// insert asset exports into the hash map
	//{
	//	TObjectIterator< AssetExport >	it( *this );
	//	while( it.IsValid() )
	//	{
	//		AssetExport& assetExport = it.Value();

	//		assetExport.Register();

	//		it.MoveToNext();
	//	}
	//}

	return ALL_OK;
}

ERet Clump::LoadAssets2()
{
	// Load assets and register them in the hash table.
	{
		TObjectIterator< AssetExport >	it( *this );
		while( it.IsValid() )
		{
			AssetExport& assetExport = it.Value();

			mxDO(LoadAsset( assetExport.o, assetExport, this ));

			it.MoveToNext();
		}
	}

	TObjectIterator< AssetImport >	it( *this );
	while( it.IsValid() )
	{
		AssetImport& assetRef = it.Value();
		mxUNUSED(assetRef);
UNDONE;
		//if( !AddAssetLoadRequest( assetRef ) ) {
		//	return false;
		//}

		it.MoveToNext();
	}

	return ERR_NOT_IMPLEMENTED;
}

void Clump::UnloadAssets()
{
	TObjectIterator< AssetExport >	it( *this );
	while( it.IsValid() )
	{
		AssetExport& assetExport = it.Value();
//UNDONE;
		//Assets::RemoveInstance(assetExport);
		it.MoveToNext();
	}
}

static inline bool IsValidObjectList( ObjectList* objectList, const mxClass& baseType )
{
	mxASSERT_PTR(objectList);
	return objectList->GetType().IsDerivedFrom( baseType )
		&& objectList->Num() > 0
		;
}

ERet Clump::Load( Assets::LoadContext2 & context )
{
#if 0
	mxASSERT2( context.userData == NULL, "just to be safe" );

	Serialization::ImageHeader	header;
	mxDO(context.Get(header));

	void* buffer = Clump::Alloc(header.payload);
	chkRET_X_IF_NIL(buffer,ERR_OUT_OF_MEMORY);

	mxDO(Serialization::LoadClumpImage(context, header.payload, buffer));

	context.o = buffer;
#endif

	return ALL_OK;
}
ERet Clump::Online( Assets::LoadContext2 & context )
{
	Clump* clump = static_cast< Clump* >( context.o );
	mxDO(clump->LoadAssets2());
	return ALL_OK;
}
void Clump::Offline( Assets::LoadContext2 & context )
{

}
void Clump::Destruct( Assets::LoadContext2 & context )
{
	Clump::Free(context.o);
}
void* Clump::Alloc( UINT32 size )
{
	return mxAlloc(size);
}
void Clump::Free( void* pointer )
{
	mxFree(pointer);
}

/*
-----------------------------------------------------------------------------
	ObjectListIterator
-----------------------------------------------------------------------------
*/
Clump::ObjectListIterator::ObjectListIterator( const Clump& clump, const mxClass& type )
	: m_baseClass( type )
{
	m_currentList = clump.m_objectLists;
	// move to the first valid object list
	while( m_currentList && !IsValidObjectList(m_currentList, type) )
	{
		m_currentList = m_currentList->_next;
	}
}

bool Clump::ObjectListIterator::IsValid() const
{
	return m_currentList != NULL;
}

void Clump::ObjectListIterator::MoveToNext()
{
	mxASSERT(this->IsValid());
	do 
	{
		m_currentList = m_currentList->_next;
		if( !m_currentList ) {
			break;
		}
	}
	while( !IsValidObjectList(m_currentList, m_baseClass) );
}

UINT32 Clump::ObjectListIterator::NumObjects() const
{
	mxASSERT(this->IsValid());
	return m_currentList->Num();
}

ObjectList* Clump::ObjectListIterator::ToPtr() const
{
	mxASSERT(this->IsValid());
	return m_currentList;
}

ObjectList* FindFirstObjectListOfType( const Clump& clump, const mxClass& type )
{
	ObjectList* current = clump.GetObjectLists();
	while(PtrToBool( current ))
	{
		if( current->GetType() == type )
		{
			return current;
		}
		current = current->_next;
	}
	return NULL;
}

AssetExport* FindAssetExportByPointer( const Clump& clump, const void* assetInstance )
{
	TObjectIterator< AssetExport >	it( clump );
	while( it.IsValid() )
	{
		AssetExport& assetExport = it.Value();
		if( assetExport.o == assetInstance ) {
			return &assetExport;
		}
		it.MoveToNext();
	}
	return NULL;
}

void DBG_DumpFields( const void* _memory, const mxType& _type, ATextStream &_log )
{
	switch( _type.m_kind )
	{
	case ETypeKind::Type_Integer :
		{
			const INT64 value = GetInteger( _memory, _type.m_size );
			_log << value;
		}
		break;

	case ETypeKind::Type_Float :
		{
			const double value = GetDouble( _memory, _type.m_size );
			_log << value;
		}
		break;
	case ETypeKind::Type_Bool :
		{
			const bool value = TPODCast<bool>::GetConst( _memory );
			_log << value;
		}
		break;

	case ETypeKind::Type_Enum :
		{
			const mxEnumType& enumType = _type.UpCast< mxEnumType >();
			const UINT32 enumValue = enumType.m_accessor.Get_Value( _memory );
			const char* valueName = enumType.GetStringByValue( enumValue );
			_log << valueName;
		}
		break;

	case ETypeKind::Type_Flags :
		{
			const mxFlagsType& flagsType = _type.UpCast< mxFlagsType >();
			String512 temp;
			Dbg_FlagsToString( _memory, flagsType, temp );
			_log << temp;
		}
		break;

	case ETypeKind::Type_String :
		{
			const String & stringReference = TPODCast< String >::GetConst( _memory );
			_log << stringReference;
		}
		break;

	case ETypeKind::Type_Class :
		{
			const mxClass& classType = _type.UpCast< mxClass >();

			const mxClass* parentType = classType.GetParent();
			while( parentType != nil )
			{
				DBG_DumpFields( _memory, *parentType, _log );
				parentType = parentType->GetParent();
			}

			const mxClassLayout& layout = classType.GetLayout();
			for( UINT fieldIndex = 0 ; fieldIndex < layout.numFields; fieldIndex++ )
			{
				const mxField& field = layout.fields[ fieldIndex ];

				const void* memberVarPtr = mxAddByteOffset( _memory, field.offset );
				_log << field.name << "=";
				DBG_DumpFields( memberVarPtr, field.type, _log );
			}
		}
		break;

	case ETypeKind::Type_Pointer :
		{
			_log << "(Pointers not impl)";
		}
		break;

	case ETypeKind::Type_AssetId :
		{
			const AssetID& assetId = *static_cast< const AssetID* >( _memory );
			_log << AssetId_ToChars( assetId );
		}
		break;

	case ETypeKind::Type_ClassId :
		{
			_log << "(ClassId not impl)";
		}
		break;

	case ETypeKind::Type_UserData :
		{
			_log << "(UserData not impl)";
		}
		break;

	case ETypeKind::Type_Blob :
		{
			_log << "(Blobs not impl)";
		}
		break;

	case ETypeKind::Type_Array :
		{
			_log << "(Arrays not impl)";
		}
		break;

		mxNO_SWITCH_DEFAULT;
	}//switch

	_log << ' ';
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
