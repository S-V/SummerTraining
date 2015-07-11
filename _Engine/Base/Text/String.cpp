/*
=============================================================================
	File:	String.cpp
	Desc:	String class.
=============================================================================
*/
#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

namespace {
	enum { MAX_LENGTH = 4096 };
	enum { GRANULARITY = 16 };
	static char* AllocateStringData( UINT size ) {
		return new char[ size ];
	}
	static void FreeStringData( void* ptr ) {
		delete [] ptr;
	}
}//namespace

// this should only be called on a freshly constructed string
void String::Initialize()
{
	m_start = nil;
	m_length = 0;
	m_alloced = 0;
}

String::String()
{
	this->Initialize();
}

String::String( const String& other )
{
	this->Initialize();
	*this = other;
}

String::~String()
{
	this->Clear();
}

void String::SetExternalStorage( char* buffer, UINT size )
{
	mxASSERT_PTR(buffer);
	mxASSERT(size > 0);
	this->Clear();
	m_start = buffer;
	m_length = 0;
	m_alloced = size | DONOT_FREE_MEMORY_MASK;
}

bool String::OwnsMemory() const
{
	return m_start && (m_alloced & DONOT_FREE_MEMORY_MASK) == 0;
}

UINT String::Length() const
{
	return m_length;
}

UINT String::Capacity() const
{
	return (m_alloced & GET_BUFFER_LENGTH_MASK);
}

void String::Empty()
{
	if( this->OwnsMemory() ) {
		m_start[0] = '\0';
		m_length = 0;
	} else {
		this->Initialize();
	}
}

void String::Clear()
{
	if( m_start && this->OwnsMemory() ) {
		FreeStringData( m_start );
	}
	this->Initialize();
}

ERet String::Resize( UINT _length )
{
	mxDO(this->Reserve( _length + 1 ));
	m_start[ _length ] = '\0';
	m_length = _length;
	return ALL_OK;
}

ERet String::Reserve( UINT _capacity )
{
	const UINT length = this->Length();
	const UINT capacity = this->Capacity();	
	if( _capacity > capacity )
	{
		char* newBuffer = AllocateStringData( _capacity );
		if( !newBuffer ) {
			return ERR_OUT_OF_MEMORY;
		}
		if( length ) {
			memcpy( newBuffer, m_start, length+1 );	// copy trailing zero
		}
		if( this->OwnsMemory() ) {
			FreeStringData( m_start );
		}
		m_start = newBuffer;
		m_alloced = _capacity;
	}
	return ALL_OK;
}

void String::CapLength( UINT _length )
{
	if( m_length > _length ) {
		m_start[ _length ] = '\0';
		m_length = _length;
	}
}

UINT String::Num() const
{
	return this->Length();
}

char* String::ToPtr()
{
	mxASSERT_PTR(m_start);
	return m_start;
}

const char* String::ToPtr() const
{
	mxASSERT_PTR(m_start);
	return m_start;
}

String& String::Copy( const Chars& chars )
{
	if( chars.length ) {
		mxASSERT(m_start != chars.buffer);
		this->Resize( chars.length );
		strncpy( m_start, chars.buffer, chars.length );
	} else {
		this->Empty();
	}
	return *this;
}

String& String::SetReference( const Chars& chars )
{
	this->Clear();
	this->SetExternalStorage( const_cast< char* >( chars.buffer ), chars.length+1 );
	m_length = chars.length;
	return *this;
}

void String::EnsureOwnsMemory()
{
	if( m_length > 0 && !this->OwnsMemory() )
	{
		char* newBuffer = AllocateStringData( m_length + 1 );
		memcpy( newBuffer, m_start, m_length );
		m_start = newBuffer;
		m_start[m_length] = '\0';
		m_alloced = m_length + 1;
	}
}

String::operator const Chars () const
{
	return Chars( m_start, m_length );
}

String& String::operator = ( const Chars& chars )
{
	return this->Copy(chars);
}

String& String::operator = ( const String& other )
{
	return this->Copy(other);
}

bool String::operator == ( const String& other ) const
{
	if( m_start == other.m_start ) {
		return true;
	}
	return 0 == strcmp( this->SafeGetPtr(), other.SafeGetPtr() );
}

bool String::operator != ( const String& other ) const
{
	return !( *this == other );
}

void String::DoNotFreeMemory()
{
	m_alloced |= DONOT_FREE_MEMORY_MASK;
}

