/*
=============================================================================
=============================================================================
*/
#include <Base/Base.h>
#include <TxTSupport/TxTCommon.h>
#include <TxTSupport/TxTPrivate.h>
mxTODO("remove strlen() calls");
namespace SON
{

const char* ETypeTag_To_Chars( ETypeTag _tag )
{
	switch( _tag )
	{
	case TypeTag_Nil :	return "Nil";

	case TypeTag_List :	return "List";
	case TypeTag_Array :	return "Array";
	case TypeTag_Object :	return "Object";

	case TypeTag_String :	return "String";
	case TypeTag_Number :	return "Number";
	//case TypeTag_Boolean :	return "Boolean";
	//case TypeTag_HashKey :	return "HashKey";
	//case TypeTag_DateTime :	return "DateTime";
	mxNO_SWITCH_DEFAULT;
	}
	return "Unknown";
}

const char* AsString( const Node* _value )
{
	if(!_value) { return ""; }
	mxASSERT(_value->tag.type == TypeTag_String);
	return _value->value.s.start;
}

const double AsDouble( const Node* _value )
{
	if(!_value) { return 0.0f; }
	mxASSERT(_value->tag.type == TypeTag_Number);
	return _value->value.n.f;
}

const bool AsBoolean( const Node* _value )
{
	if(!_value) { return false; }
	return _value->value.u != 0;
}

const Node* FindValue( const Node* _object, const char* _key )
{
	if(!_object) { return nil; }
	mxASSERT(_object->tag.type == TypeTag_Object);
	const Node* child = _object->value.l.kids;
	while( child ) {
		if( !strcmp(child->name, _key) ) {
			return child;
		}
		child = child->next;
	}
	return nil;
}

const UINT32 GetArraySize( const Node* _array )
{
	if(!_array) { return 0; }
	mxASSERT(_array->tag.type == TypeTag_Array);
	return _array->value.a.size;
}

const Node* GetArrayItem( const Node* _array, NodeIndexT _index )
{
	if(!_array) { return nil; }
	mxASSERT(_array->tag.type == TypeTag_Array);
	mxASSERT(_index < _array->value.a.size);
	return _array->value.a.kids + _index;
}

void MakeNumber( Node *_node, double _value )
{
	_node->value.n.f = _value;
	_node->next = nil;
	_node->name = nil;
	_node->tag.type = TypeTag_Number;
}

void MakeString( Node *_node, const char* _value )
{
	_node->value.s.start = _value;
	_node->value.s.length = strlen(_value);
	_node->next = nil;
	_node->name = nil;
	_node->tag.type = TypeTag_String;
}

void MakeBoolean( Node *_node, bool _value )
{
	_node->value.n.f = _value ? 1.0 : 0.0;
	_node->next = nil;
	_node->name = nil;
	_node->tag.type = TypeTag_Number;
}

void MakeObject( Node *_node )
{
	_node->value.o.kids = nil;
	_node->value.o.size = 0;
	_node->next = nil;
	_node->name = nil;
	_node->tag.type = TypeTag_Object;
}

void MakeArray( Node *_node )
{
	_node->value.a.kids = nil;
	_node->value.a.size = 0;
	_node->next = nil;
	_node->name = nil;
	_node->tag.type = TypeTag_Array;
}

void MakeList( Node *_node )
{
	_node->value.l.kids = nil;
	_node->value.l.size = 0;
	_node->next = nil;
	_node->name = nil;
	_node->tag.type = TypeTag_List;
}

Node* NewNumber( double _value, Allocator & _allocator )
{
	Node* newNode = _allocator.AllocateNode();
	newNode->value.n.f = _value;
	newNode->next = nil;
	newNode->name = nil;
	newNode->tag.type = TypeTag_Number;
	return newNode;
}

Node* NewString( const char* _value, UINT32 _length, Allocator & _allocator )
{
	Node* newNode = _allocator.AllocateNode();
	newNode->value.s.start = _value;
	newNode->value.s.length = _length;
	newNode->next = nil;
	newNode->name = nil;
	newNode->tag.type = TypeTag_String;
	return newNode;
}
Node* NewString( const char* _value, Allocator & _allocator )
{
	Node* newNode = _allocator.AllocateNode();
	newNode->value.s.start = _value;
	newNode->value.s.length = strlen(_value);
	newNode->next = nil;
	newNode->name = nil;
	newNode->tag.type = TypeTag_String;
	return newNode;
}

Node* NewBoolean( bool _value, Allocator & _allocator )
{
	Node* newNode = _allocator.AllocateNode();
	MakeBoolean( newNode, _value );
	return newNode;
}

Node* NewObject( Allocator & _allocator )
{
	Node* objectNode = _allocator.AllocateNode();
	objectNode->value.u = 0;
	objectNode->next = nil;
	objectNode->name = nil;
	objectNode->tag.type = TypeTag_Object;
	return objectNode;
}

Node* NewArray( UINT32 _size, Allocator & _allocator )
{
	Node* arrayNode = _allocator.AllocateNode();
	arrayNode->value.a.kids = nil;
	arrayNode->value.a.size = 0;
	arrayNode->next = nil;
	arrayNode->name = nil;
	arrayNode->tag.type = TypeTag_Array;
	if( _size ) {
		Node* arrayItems = _allocator.AllocateArray( _size );
		arrayNode->value.a.kids = arrayItems;
	}
	return arrayNode;
}

Node* NewList( Allocator & _allocator )
{
	Node* listNode = _allocator.AllocateNode();
	listNode->value.a.kids = nil;
	listNode->value.a.size = 0;
	listNode->next = nil;
	listNode->name = nil;
	listNode->tag.type = TypeTag_List;
	return listNode;
}

void AddChild( Node* _object, const char* _name, Node* _child )
{
	mxASSERT(_object->tag.type == TypeTag_Object);
	mxASSERT(_child->name == nil);
	PrependChild( _object, _child );
	_child->name = _name;
}

void AddChild( Node* _list, Node* _child )
{
	mxASSERT(_list->tag.type == TypeTag_List);
	mxASSERT(_child->name == nil);
	PrependChild( _list, _child );
}

void AppendChild( Node* _list, Node* _child )
{
	mxASSERT(_list->tag.type == TypeTag_List);
	mxASSERT(_child->name == nil);
	mxASSERT(_child->next == nil);

	Node** tail = &_list->value.l.kids;
	while( *tail ) {
		tail = &((*tail)->next);
	}
	*tail = _child;

	_list->value.l.size++;
}

#if 0
	// IEEE 754 double-precision binary floating-point format:
	// 1 sign bit, 11 bits of exponent, 52 bits - significand.
	// http://steve.hollasch.net/cgindex/coding/ieeefloat.html
	// The encoding makes use of unused NaN space in the IEEE754 representation.
	// NaN-boxing allows all values to be stored as a 64-bit float
	// by exploiting redundancy in NaN encodings:
	// http://www.contrib.andrew.cmu.edu/~acrichto/joule.pdf
	// See:
	// http://wingolog.org/archives/2011/05/18/value-representation-in-javascript-implementations
	// http://blog.mozilla.com/rob-sayre/2010/08/02/mozillas-new-javascript-value-representation/
	// https://github.com/vivkin/gason

