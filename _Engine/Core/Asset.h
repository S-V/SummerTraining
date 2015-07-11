/*
=============================================================================
	File:	Asset.h
	Desc:	Asset (Resource) management.
	ToDo:	use numerical asset ids (32-bit ints or 128-bit GUIDs),
			strings should only used in editor/development mode!
=============================================================================
*/
#pragma once

#include <Base/Util/UUID.h>

#include <Core/Core.h>
#include <Core/Text/NameTable.h>
#include <Core/bitsquid/memory.h>

/*
=====================================================================
    HARDCODED ASSET TYPES
=====================================================================
*/
namespace AssetTypes {
	enum EAssetTypeIDs {
#define DECLARE_ASSET_TYPE( name, class, file_extension, version, description )		name,
#include <Core/AssetTypes.inl>
#undef DECLARE_ASSET_TYPE
		MAX_ASSET_TYPES
	};
}//namespace AssetTypes

mxDECLARE_ENUM( AssetTypes::EAssetTypeIDs, UINT8, AssetTypeT );

/*
--------------------------------------------------------------
	AssetID - (static)(globally) unique resource identifier,
	usually assigned by resource compiler
	and known before running the program
	and used for locating the asset data.

	AssetID is an identifier of a remote asset.
	This should be unique among all the servers we work with.

	NOTE: asset id is just the file name of an asset without path.
--------------------------------------------------------------
*/
struct AssetID : public CStruct
{
	NameID	d;	// e.g. "material_default.material"

	enum { MAX_LENGTH = 256 };
};

bool AssetId_IsNull( const AssetID& assetId );
bool AssetId_IsValid( const AssetID& assetId );
bool AssetIds_AreEqual( const AssetID& assetIdA, const AssetID& assetIdB );
AssetID AssetId_GetNull();
UINT32 AssetId_GetHash32( const AssetID& assetId );
const char* AssetId_ToChars( const AssetID& assetId );
const AssetID MakeAssetID( const char* _name );
ERet ReadAssetID( AStreamReader& stream, AssetID *assetId );
ERet WriteAssetID( const AssetID& assetId, AStreamWriter &stream );

template<>
struct THashTrait< AssetID > {
	static mxFORCEINLINE UINT GetHashCode( const AssetID& k ) {
		return AssetId_GetHash32( k );
	}
};
template<>
struct TEqualsTrait< AssetID > {
	static mxFORCEINLINE bool Equals( const AssetID& a, const AssetID& b ) {
		return AssetIds_AreEqual( a, b );
	}
};
template<>
inline const mxType& T_DeduceTypeInfo< AssetID >() {
	static mxBuiltInType< AssetID >	assetIdType( ETypeKind::Type_AssetId, mxEXTRACT_TYPE_NAME(AssetID) );
	return assetIdType;
}
template<>
inline ETypeKind T_DeduceTypeKind< AssetID >() {
	return ETypeKind::Type_AssetId;
}

/*
--------------------------------------------------------------
	AssetKey
	used for uniquely identifying each asset instance
	(and for sharing asset instances by mapping asset ids to pointers)
--------------------------------------------------------------
*/
struct AssetKey : public CStruct
{
	AssetID		id;		// unique id of asset (can be null if pointing to fallback asset instance)
	AssetTypeT	type;	// type of asset, must always be valid
public:
	mxDECLARE_CLASS( AssetKey, CStruct );
	mxDECLARE_REFLECTION;
	AssetKey()
		: id( AssetId_GetNull() )
		, type( AssetTypes::UNKNOWN )
	{}
	AssetKey( const AssetID& assetId, const AssetTypeT& assetType )
		: id( assetId )
		, type( assetType )
	{}
	bool IsOk() const;
};

/*
-----------------------------------------------------------------------------
	AFilePackage
	resource/asset data load interface (resource/content provider)
-----------------------------------------------------------------------------
*/
struct AFilePackage : public TSinglyLinkedList< AFilePackage >
{
	// low-level real-only file stream (for reading asset data)
	//NOTE: this structure must be bitwise-copyable
	struct Stream
	{
		// these fields are for internal use only!
		// Don't touch them!
		UINT64	fileHandle;	// e.g. could be an OS file handle or offset within a PAK file
		UINT32	dataSize;	// uncompressed size
		UINT32	offset;		// current read offset
		//UINT32	sortKey;	// for sorting by file offsets
	public:
		Stream();
	};

