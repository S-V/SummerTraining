/*
=============================================================================
	File:	String.h
	Desc:	String class and string-related functions.
	ToDo:	fix bugs
=============================================================================
*/
#pragma once

#include <Base/Object/Reflection.h>
#include <Base/Text/StringTools.h>

/*
===============================================================================
	Character string

	Stores a length and maintains a terminating null byte.
	Uses small local storage to avoid memory allocations.
===============================================================================
*/

struct StringBase
{
#if mxARCH_TYPE == mxARCH_64BIT
	char *	m_start;	// should always be null-terminated
	UINT32	m_length;	// length not including the last null byte
	UINT32	m_alloced;	// 2 upper bits contain memory ownership status
	// 16 bytes
	enum Constants {
		ALIGNMENT = 4,
		DONOT_FREE_MEMORY_MASK = UINT16(1u << 31),	// should we deallocate the memory block?
		GET_BUFFER_LENGTH_MASK = UINT16(~DONOT_FREE_MEMORY_MASK),
	};
#elif mxARCH_TYPE == mxARCH_32BIT
	char *	m_start;	// should always be null-terminated
	UINT16	m_length;	// length not including the last null byte
	UINT16	m_alloced;	// 2 upper bits contain memory ownership status
	// 8 bytes
	enum Constants {
		ALIGNMENT = 4,
		//CAN_MODIFY_MEMORY_MASK = UINT16(1u << 14),	// can we write to the pointed memory?
		DONOT_FREE_MEMORY_MASK = UINT16(1u << 15),	// should we deallocate the memory block?
		GET_BUFFER_LENGTH_MASK = UINT16(~DONOT_FREE_MEMORY_MASK),
	};
#endif
};

class String : public StringBase, public TStringBase< char, String >
{
	void Initialize();

public:
	String();
	String( const String& other );
	~String();

	//NOTE: maximum string length will be (size - 1)
	void SetExternalStorage( char* buffer, UINT size );
	bool OwnsMemory() const;

	UINT Length() const;
	UINT Capacity() const;	// returns the number of allocated items

	void Empty();
	void Clear();

	ERet Resize( UINT _length );
	ERet Reserve( UINT _capacity );
	void CapLength( UINT _length );

	//=== TArrayBase
	UINT Num() const;
	char* ToPtr();
	const char* ToPtr() const;

	String& Copy( const Chars& _chars );

	// NOTE: dangerous - writing to read-only memory results in a crash!
	String& SetReference( const Chars& _chars );

	// you may want to call this before writing to the string
	void EnsureOwnsMemory();

	operator const Chars () const;

	String& operator = ( const Chars& _chars );
	String& operator = ( const String& other );

	// case sensitive comparison
	bool operator == ( const String& other ) const;
	bool operator != ( const String& other ) const;

public:	// binary serialization
	ERet SaveToStream( AStreamWriter &_stream ) const;
	ERet LoadFromStream( AStreamReader& _stream );
	friend AStreamWriter& operator << ( AStreamWriter& file, const String& obj );
	friend AStreamReader& operator >> ( AStreamReader& file, String& obj );
	friend mxArchive& operator && ( mxArchive& archive, String& o );

	void DoNotFreeMemory();

	void* GetBufferAddress() { return &m_start; }
};

mxSTATIC_ASSERT_ISPOW2(sizeof(String));

mxDECLARE_BUILTIN_TYPE( String,	ETypeKind::Type_String );

mxIMPLEMENT_SERIALIZE_FUNCTION( String, SerializeString );

typedef TArray< String >	StringListT;

// Forward declarations.
UINT mxGetHashCode( const String& _str );

/*
-----------------------------------------------------------------------------
	TLocalString< SIZE >

	aka Stack String / Static String / Fixed String

	Dynamic string with a small embedded storage.
	Grows automatically when the local buffer is not big enough.

	NOTE: 'SIZE' is the size of the string in bytes, not its maximum capacity!
-----------------------------------------------------------------------------
*/
template< UINT SIZE >
struct TLocalString : public String
{
	char	m_storage[ SIZE - sizeof(StringBase) ];	// local storage to avoid dynamic memory allocation

	void SetLocalBuffer() {
		m_storage[0] = '\0';
		String::SetExternalStorage( m_storage, mxCOUNT_OF(m_storage) );
	}
public:
	TLocalString()
	{
		this->SetLocalBuffer();
	}
	TLocalString( const TLocalString& other )
	{
		this->SetLocalBuffer();
		*this = other;
	}
	TLocalString( const Chars& s )
	{
		this->SetLocalBuffer();
		String::Copy( s );
	}

