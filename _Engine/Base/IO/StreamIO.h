/*
=============================================================================
	File:	StreamIO.h
	Desc:
=============================================================================
*/
#pragma once

struct NullStreamWriter : public AStreamWriter
{
	virtual ERet Write( const void* buffer, size_t numBytes ) override
	{
		(void)buffer;
		return ALL_OK;
	}
};

//
//	mxStreamWriter_CountBytes
//
class mxStreamWriter_CountBytes : public AStreamWriter
{
	size_t	m_bytesWritten;

public:
	mxStreamWriter_CountBytes();
	~mxStreamWriter_CountBytes();

	virtual ERet Write( const void* buffer, size_t numBytes ) override;

	inline size_t NumBytesWritten() const { return m_bytesWritten; }
};

//
//	mxStreamReader_CountBytes
//
class mxStreamReader_CountBytes : public AStreamReader
{
	AStreamReader &	m_reader;

	size_t	m_bytesRead;

public:
	mxStreamReader_CountBytes( AStreamReader & stream );
	~mxStreamReader_CountBytes();

	virtual size_t Tell() const override;
	virtual ERet Read( void *buffer, size_t numBytes ) override;

	inline size_t NumBytesRead() const { return m_bytesRead; }
};

//class RedirectingStream : public AStreamReader
//{
//	AStreamReader &	m_source;
//	AStreamWriter &	m_destination;
//
//public:
//	RedirectingStream( AStreamReader& source, AStreamWriter &destination );
//
//	virtual ERet Read( void *buffer, size_t numBytes ) override;
//	virtual size_t GetSize() const override;
//};

class MemoryWriter : public AStreamWriter
{
	void *			m_destination;
	const UINT32	m_bufferSize;
	UINT32			m_bytesWritten;

public:
	MemoryWriter( void* destination, UINT32 bufferSize, UINT32 startOffset = 0 )
		: m_destination( destination )
		, m_bufferSize( bufferSize )
	{
		m_bytesWritten = startOffset;
	}
	virtual ERet Write( const void* buffer, size_t numBytes ) override
	{
		chkRET_X_IF_NOT(m_bytesWritten + numBytes <= m_bufferSize, ERR_BUFFER_TOO_SMALL);
		memcpy(m_destination, buffer, numBytes);
		m_destination = mxAddByteOffset(m_destination, numBytes);
		m_bytesWritten += numBytes;
		return ALL_OK;
	}
	inline size_t NumBytesWritten() const { return m_bytesWritten; }
};


//
//	MemoryReader
//
class MemoryReader : public AStreamReader
{
public:
	MemoryReader( const void* data, size_t dataSize );
	MemoryReader( const ByteBuffer& memBlob );
	~MemoryReader();

	virtual size_t Tell() const override;
	virtual size_t GetSize() const override;
	virtual ERet Read( void *pDest, size_t numBytes ) override;

	inline const char* GetPtr() const
	{
		return mData + mReadOffset;
	}
	inline char* GetPtr()
	{
		return const_cast<char*>( mData + mReadOffset );
	}
	inline size_t BytesRead() const
	{
		return mReadOffset;
	}

	inline size_t Size() const
	{
		return mDataSize;
	}

	inline void Seek( size_t absOffset )
	{
		mxASSERT( absOffset < mDataSize );
		mReadOffset = absOffset;
	}

private:
	const char* mData;
	size_t mReadOffset;
	const size_t mDataSize;

private:	PREVENT_COPY( MemoryReader );
};

//
//	InPlaceMemoryWriter
//
class InPlaceMemoryWriter : public AStreamWriter
{
public:
	InPlaceMemoryWriter( void *dstBuf, size_t bufSize );
	~InPlaceMemoryWriter();

	virtual ERet Write( const void *pSrc, size_t numBytes ) override;

	inline const BYTE* GetPtr() const
	{
		return mCurrPos;
	}
	inline size_t BytesWritten() const
	{
		return mCurrPos - mBuffer;
	}

private:
	BYTE	*mBuffer;
	BYTE	*mCurrPos;
	const size_t mMaxSize;

private:	PREVENT_COPY( InPlaceMemoryWriter );
};


//---------------------------------------------------------------------------
//	Helper macros
//---------------------------------------------------------------------------

mxIMPLEMENT_FUNCTION_READ_SINGLE(INT8,ReadInt8);
mxIMPLEMENT_FUNCTION_WRITE_SINGLE(INT8,WriteInt8);

mxIMPLEMENT_FUNCTION_READ_SINGLE(UINT8,ReadUInt8);
mxIMPLEMENT_FUNCTION_WRITE_SINGLE(UINT8,WriteUInt8);


mxIMPLEMENT_FUNCTION_READ_SINGLE(INT16,ReadInt16);
mxIMPLEMENT_FUNCTION_WRITE_SINGLE(INT16,WriteInt16);

