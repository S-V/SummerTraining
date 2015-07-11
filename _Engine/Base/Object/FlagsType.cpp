#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#include <Base/Object/FlagsType.h>

int mxFlagsType::GetItemIndexByName( const char* name ) const
{
	for( int i = 0; i < m_numFlags; i++ )
	{
		const Member& item = m_members[ i ];
		if( !strcmp( item.name, name ) ) {
			return i;
		}
	}
	ptERROR("Unknown flag: %s", name);
	mxDBG_UNREACHABLE;
	return -1;
}

mxFlagsType::Mask mxFlagsType::GetItemValueByName( const char* name ) const
{
	const UINT itemIndex = this->GetItemIndexByName( name );
	chkRET_NIL_IF_NOT(itemIndex != INDEX_NONE);
	return m_members[ itemIndex ].mask;
}

void Dbg_FlagsToString( const void* _o, const mxFlagsType& _type, String &_string )
{
	_string.Empty();
	const UINT32 mask = _type.Get_Value( _o );
	for( int i = 0; i < _type.m_numFlags; i++ )
	{
		const mxFlagsType::Member& member = _type.m_members[i];
		if( member.mask & mask )
		{
			if( !_string.IsEmpty() )
			{
				Str::Append( _string, '|' );
			}
			Str::AppendS( _string, member.name );
		}		
	}
}
