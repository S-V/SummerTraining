/*
=============================================================================
=============================================================================
*/
#pragma once

#if MX_AUTOLINK
#pragma comment( lib, "TxTSupport.lib" )
#endif //MX_AUTOLINK

#include <Base/Memory/FreeList/FreeList.h>

// preserve the original values' order as they appear in the source text?
// (slightly slower during parsing, but leads to faster query operations)
#define txtPARSER_PRESERVE_ORDER	(1)

#define txtPARSER_ALLOW_C_STYLE_COMMENTS	(1)

namespace SON
{
	struct Node;

	typedef UINT64 ValueT;

	// EValueType
	enum ETypeTag
	{
		TypeTag_Nil,		// None, empty value: 'nil'

		TypeTag_List,		// linked list of values: ()
		TypeTag_Array,		// contiguous array/vector of values: []
		TypeTag_Object,		// unordered set/hash/map/table/dictionary/associative array: {}

		TypeTag_String,		// null-terminated string: ""
		TypeTag_Number,		// double-precision floating-point number
		//TypeTag_Boolean,	// 'true' or 'false'
		//TypeTag_HashKey,	// hashed identifier: #name
		//TypeTag_DateTime,	// Coordinated Universal Time (UTC)

		TypeTag_MAX = 7	//<= we use 3 bits for storing a type tag
	};

	static const ETypeTag FirstLeafType = TypeTag_String;
	static const ETypeTag LastLeafType = TypeTag_Number;

	const char* ETypeTag_To_Chars( ETypeTag _tag );

	struct ListValue {
		Node *	kids;	// linked list of children, 0 if empty
		UINT32	size;	// number of children / array elements
	};
	struct ArrayValue {
		Node *	kids;	// linked list of children, 0 if empty
		UINT32	size;	// number of children / array elements
	};
	struct ObjectValue {
		Node *	kids;	// linked list of children, 0 if empty
		UINT32	size;	// number of children / array elements
	};
	struct StringValue {
	#if 0
		UINT32	offset;
		UINT32	length;
	#else
		const char*	start;
		UINT32		length;
	#endif
	};
	union NumberValue {
		double	f;
		UINT64	i;
	};
	struct HashKeyValue {
		UINT32	hash;
		UINT32	offset;
	};
	struct DateTimeValue {
		BITFIELD	year   : 32;// year starting from 0
		BITFIELD	month  : 4;	// [0-11]
		BITFIELD	day    : 5;	// [0-30] (day of month)
		BITFIELD	hour   : 5;	// [0-23] (hours since midnight)
		BITFIELD	minute : 6;	// minutes after the hour - [0,59]
		BITFIELD	second : 6;	// seconds after the minute - [0,59]
	};

	union ValueU
	{
		ListValue		l;
		ArrayValue		a;
		ObjectValue		o;

		StringValue		s;
		NumberValue		n;
		HashKeyValue	h;
		DateTimeValue	d;

		UINT64			u;
	};//8
	union NodeFlagsU
	{
		struct {
			BITFIELD	hash : 28;	// hash key for searching faster than by name string
			ETypeTag	type : 4;	// the type of this node
		};
		UINT32			bits;
	};//4

	struct Node
	{
		ValueU		value;	// the value stored in this node
		Node *		next;	// index of the next sibling in the nodes array/list
		const char*	name;	// the string key of this value (iff. the parent is object)
		NodeFlagsU	tag;	// type and hash of this node
	};//20/28

	typedef UINT32 NodeIndexT;

	struct Allocator
	{
		FreeListAllocator	nodes;	// node allocator

	public:
		Allocator();
		~Allocator();

		Node* AllocateNode();
		Node* AllocateArray( UINT32 _count );
		void ReleaseNode( Node* _node );
	};

	const char* AsString( const Node* _value );
	const double AsDouble( const Node* _value );
	const bool AsBoolean( const Node* _value );

	const Node* FindValue( const Node* _object, const char* _key );

	const UINT32 GetArraySize( const Node* _array );
	const Node* GetArrayItem( const Node* _array, NodeIndexT _index );

	const Node* FindValueByHash( const UINT32 _hash );
	const Node* FindValueByHash( const UINT32 _hash );

	void MakeNumber( Node *_node, double _value );
	void MakeString( Node *_node, const char* _value );
	void MakeBoolean( Node *_node, bool _value );
	void MakeObject( Node *_node );
	void MakeArray( Node *_node, UINT32 _size );
	void MakeList( Node *_node );

	Node* NewNumber( double _value, Allocator & _allocator );
	Node* NewString( const char* _value, UINT32 _length, Allocator & _allocator );
	Node* NewBoolean( bool _value, Allocator & _allocator );
	Node* NewObject( Allocator & _allocator );
	Node* NewArray( UINT32 _size, Allocator & _allocator );
	Node* NewList( Allocator & _allocator );

	// O(1)
	//NOTE: children are added in reverse order (i.e. prepended to parent)
	void AddChild( Node* _object, const char* name, Node* _child );
	void AddChild( Node* _list, Node* _child );

	// O(n)
	void AppendChild( Node* _list, Node* _child );

	Node* NewString( const char* _value, Allocator & _allocator );

	enum EResultCode
	{
		Ret_OK = 0,
		Ret_BAD_NUMBER,
		Ret_BAD_STRING,
		Ret_BAD_IDENTIFIER,
		Ret_STACK_OVERFLOW,
		Ret_STACK_UNDERFLOW,
		Ret_MISMATCH_BRACKET,
		Ret_UNEXPECTED_CHARACTER,
		Ret_UNQUOTED_KEY,
		Ret_BREAKING_BAD
	};

	namespace Debug
	{
		void PrintTree( const Node& _root );

	}//namespace Debug

}//namespace SON

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
