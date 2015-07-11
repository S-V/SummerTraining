// watching changed asset files and initiating their hot-reloading
#include "EditorSupport_PCH.h"
#pragma hdrstop
#include <EditorSupport/FileWatcher.h>

const char* efsw_Action_To_Chars( efsw::Action action )
{
	switch( action ) {
		case efsw::Actions::Add :		return "Add";
		case efsw::Actions::Delete :	return "Delete";
		case efsw::Actions::Modified :	return "Modified";
		case efsw::Actions::Moved :		return "Moved";
			mxDEFAULT_UNREACHABLE;
	}
	return mxEMPTY_STRING;
}

MyFileWatcher::MyFileWatcher()
{
}

MyFileWatcher::~MyFileWatcher()
{
	this->Shutdown();
}

ERet MyFileWatcher::Initialize()
{
	m_notificationsCS.Initialize();
	return ALL_OK;
}

void MyFileWatcher::Shutdown()
{
	this->RemoveFileWatchers();
	m_notificationsCS.Shutdown();
}

void MyFileWatcher::StartWatching()
{
	m_fileWatcher.watch();
}

void ConvertToNativePath( String & _path )
{
	if( _path.NonEmpty() )
	{
		Str::ReplaceChar( _path, '/', '\\' );
		if( _path.GetLast() != '\\' ) {
			Str::Append( _path, '\\' );
		}
	}
}

efsw::WatchID MyFileWatcher::AddFileWatcher( const char* folder, bool recursive )
{
	String256 nativePath;
	Str::CopyS( nativePath, folder );
	ConvertToNativePath( nativePath );

	efsw::WatchID watchID = m_fileWatcher.addWatch( nativePath.c_str(), this, recursive );
	if( watchID < 0 ) {
		ptERROR( "%s\n", efsw::Errors::Log::getLastErrorLog().c_str() );
		return watchID;
	}
	ptPRINT("Started watching folder: '%s'\n", folder);
	m_watchedFolders.push_back( watchID );
	return watchID;
}

void MyFileWatcher::RemoveFileWatchers()
{
	std::vector< efsw::WatchID >::iterator it( m_watchedFolders.begin() );
	while( it != m_watchedFolders.end() ) {
		m_fileWatcher.removeWatch( *it );
		++it;
	}
	m_watchedFolders.clear();
}

void MyFileWatcher::ProcessNotifications( AFileWatchListener* listener )
{
	SpinWait::Lock scopedLock( m_notificationsCS );
	std::vector< efswData >::iterator it( m_notifications.begin() );
	while( it != m_notifications.end() )
	{
		const efswData& data = *it;
		DBGOUT("Folder: '%s', FileName: '%s', Action: '%s', OldFileName: '%s'\n",
			data.folder.c_str(), data.filename.c_str(), efsw_Action_To_Chars(data.action), data.oldFilename.c_str());
		listener->Process( data );
		++it;
	}
	m_notifications.clear();
}

void MyFileWatcher::handleFileAction(
		efsw::WatchID watchid,
		const std::string& dir,
		const std::string& filename,
		efsw::Action action, std::string oldFilename
	)
{
	SpinWait::Lock scopedLock( m_notificationsCS );
	efswData notification = { watchid, dir, filename, oldFilename, action };
	m_notifications.push_back( notification );
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