ERet String::SaveToStream( AStreamWriter &_stream ) const
{
	const UINT32 length = this->Length();
	mxDO(_stream.Put( length ));
	if( length > 0 ) {
		mxDO(_stream.Write( this->ToPtr(), length ));
	}
	return ALL_OK;
}

ERet String::LoadFromStream( AStreamReader& _stream )
{
	UINT32	length;
	mxDO(_stream.Get( length ));
	if( length > 0 ) {
		mxDO(this->Resize( length ));
		mxDO(_stream.Read( this->ToPtr(), length ));
	} else {
		this->Empty();
	}
	return ALL_OK;
}

AStreamWriter& operator << ( AStreamWriter& stream, const String& obj )
{
	obj.SaveToStream(stream);
	return stream;
}

AStreamReader& operator >> ( AStreamReader& stream, String& obj )
{
	obj.LoadFromStream(stream);
	return stream;
}

namespace Str
{
	String& CopyS( String & _dst, const char* _str, int _len )
	{
		_dst.Resize( _len );
		char* ptr = _dst.ToPtr();
		strncpy( ptr, _str, _len );
		ptr[ _len ] = '\0';
		return _dst;
	}
	String& CopyS( String & _dst, const char* _str )
	{
		return CopyS( _dst, _str, strlen(_str) );
	}
	String& Copy( String & _dst, const Chars& _chars )
	{
		return CopyS( _dst, _chars.buffer, _chars.length );
	}

	String& Append( String & _dst, char _c )
	{
		return AppendS( _dst, &_c, 1 );
	}
	String& AppendS( String & _dst, const char* _str, int _len )
	{
		const int oldLen = _dst.Length();
		_dst.Resize( oldLen + _len );
		char* ptr = _dst.ToPtr();
		memcpy( ptr + oldLen, _str, _len );
		ptr[ oldLen + _len ] = '\0';
		return _dst;
	}
	String& Append( String & _dst, const Chars& _chars )
	{
		return AppendS( _dst, _chars.buffer, _chars.length );
	}
	String& AppendS( String & _dst, const char* _str )
	{
		return AppendS( _dst, _str, strlen(_str) );
	}

	int Cmp( const String& _str, const Chars& _chars )
	{
		return strcmp( _str.c_str(), _chars.buffer );
	}
	bool Equal( const String& _str, const Chars& _chars )
	{
		return Cmp( _str, _chars ) == 0;
	}
	bool EqualS( const String& _str, const char* _chars )
	{
		return Cmp( _str, Chars(_chars) ) == 0;
	}

	bool StartsWith( const String& _string, const Chars& _text )
	{
		const char* s1 = _string.c_str();
		const char* s2 = _text.c_str();
		const int l1 = _string.Num();
		const int l2 = _text.Num();

		if( l1 >= l2 )
		{
			int n = l2;
			while( n-- )
			{
				if( *s1++ != *s2++ ) {
					return false;
				}
			}
			return true;
		}
		return false;
	}

	String& mxVARARGS SAppendF( String & _dst, const char* _fmt, ... )
	{
		va_list	args;
		va_start( args, _fmt );
		ptPRINT_VARARGS_BODY( _fmt, args, Str::AppendS(_dst, ptr_, len_) );
		va_end( args );
		return _dst;
	}
	String& mxVARARGS SPrintF( String & _dst, const char* _fmt, ... )
	{
		va_list	args;
		va_start( args, _fmt );
		ptPRINT_VARARGS_BODY( _fmt, args, Str::CopyS(_dst, ptr_, len_) );
		va_end( args );
		return _dst;
	}

	String& SetReference( String & _string, const char* _buffer )
	{
		const size_t length = strlen(_buffer);
		_string.SetReference(Chars(_buffer,length));
		return _string;
	}

	String& SetBool( String & _str, bool _value )
	{
		_str = _value ? Chars("true") : Chars("false");
		return _str;
	}
	String& SetChar( String & _str, char _value )
	{
		_str.Resize(1);
		*(_str.ToPtr()) = _value;
		return _str;
	}
	String& SetInt( String & _str, int _value )
	{
		return SPrintF( _str, "%d", _value );
	}
	String& SetUInt( String & _str, unsigned int _value )
	{
		return SPrintF( _str, "%u", _value );
	}
	String& SetFloat( String & _str, float _value )
	{
		return SPrintF( _str, "%f", _value );
	}
	String& SetDouble( String & _str, double _value )
	{
		return SPrintF( _str, "%g", _value );
	}