	//static const UINT64 VALUE_PAYLOAD_MASK	= 0x00007FFFFFFFFFFFULL;
	//static const UINT64 VALUE_NAN_MASK		= 0x7FF8000000000000ULL;
	//static const UINT64 VALUE_NULL			= 0x7FFF800000000000ULL;
	static const UINT64 VALUE_NAN_MASK		= 0xFFF0000000000000ULL;
	static const UINT64 VALUE_TAG_MASK		= 0xFULL;	// 4 bits are used for storing a type tag
	static const UINT64 VALUE_TAG_SHIFT		= 47ULL;// 48 bits are enough to store any pointer on 64-bit systems

	static inline UINT64 EncodeValue( ETypeTag _tag, UINT64 _payload )
	{
		// set the top 12 bits to 1 to create a NaN (to distinguish the value from normal doubles)
		return (~0ULL << 51ULL) | (UINT64(_tag) << 47ULL) | _payload;
	}
	static inline bool IsValidDouble( UINT64 _value )
	{
		// test the top 12 bits to check if it's a valid floating-point number (i.e. not a NaN)
		return (_value & VALUE_NAN_MASK) != VALUE_NAN_MASK;
	}
	static inline ETypeTag GetTypeTag( UINT64 _value )
	{
		// test the top 12 bits to check if it's a valid floating-point number (i.e. not a NaN)
		return IsValidDouble(_value) ? TypeTag_Double : ETypeTag((_value >> 47ULL) & 0xF);
	}
#endif

Allocator::Allocator()
{
	this->nodes.Initialize( sizeof(Node), 512 );
}

Allocator::~Allocator()
{
	this->nodes.ReleaseMemory();
}

Node* Allocator::AllocateNode()
{
	Node * newNode = (Node*) this->nodes.AllocateItem();
//	memset(newNode, 0, sizeof(Node));
	return newNode;
}

Node* Allocator::AllocateArray( UINT32 _count )
{
	Node * newNodes = (Node*) this->nodes.AllocateNewBlock( _count );
//	memset(newNodes, 0, sizeof(Node)*_count);
	return newNodes;
}

void Allocator::ReleaseNode( Node* _node )
{
	this->nodes.ReleaseItem( _node );
}

}//namespace SON

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
