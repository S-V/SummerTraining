/*
=============================================================================
	File:	Core.h
	Desc:	
=============================================================================
*/
#pragma once

#if MX_AUTOLINK
#pragma comment( lib, "Core.lib" )
#endif //MX_AUTOLINK

#include <Base/Memory/Stack/StackAlloc.h>

class ConfigFile;
class FileSystem;
class AssetCache;
class AsyncLoader;
class AFilePackage;
class AssetInfo;
class ObjectList;
class Clump;
class Entity;
class EntityComponent;

class AEditorInterface;

ERet SetupCoreSubsystem();
void ShutdownCoreSubsystem();
bool CoreSubsystemIsOpen();

struct SetupCoreUtil {
	SetupCoreUtil() { SetupCoreSubsystem(); }
	~SetupCoreUtil() { ShutdownCoreSubsystem(); }
};

//--------------------------------------------------------------//
//	Reflection.
//--------------------------------------------------------------//

mxDECLARE_STRUCT(FileTimeT);

/*
--------------------------------------------------------------
	AConfigFile
--------------------------------------------------------------
*/
struct AConfigFile
{
	virtual const char* FindString( const char* _key ) const = 0;
	virtual bool FindInteger( const char* _key, int &_value ) const = 0;
	virtual bool FindSingle( const char* _key, float &_value ) const = 0;
	virtual bool FindBoolean( const char* _key, bool &_value ) const = 0;

	bool GetInteger( const char* _key, int &_value, int min = MIN_INT32, int max = MAX_INT32 ) const;
	bool GetSingle( const char* _key, float &_value, float min = MIN_FLOAT32, float max = MAX_FLOAT32 ) const;
	bool GetBoolean( const char* _key, bool &_value ) const;

public:
	// returns the default string if the key doesn't exist
	const char* GetString( const char* _key, const char* _default = "" ) const;
	// doesn't touch the string if the key doesn't exist
	bool GetStringOptional( const char* key, String &_value ) const;

public:
	virtual ~AConfigFile() {}
};

/*
--------------------------------------------------------------
	ANetwork
--------------------------------------------------------------
*/
struct ANetwork
{
	virtual void TickFrame( FLOAT deltaSeconds ) = 0;

protected:
	virtual ~ANetwork() {}
};

enum {
	MAIN_THREAD_INDEX = 0,
	ASYNC_IO_THREAD_INDEX,
	RENDER_THREAD_INDEX,
	FIRST_WORKER_THREAD_INDEX,
};

/*
=============================================================================
	Core global variables
=============================================================================
*/

// engine config - must always be valid
extern AConfigFile *	gINI;

extern AConfigFile *	gUserINI;

/*
--------------------------------------------------------------
	single point of access to all core globals
--------------------------------------------------------------
*/
struct SCoreGlobals : SingleInstance< SCoreGlobals >
{
	STimeStamp		startTime;	// time of engine initialization

	// ultra-fast stack-like allocator for short-lived (scratch) per-frame memory on the main thread
	StackAllocator	frameAlloc;

	//String256	basePath;

public:
	SCoreGlobals();
};

extern SCoreGlobals	gCore;

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