	String& ToUpper( String & _str )
	{
		int	len = _str.Length();
		char *	ptr = _str.ToPtr();		
		for( int i = 0; i < len; i++ ) {
			ptr[i] = toupper( ptr[i] );
		}
		return _str;
	}

	String& StripTrailing( String & _str, char _c )
	{
		char *	p = _str.ToPtr();
		int		i = _str.Length();
		while( (i > 0) && (_str[i-1] == _c) )
		{
			p[i-1] = '\0';
			i--;
		}
		_str.CapLength(i);
		return _str;
	}

	String& ReplaceChar( String & _str, char _old, char _new )
	{
		char *	p = _str.ToPtr();
		while( *p ) {
			if( *p == _old ) {
				*p = _new;
			}
			p++;
		}
		return _str;
	}

	// removes the path from the filename
	String& StripPath( String & _str )
	{
		String256	tmp;
		Str::Copy( tmp, _str );
		int		i = tmp.Length();
		char *	p = tmp.ToPtr();
		while( ( i > 0 ) && ( p[i-1] != '/' ) && ( p[i-1] != '\\' ) ) {
			i--;
		}
		Str::CopyS( _str, p + i, _str.Length() - i );
		return _str;
	}

	String& StripFileName( String & _str )
	{
		// scan the string backwards and find the position of the last path separator
		int		i = _str.Length() - 1;
		char *	p = _str.ToPtr();
		while( (i > 0) && !IsPathSeparator( p[i] ) ) {
			i--;
		}
		if( i < 0 ) {
			i = 0;
		}
		_str.CapLength(i+1);
		return _str;
	}

	// removes any file extension
	String& StripFileExtension( String & _str )
	{
		char *	charsPntr = _str.ToPtr();
		int	oldLength = _str.Length();
		int	_length = oldLength;
		for( INT i = oldLength-1; i >= 0; i-- )
		{
			if( charsPntr[i] == '.' ) {
				charsPntr[i] = '\0';
				_length = i;
				break;
			}
		}
		_str.Resize( _length );
		return _str;
	}

	String& SetFileExtension( String & _str, const char* extension )
	{
		StripFileExtension(_str);
		if ( *extension != '.' ) {
			Str::Append( _str, '.' );
		}
		Str::AppendS( _str, extension );
		return _str;
	}

	// changes all '\' to '/'.
	String& FixBackSlashes( String & _str )
	{
		return ReplaceChar( _str, '\\', '/' );
	}

	String& AppendSlash( String & _str )
	{
		if( _str.Length() > 0 && !IsPathSeparator(_str.GetLast()) )
		{
			Append( _str, '/' );
		}
		return _str;
	}

	// Converts a path to the uniform form (Unix-style) in place
	String& NormalizePath( String & _str )
	{
		if( _str.NonEmpty() )
		{
			FixBackSlashes( _str );
			if( _str.GetLast() != '/' ) {
				Append( _str, '/' );
			}
		}
		return _str;
	}

	const char* FindExtensionS( const char* filename )
	{
		int length = strlen(filename);
		for(int i = length-1; i >= 0; i-- )
		{
			if( filename[i] == '.' ) {
				return filename + i + 1;
			}
		}
		return nil;
	}
	bool HasExtensionS( const char* filename, const char* extension )
	{
		// skip '.' in the given extension string
		while( extension && *extension == '.' ) {
			extension++;
		}
		// back up until a '.' or the start
		const char* fileExt = FindExtensionS(filename);
		if( fileExt && extension )
		{
			return stricmp(fileExt, extension) == 0;
		}
		return false;
	}
	void ExtractExtension( String & _extension, const char* filename )
	{
		const char* extension = FindExtensionS(filename);
		if( extension ) {
			Str::CopyS( _extension, extension );
		} else {
			_extension.Empty();
		}
	}
	mxSWIPED("Valve's Source Engine");
	//-----------------------------------------------------------------------------
	// Purpose: Returns a pointer to the beginning of the unqualified file name 
	//			(no path information)
	// Input:	in - file name (may be unqualified, relative or absolute path)
	// Output:	pointer to unqualified file name
	//-----------------------------------------------------------------------------
	const char* GetFileName( const char* filepath )
	{
		// back up until the character after the first path separator we find,
		// or the beginning of the string
		const char * out = filepath + strlen( filepath ) - 1;
		while ( ( out > filepath ) && ( !IsPathSeparator( *( out-1 ) ) ) )
			out--;
		return out;
	}

}//namespace Str

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
