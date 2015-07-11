/*
=============================================================================
	File:	NameTable.cpp
	Desc:	global string table,
			shared reference-counted strings for saving memory
	Note:	NOT thread save!
=============================================================================
*/

#include <Core/Core_PCH.h>
#pragma hdrstop

#include "NameTable.h"

size_t  NameID::strnum = 0;
size_t  NameID::strmemory = 0;
NameID::StringPointer  NameID::empty = { 0, 1, 0, 0, 0, 0, 0, 0 };
NameID::StringPointer *NameID::table[HASH_SIZE] = {0};
NameID::ListNode *NameID::pointers[CACHE_SIZE] = {0};
NameID::ListNode *NameID::allocs = 0;
static bool gs_NameTableIsInitialized = false;

void NameID::StaticInitialize() throw()
{
	mxASSERT(!gs_NameTableIsInitialized);
	NameID::empty.next = NULL;
	NameID::empty.refcounter = 1;
	NameID::empty.length = 0;
	NameID::empty.hash = 0;
	mxZERO_OUT(NameID::empty.body);
	gs_NameTableIsInitialized = true;
}

void NameID::StaticShutdown() throw()
{
	mxASSERT(gs_NameTableIsInitialized);
	gs_NameTableIsInitialized = false;
}

void* NameID::Alloc( size_t bytes )
{
	return mxAlloc(bytes);
}

void NameID::Free( void* ptr )
{
	mxFree(ptr);
}

NameID::StringPointer* NameID::alloc_string( UINT32 length )
{
	UINT32 alloc = sizeof( StringPointer ) + length - 3; 
	UINT32 p = ( alloc - 1 ) / ALLOC_GRAN + 1;

	if( p >= CACHE_SIZE )
	{
		return (StringPointer *)Alloc( alloc );
	}

	alloc = p * ALLOC_GRAN;

	if( pointers[p] == 0 )
	{
		char *data = (char*) Alloc( ALLOC_SIZE );

		strmemory += ALLOC_SIZE;

		ListNode *chunk = (ListNode *)( data );
		chunk->next = allocs;
		allocs = chunk;

		for( size_t i = sizeof( ListNode ); i + alloc <= ALLOC_SIZE; i += alloc )
		{
			ListNode *result = (ListNode *)( data + i );
			result->next = pointers[p]; 
			pointers[p] = result;
		}
	}

	ListNode *result = pointers[p];
	pointers[p] = result->next;
	return (StringPointer *)result;
}

void NameID::release_string( StringPointer *ptr )
{
	UINT32 alloc = sizeof( StringPointer ) + ptr->length - 3; 
	UINT32 p = ( alloc - 1 ) / ALLOC_GRAN + 1;

	if( p >= CACHE_SIZE )
	{   
		Free( ptr );
		return;
	}

	ListNode *result = (ListNode *)ptr;
	result->next = pointers[p];
	pointers[p] = result;
}

void NameID::clear()
{
	UINT32 position = pointer->hash % HASH_SIZE;
	StringPointer *entry = table[position];
	StringPointer **prev = &table[position];
	while( entry )
	{
		if( entry == pointer )
		{
			*prev = pointer->next;
			release_string( pointer );

			if( --strnum == 0 )
			{
				while( allocs )
				{
					ListNode *next = allocs->next;
					Free( allocs );
					allocs = next;
					strmemory -= ALLOC_SIZE;
				}

				for( size_t i = 0; i < CACHE_SIZE; ++i )
				{
					pointers[i] = 0;
				}
			}

			return;
		}
		prev = &entry->next;
		entry = entry->next;
	}
}

AStreamWriter& operator << ( AStreamWriter& file, const NameID& o )
{
	const UINT16 len = o.size();
	file << len;
	if( len > 0 ) {
		file.Write( o.ToPtr(), len );
	}
	return file;
}

AStreamReader& operator >> ( AStreamReader& file, NameID& o )
{
	UINT16 len;
	file >> len;

	if( len > 0 )
	{
		char buffer[ NameID::MAX_LENGTH ];
		mxASSERT(len < mxCOUNT_OF(buffer));
		len = smallest(len, mxCOUNT_OF(buffer)-1);

		file.Read( buffer, len );

		buffer[ len ] = 0;

		o = NameID( buffer );
	}
	else
	{
		o = NameID();
	}

	return file;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
