/*
=============================================================================
	File:	TDictionary.h
	Desc:	A very simple table which maps keys to arbitrary values.
	Note:	the dictionary is implemented as a sorted array.
	ToDo:	Stop reinventing the wheel.
=============================================================================
*/

#ifndef __MX_TEMPLATE_DICTIONARY_H__
#define __MX_TEMPLATE_DICTIONARY_H__

#include <Base/Template/Containers/HashMap/TKeyValue.h>



template<
	typename KEY,
	typename VALUE,
>
class TDictionary
{
public:
	typedef TDictionary
	<
		KEY,
		VALUE
	> THIS_TYPE;

	typedef TKeyValue
	<
		KEY,
		VALUE
	> KEY_VALUE;

	explicit TDictionary( HMemory hMemoryMgr )
	{
		mTable = nil;
		mTableMask = 0;
	}
	~TDictionary()
	{
		//
	}

private:	PREVENT_COPY(THIS_TYPE);

	TArray< KEY_VALUE >	mPairs;
};



#endif // ! __MX_TEMPLATE_DICTIONARY_H__

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
