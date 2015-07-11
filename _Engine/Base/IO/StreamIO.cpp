/*
=============================================================================
	File:	StreamIO.cpp
	Desc:	
=============================================================================
*/

#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#define DBGSTREAMS	MX_DEBUG



/*================================
	AStreamReader
================================*/

AStreamReader::~AStreamReader()
{
#if DBGSTREAMS
	mxGetLog().PrintF(LL_Trace,"~ Stream Reader: '%s'",this->DbgGetName());
#endif // MX_DEBUG
}

/*================================
	AStreamWriter
================================*/

AStreamWriter::~AStreamWriter()
{
#if DBGSTREAMS
	mxGetLog().PrintF(LL_Trace,"~ Stream Writer: '%s'",this->DbgGetName());
#endif // MX_DEBUG
}

/*================================
	mxStreamWriter_CountBytes
================================*/

mxStreamWriter_CountBytes::mxStreamWriter_CountBytes()
{
	this->DbgSetName( "mxStreamWriter_CountBytes" );
	m_bytesWritten = 0;
}

mxStreamWriter_CountBytes::~mxStreamWriter_CountBytes()
{

}

ERet mxStreamWriter_CountBytes::Write( const void* pBuffer, size_t numBytes )
{
	m_bytesWritten += numBytes;
	return ALL_OK;
}

/*================================
	mxStreamReader_CountBytes
================================*/

mxStreamReader_CountBytes::mxStreamReader_CountBytes( AStreamReader & stream )
	: m_reader( stream )
{
	this->DbgSetName( "mxStreamReader_CountBytes" );
	m_bytesRead = 0;
}

mxStreamReader_CountBytes::~mxStreamReader_CountBytes()
{

}
size_t mxStreamReader_CountBytes::Tell() const
{
	return m_bytesRead;
}
ERet mxStreamReader_CountBytes::Read( void *pBuffer, size_t numBytes )
{
	const size_t bytesRead = m_reader.Read( pBuffer, numBytes );
	m_bytesRead += bytesRead;
	return ALL_OK;
}

//RedirectingStream::RedirectingStream( AStreamReader& source, AStreamWriter &destination )
//	: m_source( source ), m_destination( destination )
//{}
//
//size_t RedirectingStream::Read( void *pBuffer, size_t numBytes )
//{
//	return m_destination.Write( pBuffer, numBytes );
//}
//
//size_t RedirectingStream::GetSize() const
//{
//	return m_source.GetSize();
//}


/*================================
		MemoryReader
================================*/

MemoryReader::MemoryReader( const void* data, size_t dataSize )
	: mData( c_cast(const char*)data ), mDataSize( dataSize ), mReadOffset( 0 )
{
	this->DbgSetName( "MemoryReader" );
	mxASSERT_PTR(data);
	mxASSERT(dataSize > 0);
}

MemoryReader::MemoryReader( const ByteBuffer& memBlob )
	: mData(memBlob.ToPtr()), mDataSize(memBlob.GetDataSize()), mReadOffset( 0 )
{

}

MemoryReader::~MemoryReader()
{
	//const UINT bytesRead = c_cast(UINT) mReadOffset;
	//DBGOUT("~MemoryReader: %u bytes read.\n",bytesRead);
}
size_t MemoryReader::Tell() const
{
	return mReadOffset;
}
size_t MemoryReader::GetSize() const
{
	return mDataSize;
}

ERet MemoryReader::Read( void *pDest, size_t numBytes )
{
	mxASSERT(mReadOffset < mDataSize + numBytes);
	size_t bytesToRead = Min<size_t>( mDataSize - mReadOffset, numBytes );
	memcpy( pDest, mData + mReadOffset, bytesToRead );
	mReadOffset += bytesToRead;
	return ALL_OK;
}

/*================================
		InPlaceMemoryWriter
================================*/

InPlaceMemoryWriter::InPlaceMemoryWriter( void *dstBuf, size_t bufSize )
	: mBuffer( c_cast(BYTE*)dstBuf ), mCurrPos( mBuffer ), mMaxSize( bufSize )
{
	this->DbgSetName( "InPlaceMemoryWriter" );
	mxASSERT_PTR(dstBuf);
	mxASSERT(bufSize > 0);
}

InPlaceMemoryWriter::~InPlaceMemoryWriter()
{
	//const UINT bytesWritten = c_cast(UINT) this->BytesWritten();
	//DBGOUT("~InPlaceMemoryWriter: %u bytes written.\n",bytesWritten);
}

ERet InPlaceMemoryWriter::Write( const void *pSrc, size_t numBytes )
{
	const size_t writtenSoFar = this->BytesWritten();
	mxASSERT(writtenSoFar + numBytes < mMaxSize );
	size_t bytesToWrite = Min<size_t>( mMaxSize - writtenSoFar, numBytes );
	memcpy( mCurrPos, pSrc, bytesToWrite );
	mCurrPos += bytesToWrite;
	return ALL_OK;
}

ERet ByteWriter::Write( const void* data, size_t size )
{
	const UINT32 oldNum = m_data.Num();
	const UINT32 newNum = oldNum + size;
	mxDO(m_data.SetNum( newNum ));
	void* destination = mxAddByteOffset( m_data.ToPtr(), oldNum );
	memcpy( destination, data, size );
	return ALL_OK;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
