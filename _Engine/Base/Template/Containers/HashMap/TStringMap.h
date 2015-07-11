/*
=============================================================================
	File:	TStringMap.h
	Desc:	A templated hash table with string keys.
	ToDo:	Stop reinventing the wheel.
=============================================================================
*/

#ifndef __MX_STRING_HASH_H__
#define __MX_STRING_HASH_H__

#include <Base/Template/Containers/HashMap/THashMap.h>

//
//	TStringMap
//
template<
	typename VALUE,
	class HASH_FUNC = THashTrait< String >,
	class EQUALS_FUNC = TEqualsTrait< String >
>
class TStringMap
	: public THashMap
	<
		String,
		VALUE,
		HASH_FUNC,
		EQUALS_FUNC
	>
{
public:
	explicit TStringMap( ENoInit )
		: THashMap( _NoInit )
	{}

	explicit TStringMap( UINT tableSize = DEFAULT_HASH_TABLE_SIZE )
		: THashMap( tableSize )
	{}
};

#endif // ! __MX_STRING_HASH_H__

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