	//[must be threadsafe]
	virtual ERet OpenFile( const AssetID& fileId, Stream *stream ) = 0;

	//[must be threadsafe]
	virtual ERet CloseFile( Stream * stream ) = 0;

	// Reads the file into the preallocated buffer
	//[must be threadsafe]
	virtual ERet ReadFile( Stream * stream, void *buffer, UINT bytesToRead ) = 0;

	virtual size_t TellPosition( const Stream& stream ) const = 0;

	virtual ~AFilePackage();
};

// Load request priority.
enum ELoadPriority
{
	Load_Priority_Low = 0,
	Load_Priority_Normal = 64,
	Load_Priority_High = 128,
	Load_Priority_Critical = 255,
	Load_Priority_MAX,
};
enum {
	Load_Priority_Collision_Hull = Load_Priority_Critical,
	Load_Priority_Animation = Load_Priority_Critical,
	Load_Priority_Sound = Load_Priority_High,
	Load_Priority_Music = Load_Priority_Normal,
	Load_Priority_Texture = Load_Priority_Low,
};
enum ELoadStatus
{
	LoadStatus_HadErrors = 0,
	LoadStatus_Loaded,
	LoadStatus_Unloaded,
	LoadStatus_Pending,
	LoadStatus_Cancelled,
};
enum { MAX_IO_READ_SIZE = 2*mxMEGABYTE };

// Numeric identifier for an asset load request,
// it could be 64-bit to protect against wraparound errors
mxDECLARE_32BIT_HANDLE(LoadRequestId);

struct AssetLoadContext
{
//	AFilePackage::Stream	stream;
	AFilePackage *			package;
	CStruct *				instance;	// a pointer to the asset instance (e.g. mesh, texture, model, sound object)
	LoadRequestId			requestId;	// id of the I/O request
};

typedef ERet F_Load( AssetLoadContext& context );

// these are called in the background loading thread
// to manage temporary memory if the user didn't supply his output buffer;
// default implementation uses a special memory heap for streaming
typedef void* F_Allocate( UINT numBytes );
typedef void F_Free( void* pointer, UINT numBytes );

extern void* StreamBuffer_AllocateMemory( UINT numBytes );
extern void StreamBuffer_ReleaseMemory( void* pointer, UINT numBytes );

/*
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
*/
namespace Assets
{
	struct LoadContext2 : AStreamReader
	{
		void *	o;	// a pointer to the asset instance (e.g. mesh, texture, model, sound object)
		Clump *	clump;	// user data pointer (e.g. a Clump to allocate from)
		//foundation::Allocator *	memory;
		AFilePackage *			package;
		AFilePackage::Stream	stream;
	public:
		LoadContext2();
		virtual size_t Tell() const override;
		virtual size_t GetSize() const override;
		virtual ERet Read( void *buffer, size_t size ) override;
	};

	// allocates memory and constructs a new instance
	//typedef ERet F_Allocate( AStreamReader& stream, void* userData, void *& o );

	typedef ERet F_LoadData( LoadContext2 & context );
	// initializes the instance (e.g. uploads buffers to GPU) ('finalize', 'bring-in')
	typedef ERet F_Finalize( LoadContext2 & context );
	// un-initializes the instance (e.g. frees GPU memory)
	typedef void F_BringOut( LoadContext2 & context );
	// frees the allocated memory, but doesn't destroy the instance
	typedef void F_FreeData( LoadContext2 & context );
	// optional callback for hot asset reloading
	typedef void F_ReloadIt( LoadContext2 & context );


	struct AssetMetaType
	{
		const char* extension;
		TypeID		classID;
		UINT32		version;	// always increasing

