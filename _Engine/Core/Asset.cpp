/*
=============================================================================
	File:	Asset.cpp
	Desc:	Asset (Resource) management.
	References:

	Solving Problems With Asynchrony: Asset Loading:
	http://www.altdevblogaday.com/2011/04/19/solving-problems-with-asynchrony-asset-loading/

	Asset loading in emscripten and PNaCl:
	http://flohofwoe.blogspot.ru/2013/12/asset-loading-in-emscripten-and-pnacl.html

	Asynchronous Asset Loading (Unreal Engine 4.5)
	https://docs.unrealengine.com/latest/INT/Programming/Assets/AsyncLoading/index.html

	Asset Manager Guide
	https://github.com/GarageGames/Torque2D/wiki/Asset-Manager-Guide

	Asset Management (Explorer, Pipeline, and Database) (Visual 3D)
	http://game-engine.visual3d.net/wiki/visual3d-asset-database-asset-explorer-pipeline-and-persistence

	A Resource Manager for Game Assets
	http://www.gamedev.net/page/resources/_/technical/game-programming/a-resource-manager-for-game-assets-r3807
=============================================================================
*/
#include <Core/Core_PCH.h>
#pragma hdrstop
#include <Base/Math/Hashing/HashFunctions.h>
#include <Base/Util/Sorting.h>
#include <Core/Core.h>
#include <Core/Asset.h>
#include <Core/ObjectModel.h>
#include <Core/Serialization.h>

mxBEGIN_REFLECT_ENUM( AssetTypeT )
#define DECLARE_ASSET_TYPE( name, class, file_extension, version, description )		mxREFLECT_ENUM_ITEM( name, AssetTypes::name ),
	#include <Core/AssetTypes.inl>
#undef DECLARE_ASSET_TYPE
mxEND_REFLECT_ENUM

/*
-----------------------------------------------------------------------------
	AssetID
-----------------------------------------------------------------------------
*/
bool AssetId_IsNull( const AssetID& assetId ) {
	return assetId.d.IsEmpty();
}

bool AssetId_IsValid( const AssetID& assetId ) {
	return !AssetId_IsNull( assetId );
}

bool AssetIds_AreEqual( const AssetID& assetIdA, const AssetID& assetIdB ) {
	return assetIdA.d == assetIdB.d;
}

AssetID AssetId_GetNull() {
	return AssetID();
}

UINT32 AssetId_GetHash32( const AssetID& assetId ) {
	return assetId.d.hash();
}

const char* AssetId_ToChars( const AssetID& assetId ) {
	return assetId.d.IsEmpty() ? "NULL" : assetId.d.ToPtr();
}
const AssetID MakeAssetID( const char* _name ) {
	AssetID result;
	result.d = _name;
	return result;
}
ERet ReadAssetID( AStreamReader& stream, AssetID *assetId )
{
	UINT32 len;
	mxTRY(stream.Get( len ));
	if( len > 0 )
	{
		len = TAlignUp< String::ALIGNMENT >( len );
		char buffer[ AssetID::MAX_LENGTH ];
		if( len >= mxCOUNT_OF(buffer) ) {
			return ERR_BUFFER_TOO_SMALL;
		}
		len = smallest(len, mxCOUNT_OF(buffer)-1);
		mxTRY(stream.Read( buffer, len ));
		buffer[ len ] = 0;
		assetId->d = NameID( buffer );
	}
	else
	{
		assetId->d = NameID();
	}
	return ALL_OK;
}
ERet WriteAssetID( const AssetID& assetId, AStreamWriter &stream )
{
	const UINT32 realLength = assetId.d.size();
	const UINT32 alignedLength = TAlignUp< String::ALIGNMENT >( realLength );
	RET_X_IF_NOT(alignedLength < AssetID::MAX_LENGTH, ERR_BUFFER_TOO_SMALL);
	mxTRY(stream.Put( realLength ));
	if( alignedLength > 0 ) {
		mxTRY(stream.Write( assetId.d.ToPtr(), alignedLength ));
	}
	return ALL_OK;
}

/*
--------------------------------------------------------------
	AssetKey
--------------------------------------------------------------
*/
mxDEFINE_CLASS( AssetKey );
mxBEGIN_REFLECTION( AssetKey )
	mxMEMBER_FIELD( id ),
	mxMEMBER_FIELD( type ),