	void Clear()
	{
		String::Clear();
		this->SetLocalBuffer();
	}
};

typedef TLocalString< 32 >	String32;
typedef TLocalString< 64 >	String64;
typedef TLocalString< 96 >	String96;
typedef TLocalString< 128 >	String128;
typedef TLocalString< 256 >	String256;
typedef TLocalString< 512 >	String512;

mxDECLARE_BUILTIN_TYPE( String32,	ETypeKind::Type_String );
mxDECLARE_BUILTIN_TYPE( String64,	ETypeKind::Type_String );
mxDECLARE_BUILTIN_TYPE( String128,	ETypeKind::Type_String );
mxDECLARE_BUILTIN_TYPE( String256,	ETypeKind::Type_String );
mxDECLARE_BUILTIN_TYPE( String512,	ETypeKind::Type_String );

namespace Str
{
	String& CopyS( String & _dst, const char* _str, int _len );
	String& CopyS( String & _dst, const char* _str );
	String& Copy( String & _dst, const Chars& _chars );

	String& Append( String & _dst, char _c );
	String& AppendS( String & _dst, const char* _str, int _len );
	String& Append( String & _dst, const Chars& _chars );
	String& AppendS( String & _dst, const char* _str );

	int Cmp( const String& _str, const Chars& _chars );
	bool Equal( const String& _str, const Chars& _chars );
	bool EqualS( const String& _str, const char* _chars );
	bool EqualC( const String& _str, const char _char );

	bool StartsWith( const String& _string, const Chars& _text );

	String& mxVARARGS SAppendF( String & _dst, const char* _fmt, ... );
	String& mxVARARGS SPrintF( String & _dst, const char* _fmt, ... );

	template< class STRING >
	STRING mxVARARGS SPrintF( const char* _fmt, ... )
	{
		STRING	str;
		va_list	args;
		va_start( args, _fmt );
		ptPRINT_VARARGS_BODY( _fmt, args, {Str::CopyS(str, ptr_, len_);return str;} );
	}

	String& SetReference( String & _string, const char* _buffer );

	String& SetBool( String & _str, bool _value );
	String& SetChar( String & _str, char _value );
	String& SetInt( String & _str, int _value );
	String& SetUInt( String & _str, unsigned int _value );
	String& SetFloat( String & _str, float _value );
	String& SetDouble( String & _str, double _value );

	String& ToUpper( String & _str );

	String& StripTrailing( String & _str, char _c );

	String& ReplaceChar( String & _str, char _old, char _new );

	inline bool IsPathSeparator( char _c )
	{
		return _c == '/' || _c == '\\';
	}

	// removes the path from the filename
	String& StripPath( String & _str );

	// assumes the the string ends with a path separator
	String& StripFileName( String & _str );

	// removes any file extension
	String& StripFileExtension( String & _str );

	String& SetFileExtension( String & _str, const char* extension );

	String& AppendSlash( String & _str );

	// changes all '\' to '/'.
	String& FixBackSlashes( String & _str );

	// Converts a path to the uniform form (Unix-style) in place
	String& NormalizePath( String & _str );

	const char* FindExtensionS( const char* filename );
	bool HasExtensionS( const char* filename, const char* extension );
	void ExtractExtension( String & _extension, const char* filename );
	const char* GetFileName( const char* filepath );

	template< class STRING >
	STRING ComposeFilePath( const char* _filepath, const char* _extension )
	{
		STRING	result;
		Str::CopyS(result, _filepath);
		Str::SetFileExtension(result, _extension);
		return result;
	}
	template< class STRING >
	STRING ComposeFilePath( const char* _path, const char* _name, const char* _extension )
	{
		STRING	result;
		Str::CopyS(result, _path);
		Str::NormalizePath(result);
		Str::StripFileName(result);
		Str::AppendS(result, _name);
		Str::SetFileExtension(result, _extension);
		return result;
	}

}//namespace Str

template< class TYPE >	// where TYPE has member 'name' of type 'String'
int FindIndexByName( const TArray< TYPE >& items, const String& name )
{
	for( int i = 0; i < items.Num(); i++ )
	{
		const TYPE& item = items[ i ];
		if( item.name == name ) {
			return i;
		}
	}
	return -1;
}
template< class TYPE >	// where TYPE has member 'name' of type 'String'
int FindIndexByName( const TBuffer< TYPE >& items, const String& name )
{
	for( int i = 0; i < items.Num(); i++ )
	{
		const TYPE& item = items[ i ];
		if( item.name == name ) {
			return i;
		}
	}
	return -1;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