mxIMPLEMENT_FUNCTION_READ_SINGLE(UINT16,ReadUInt16);
mxIMPLEMENT_FUNCTION_WRITE_SINGLE(UINT16,WriteUInt16);


mxIMPLEMENT_FUNCTION_READ_SINGLE(INT32,ReadInt32);
mxIMPLEMENT_FUNCTION_WRITE_SINGLE(INT32,WriteInt32);

mxIMPLEMENT_FUNCTION_READ_SINGLE(UINT32,ReadUInt32);
mxIMPLEMENT_FUNCTION_WRITE_SINGLE(UINT32,WriteUInt32);

mxIMPLEMENT_FUNCTION_READ_SINGLE(INT64,ReadInt64);
mxIMPLEMENT_FUNCTION_WRITE_SINGLE(INT64,WriteInt64);

mxIMPLEMENT_FUNCTION_READ_SINGLE(UINT64,ReadUInt64);
mxIMPLEMENT_FUNCTION_WRITE_SINGLE(UINT64,WriteUInt64);


mxIMPLEMENT_FUNCTION_READ_SINGLE(FLOAT32,ReadFloat32);
mxIMPLEMENT_FUNCTION_WRITE_SINGLE(FLOAT32,WriteFloat32);

mxIMPLEMENT_FUNCTION_READ_SINGLE(FLOAT64,ReadFloat64);
mxIMPLEMENT_FUNCTION_WRITE_SINGLE(FLOAT64,WriteFloat64);

//mxIMPLEMENT_FUNCTION_READ_SINGLE(SIMD_QUAD,ReadSimdQuad);
//mxIMPLEMENT_FUNCTION_WRITE_SINGLE(SIMD_QUAD,WriteSimdQuad);

mxFORCEINLINE void Skip_1_byte( AStreamReader& stream )
{
	ReadUInt8( stream );
}
mxFORCEINLINE void Skip_4_bytes( AStreamReader& stream )
{
	ReadUInt32( stream );
}

inline void Skip_N_bytes_Internal( AStreamReader& stream, const UINT32 numBytes )
{
	BYTE	buffer[1024];
	const UINT32 num = numBytes / sizeof buffer;
	const UINT32 rem = numBytes - num * sizeof buffer;

	for( UINT32 i = 0; i < num; i++ )
	{
		stream.Read( buffer, sizeof buffer );
	}

	if( rem > 0 ) {
		stream.Read( buffer, rem );
	}
}

mxFORCEINLINE void Skip_N_bytes( AStreamReader& stream, const UINT32 numBytes )
{
	if( numBytes == 0 ) {
		return;
	}
	Skip_N_bytes_Internal( stream, numBytes );
}

template< UINT32 numBytes >
mxFORCEINLINE void Skip_N_bytes( AStreamReader& stream )
{
	mxSTATIC_ASSERT( numBytes > 0 && numBytes < 1024 );	// don't overflow stack
	char	buffer[ numBytes ];
	stream.Read( buffer, numBytes );
}

inline void Write_N_bytes( AStreamWriter& stream, UINT32 value, UINT32 numBytes )
{
	const UINT32 numInt32s = numBytes / sizeof(value);
	for( UINT32 i = 0; i < numInt32s; i++ )
	{
		stream.Write( &value, sizeof(value) );
	}
	const UINT32 remainingBytes = numBytes % sizeof(value);
	if( remainingBytes > 0 ) {
		stream.Write( &value, remainingBytes );
	}
}


void EncodeString_Xor( const char* src, size_t size, UINT32 key, char *dest );
void DecodeString_Xor( const char* src, size_t size, UINT32 key, char *dest );


void WriteEncodedString( AStreamWriter & stream, const char* src, size_t size, UINT32 seed );
void ReadDecodedString( AStreamReader & stream, char* dest, UINT32 seed );

// (de-)serializes from/to archive via << and >> stream operators
//
template< typename T >
inline mxArchive& Serialize_ArcViaStreams( mxArchive& archive, T & o )
{
	if( AStreamWriter* stream = archive.IsWriter() )
	{
		*stream << o;
	}
	if( AStreamReader* stream = archive.IsReader() )
	{
		*stream >> o;
	}
	return archive;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

template< typename T >
mxArchive& TSerializeArray( mxArchive & archive, T* a, UINT32 num )
{
	// even though pointers are POD types,
	// they cannot be serialized as POD types.
	// i.e.:
	// archive.SerializeMemory( a, num * sizeof a[0] );
	// is wrong.

	for( UINT32 i = 0; i < num; i++ )
	{
		archive && a[i];
	}

	return archive;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

template< class S, class T, const size_t size >
void TSerializeArray( S& s, T (&a)[size] )
{
	mxSTATIC_ASSERT( size > 0 );
	if( TypeTrait< T >::IsPlainOldDataType )
	{
		s.SerializeMemory( a, size * sizeof T );
	}
	else
	{
		for( UINT32 i = 0; i < size; i++ )
		{
			s & a[i];
		}
	}
	return s;
}


//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