mxEND_REFLECTION

bool AssetKey::IsOk() const
{
	return AssetId_IsValid(id) && (type != AssetTypes::UNKNOWN);
}

template<>
struct THashTrait< AssetKey > {
	static UINT GetHashCode( const AssetKey& k ) {
		return AssetId_GetHash32( k.id ) ^ long( k.type );
	}
};
template<>
struct TEqualsTrait< AssetKey > {
	static bool Equals( const AssetKey& a, const AssetKey& b ) {
		return AssetIds_AreEqual( a.id, b.id ) && a.type == b.type;
	}
};

/*
--------------------------------------------------------------
	AFilePackage
--------------------------------------------------------------
*/
AFilePackage::~AFilePackage()
{
	mxASSERT(!Assets::IsPackageMounted(this));
}
AFilePackage::Stream::Stream()
{
	mxZERO_OUT(*this);
}

#if 0
static ERet DefaultLoadFunction( AssetLoadContext& context )
{
	mxUNUSED(context);
	return ALL_OK;
}
void* StreamBuffer_AllocateMemory( UINT numBytes )
{
	return mxAlloc( numBytes );
}
void StreamBuffer_ReleaseMemory( void* pointer, UINT numBytes )
{
	mxUNUSED(numBytes);
	return mxFree( pointer );
}
#endif

/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/
namespace Assets
{
	AssetMetaType gs_assetTypes[AssetTypes::MAX_ASSET_TYPES] = {
	#define DECLARE_ASSET_TYPE( name, class, file_extension, version, description )	{ file_extension, class, version },
		#include <Core/AssetTypes.inl>
	#undef DECLARE_ASSET_TYPE
	};

	AssetTypeT FindAssetTypeByExtension( const char* ext )
	{
		for( int i = 0; i < mxCOUNT_OF(gs_assetTypes); i++ )
		{
			if( strcmp( gs_assetTypes[i].extension, ext ) == 0 )
			{
				return (AssetTypes::EAssetTypeIDs) i;
			}
		}
		return AssetTypes::UNKNOWN;
	}
	AssetTypeT FindAssetTypeByClassID( const TypeID classID )
	{
		for( int i = 0; i < mxCOUNT_OF(gs_assetTypes); i++ )
		{
			if( gs_assetTypes[i].classID == classID )
			{
				return (AssetTypes::EAssetTypeIDs) i;
			}
		}
		return AssetTypes::UNKNOWN;
	}

	struct AssetEntry
	{
		void *	o;		// a pointer to the asset instance
		Clump *	clump;	// a pointer to the clump where the above object 'lives'
	};

	// maps pairs (id,type) to asset instance pointers
	typedef THashMap< AssetKey, AssetEntry >	AssetMap;

	struct AssetManagerData
	{
		AssetMap	assets;	// loaded assets for sharing asset instances

		AFilePackage::Head	packages;	// linked list of mounted file packages
	};
	mxDECLARE_PRIVATE_DATA( AssetManagerData, gAssetManagerData );

#define me	mxGET_PRIVATE_DATA( AssetManagerData, gAssetManagerData )

	static UINT32 mxPASCAL ResourceLoaderThread( void* userData );

	ERet Initialize()
	{
		mxINITIALIZE_PRIVATE_DATA( gAssetManagerData );

		me.packages = NULL;

		return ALL_OK;
	}

	void Shutdown()
	{
		mxASSERT(me.packages == NULL); 

		mxSHUTDOWN_PRIVATE_DATA( gAssetManagerData );
	}

	ERet MountPackage( AFilePackage* package )
	{
		chkRET_X_IF_NIL(package, ERR_NULL_POINTER_PASSED);
		chkRET_X_IF_NOT(!package->FindSelfInList( me.packages ), ERR_SUCH_OBJECT_ALREADY_EXISTS);
		//DBGOUT("Asset Manager: Mounting package.\n");
		package->PrependSelfToList( &me.packages );
		return ALL_OK;
	}

	void UnmountPackage( AFilePackage* package )
	{
		chkRET_IF_NIL(package);
		package->RemoveSelfFromList( &me.packages );
	}

