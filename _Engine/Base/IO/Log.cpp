/*
=============================================================================
	File:	Log.cpp
	Desc:	Log.
=============================================================================
*/

#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

/*================================
			ALog
================================*/

void ALog::PrintV( ELogLevel level, const char* fmt, va_list args )
{
	ptPRINT_VARARGS_BODY( fmt, args, this->VWrite(level, ptr_, len_) );
}
void ALog::PrintF( ELogLevel level, const char* fmt, ... )
{
	va_list	args;
	va_start( args, fmt );
	/*int len = */this->PrintV( level, fmt, args );
	va_end( args );
	/*mxUNUSED(len);*/
}

/*
-----------------------------------------------------------------------------
	ATextStream
-----------------------------------------------------------------------------
*/
ATextStream& ATextStream::PrintV( const char* fmt, va_list args )
{
	ptPRINT_VARARGS_BODY( fmt, args, this->VWrite(ptr_, len_) );
	return *this;
}

ATextStream& ATextStream::PrintF( const char* fmt, ... )
{
	va_list args;
	va_start(args, fmt);
	this->PrintV( fmt, args );
	va_end(args);
	return *this;
}

ATextStream& ATextStream::operator << (const void* p)
{
	this->PrintF("%p",p);
	return *this;
}

ATextStream& ATextStream::operator << (bool b)
{
	return (*this) << (b ? Chars("true") : Chars("false"));
}

ATextStream& ATextStream::operator << (char c)
{
	this->PrintF("%c",c);
	return *this;
}

ATextStream& ATextStream::operator << (const char* s)
{
	this->PrintF("%s",s);
	return *this;
}

ATextStream& ATextStream::operator << (const signed char* s)
{
	this->PrintF("%s",s);
	return *this;
}

ATextStream& ATextStream::operator << (const unsigned char* s)
{
	this->PrintF("%s",s);
	return *this;
}

ATextStream& ATextStream::operator << (short s)
{
	this->PrintF("%d",(INT)s);
	return *this;
}

ATextStream& ATextStream::operator << (unsigned short s)
{
	this->PrintF("%u",(UINT)s);
	return *this;
}

ATextStream& ATextStream::operator << (int i)
{
	this->PrintF("%d",i);
	return *this;
}

ATextStream& ATextStream::operator << (unsigned int u)
{
	this->PrintF("%u",u);
	return *this;
}

ATextStream& ATextStream::operator << (float f)
{
	this->PrintF("%f",f);
	return *this;
}

ATextStream& ATextStream::operator << (double d)
{
	this->PrintF("%lf",d);
	return *this;
}

ATextStream& ATextStream::operator << (INT64 i)
{
	this->PrintF("%ld",i);
	return *this;
}

ATextStream& ATextStream::operator << (UINT64 u)
{
	this->PrintF("%lu",u);
	return *this;
}

ATextStream& ATextStream::operator << (const String& _str)
{
	this->VWrite( _str.c_str(), _str.GetDataSize() );
	return *this;
}

ATextStream& ATextStream::operator << (const Chars& _str)
{
	this->VWrite( _str.c_str(), _str.GetDataSize() );
	return *this;
}

ATextStream& ATextStream::Repeat( char c, int count )
{
	while( count-- ) {
		*this << c;
	}
	return *this;
}

/*================================
		LogStream
================================*/

StringStream::StringStream( String &_string )
	: m_string(_string)
{
}
StringStream::~StringStream()
{
}
void StringStream::VWrite( const char* text, int length )
{
	//@todo: insert the terminating null?
	const int offset = m_string.Num();
	m_string.Resize(offset + length);
	char* start = m_string.ToPtr();
	char* destination = start + offset;
	strncpy(destination, text, length);
}
const String& StringStream::GetString() const
{
	return m_string;
}

/*================================
		LogStream
================================*/

LogStream::LogStream( ELogLevel level )
	: StringStream(m_buffer), m_level(level)
{
}
LogStream::~LogStream()
{
	this->Flush();
}
void LogStream::Flush()
{
	if( m_buffer.Num() ) {
		mxGetLog().VWrite(m_level, m_buffer.ToPtr(), m_buffer.Num());
		m_buffer.Empty();
	}
}

/*================================
		TextStream
================================*/

TextStream::TextStream( AStreamWriter& stream )
	: mStream( stream )
{
}

TextStream::~TextStream()
{
}

void TextStream::VWrite( const char* text, int length )
{
	mStream.Write( text, length );
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