		//F_Allocate *	allocate;
		F_LoadData *	loadData;	//<= !required!
		F_Finalize *	finalize;	//<= !required!
		F_BringOut *	bringOut;	//<= !required!
		F_FreeData *	freeData;	//<= !required!
		F_ReloadIt *	reloadIt;	//<= [optional]
	};
	extern AssetMetaType gs_assetTypes[AssetTypes::MAX_ASSET_TYPES];

	AssetTypeT FindAssetTypeByExtension( const char* ext );
	AssetTypeT FindAssetTypeByClassID( const TypeID classID );

	//
	// Initialization/Shutdown
	//
	ERet Initialize();
	void Shutdown();

	//
	// Lightweight virtual file system
	//
	ERet MountPackage( AFilePackage* package );
	void UnmountPackage( AFilePackage* package );
	bool IsPackageMounted( AFilePackage* package );
	//[must be threadsafe] returns null on failure
	AFilePackage* OpenFile( const AssetID& fileId, AFilePackage::Stream *stream );

	//
	// Resource management
	//
	void* Find( const AssetKey& key );
	void Set( const AssetKey& key, void* o, Clump* clump );	//<= replaces existing instance
#if 0
	void* Load( const AssetKey& key, void* userData = nil );

	template< class ASSET >
	ASSET* Load( const AssetID& id, void* userData = nil ) {
		AssetKey key;
		key.id = id;
		key.type = FindAssetTypeByClassID( ASSET::MetaClass().GetTypeID() );
		return static_cast< ASSET* >( Load( key, userData ) );
	}
#endif


	// Development

	// attempts to reload the given asset instance, does nothing if the asset is not loaded
	ERet ReloadAsset( const AssetKey& key );	// thread-safe
	ERet ReloadAssetsOfType( const AssetTypeT& type );	// thread-safe
	ERet ReloadAllAssets();	// thread-safe

#if 0
	//
	// Resource management
	//
	ERet InsertInstance( const AssetKey& key, CStruct* o );	//<= replaces existing instance
	bool RemoveInstance( const AssetKey& key );
	CStruct* FindInstance( const AssetKey& key );	//<= may return a null pointer

	const AssetKey* FindByPointer( const CStruct* assetInstance );	//<= very slow!

	// doesn't insert the instance into the asset map:
	ERet LoadInstance( const AssetKey& key, CStruct* instance );

// creates a new asset instance if needed and updates the asset map
//CStruct* GetInstance( const AssetKey& key, Clump & clump );	//<= never returns null, may return fallback instance

	template< class ASSET >
	ASSET* FindInstance( const AssetID& id ) {
		AssetKey key;
		key.id = id;
		key.type = &ASSET::MetaClass();
		return static_cast< ASSET* >( FindInstance( key ) );
	}
	template< class ASSET >
	ASSET* GetInstance( const AssetID& id, Clump & clump ) {
		AssetKey key;
		key.id = id;
		key.type = &ASSET::MetaClass();
		return static_cast< ASSET* >( GetInstance( key, clump ) );
	}

	// Development

	mxREFACTOR(CreateReferenceAndLoad);
	// creates an object of type AssetExport in the given clump
	// and tries to load the asset instance
	ERet CreateReferenceAndLoad(
		Clump & storage,
		const AssetID& id,
		const mxClass& type,
		CStruct* assetInstance
		);

	//
	// Asynchronous I/O
	//

	// this function must be called regularly (e.g. per frame)
	// to complete any pending asset loading operations;
	// returns true if all requests have been processed & completed
	bool FinalizeCompletedRequests( UINT maxNumRequests = 0 );	//[main thread only]
#endif
}//namespace Assets

/*
Asset Exports are asset instances that exist in a Clump.
Asset Imports are references to objects in other Clumps.
*/

/*
--------------------------------------------------------------
	AssetExport

	'exports' assets in a clump -> asset cache

	points to the memory-resident part of asset:
	the asset instance which is always stored in a clump.

	Asset instances are initialized with data
	retrieved by their id and then get registered in the asset cache.

	Asset instances are unregistered from the asset cache
	when the corresponding clump is unloaded.
--------------------------------------------------------------
*/
struct AssetExport : public AssetKey
{
	CStruct *	o;	// pointer to the asset instance (e.g. inside the clump)
	//Clump *		owner;	// null if the asset instance is not stored in a clump
public:
	mxDECLARE_CLASS(AssetExport,AssetKey);
	mxDECLARE_REFLECTION;
	AssetExport();
	bool CheckIsValid() const;
};