	bool IsPackageMounted( AFilePackage* package )
	{
		return package->FindSelfInList( me.packages );
	}

	AFilePackage* OpenFile( const AssetID& fileId, AFilePackage::Stream *stream )
	{
		chkRET_FALSE_IF_NIL(stream);
		AFilePackage* current = me.packages;
		while(PtrToBool( current ))
		{
			if( current->OpenFile( fileId, stream ) == ALL_OK )
			{
				return current;
			}
			current = current->_next;
		}
		return NULL;
	}

	LoadContext2::LoadContext2()
	{
		o = NULL;
		clump = NULL;
		//clump = NULL;
		//memory = NULL;
		package = NULL;
	}
	size_t LoadContext2::Tell() const
	{
		return package->TellPosition( stream );
	}
	size_t LoadContext2::GetSize() const
	{
		return stream.dataSize;
	}
	ERet LoadContext2::Read( void *buffer, size_t size )
	{
		return package->ReadFile( &stream, buffer, size );
	}

	void* Find( const AssetKey& key )
	{
		AssetEntry* existing = me.assets.Find( key );
		return existing ? existing->o : NULL;
	}

	void Set( const AssetKey& key, void* o, Clump* clump )
	{
		AssetEntry newEntry;
		newEntry.o = o;
		newEntry.clump = clump;

		me.assets.Set( key, newEntry );
	}

	// Attempts to reload the given asset instance.
	ERet ReloadInstance( const AssetKey& key, AssetEntry& entry )
	{
		DBGOUT("Asset Manager: Reloading '%s'...\n", AssetId_ToChars(key.id));

		Assets::LoadContext2	context;

		context.o = entry.o;
		context.clump = entry.clump;

		context.package = Assets::OpenFile( key.id, &context.stream );

		if( !context.package )
		{
			ptERROR("Failed to find asset '%s' of type '%s'.\n",
				AssetId_ToChars(key.id), mxGET_ENUM_TYPE(AssetTypeT).GetStringByValue(key.type));
			return ERR_FAILED_TO_OPEN_FILE;	// file not found
		}

		context.DbgSetName( AssetId_ToChars( key.id ) );

		const Assets::AssetMetaType& metaType = Assets::gs_assetTypes[ key.type ];

		(*metaType.bringOut)( context );

		mxDO((*metaType.loadData)( context ));

		mxDO((*metaType.finalize)( context ));

		return ALL_OK;
	}

	ERet ReloadAsset( const AssetKey& key )
	{
		AssetEntry* existing = me.assets.Find( key );
		if( existing ) {
			mxDO(ReloadInstance( key, *existing ));
		}
		return ALL_OK;
	}

	ERet ReloadAssetsOfType( const AssetTypeT& type )
	{
		DBGOUT("Reloading all assets of type '%s'...\n",
			mxGET_ENUM_TYPE(AssetTypeT).GetStringByValue(type));

		AssetMap::Iterator it( me.assets );
		while( it.IsValid() )
		{
			const AssetKey& key = it.Key();
			AssetEntry& entry = it.Value();

			if( key.type == type )
			{
				mxDO(ReloadInstance( key, entry ));
			}

			it.MoveToNext();
		}
		return ALL_OK;
	}

	ERet ReloadAllAssets()
	{
		DBGOUT("Asset Manager: Reloading all assets...\n");
		AssetMap::Iterator it( me.assets );
		while( it.IsValid() )
		{
			const AssetKey& key = it.Key();
			AssetEntry& entry = it.Value();

			mxDO(ReloadInstance( key, entry ));

			it.MoveToNext();
		}
		return ALL_OK;
	}

#if 0
	ERet DoLoad( const AssetKey& key, LoadContext2 & context )
	{
		const AssetMetaType& metaType = gs_assetTypes[ key.type ];
		mxASSERT_PTR(metaType.loadData);

		mxDO((*metaType.loadData)( context ));

		mxASSERT_PTR(context.o);

		mxDO((*metaType.finalize)( context ));

		return ALL_OK;
	}

