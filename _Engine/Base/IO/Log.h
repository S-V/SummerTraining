/*
=============================================================================
	File:	Log.h
	Desc:	Logging.
	References:
	"Efficient logging in multithreaded C++ server" by Shuo Chen
=============================================================================
*/
#pragma once

#include <Base/Template/Containers/LinkedList/TSinglyLinkedList.h>

/*
=================================================================
	A very simple logging device.
	
	Todo:	Push/Pop log levels?
			Overload '<<' for vectors,matrices, etc?
			Pass string sizes to improve speed?
=================================================================
*/

//
//	ALog - base class for logging devices.
//
struct ALog : public TSinglyLinkedList< ALog >
{
	virtual void VWrite( ELogLevel _level, const char* _message, int _length ) = 0;

	void PrintV( ELogLevel level, const char* fmt, va_list args );
	void PrintF( ELogLevel level, const char* fmt, ... );

	// Writes any pending output.
	virtual void Flush() {}

	// Prepares for destruction.
	virtual void Close() {}

	virtual void VWriteLinePrefix( ELogLevel _level, const char* _prefix, int _length )
	{
		this->VWrite( _level, _prefix, _length );
	}

protected:
	ALog() {}
	virtual ~ALog() {}
};

//
//	ALogManager
//
class ALogManager : public ALog
{
public:
	virtual bool Attach( ALog* logger ) = 0;
	virtual bool Detach( ALog* logger ) = 0;
	virtual bool IsRedirectingTo( ALog* logger ) = 0;

	// Set the level below which messages are ignored.
	virtual void SetThreshold( ELogLevel _level ) = 0;

	//// Enters a new (optionally) named scope.
	//virtual void PushScope(const char* name) = 0;

	//// Exit the current scope.
	//virtual void PopScope() = 0;

protected:
	virtual	~ALogManager()
	{}
};

ALogManager& mxGetLog();

/*
//
//	LogManipulator - controls behaviour of a logging device.
//
class LogManipulator
{
public:
	virtual ALog & operator () ( ALog& logger ) const = 0;
};

// executes the log manipulator on the logger (the stream)
//
inline ALog & operator << ( ALog& logger, const LogManipulator& manip )
{
	manip( logger );
	return logger;
}

	//virtual void PushLevel();
	//virtual void PopLevel();
	virtual void SetSeverity( ELogLevel level );

//
//	LogLevel
//
//	Usage:
//
//	Logger::get_ref() << LogLevel(LL_Info) << "X = " << X << endl;
//
class LogLevel : public LogManipulator
{
	const ELogLevel	mLogLevel;

public:
	LogLevel( ELogLevel ll )
		: mLogLevel( ll )
	{}
	virtual ALog & operator () ( ALog& logger ) const
	{
		logger.SetLogLevel( mLogLevel );
		return logger;
	};
};

//
//	ScopedLogLevel - automatically restores saved logging level.
//
class ScopedLogLevel
{
	ALog &	mLogger;
	ELogLevel	mSavedLogLevel;

public:
	ScopedLogLevel( ALog& logger )
		: mLogger( logger ), mSavedLogLevel( logger.GetLogLevel() )
	{}
	~ScopedLogLevel()
	{
		mLogger.SetLogLevel( mSavedLogLevel );
	}
};
*/

/*
-----------------------------------------------------------------------------
	ATextStream/ATextOutput

The API should feel like streams to relieve users from string formatting issues.

The logical record can be defined as the longest sequence of chained calls to stream insertion operator,
so that the following expression:
log << a << b << c << " and " << x() << y() << z();
defines a logging unit that is atomic in the resulting log output.

See: Atomic Log Stream for C++ http://www.inspirel.com/articles/Atomic_Log_Stream.html
-----------------------------------------------------------------------------
*/
class ATextStream {
public:
	// all text writing is redirected to this function
	virtual void VWrite( const char* text, int length ) = 0;

	/// Outputs a string with a variable arguments list.
	ATextStream& PrintV( const char* fmt, va_list args );

	/// Outputs formatted data.
	ATextStream& PrintF( const char* fmt, ... );

	/// Outputs a hex address.
	ATextStream& operator << (const void* p);

	/// Outputs a bool.
	ATextStream& operator << (bool b);

	/// Outputs a char.
	ATextStream& operator << (char c);

	/// Outputs a string.
	ATextStream& operator << (const char* s);

	/// Outputs a string.
	ATextStream& operator << (const signed char* s);

	/// Outputs a string.
	ATextStream& operator << (const unsigned char* s);

	/// Outputs a short.
	ATextStream& operator << (short s);

	/// Outputs an unsigned short
	ATextStream& operator << (unsigned short s);

	/// Outputs an int
	ATextStream& operator << (int i);

	/// Outputs an unsigned int.
	ATextStream& operator << (unsigned int u);

	/// Outputs a float.
	ATextStream& operator << (float f);

	/// Outputs a double.
	ATextStream& operator << (double d);

	/// Outputs a 64 bit int.
	ATextStream& operator << (INT64 i);

	/// Outputs a 64 bit unsigned int.
	ATextStream& operator << (UINT64 u);

	/// Outputs a String.
	ATextStream& operator << (const String& _str);

	ATextStream& operator << (const Chars& _str);

	ATextStream& Repeat( char c, int count );

protected:
	ATextStream() {}
	virtual ~ATextStream() {}
};

class StringStream : public ATextStream {
protected:
	String & 	m_string;
public:
	StringStream( String &_string );
	virtual ~StringStream();

	//=-- ATextStream
	virtual void VWrite( const char* text, int length ) override;

	const String& GetString() const;

	PREVENT_COPY(StringStream);
};

// Distributes log messages to their destination(s) (e.g. a text log or a byte stream).
// Records formatting in a private buffer and flushes the resulting buffer in the destructor.
class LogStream : public StringStream {
public:
	String512 		m_buffer;
	const ELogLevel	m_level;
public:
	LogStream( ELogLevel level );
	virtual ~LogStream();

	PREVENT_COPY(LogStream);
};

//
//	TextStream - provides formatted text output.
//
class TextStream : public ATextStream {
protected:
	AStreamWriter &	mStream;
public:
	explicit TextStream( AStreamWriter& stream );
	virtual ~TextStream();

	//=-- ATextStream
	virtual void VWrite( const char* text, int length ) override;

public_internal:
	AStreamWriter & GetStream() { return mStream; }

	PREVENT_COPY(TextStream);
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