/*
--------------------------------------------------------------
	AssetImport

	External Assets -> Pointers in Clump

	asset references are stored in clumps;
	they keep references to external assets
--------------------------------------------------------------
*/
struct AssetImport : public CStruct
{
	AssetID		id;		// asset id
	AssetTypeT 	type;	// type of asset
	CStruct **	o;		// linked list of pointers to asset instances
public:
	mxDECLARE_CLASS(AssetImport,CStruct);
	mxDECLARE_REFLECTION;
	AssetImport();
	~AssetImport();
	bool CheckIsValid() const;
	// this could be done asynchronously
	bool Resolve() const;
};

#if 0
typedef const mxClass* AssetType;
class AssetReader : public AStreamReader
{
	AFilePackage *			m_package;
	AFilePackage::Stream *	m_stream;

public:
	AssetReader( AFilePackage* package, AFilePackage::Stream* stream )
		: m_package(package), m_stream(stream)
	{
	}
	~AssetReader()
	{
	}
	virtual size_t GetSize() const override
	{
		return m_stream->dataSize;
	}
	virtual ERet Read( void *buffer, size_t numBytes ) override
	{
		mxDO(m_package->ReadFile(m_stream, buffer, numBytes));
		return ALL_OK;
	}
};

// this marker is used to mark string asset IDs  inside the source code
// so that we can quickly local and replace them with numerical IDs.
inline AssetID ASSID( const char* file ) {
	AssetID	assetId;
	assetId.d = NameID(file);
	return assetId;
}
#endif
/*
=====================================================================
    HARDCODED STUFF
=====================================================================
*/
#if 0//MX_EDITOR

extern const char* AssetType_To_Chars( const AssetType& type );
extern const char* GetSourceAssetFileExtension( const AssetType& type );
extern const mxClass* FindAssetTypeByExtension( const char* ext );
extern UINT32 GetAssetTypeVersion( const AssetType& type );
extern AssetID GetAssetIdFromFilePath( const char* filePath );

#endif // MX_EDITOR

//ERet GetAsset( void* o, const AssetKey& key, void* userData = nil );
ERet LoadAsset( void* o, const AssetKey& key, Clump* clump );
ERet LoadClump( const AssetID& id, Clump *& clump, void* userData = nil );

template< class ASSET >
ASSET* CreateAsset( const AssetID& id, Clump* clump )
{
	AssetKey key;
	key.id = id;
	key.type = ASSET::GetAssetType();

	mxASSERT(Assets::Find(key)==NULL);

	ASSET* o = clump->New< ASSET >();
	if( !o ) {
		return nil;	// out of memory
	}

	LoadAsset( o, key, clump );

	return o;
}

template< class ASSET >
ASSET* FindAsset( const AssetID& id )
{
	AssetKey key;
	key.id = id;
	key.type = ASSET::GetAssetType();

	void* o = Assets::Find( key );

	return static_cast< ASSET* >( o );
}

template< class ASSET >
mxDEPRECATED
ASSET* GetAsset( const AssetID& id, Clump* clump )
{
	AssetKey key;
	key.id = id;
	key.type = ASSET::GetAssetType();

	void* o = Assets::Find( key );
	if( !o )
	{
		o = clump->New< ASSET >();
		if( o )
		{
			LoadAsset( o, key, clump );
		}		
	}
	return static_cast< ASSET* >( o );
}

template< class ASSET >
ERet GetAsset( ASSET *&_o, const AssetID& id, Clump* clump )
{
	AssetKey key;
	key.id = id;
	key.type = ASSET::GetAssetType();

	void* o = Assets::Find( key );
	if( !o )
	{
		o = clump->New< ASSET >();
		if( o )
		{
			mxDO(LoadAsset( o, key, clump ));
		}		
	}
	_o = static_cast< ASSET* >( o );
	return ALL_OK;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
