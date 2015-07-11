/*
=============================================================================
	File:	Core.cpp
	Desc:	Base system, init/cleanup.
=============================================================================
*/
#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>
#include <process.h>	// _cexit

#include <Base/Memory/Memory_Private.h>

#include <Base/Math/Math.h>

#include <Base/Object/TypeRegistry.h>

namespace
{

class MSVCppDebugOutput : public ALog
{
public:
	virtual void VWrite( ELogLevel _level, const char* _message, int _length ) override
	{
		mxUNUSED(_level);
		mxASSERT(_length > 0);
		// make sure that the string is null-terminated
		char	tmp[1024];
		int		len = smallest(_length, sizeof(tmp)-1);
		strncpy(tmp, _message, len);
		tmp[len] = '\0';
		_message = tmp;
		::OutputDebugStringA( _message );

		::OutputDebugStringA( "\n" );
	}
	virtual void VWriteLinePrefix( ELogLevel _level, const char* _prefix, int _length ) override
	{}
};

class LogManager : public ALogManager, SingleInstance< LogManager >
{
	ALog *				m_loggers;
	MSVCppDebugOutput	m_nativeOutput;
	SpinWait			m_criticalSection;
	THREAD_ID			m_mainThreadId;
	ELogLevel			m_threshhold;

	void GetMessagePrefix(ELogLevel _level, String &_prefix)
	{
		switch(_level)
		{
		case LL_Trace :	break;
		case LL_Debug :	_prefix.Copy(Chars("DBG ")); break;
		case LL_Info :	_prefix.Copy(Chars("INFO ")); break;
		case LL_Warn :	_prefix.Copy(Chars("WARN ")); break;
		case LL_Error :	_prefix.Copy(Chars("ERROR ")); break;
		case LL_Fatal :	_prefix.Copy(Chars("FATAL ")); break;
		default:		_prefix.Copy(Chars("UNKNOWN ")); break;
		}
		const CalendarTime time( CalendarTime::GetCurrentLocalTime() );
		const THREAD_ID threadId = ptGetCurrentThreadID();

		String64	timeOfDayStr;
		GetTimeOfDayString( timeOfDayStr, time.hour, time.minute, time.second );

		if( threadId == m_mainThreadId ) {
			Str::SAppendF(_prefix, "%s: ", timeOfDayStr.ToPtr());
		} else {
			Str::SAppendF(_prefix, "%s[ThreadId = 0x%x]: ", timeOfDayStr.ToPtr(), (int)threadId);
		}
	}

public:
	LogManager()
	{
		m_loggers = nil;
		m_nativeOutput.PrependSelfToList(&m_loggers);
		m_criticalSection.Initialize();
		m_mainThreadId = ptGetMainThreadID();
		m_threshhold = LL_Trace;
	}
	virtual ~LogManager()
	{
		m_criticalSection.Shutdown();
	}
	virtual	void VWrite( ELogLevel _level, const char* _message, int _length ) override
	{
		mxASSERT_PTR(_message);
		mxASSERT(_length > 0);

		if( _level >= m_threshhold )
		{
			SpinWait::Lock	scopedLock(m_criticalSection);

			//String64	prefix;
			//this->GetMessagePrefix(_level, prefix);

			ALog* current = m_loggers;
			while( current ) {
				//if( prefix.NonEmpty() ) {
				//	current->VWriteLinePrefix( _level, prefix.ToPtr(), prefix.Num() );
				//}
				current->VWrite( _level, _message, _length );
				current = current->_next;
			}
		}
		/*
		if( _level == ELogLevel::LL_Error )
		{
			const int result = ::MessageBoxA( nil, _message, "An unexpected error occurred - Do you want to exit?", MB_YESNO );
			if( IDYES == result ) {
				mxForceExit(-1);
			}
		}
		*/
		if( _level == ELogLevel::LL_Fatal )
		{
			::MessageBoxA( nil, _message, "A critical error occurred,\nthe program will now exit.", MB_OK );
			mxForceExit(-1);
		}
	}
	virtual void Flush() override
	{
		ALog* current = m_loggers;
		while( current ) {
			current->Flush();
			current = current->_next;
		}
	}
	virtual void Close() override
	{
		ALog* current = m_loggers;
		while( current ) {
			current->Close();
			current = current->_next;
		}
	}
	virtual bool Attach( ALog* logger ) override
	{
		chkRET_FALSE_IF_NIL(logger);
		logger->AppendSelfToList( &m_loggers );
		return true;
	}
	virtual bool Detach( ALog* logger ) override
	{
		chkRET_FALSE_IF_NIL(logger);
		logger->RemoveSelfFromList( &m_loggers );
		return true;
	}
	virtual bool IsRedirectingTo( ALog* logger ) override
	{
		chkRET_FALSE_IF_NIL(logger);
		return logger->FindSelfInList(m_loggers);
	}
	virtual void SetThreshold( ELogLevel _level ) override
	{
		m_threshhold = _level;
	}
};

//
// this tells how many times the base system has been requested to initialize
// (think 'reference-counting')
//
static NiftyCounter	g_iBaseSystemReferenceCount;

static TStaticBlob16< PtBaseSubsystem >	g_BaseSubsystem;

static FCallback *	g_pExitHandler = nil;
static void *		g_pExitHandlerArg = nil;

static TStaticBlob16< LogManager >	g_Logger;

}//anonymous namespace

