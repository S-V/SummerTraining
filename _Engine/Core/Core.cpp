/*
=============================================================================
	File:	Core.cpp
	Desc:	
=============================================================================
*/
#include <Core/Core_PCH.h>
#pragma hdrstop
#include <Core/Core.h>
#include <Core/Event.h>
#include <Core/Editor.h>
#include <Core/Asset.h>
//#include <Core/Kernel.h>
//#include <Core/IO/IOSystem.h>
//#include <Core/Resources.h>
//#include <Core/System/StringTable.h>
//#include <Core/Serialization.h>
//#include <Core/Entity/System.h>
#include <Core/Text/NameTable.h>
//#include <Core/Util/Tweakable.h>
#include <Core/Util/Tweakable.h>
#include <Core/Input.h>
#include <Core/bitsquid/memory.h>
#include <Core/ObjectModel.h>

bool AConfigFile::GetInteger( const char* _key, int &_value, int min, int max ) const
{
	bool found = this->FindInteger(_key, _value);
	if( found )
	{
		if( _value < min || _value > max ) {
			ptWARN("'%s'=%d should be in range [%d..%d]\n", _key, _value, min, max);
			_value = Clamp(_value, min, max);
		}
		return true;
	}
	else {
		ptWARN("Couldn't find integer '%s'\n", _key);
	}
	return false;
}
bool AConfigFile::GetSingle( const char* _key, float &_value, float min, float max ) const
{
	bool found = this->FindSingle(_key, _value);
	if( found )
	{
		if( _value < min || _value > max ) {
			ptWARN("'%s'=%d should be in range [%f..%f]\n", _key, _value, min, max);
			_value = Clamp(_value, min, max);
		}
		return true;
	}
	else {
		ptWARN("Couldn't find single '%s'\n", _key);
	}
	return false;
}
bool AConfigFile::GetBoolean( const char* _key, bool &_value ) const
{
	bool found = this->FindBoolean(_key, _value);
	if( !found ) {
		ptWARN("Couldn't find boolean '%s'\n", _key);
	}
	return found;
}

const char* AConfigFile::GetString( const char* _key, const char* _default ) const
{
	const char* existing = this->FindString(_key);
	return existing ? existing : _default;
}
bool AConfigFile::GetStringOptional( const char* _key, String &_value ) const
{
	const char* existing = this->FindString(_key);
	if( existing ) {
		Str::CopyS( _value, existing );
		return true;
	}
	return false;
}

static struct NullConfig : AConfigFile
{
	virtual const char* FindString( const char* _key ) const override {
		return NULL;
	}
	virtual bool FindInteger( const char* _key, int &_value ) const override {
		return false;
	}
	virtual bool FindSingle( const char* _key, float &_value ) const override {
		return false;
	}
	virtual bool FindBoolean( const char* _key, bool &_value ) const override {
		return false;
	}
} gs_NullConfig;

AConfigFile *	gINI = &gs_NullConfig;
AConfigFile *	gUserINI = &gs_NullConfig;

/*
--------------------------------------------------------------
	SCoreGlobals
--------------------------------------------------------------
*/
SCoreGlobals	gCore;

SCoreGlobals::SCoreGlobals()
{
}

//---------------------------------------------------------------------------

static NiftyCounter	gs_CoreSystemRefCounter;

