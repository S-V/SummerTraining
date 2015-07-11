/*
=============================================================================
	File:	TStatic2DArray.h
	Desc:	A fixed-size 2-dimensional array.
	ToDo:	Stop reinventing the wheel.
=============================================================================
*/

#ifndef __MX_CONTAINTERS_STATIC_2D_ARRAY_H__
#define __MX_CONTAINTERS_STATIC_2D_ARRAY_H__



//
//	TStatic2DArray
//

template<
	typename TYPE,
	UINT	ROW_COUNT,
	UINT	COLUMN_COUNT
>
class TStatic2DArray {
public:
	mxFORCEINLINE TStatic2DArray()
	{}

	mxFORCEINLINE TStatic2DArray(EInitZero)
	{
		this->SetZero();
	}

	mxFORCEINLINE ~TStatic2DArray()
	{}

	mxFORCEINLINE UINT GetRowCount()
	{
		return ROW_COUNT;
	}
	mxFORCEINLINE UINT GetColumnCount()
	{
		return COLUMN_COUNT;
	}

	mxFORCEINLINE TYPE & Get( UINT row, UINT column )
	{	mxASSERT( row < GetRowCount() && column < GetColumnCount() );
		return m_data[ row ][ column ];
	}

	mxFORCEINLINE const TYPE & Get( UINT row, UINT column ) const
	{	mxASSERT( row < GetRowCount() && column < GetColumnCount() );
		return m_data[ row ][ column ];
	}

	mxFORCEINLINE void Set( UINT row, UINT column, const TYPE& newValue )
	{	mxASSERT( row < GetRowCount() && column < GetColumnCount() );
		m_data[ row ][ column ] = newValue;
	}

	mxFORCEINLINE void SetZero()
	{
		mxZERO_OUT( m_data );
	}


	mxFORCEINLINE TYPE * ToPtr()
	{ return m_data; }

	mxFORCEINLINE const TYPE * ToPtr() const
	{ return m_data; }

private:
	TYPE	m_data[ ROW_COUNT ][ COLUMN_COUNT ];
};



#endif // ! __MX_CONTAINTERS_STATIC_2D_ARRAY_H__

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