	void* Load( const AssetKey& key, void* userData )
	{
		mxASSERT(key.IsOk());
		void* o = Find( key );
		if( o ) {
			return o;
		}

		DBGOUT("Loading asset '%s' of type '%s'.", AssetId_ToChars(key.id), mxGET_ENUM_TYPE(AssetTypeT).GetStringByValue(key.type));

		LoadContext2	context;
		context.package = OpenFile( key.id, &context.stream );
		if( !context.package ) {
			ptERROR("Failed to find asset '%s' of type '%s'.\n",
				AssetId_ToChars(key.id), mxGET_ENUM_TYPE(AssetTypeT).GetStringByValue(key.type));
			return nil;	// file not found
		}

		context.DbgSetName( AssetId_ToChars( key.id ) );

		context.userData = userData;

		DoLoad( key, context );

		o = context.o;

#if 0
		const mxClass* assetClass = TypeRegistry::Get().FindClassByGuid( metaType.classID );
		chkRET_NIL_IF_NIL(assetClass);

		if( clump ) {
			o = clump->Allocate( *assetClass );
		} else {
			o = mxAlloc( assetClass->GetInstanceSize() );
		}

		if( !o ) {
			return nil;	// out of memory
		}

		mxASSERT_PTR(o);

		F_ConstructObject* ctor = assetClass->GetConstructor();
		if( ctor ) {
			(*ctor)( o );
		}

		loadContext.o = o;

		const ERet result = (*metaType.loader)( loadContext );
		chkRET_NIL_IF_NOT(mxSUCCEDED(result));
#endif

		return o;
	}
#endif

}//namespace Assets

#if 0
namespace Assets
{
	// maps (assetId,assetType) pairs to asset instance pointers
	typedef THashMap< AssetKey, CStruct* >	AssetMap;

	// queued asynchronous asset load request
	//NOTE: this structure must be bitwise-copyable
	struct LoadRequest
	{
		AssetID			assetId;
		const mxClass *	assetType;
		CStruct *		assetObject;

		//AFilePackage::Stream stream;
		//AFilePackage *	package;

		//ELoadPriority	priority;

		//callbacks
		//void *			userData;
		//F_Load *		load;	// called in the background thread
		//F_Finalize *	finalize;	// called in the main thread

		//F_Allocate *	asyncAlloc;	// [background thread]
		//F_Free *		asyncFree;	// [background thread]

		//LoadRequestId	uid;	// unique id

		//RequestFlags	flags;
	public:
		void CheckValid() const
		{
			mxASSERT(AssetId_IsValid(assetId));
			mxASSERT_PTR(assetType->ass);
			mxASSERT_PTR(assetObject);
		}
	};

	struct AssetManagerData
	{
		AssetMap	loadedAssets;	// for sharing asset instances
		//SpinWait	lookupTableCS;	// for safely updating the asset map

		AFilePackage::Head	packages;	// linked list of mounted file packages

		TArray< LoadRequest >	pendingRequests;	// queued asset load requests
		SpinWait				pendingRequestsCS;	// for serializing access to the load queue

		TArray< LoadRequest >	completedRequests;	// executed load requests
		SpinWait				completedRequestsCS;
	};
	mxDECLARE_PRIVATE_DATA( AssetManagerData, gAssetManagerData );

#define me	mxGET_PRIVATE_DATA( AssetManagerData, gAssetManagerData )

	static UINT32 mxPASCAL ResourceLoaderThread( void* userData );

	ERet Initialize()
	{
		mxINITIALIZE_PRIVATE_DATA( gAssetManagerData );

		//me.loadedAssets.Setup( 1024, 2048 );
		//me.lookupTableCS.Initialize();

		me.packages = NULL;

		me.pendingRequestsCS.Initialize();
		me.completedRequestsCS.Initialize();

		return ALL_OK;
	}

	void Shutdown()
	{
		mxASSERT(me.packages == NULL); 

		me.loadedAssets.Empty();

		me.pendingRequestsCS.Shutdown();
		me.completedRequestsCS.Shutdown();

		mxSHUTDOWN_PRIVATE_DATA( gAssetManagerData );
	}

	ERet InsertInstance( const AssetKey& key, CStruct* o )
	{
		mxASSERT_PTR(key.type);
		mxASSERT_PTR(o);
		DBGOUT("Asset Manager: Inserting '%s' of type '%s'.\n", AssetId_ToChars(key.id), key.type->GetTypeName());
		//SpinWait::Lock	scopedLock( me.lookupTableCS );
		me.loadedAssets.Set( key, o );
		return ALL_OK;
	}