ERet SetupCoreSubsystem()
{
	if( gs_CoreSystemRefCounter.IncRef() )
	{
		mxInitializeBase();

		//DBGOUT("Initializing Core...\n");

		NameID::StaticInitialize();

		gCore.startTime = STimeStamp::GetCurrent();

#if MX_DEVELOPER

		{
			ANSICHAR	timeStampStr[ 512 ];
			gCore.startTime.ToPtr( timeStampStr, mxCOUNT_OF(timeStampStr) );
			DBGOUT("Timestamp: %s\n", timeStampStr );
		}

		{
			DevBuildOptionsList	buildOptions;

			if( MX_DEBUG ) {
				buildOptions.Add("MX_DEBUG");
			}
			if( MX_MULTITHREADED ) {
				buildOptions.Add("MX_MULTITHREADED");
			}
			if( MX_DEVELOPER ) {
				buildOptions.Add("MX_DEVELOPER");
			}
			if( MX_ENABLE_PROFILING ) {
				buildOptions.Add("MX_ENABLE_PROFILING");
			}
			if( MX_ENABLE_REFLECTION ) {
				buildOptions.Add("MX_ENABLE_REFLECTION");
			}
			if( MX_EDITOR ) {
				buildOptions.Add("MX_EDITOR");
			}

			if( MX_DEMO_BUILD ) {
				buildOptions.Add("MX_DEMO_BUILD");
			}

			ANSICHAR	buildOptionsStr[ 1024 ];
			buildOptions.ToChars( buildOptionsStr, mxCOUNT_OF(buildOptionsStr) );

			DBGOUT("Core library build settings:\n\t%s\n", buildOptionsStr);
		}
#endif // MX_DEVELOPER
 

		//Kernel::Initialize();

//#if MX_EDITOR
//		mxASSERT2( gCore.editorApp.IsValid(), "Editor system must be initialized" );
//#endif // MX_EDITOR

		// Initialize Object system.

		//STATIC_IN_PLACE_CTOR_X( gCore.objectSystem, mxObjectSystem );



//#if MX_EDITOR
//		gCore.tweaks.ConstructInPlace();
//#endif // MX_EDITOR

		EventSystem::Initialize();

		// Initialize Input/Output system.
		//gCore.ioSystem.ConstructInPlace();

		// Initialize parallel job manager.
		{
//			Async::CreationInfo	cInfo;

//			Async::Initialize( cInfo );
		}

		foundation::memory_globals::init();

		// Initialize frame allocator.
		{
			int frameMemorySizeMb = 16;
			gINI->GetInteger("FrameStackSizeMb", frameMemorySizeMb, 4, 64);
			const int frameMemorySizeBytes = frameMemorySizeMb * mxMEBIBYTE;
			void* frameMemory = mxAlloc( frameMemorySizeBytes );
			mxDO(gCore.frameAlloc.Initialize( frameMemory, frameMemorySizeBytes ));
		}

		// Initialize resource system.
		mxDO(Assets::Initialize());

		//{
		//	Assets::AssetMetaType& assetCallbacks = Assets::gs_assetTypes[AssetTypes::CLUMP];
		//	assetCallbacks.loadData = &Clump::Load;
		//	assetCallbacks.finalize = &Clump::Online;
		//	assetCallbacks.bringOut = &Clump::Offline;
		//	assetCallbacks.freeData = &Clump::Destruct;
		//}

#if MX_EDITOR
		TweakUtil::Setup();
#endif // MX_EDITOR
	}
	return ALL_OK;
}
//---------------------------------------------------------------------------
void ShutdownCoreSubsystem()
{
	if( gs_CoreSystemRefCounter.DecRef() )
	{
		//DBGOUT("Shutting Down Core...\n");

		// Shutdown resource system.
		Assets::Shutdown();

		// Shutdown frame allocator.
		{
			mxFree( gCore.frameAlloc.GetBufferPtr() );
			gCore.frameAlloc.Shutdown();
		}

		foundation::memory_globals::shutdown();

		//EntitySystem_Close();

		// Shutdown parallel job manager.
		{
//			Async::Shutdown();
		}

#if MX_EDITOR
		TweakUtil::Close();
#endif // MX_EDITOR

		EventSystem::Shutdown();

		//Kernel::Shutdown();

		NameID::StaticShutdown();

		mxShutdownBase();
	}
}

bool CoreSubsystemIsOpen()
{
	return gs_CoreSystemRefCounter.IsOpen();
}

//--------------------------------------------------------------//
//	Reflection.
//--------------------------------------------------------------//

mxBEGIN_STRUCT(FileTimeT)
	mxMEMBER_FIELD(time.dwLowDateTime),
	mxMEMBER_FIELD(time.dwHighDateTime),
mxEND_REFLECTION

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