/*================================
		PtBaseSubsystem
================================*/

PtBaseSubsystem::PtBaseSubsystem()
{
	DBG_ENSURE_ONLY_ONE_CALL;

	// Initialize platform-specific services.
	mxPlatform_Init();

	// Initialize the global math.
	Math::Init();

	F_SetupMemorySubsystem();

	g_Logger.Construct();

	TypeRegistry::Initialize();

	//mxUtil_StartLogging( &mxGetLog() );
}
//---------------------------------------------------------------------------
PtBaseSubsystem::~PtBaseSubsystem()
{
	if( g_pExitHandler ) {
		g_pExitHandler( g_pExitHandlerArg );
	}

	// Destroy the type system.
	TypeRegistry::Destroy();

	// Shutdown the global math.
	Math::Shutdown();

	//mxUtil_EndLogging( &mxGetLog() );

#if MX_ENABLE_PROFILING
	PtProfileManager::CleanupMemory();
#endif

	g_Logger.Get().Close();
	g_Logger.Destruct();

	F_ShutdownMemorySubsystem();

	mxPlatform_Shutdown();
}

/*
================================
	mxGetLog
================================
*/
ALogManager& mxGetLog()
{
	return g_Logger.Get();
}

/*
================================
	mxBaseSystemIsInitialized
================================
*/
bool mxBaseSystemIsInitialized() {
	return (g_iBaseSystemReferenceCount.GetCount() > 0);
}

/*
================================
	mxInitializeBase
================================
*/
bool mxInitializeBase()
{
	if( g_iBaseSystemReferenceCount.IncRef() ) {
		g_BaseSubsystem.Construct();
	}
	return true;
}

/*
================================
	mxShutdownBase
================================
*/
bool mxShutdownBase()
{
	//if( g_iBaseSystemReferenceCount.NumRefs() <= 0 ) {
	//	ptWARN("Base system has already been shut down.");
	//	return true;
	//}

	if( g_iBaseSystemReferenceCount.DecRef() ) {
		g_BaseSubsystem.Destruct();
		return true;
	}

	return false;
}

/*
================================
	ForceExit_BaseSystem
================================
*/
void mxForceExit( int exitCode )
{
	//ptPRINT( "Exiting forcefully at your request.\n" );
	ptPRINT( "Program ended at your request.\n" );

	//@todo: call user callback to clean up resources

	while( !g_iBaseSystemReferenceCount.DecRef() )
		;
	g_BaseSubsystem.Destruct();

	// shutdown the C runtime, this cleans up static objects but doesn't shut 
	// down the process
	::_cexit();

	//::exit( exitCode );
	::ExitProcess( exitCode );
}

/*
================================
	mxSetExitHandler
================================
*/
void mxSetExitHandler( FCallback* pFunc, void* pArg )
{
	g_pExitHandler = pFunc;
	g_pExitHandlerArg = pArg;
}

/*
================================
	mxSetExitHandler
================================
*/
void mxGetExitHandler( FCallback **pFunc, void **pArg )
{
	mxASSERT_PTR(pFunc);
	mxASSERT_PTR(pArg);
	*pFunc = g_pExitHandler;
	*pArg = g_pExitHandlerArg;
}



//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