	bool RemoveInstance( const AssetKey& key )
	{
		DBGOUT("Asset Manager: Removing '%s' of type '%s'.\n", AssetId_ToChars(key.id), key.type->GetTypeName());
		//const void* existingEntry = FindInstance( key );
		//chkRET_FALSE_IF_NIL(existingEntry);
		//SpinWait::Lock	scopedLock( me.lookupTableCS );
		me.loadedAssets.Remove( key );
		return true;
	}

	CStruct* FindInstance( const AssetKey& key )
	{
		CStruct* assetInstance = me.loadedAssets.FindRef( key );
		return assetInstance;
	}

	const AssetKey* FindByPointer( const CStruct* assetInstance )
	{
		return me.loadedAssets.FindKeyByValue( (CStruct*)assetInstance );
	}

	ERet LoadInstance( const AssetKey& key, CStruct* instance )
	{
		mxASSERT(AssetId_IsValid( key.id ));
		mxASSERT_PTR(instance);

		DBGOUT("Asset Manager: Loading '%s'...\n", AssetId_ToChars(key.id));

		ERet retCode = ALL_OK;

		AssetCallbacks* assetMgr = key.type->ass;
		mxASSERT_PTR(assetMgr);

		AssetLoadContext	loadContext;
		AFilePackage* package = Assets::OpenFile( key.id, &loadContext.stream );
		if( !package )
		{
			retCode = ERR_FAILED_TO_OPEN_FILE;
			goto L_Error;
		}

		loadContext.package = package;
		loadContext.instance = instance;
		loadContext.requestId.id = 0;

		retCode = (*assetMgr->load)( loadContext );
		if( mxFAILED(retCode) ) {
			goto L_Error;
		}

		package->CloseFile( &loadContext.stream );

		return ALL_OK;

L_Error:
		ptERROR("Failed to load asset '%s' of type '%s'.\n", AssetId_ToChars(key.id), key.type->GetTypeName());
		return retCode;
	}

	// Attempts to reload the given asset instance.
	ERet ReloadInstance( const AssetKey& key, CStruct* instance )
	{
		DBGOUT("Asset Manager: Reloading '%s'...\n", AssetId_ToChars(key.id));

		LoadRequest		loadRequest;
		{
			loadRequest.assetId = key.id;
			loadRequest.assetType = key.type;
			loadRequest.assetObject = instance;
		}

		SpinWait::Lock	scopedLock(me.pendingRequestsCS);
		me.pendingRequests.Add(loadRequest);

		return ALL_OK;
	}

#if 0
	void* GetInstance( const AssetKey& key, Clump & clump )
	{
		void* existingInstance = me.loadedAssets.FindRef( key );
		if( existingInstance ) {
			return existingInstance;
		}
		void* result = key.type->fallbackInstance;
		if(AssetId_IsValid( key.id ))
		{
			void* newInstance = clump.AllocateAndConstruct( *key.type );
			if( LoadInstance( key, newInstance ) ) {
				result = newInstance;
			} else {
				clump.DeleteOne( *key.type, newInstance );
			}
		}
		if( result ) {
			InsertInstance( key, result );
		}
		return result;
	}
#endif

	ERet CreateReferenceAndLoad(
		Clump & storage,
		const AssetID& id,
		const mxClass& type,
		CStruct* assetInstance
		)
	{
		if( FindInstance(AssetKey(id, &type)) ) {
			return ERR_SUCH_OBJECT_ALREADY_EXISTS;
		}

		// Create a new asset reference inside the clump.
		AssetExport* newAssetExport = storage.New< AssetExport >();
		RET_X_IF_NIL(newAssetExport, ERR_OUT_OF_MEMORY);
		{
			newAssetExport->id = id;
			newAssetExport->type = &type;
			newAssetExport->o = assetInstance;
			newAssetExport->owner = &storage;
		}

		// Try to load the asset instance.
		mxDO(LoadInstance(AssetKey(id, &type), assetInstance));

		// Register the asset instance in the loaded assets table.
		mxDO(InsertInstance(AssetKey(id, &type), assetInstance));

		return ALL_OK;
	}

