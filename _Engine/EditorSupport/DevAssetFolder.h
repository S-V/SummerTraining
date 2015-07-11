#pragma once

#include <Core/Filesystem.h>
#include <EditorSupport/EditorSupport.h>
#include <EditorSupport/FileWatcher.h>

class LocalFileSystem : public AFileSystem
{
public:
	virtual ERet Mount( const char* path ) override;
	virtual void Unmount() override;
	virtual ERet GetAbsolutePath( const char* _fileName, String &_filePath ) override;

	const char* GetPath() const { return m_path.c_str(); }

protected:
	String256	m_path;
};

/*
-----------------------------------------------------------------------------
	DevAssetFolder
	can be used for reading asset files in development mode
-----------------------------------------------------------------------------
*/
class DevAssetFolder : public LocalFileSystem, public AFilePackage
{
	// for detecting file changes and hot-reloading changed assets
	MyFileWatcher	m_fileWatcher;

public:
	DevAssetFolder();
	~DevAssetFolder();

	ERet Initialize();
	void Shutdown();

	//@ AFileSystem
	virtual ERet Mount( const char* path ) override;
	virtual void Unmount();

	void ProcessChangedAssets( AFileWatchListener* callback );

	//@ AFilePackage
	virtual ERet OpenFile( const AssetID& fileId, Stream *stream ) override;
	virtual ERet CloseFile( Stream * stream ) override;
	virtual ERet ReadFile( Stream * stream, void *buffer, UINT bytesToRead ) override;
	virtual size_t TellPosition( const Stream& stream ) const override;
};
#if 0

/*
-----------------------------------------------------------------------------
	LocalFolder
	can be used for reading asset files in development mode
-----------------------------------------------------------------------------
*/
class LocalFolder : public AFilePackage
{
	String256	m_path;
public:
	LocalFolder();
	~LocalFolder();
	ERet Mount( const char* path );
	void Unmount();
	virtual ERet OpenFile( const AssetID& fileId, Stream *stream ) override;
	virtual ERet CloseFile( Stream * stream ) override;
	virtual ERet ReadFile( Stream * stream, void *buffer, UINT bytesToRead ) override;
	virtual size_t TellPosition( const Stream& stream ) const override;
};
#endif

class AssetHotReloader : public AFileWatchListener
{
public:
	virtual void Process( const efswData& _data ) override;
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
