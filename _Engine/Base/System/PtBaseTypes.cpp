/*
=============================================================================
	File:	Common.cpp
	Desc:
=============================================================================
*/

#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

const char* mxSTRING_Unknown = "Unknown";
const char* mxSTRING_UNKNOWN_ERROR = "unknown error";
const char* mxSTRING_DOLLAR_SIGN = "$";
const char* mxSTRING_QUESTION_MARK = "?";
const char* mxEMPTY_STRING = "";


mxINLINE INT SimpleNameHash( const char *name )
{
	INT hash, i;

	hash = 0;
	for ( i = 0; name[i]; i++ ) {
		hash += name[i] * (119 + i);
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	return hash;
}

UINT mxGetHashCode( const String& _str )
{
	return SimpleNameHash( _str.ToPtr() );
}

/*
-----------------------------------------------------------------------------
	Blob
-----------------------------------------------------------------------------
*/

//------------------------------------------------------------------------------
/**
*/
ByteBuffer::ByteBuffer() :
ptr(nil),
size(0),
allocSize(0)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
bool
ByteBuffer::IsValid() const
{
	return (nil != this->ptr);
}

//------------------------------------------------------------------------------
/**
*/
void
ByteBuffer::Delete()
{
	if (this->IsValid())
	{
		mxFree( (void*)this->ptr );
		this->ptr = nil;
		this->size = 0;
		this->allocSize = 0;
	}
}

//------------------------------------------------------------------------------
/**
*/
ByteBuffer::~ByteBuffer()
{
	this->Delete();
}

//------------------------------------------------------------------------------
/**
*/
void
ByteBuffer::Allocate(size_t s)
{
	this->ptr = (char*)mxAlloc( s );
	this->allocSize = s;
	this->size = s;
}

void ByteBuffer::Reallocate( size_t newSize )
{
	// only re-allocate if not enough space
	if ((0 == this->ptr) || (this->allocSize < newSize))
	{
		const void* oldPtr = this->ptr;
		const size_t oldSize = this->size;

		this->Allocate(newSize);

		if( oldPtr != nil )
		{
			memcpy( (void*) this->ptr, oldPtr, oldSize );
			mxFree( (void*) oldPtr );
		}
	}
	this->size = newSize;
}

//------------------------------------------------------------------------------
/**
*/
void
ByteBuffer::Copy(const void* fromPtr, size_t fromSize)
{
	mxASSERT((nil != fromPtr) && (fromSize > 0));

	this->Reallocate( fromSize );
	memcpy( (void*) this->ptr, fromPtr, fromSize);
}

//------------------------------------------------------------------------------
/**
*/
ByteBuffer::ByteBuffer(const void* fromPtr, size_t fromSize) :
ptr(0),
size(0),
allocSize(0)
{    
	this->Copy(fromPtr, fromSize);
}

//------------------------------------------------------------------------------
/**
*/
ByteBuffer::ByteBuffer(const ByteBuffer& rhs) :
ptr(0),
size(0),
allocSize(0)
{
	if (rhs.IsValid())
	{
		this->Copy(rhs.ptr, rhs.size);
	}
}

//------------------------------------------------------------------------------
/**
*/
ByteBuffer::ByteBuffer(size_t s) :
ptr(0),
size(0),
allocSize(0)
{
	this->Allocate(s);
}

//------------------------------------------------------------------------------
/**
*/
void
ByteBuffer::operator=(const ByteBuffer& rhs)
{
	if (rhs.IsValid())
	{
		this->Copy(rhs.ptr, rhs.size);
	}
}

//------------------------------------------------------------------------------
/**
*/
bool
ByteBuffer::operator==(const ByteBuffer& rhs) const
{
	return (this->BinaryCompare(rhs) == 0);
}

//------------------------------------------------------------------------------
/**
*/
bool
ByteBuffer::operator!=(const ByteBuffer& rhs) const
{
	return (this->BinaryCompare(rhs) != 0);
}

//------------------------------------------------------------------------------
/**
*/
bool
ByteBuffer::operator>(const ByteBuffer& rhs) const
{
	return (this->BinaryCompare(rhs) > 0);
}

//------------------------------------------------------------------------------
/**
*/
bool
ByteBuffer::operator<(const ByteBuffer& rhs) const
{
	return (this->BinaryCompare(rhs) < 0);
}

//------------------------------------------------------------------------------
/**
*/
bool
ByteBuffer::operator>=(const ByteBuffer& rhs) const
{
	return (this->BinaryCompare(rhs) >= 0);
}

//------------------------------------------------------------------------------
/**
*/
bool
ByteBuffer::operator<=(const ByteBuffer& rhs) const
{
	return (this->BinaryCompare(rhs) <= 0);
}

//------------------------------------------------------------------------------
/**
*/
void
ByteBuffer::Reserve(size_t s)
{
	if (this->allocSize < s)
	{
		if(0)//this destroys old contents
		{
			this->Delete();
			this->Allocate(s);
		}
		else
		{
			char* pNewMem = (char*) mxAlloc( s );
			memcpy(pNewMem,this->ptr,this->size);
			this->Delete();
			this->ptr = pNewMem;
			this->allocSize = s;
			this->size = s;
		}
	}
	this->size = s;
}

//------------------------------------------------------------------------------
/**
*/
void
ByteBuffer::Trim(size_t trimSize)
{
	mxASSERT(trimSize <= this->size);
	this->size = trimSize;
}

//------------------------------------------------------------------------------
/**
*/
void
ByteBuffer::Set(const void* fromPtr, size_t fromSize)
{
	this->Copy(fromPtr, fromSize);
}

//------------------------------------------------------------------------------
/**
*/
size_t
ByteBuffer::GetDataSize() const
{
	mxASSERT(this->IsValid());
	return this->size;
}

//------------------------------------------------------------------------------
/**
*/
UINT
ByteBuffer::HashCode() const
{
	UINT hash = 0;
	const char* charPtr = (const char*) this->ptr;
	UINT i;
	for (i = 0; i < this->size; i++)
	{
		hash += charPtr[i];
		hash += hash << 10;
		hash ^= hash >>  6;
	}
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;
	hash &= ~(1<<31);       // don't return a negative number (in case UINT is defined signed)
	return hash;
}

/**
    Like strcmp(), but checks the blob contents.
*/
int ByteBuffer::BinaryCompare(const ByteBuffer& rhs) const
{
    mxASSERT(0 != this->ptr);
    mxASSERT(0 != rhs.ptr);
    if (this->size == rhs.size)
    {
        return memcmp(this->ptr, rhs.ptr, this->size);
    }
    else if (this->size > rhs.size)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

void ByteBuffer::SetSize( size_t size )
{
	this->Reallocate( size );
}

AStreamWriter& operator << ( AStreamWriter& file, const ByteBuffer & o )
{
	const UINT32 len = o.GetDataSize();
	file << len;

	if( len > 0 )
	{
		file.Write( o.ToPtr(), len );
	}

	return file;
}

AStreamReader& operator >> ( AStreamReader& file, ByteBuffer & o )
{
	UINT32 len;
	file >> len;

	o.SetSize( len );
	if( len > 0 )
	{
		file.Read( o.ToPtr(), len );
	}

	return file;
}

mxArchive& operator && ( mxArchive& archive, ByteBuffer & o )
{
	return Serialize_ArcViaStreams( archive, o );
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
