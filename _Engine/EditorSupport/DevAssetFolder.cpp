#include "EditorSupport_PCH.h"
#pragma hdrstop
#include <Base/Util/PathUtils.h>
#include <EditorSupport/FileWatcher.h>
#include <EditorSupport/DevAssetFolder.h>

ERet LocalFileSystem::Mount( const char* path )
{
	Str::CopyS( m_path, path );
	Str::NormalizePath( m_path );

	chkRET_X_IF_NOT(OS::IO::PathExists(m_path.c_str()), ERR_FILE_OR_PATH_NOT_FOUND);

	return ALL_OK;
}
void LocalFileSystem::Unmount()
{
	m_path.Empty();
}
ERet LocalFileSystem::GetAbsolutePath( const char* _fileName, String &_filePath )
{
	SFindFileInfo	fileInfo;
	if( Win32_FindFileInDirectory( m_path.c_str(), _fileName, fileInfo ) )
	{
		Str::Copy( _filePath, fileInfo.fullFileName );
		return ALL_OK;
	}
	return ERR_OBJECT_NOT_FOUND;
}

/*
-----------------------------------------------------------------------------
	DevAssetFolder
-----------------------------------------------------------------------------
*/
DevAssetFolder::DevAssetFolder()
{
}
DevAssetFolder::~DevAssetFolder()
{
	mxASSERT(!Assets::IsPackageMounted(this));
}
ERet DevAssetFolder::Initialize()
{
	mxDO(m_fileWatcher.Initialize());
	return ALL_OK;
}
void DevAssetFolder::Shutdown()
{
	m_fileWatcher.Shutdown();
}
ERet DevAssetFolder::Mount( const char* path )
{
	mxASSERT(!Assets::IsPackageMounted(this));

	mxDO(LocalFileSystem::Mount( path ));

	DBGOUT("Mounting folder: '%s'", path);

	mxDO(Assets::MountPackage(this));

	m_fileWatcher.AddFileWatcher( path, true /*recursive*/ );
	m_fileWatcher.StartWatching();

	return ALL_OK;
}
void DevAssetFolder::Unmount()
{
	m_fileWatcher.RemoveFileWatchers();
	Assets::UnmountPackage(this);

	LocalFileSystem::Unmount();
}
void DevAssetFolder::ProcessChangedAssets( AFileWatchListener* callback )
{
	m_fileWatcher.ProcessNotifications( callback );
}
ERet DevAssetFolder::OpenFile( const AssetID& fileId, Stream *stream )
{
	chkRET_X_IF_NIL(stream, ERR_NULL_POINTER_PASSED);
	//DBGOUT("searching '%s' in '%s'",AssetId_ToChars(fileId),m_path.c_str());

	SFindFileInfo	fileInfo;
	if( !Win32_FindFileInDirectory( m_path.c_str(), fileId.d.c_str(), fileInfo ) )
	{
		return ERR_OBJECT_NOT_FOUND;
	}

	FileHandleT fileHandle = OS::IO::Create_File(
		fileInfo.fullFileName.c_str(),
		OS::IO::AccessMode::ReadAccess
	);
	if( fileHandle == INVALID_HANDLE_VALUE ) {
		return ERR_FAILED_TO_OPEN_FILE;
	}

	LogStream(LL_Debug) << "found '" << fileInfo.fileNameOnly << "' in '" << fileInfo.relativePath << "'";

	//stream->DbgSetName( AssetId_ToChars(fileId) );
	mxSTATIC_ASSERT(FIELD_SIZE(AFilePackage::Stream,fileHandle) >= sizeof(fileHandle));
	stream->fileHandle = (UINT64)fileHandle;
	stream->dataSize = OS::IO::Get_File_Size(fileHandle);
	stream->offset = 0;

	return ALL_OK;
}
ERet DevAssetFolder::CloseFile( Stream * stream )
{
	chkRET_X_IF_NIL(stream, ERR_NULL_POINTER_PASSED);

	const FileHandleT fileHandle = (FileHandleT)stream->fileHandle;
	chkRET_X_IF_NOT(fileHandle != INVALID_HANDLE_VALUE, ERR_INVALID_PARAMETER);
	OS::IO::Close_File( fileHandle );
	return ALL_OK;
}
ERet DevAssetFolder::ReadFile( Stream * stream, void *buffer, UINT bytesToRead )
{
	chkRET_X_IF_NIL(stream, ERR_NULL_POINTER_PASSED);
	chkRET_X_IF_NIL(buffer, ERR_NULL_POINTER_PASSED);
	chkRET_X_IF_NOT(bytesToRead > 0, ERR_INVALID_PARAMETER);

	const FileHandleT fileHandle = (FileHandleT) stream->fileHandle;
	const size_t numReadBytes = OS::IO::Read_File( fileHandle, buffer, bytesToRead );
	mxASSERT(numReadBytes == bytesToRead);
	stream->offset += numReadBytes;
	return ALL_OK;
}
size_t DevAssetFolder::TellPosition( const Stream& stream ) const
{
	const FileHandleT fileHandle = (FileHandleT) stream.fileHandle;
	return OS::IO::Tell_File_Position( fileHandle );
}

