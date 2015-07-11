/*
=============================================================================
	File:	NameTable.h
	Desc:	global string table,
			shared reference-counted strings for saving memory
	Note:	NOT thread save!
=============================================================================
*/
#pragma once

mxSWIPED("static_string by IronPeter");

/*
-----------------------------------------------------------------------------
	NameID
	is a shared immutable reference-counted string
	(for saving memory and fast comparisons)
-----------------------------------------------------------------------------
*/
class NameID {
public:
	inline const char *c_str() const throw()
	{
		return &pointer->body[0];
	}

	inline bool operator == ( const NameID& other ) const throw()
	{
		return pointer == other.pointer;
	}
	inline bool operator != ( const NameID& other ) const throw()
	{
		return pointer != other.pointer;
	}
	// for std::map / std::multimap
	inline bool operator < ( const NameID& other ) const {
		return pointer < other.pointer;
	}

	inline NameID &operator = ( const NameID& other ) throw()
	{
		if( *this != other )
		{
			NameID swp( other );
			StringPointer *temp = swp.pointer;
			swp.pointer = pointer;
			pointer = temp;
		}
		return *this;
	}

	inline NameID() throw()
	{
		pointer = &empty;
		pointer->refcounter++;
	}

	inline NameID( const NameID& other  ) throw()
	{
		pointer = other.pointer;
		pointer->refcounter++;
	}

	inline NameID( const char *buff ) throw()
	{
		UINT32 hash, length;

		if( buff == 0 || buff[0] == 0 )
		{
			pointer = &empty;
			pointer->refcounter++;
			return;
		}

		GetStringHash( buff, hash, length );

		UINT32 position = hash % HASH_SIZE;
		StringPointer *entry = table[position];
		while( entry )
		{
			if( entry->length == length && entry->hash == hash && !memcmp( entry->body, buff, length ) )
			{
				pointer = entry;
				++entry->refcounter;
				return;
			}
			entry = entry->next;
		}

		++strnum;

		pointer = alloc_string( length );
		char *buffer = &pointer->body[0];
		memcpy( buffer, buff, length + 1 );
		pointer->hash = hash;
		pointer->next = table[position];
		pointer->refcounter = 1;
		pointer->length = length;
		table[position] = pointer;
	}

	inline ~NameID() throw()
	{
		if( --pointer->refcounter == 0 )
		{
			clear();
		}
	}

	inline UINT32 size() const throw()
	{
		return pointer->length;
	}

	inline UINT32 hash() const throw()
	{
		return pointer->hash;
	}

	//============================================================
	inline const char* ToPtr() const throw()
	{
		return c_str();
	}
	inline UINT Length() const throw()
	{
		return size();
	}
	inline bool IsEmpty() const throw()
	{
		return size() == 0;
	}

	inline UINT NumRefs() const
	{
		return this->pointer->refcounter;
	}

	friend AStreamWriter& operator << ( AStreamWriter& file, const NameID& o );
	friend AStreamReader& operator >> ( AStreamReader& file, NameID& o );

	template< class S >
	friend S& operator & ( S & serializer, NameID & o )
	{
		return serializer.SerializeViaStream( o );
	}

public:

	static size_t get_str_num() throw()
	{
		return strnum;
	}

	static size_t get_str_memory() throw()
	{
		return strmemory;
	}

	static void StaticInitialize() throw();
	static void StaticShutdown() throw();


	enum
	{
		HASH_SIZE	= 65536,
		CACHE_SIZE	= 32,
		ALLOC_GRAN	= 8,
		ALLOC_SIZE	= 1024,
		MAX_LENGTH	= 512,
	};

	struct StringPointer
	{
		StringPointer *	next;
		UINT32			refcounter;
		UINT32			length;
		UINT32			hash;
		char			body[4];
	};

	struct ListNode
	{
		ListNode *	next;
	};

protected:
	StringPointer *	pointer;

	static StringPointer    empty;
	static size_t			strnum;
	static size_t			strmemory;
	static StringPointer    *table[HASH_SIZE];
	static ListNode *pointers[CACHE_SIZE];
	static ListNode *allocs;

	void* Alloc( size_t bytes );
	void Free( void* ptr );

	StringPointer *alloc_string( UINT32 length ) throw();

	void release_string( StringPointer *ptr ) throw();

	inline void GetStringHash( const char *_str, UINT32 &hash, UINT32 &length ) throw()
	{       
		UINT32 res = 0;
		UINT32 i = 0;

		for(;;)
		{
			UINT32 v = _str[i];
			res = res * 5 + v;
			if( v == 0 )
			{
				hash = res;
				length = i;
				return;
			}
			++i;
		};
	}

	void clear() throw();
};

template<>
struct THashTrait< NameID >
{
	static mxFORCEINLINE UINT GetHashCode( const NameID& key )
	{
		return key.hash();
	}
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
