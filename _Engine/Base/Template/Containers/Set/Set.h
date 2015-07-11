/*
=============================================================================
	File:	Set.h
	Desc:	Simple array-based set
=============================================================================
*/
#pragma once

template< typename TYPE >
class TSet
{
	TArray< TYPE >	m_elems;

public:
	enum { MAX_CAPACITY = MAX_UINT16-1 };

	explicit TSet()
	{
	}

	UINT Num() const
	{
		return m_elems.Num();
	}

	TYPE & Add( const TYPE& newOne )
	{
		return m_elems.Add( newOne );
	}

	void Insert( const TYPE& item )
	{
		m_elems.AddUnique( item );
	}

	bool Contains( const TYPE& newOne )
	{
		return m_elems.Contains( newOne );
	}
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