#if 0
/*
-----------------------------------------------------------------------------
	LocalFolder
-----------------------------------------------------------------------------
*/
LocalFolder::LocalFolder()
{
}
LocalFolder::~LocalFolder()
{
	mxASSERT(!Assets::IsPackageMounted(this));
}
ERet LocalFolder::Mount( const char* path )
{
	mxASSERT(!Assets::IsPackageMounted(this));

	Str::CopyS( m_path, path );
	Str::NormalizePath( m_path );

	chkRET_X_IF_NOT(OS::IO::PathExists(m_path.c_str()), ERR_FILE_OR_PATH_NOT_FOUND);

	DBGOUT("Mounting folder: '%s'", path);

	mxDO(Assets::MountPackage(this));

	return ALL_OK;
}
void LocalFolder::Unmount()
{
	Assets::UnmountPackage(this);
}
ERet LocalFolder::OpenFile( const AssetID& fileId, Stream *stream )
{
	chkRET_X_IF_NIL(stream, ERR_NULL_POINTER_PASSED);
	//DBGOUT("searching '%s' in '%s'",AssetId_ToChars(fileId),m_path.c_str());

	String256	filePath( m_path );
	Str::AppendS( filePath, fileId.d.ToPtr(), fileId.d.Length() );

	const FileHandleT fileHandle = OS::IO::Create_File(
		filePath.ToPtr(),
		OS::IO::AccessMode::ReadAccess
	);
	if( fileHandle == INVALID_HANDLE_VALUE ) {
		return ERR_FAILED_TO_OPEN_FILE;
	}
	//stream->DbgSetName( AssetId_ToChars(fileId) );
	mxSTATIC_ASSERT(FIELD_SIZE(AFilePackage::Stream,fileHandle) >= sizeof(fileHandle));
	stream->fileHandle = (UINT64)fileHandle;
	stream->dataSize = OS::IO::Get_File_Size(fileHandle);

	return ALL_OK;
 }
ERet LocalFolder::CloseFile( Stream * stream )
{
	chkRET_X_IF_NIL(stream, ERR_NULL_POINTER_PASSED);

	const FileHandleT fileHandle = (FileHandleT)stream->fileHandle;
	chkRET_X_IF_NOT(fileHandle != INVALID_HANDLE_VALUE, ERR_INVALID_PARAMETER);
	OS::IO::Close_File( fileHandle );
	return ALL_OK;
}
ERet LocalFolder::ReadFile( Stream * stream, void *buffer, UINT bytesToRead )
{
	chkRET_X_IF_NIL(stream, ERR_NULL_POINTER_PASSED);
	chkRET_X_IF_NIL(buffer, ERR_NULL_POINTER_PASSED);
	chkRET_X_IF_NOT(bytesToRead > 0, ERR_INVALID_PARAMETER);

	const FileHandleT fileHandle = (FileHandleT) stream->fileHandle;
	const size_t numReadBytes = OS::IO::Read_File( fileHandle, buffer, bytesToRead );
	mxASSERT(numReadBytes == bytesToRead);
	return ALL_OK;
}
size_t LocalFolder::TellPosition( const Stream& stream ) const
{
	const FileHandleT fileHandle = (FileHandleT) stream.fileHandle;
	return OS::IO::Tell_File_Position( fileHandle );
}
#endif

void AssetHotReloader::Process( const efswData& _data )
{
	if( _data.action == efsw::Actions::Modified )
	{
		const char* extension = Str::FindExtensionS(_data.filename.c_str());
		if( extension )
		{
			const AssetTypeT type = Assets::FindAssetTypeByExtension( extension );
			if( type != AssetTypes::UNKNOWN )
			{
				String256 assetName;
				Str::CopyS( assetName, _data.filename.c_str() );
				Str::StripPath( assetName );

				Assets::ReloadAsset(AssetKey(MakeAssetID( assetName.c_str()), type ));
			}
		}
	}
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
