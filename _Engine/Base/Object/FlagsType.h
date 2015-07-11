#pragma once

#include <Base/Object/TypeDescriptor.h>
#include <Base/Object/Reflection.h>

/*
-----------------------------------------------------------------------------
	mxFlagsType

	TBits< ENUM, STORAGE > - 8,16,32 bits of named values.
-----------------------------------------------------------------------------
*/
struct mxFlagsType : public mxType
{
	typedef UINT32 Mask;

	struct Member
	{
		const char*	name;	// name of the value in the code
		Mask		mask;
	};
	struct MemberList
	{
		const Member *	array;
		const UINT		count;
	};

	const Member*	m_members;
	const UINT32	m_numFlags;

public:
	inline mxFlagsType( const Chars& typeName, const MemberList& members, const STypeDescription& info )
		: mxType( ETypeKind::Type_Flags, typeName, info )
		, m_members( members.array )
		, m_numFlags( members.count )
	{}

	virtual Mask Get_Value( const void* flags ) const = 0;
	virtual void Set_Value( void *flags, Mask value ) const = 0;

	int		GetItemIndexByName( const char* name ) const;
	Mask	GetItemValueByName( const char* name ) const;
};


//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//!=- ENUM - name of enumeration
//!=- STORAGE - type of storage
//!=- ALIAS - typedef of TBits<ENUM,STORAGE>
//
#define mxDECLARE_FLAGS( ENUM, STORAGE, ALIAS )\
	typedef TBits< ENUM, STORAGE > ALIAS;\
	mxDECLARE_POD_TYPE( ALIAS );\
	\
	extern const mxFlagsType::MemberList& PP_JOIN_TOKEN( GetMembersOf_, ALIAS )();\
	\
	struct PP_JOIN_TOKEN(MetaType_,ALIAS) : public mxFlagsType\
	{\
		mxSTATIC_ASSERT( sizeof(STORAGE) <= sizeof(Mask) );\
		PP_JOIN_TOKEN(MetaType_,ALIAS)()\
			: mxFlagsType( mxEXTRACT_TYPE_NAME(ALIAS), PP_JOIN_TOKEN(GetMembersOf_,ALIAS)(), STypeDescription::For_Type< ALIAS >() )\
		{}\
		virtual Mask Get_Value( const void* flags ) const override\
		{\
			return *static_cast< const STORAGE* >( flags );\
		}\
		virtual void Set_Value( void *flags, Mask value ) const override\
		{\
			*((STORAGE*) flags) = value;\
		}\
	};\
	\
	extern const PP_JOIN_TOKEN(MetaType_,ALIAS)& PP_JOIN_TOKEN(GetTypeOf_,ALIAS)();\
	\
	template<>\
	struct TypeDeducer< ALIAS >\
	{\
		static inline const mxType& GetType()\
		{\
			return PP_JOIN_TOKEN(GetTypeOf_,ALIAS)();\
		}\
		static inline ETypeKind GetTypeKind()\
		{\
			return ETypeKind::Type_Flags;\
		}\
	};

//!--------------------------------------------------------------------------



//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//!=- should be placed into source files
//
#define mxBEGIN_FLAGS( ALIAS )\
	const PP_JOIN_TOKEN(MetaType_,ALIAS)& PP_JOIN_TOKEN( GetTypeOf_, ALIAS )()\
	{\
		static const PP_JOIN_TOKEN(MetaType_,ALIAS) staticInstance;\
		return staticInstance;\
	}\
	const mxFlagsType::MemberList& PP_JOIN_TOKEN( GetMembersOf_, ALIAS )() {\
		static mxFlagsType::Member items[] = {\


//!=- MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
#define mxREFLECT_BIT( NAME, VALUE )\
	{\
		mxEXTRACT_NAME(NAME).buffer,\
		VALUE\
	}


//!=- MACRO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
#define mxEND_FLAGS\
		};\
		static mxFlagsType::MemberList reflectionMetadata = { items, mxCOUNT_OF(items) };\
		return reflectionMetadata;\
	}


//!--------------------------------------------------------------------------
void Dbg_FlagsToString( const void* _o, const mxFlagsType& _type, String &_string );

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