	ERet ReloadAsset( const AssetKey& key )
	{
		CStruct* assetInstance = FindInstance(key);
		if( assetInstance ) {
			mxDO(ReloadInstance(key, assetInstance));
		}
		return ALL_OK;
	}

	ERet ReloadAssetsOfType( const mxClass& type )
	{
		DBGOUT("Asset Manager: Reloading all assets of type '%s'...\n", type.GetTypeName());
		AssetMap::Iterator it( me.loadedAssets );
		while( it.IsValid() )
		{
			const AssetKey& key = it.Key();
			CStruct* assetInstance = c_cast(CStruct*) it.Value();

			if( key.type->IsDerivedFrom(type) )
			{
				mxDO(ReloadInstance(key, assetInstance));
			}

			it.MoveToNext();
		}
		return ALL_OK;
	}

	ERet ReloadAllAssets()
	{
		DBGOUT("Asset Manager: Reloading all assets...\n");
		AssetMap::Iterator it( me.loadedAssets );
		while( it.IsValid() )
		{
			const AssetKey& key = it.Key();
			CStruct* assetInstance = c_cast(CStruct*) it.Value();

			mxDO(ReloadInstance(key, assetInstance));

			it.MoveToNext();
		}
		return ALL_OK;
	}

	// One may not want to process all the completed requests at once
	// because this may cause a noticeable temporary slow down in the game's performance.
	// So, this API could be called once per frame
	// so that only a small number work items would be set to the device on each frame,
	// thereby spreading the work load of binding resources to the device over several frames.
	//
	bool FinalizeCompletedRequests( UINT maxNumRequests )
	{
		mxPROFILE_FUNCTION;
		{
			SpinWait::Lock	scopedLock(me.pendingRequestsCS);
			for( UINT iPendingRequest = 0; iPendingRequest < me.pendingRequests.Num(); iPendingRequest++ )
			{
				LoadRequest& pendingRequest = me.pendingRequests[ iPendingRequest ];
				LoadInstance(AssetKey(pendingRequest.assetId, pendingRequest.assetType), pendingRequest.assetObject);
			}
			me.pendingRequests.Empty();
		}
		//for( UINT iCompletedRequest = 0; iCompletedRequest < numRequestsToProcess; iCompletedRequest++ )
		//{
		//	SQueueItem& completedRequest = m_completedRequests[ iCompletedRequest ];
		//	this->FinalizeCompletedItem( completedRequest );
		//}
		return true;
	}

}//namespace Assets
#endif
/*
-----------------------------------------------------------------------------
	AssetExport
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS(AssetExport);
mxBEGIN_REFLECTION(AssetExport)
	mxMEMBER_FIELD(o),
	// the 'owner' pointer gets fixed-up when the containing clump is loaded
mxEND_REFLECTION

AssetExport::AssetExport()
{
	o = NULL;
	//owner = NULL;
}

bool AssetExport::CheckIsValid() const
{
	chkRET_FALSE_IF_NOT(AssetKey::IsOk());
	chkRET_FALSE_IF_NIL(o);
	return true;
}

/*
-----------------------------------------------------------------------------
	AssetImport
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS( AssetImport );
mxBEGIN_REFLECTION( AssetImport )
	mxMEMBER_FIELD( id ),
	mxMEMBER_FIELD( type ),
	mxMEMBER_FIELD( o ),
mxEND_REFLECTION

AssetImport::AssetImport()
{
	id = AssetId_GetNull();
	type = AssetTypes::UNKNOWN;
	o = NULL;
}

AssetImport::~AssetImport()
{
}

bool AssetImport::CheckIsValid() const
{
	chkRET_FALSE_IF_NOT(AssetId_IsValid(id));
	chkRET_FALSE_IF_NIL(type);
	chkRET_FALSE_IF_NIL(o);
	return true;
}

bool AssetImport::Resolve() const
{
UNDONE;
	return true;
}

#if 0//MX_EDITOR

namespace
{
	// Associates asset types with file extensions and format versions.
	struct AssetTypeMeta {
		const TypeID	classGuid;	// stable asset type id
		const char *	extension;	// file extension of source (usually, text) assets
		const UINT32	version;	// should be incremented whenever file format changes
	} g_assetTypes[] = {
		{ mxEXTRACT_TYPE_GUID(rxMesh),		"mesh",		0 },
		{ mxEXTRACT_TYPE_GUID(rxTexture),	"texture",	0 },
		{ mxEXTRACT_TYPE_GUID(rxMaterial),	"material",	0 },
		//{ mxEXTRACT_TYPE_GUID(FxLibrary),	"fxb",		0 },
	};
}//namespace

const char* AssetType_To_Chars( const AssetType& type )
{
	return type->GetTypeName();
}

const char* GetSourceAssetFileExtension( const AssetType& type )
{
	for( UINT i = 0; i < mxCOUNT_OF(g_assetTypes); i++ )
	{
		if( g_assetTypes[i].classGuid == type->GetTypeID() )
		{
			return g_assetTypes[i].extension;
		}
	}
	return NULL;
}

const mxClass* FindAssetTypeByExtension( const char* ext )
{
	for( UINT i = 0; i < mxCOUNT_OF(g_assetTypes); i++ )
	{
		if( strcmp( g_assetTypes[i].extension, ext ) == 0 )
		{
			return TypeRegistry::Get().FindClassByGuid( g_assetTypes[i].classGuid );
		}
	}
	return NULL;
}

UINT32 GetAssetTypeVersion( const AssetType& type )
{
	for( UINT i = 0; i < mxCOUNT_OF(g_assetTypes); i++ )
	{
		if( g_assetTypes[i].classGuid == type->GetTypeID() )
		{
			return g_assetTypes[i].version;
		}
	}
	return 0;
}

AssetID GetAssetIdFromFilePath( const char* filePath )
{
	String256	tmp;
	Str::CopyS(tmp, filePath);
	Str::StripPath(tmp);
	AssetID assetId;
	assetId.d = NameID(tmp.ToPtr());
	return assetId;
}

#endif // MX_EDITOR

ERet LoadAsset( void* o, const AssetKey& key, Clump* clump )
{
	mxASSERT_PTR(o);

	DBGOUT("Loading asset '%s' of type '%s'.",
		AssetId_ToChars(key.id), mxGET_ENUM_TYPE(AssetTypeT).GetStringByValue(key.type));

	Assets::LoadContext2	context;

	context.o = o;
	context.clump = clump;

	context.package = Assets::OpenFile( key.id, &context.stream );

	if( !context.package )
	{
		ptERROR("Failed to find asset '%s' of type '%s'.\n",
			AssetId_ToChars(key.id), mxGET_ENUM_TYPE(AssetTypeT).GetStringByValue(key.type));
		return ERR_FAILED_TO_OPEN_FILE;	// file not found
	}

	context.DbgSetName( AssetId_ToChars( key.id ) );

	const Assets::AssetMetaType& metaType = Assets::gs_assetTypes[ key.type ];

	mxDO((*metaType.loadData)( context ));

	mxDO((*metaType.finalize)( context ));

	Assets::Set( key, o, clump );

	return ALL_OK;
}

ERet LoadClump( const AssetID& id, Clump *& clump, void* userData )
{
	DBGOUT("Loading clump '%s'.", AssetId_ToChars(id));

	mxASSERT2(userData==NULL,"Unused");

	Assets::LoadContext2	context;

	context.o = NULL;
	context.clump = NULL;

	context.package = Assets::OpenFile( id, &context.stream );

	if( !context.package )
	{
		ptERROR("Failed to find clump '%s'.\n", AssetId_ToChars(id));
		return ERR_FAILED_TO_OPEN_FILE;	// file not found
	}

	context.DbgSetName( AssetId_ToChars( id ) );



	Serialization::ImageHeader	header;
	mxDO(context.Get(header));

	void* buffer = Clump::Alloc(header.payload);
	chkRET_X_IF_NIL(buffer,ERR_OUT_OF_MEMORY);

	mxDO(Serialization::LoadClumpImage( context, header.payload, buffer ));

	context.o = buffer;


#if 0
	const Assets::AssetMetaType& metaType = Assets::gs_assetTypes[ AssetTypes::CLUMP ];

	mxDO((*metaType.loadData)( context ));

	mxDO((*metaType.finalize)( context ));
#endif

	mxDO(Clump::Load( context ));

	mxDO(Clump::Online( context ));

	clump = static_cast< Clump* >( buffer );

	return ALL_OK;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
