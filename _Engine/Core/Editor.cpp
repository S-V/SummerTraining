/*
=============================================================================
	File:	EditorInterface.cpp
	Desc:	Support for communication with the editor.
=============================================================================
*/

#include <Core/Core_PCH.h>
#pragma hdrstop
#include <Core/Core.h>
#include <Core/Editor.h>
#include <Core/Asset.h>

#if MX_EDITOR

/*
-----------------------------------------------------------------------------
	AEditable
-----------------------------------------------------------------------------
*/
mxDEFINE_ABSTRACT_CLASS(AEditable);

AEditable::AEditable()
{

}

AEditable::~AEditable()
{

}

/*
-----------------------------------------------------------------------------
	AProperty
-----------------------------------------------------------------------------
*/
mxDEFINE_ABSTRACT_CLASS(AProperty);

AProperty::AProperty()
{
}

AProperty::~AProperty()
{
}

mxDEFINE_ABSTRACT_CLASS(BooleanProperty);
mxDEFINE_ABSTRACT_CLASS(IntegerProperty);
mxDEFINE_ABSTRACT_CLASS(FloatProperty);
mxDEFINE_CLASS(EnumProperty);
mxDEFINE_CLASS(FlagsProperty);
mxDEFINE_ABSTRACT_CLASS(StringProperty);
mxDEFINE_ABSTRACT_CLASS(PointerProperty);
mxDEFINE_ABSTRACT_CLASS(ClassIdProperty);
mxDEFINE_ABSTRACT_CLASS(AssetIdProperty);

mxDEFINE_ABSTRACT_CLASS(APropertyList);
mxDEFINE_CLASS(PropertyList);

mxDEFINE_CLASS(AggregateProperty);

AggregateProperty::AggregateProperty()
{
}

AggregateProperty::~AggregateProperty()
{
}

mxDEFINE_ABSTRACT_CLASS(ArrayProperty);

mxDEFINE_ABSTRACT_CLASS(UserPointerProperty);
mxDEFINE_ABSTRACT_CLASS(BlobProperty);

mxDEFINE_CLASS(InvalidProperty);


template
<
	class BASE_PROPERTY,
	typename VAL_TYPE,
	typename RET_TYPE = const VAL_TYPE&,
	typename ARG_TYPE = const VAL_TYPE&,
	class MIXIN = EmptyClass
>
struct TGetSetHelper: public BASE_PROPERTY, public MIXIN
{
	typedef BASE_PROPERTY Super;

	typedef RET_TYPE GETTER( const void* const address );
	typedef void SETTER( void* const address, ARG_TYPE newValue );

protected:
	void *		m_object;	// pointer to any convertible type
	GETTER *	m_getter;
	SETTER *	m_setter;

public:
	TGetSetHelper()
	{
		m_object = nil;
		m_getter = nil;
		m_setter = nil;
	}

	template< typename CONVERTIBLE_TYPE >
	TGetSetHelper( void* address )
	{
		this->SetPointer< CONVERTIBLE_TYPE >( address );
	}

	template< typename CONVERTIBLE_TYPE >
	bool SetPointer( CONVERTIBLE_TYPE* address )
	{
		chkRET_FALSE_IF_NIL(address);
		m_object = address;
		m_getter = &StaticGet< CONVERTIBLE_TYPE >;
		m_setter = &StaticSet< CONVERTIBLE_TYPE >;
		return true;
	}

	//--- BASE_PROPERTY
	virtual RET_TYPE GetValue() const override
	{
		mxASSERT_PTR(m_getter);
		mxASSERT_PTR(m_object);
		return (*m_getter)( m_object );
	}
	virtual void SetValue( ARG_TYPE newValue ) override
	{
		mxASSERT_PTR(m_setter);
		mxASSERT_PTR(m_object);
		return (*m_setter)( m_object, newValue );
	}

	template< typename CONVERTIBLE_TYPE >
	static inline RET_TYPE StaticGet( const void* const address )
	{
		chkRET_X_IF_NIL(address, VAL_TYPE());
		const CONVERTIBLE_TYPE* o = c_cast(const CONVERTIBLE_TYPE*) address;
		return *o;
	}
	template< typename CONVERTIBLE_TYPE >
	static inline void StaticSet( void* const address, ARG_TYPE newValue )
	{
		chkRET_IF_NIL(address);
		CONVERTIBLE_TYPE* o = c_cast(CONVERTIBLE_TYPE*) address;
		*o = newValue;
	}
};

class GetSet_IntegerProperty
	: public TGetSetHelper< IntegerProperty, INT32, INT32, INT32 >
{
public:
};

class GetSet_FloatProperty
	: public TGetSetHelper< FloatProperty, double, double, double >
{
public:
};

class GetSet_BoolProperty
	: public TGetSetHelper< BooleanProperty, bool, bool, bool >
{
public:
};

class GetSetProperty_String
	: public TGetSetHelper< StringProperty, String, const String, const String& >
{
public:
};

typedef TGetSetProperty< AssetIdProperty, AssetID > GetSetProperty_AssetId;

class GetSetProperty_ClassId
	: public ClassIdProperty
{
	SClassId *	m_data;

public:
	GetSetProperty_ClassId( SClassId * classId )
		: m_data( classId )
	{
	}
	//--- ClassIdProperty
	virtual const mxClass& GetValue() const override
	{
		return *m_data->type;
	}
	virtual void SetValue( const mxClass& newValue ) override
	{
		m_data->type = &newValue;
	}
};

GetSet_IntegerProperty* CreateIntegerProperty( void* o, UINT size )
{
	chkRET_NIL_IF_NIL(o);
	chkRET_NIL_IF_NOT(size <= sizeof(long));

	GetSet_IntegerProperty* newProperty = new GetSet_IntegerProperty;

	if( size == 1 ) {
		newProperty->SetPointer( (INT8*)o );
	}
	elif( size == 2 ) {
		newProperty->SetPointer( (INT16*)o );
	}
	elif( size == 4 ) {
		newProperty->SetPointer( (INT32*)o );
	}
	elif( size == 8 ) {
		newProperty->SetPointer( (INT64*)o );
	}
	else mxUNREACHABLE;

	return newProperty;
}

GetSet_FloatProperty* CreateFloatProperty( void* o, UINT size )
{
	chkRET_NIL_IF_NIL(o);
	chkRET_NIL_IF_NOT(size <= sizeof(double));

	GetSet_FloatProperty* newProperty = new GetSet_FloatProperty;

	if( size == 4 ) {
		newProperty->SetPointer( (float*)o );
	}
	elif( size == 8 ) {
		newProperty->SetPointer( (double*)o );
	}
	else mxUNREACHABLE;

	return newProperty;
}

GetSet_BoolProperty* CreateBoolProperty( void* o, UINT size )
{
	chkRET_NIL_IF_NIL(o);
	chkRET_NIL_IF_NOT(size == sizeof(bool));

	GetSet_BoolProperty* newProperty = new GetSet_BoolProperty;

	newProperty->SetPointer( (bool*)o );

	return newProperty;
}

void CreateClassMembersProperties(
	void* o,
	const mxClassLayout& members,
	AggregateProperty* parentProperty )
{
	for( UINT iField = 0 ; iField < members.numFields; iField++ )
	{
		const mxField& field = members.fields[ iField ];
		void* fieldData = (BYTE*)o + field.offset;

		AProperty* fieldProperty = APropertyFactory::StaticCreateProperty( fieldData, field.type );
		if( fieldProperty != nil )
		{
			fieldProperty->name.SetReference(Chars(field.name));
			parentProperty->kids.Add( fieldProperty );
		}
	}
}

AProperty* CreateClassProperties( void* o, const mxClass& type )
{
	chkRET_NIL_IF_NIL(o);

	AggregateProperty* classProperty = new AggregateProperty;
	classProperty->type = &type;

	const mxClass* parentClass = type.GetParent();
	while( parentClass != nil )
	{
		if( ObjectUtil::Class_Has_Fields( *parentClass ) )
		{
			AProperty* parentClassProperty = CreateClassProperties( o, *parentClass );
			if( parentClassProperty != nil ) {
				classProperty->kids.Add( parentClassProperty );
			}
		}
		parentClass = parentClass->GetParent();
	}

	const mxClassLayout& classMembers = type.GetLayout();
	CreateClassMembersProperties( o, classMembers, classProperty );

	return classProperty;
}

struct DefaultPropertyFactory : public APropertyFactory
{
	virtual AProperty* CreateProperty( void* o, const mxType& type ) override
	{
		chkRET_NIL_IF_NIL(o);

		const ETypeKind typeKind = type.m_kind;
		const UINT typeSize = type.m_size;

		switch( typeKind )
		{
		case ETypeKind::Type_Integer :
			{
				return CreateIntegerProperty( o, typeSize );
			}
			break;

		case ETypeKind::Type_Float :
			{
				return CreateFloatProperty( o, typeSize );
			}
			break;

		case ETypeKind::Type_Bool :
			{
				return CreateBoolProperty( o, typeSize );
			}
			break;

		case ETypeKind::Type_Enum :
			{
				EnumProperty* enumProperty = new EnumProperty();
				enumProperty->o = o;
				enumProperty->type = &type.UpCast< mxEnumType >();
				return enumProperty;
			} break;

		case ETypeKind::Type_Flags :
			{
				FlagsProperty* flagsProperty = new FlagsProperty();
				flagsProperty->o = o;
				flagsProperty->type = &type.UpCast< mxFlagsType >();
				return flagsProperty;
			}
			break;

		case ETypeKind::Type_String :
			{
				GetSetProperty_String* newProperty = new GetSetProperty_String();
				newProperty->SetPointer( c_cast(String*)o );
				return newProperty;
			}
			break;

		case ETypeKind::Type_Class :
			{
				const mxClass& classInfo = type.UpCast< mxClass >();
				return CreateClassProperties( o, classInfo );
			}
			break;

		case ETypeKind::Type_Pointer :
			{
				const mxPointerType& pointerType = type.UpCast< mxPointerType >();
				PointerProperty* newProperty = new PointerProperty();
				newProperty->type = &pointerType;
				return newProperty;
			}
			break;

		case ETypeKind::Type_AssetId :
			{
				AssetID * assetId = static_cast< AssetID* >( o );
				GetSetProperty_AssetId* newProperty = new GetSetProperty_AssetId( assetId );
				return newProperty;
			}
			break;

		case ETypeKind::Type_ClassId :
			{
				SClassId * classId = static_cast< SClassId* >( o );
				GetSetProperty_ClassId* newProperty = new GetSetProperty_ClassId( classId );
				return newProperty;
			}
			break;

		case ETypeKind::Type_UserData :
			{
				const mxUserPointerType& userDataType = type.UpCast< mxUserPointerType >();
				UserPointerProperty* newProperty = new UserPointerProperty();
				newProperty->type = &userDataType;
				return newProperty;
			}
			break;

		case ETypeKind::Type_Blob :
			{
				const mxBlobType& blobType = type.UpCast< mxBlobType >();
				BlobProperty* newProperty = new BlobProperty();
				newProperty->type = &blobType;
				newProperty->blob = o;
				return newProperty;
			}
			break;

		case ETypeKind::Type_Array :
			{
				const mxArray& arrayType = type.UpCast< mxArray >();
				const UINT numObjects = arrayType.Generic_Get_Count( o );
				const void* arrayBase = arrayType.Generic_Get_Data( o );
				const mxType& itemType = arrayType.m_itemType;
				const UINT itemSize = itemType.m_size;

				if( numObjects > 64 )
				{
					InvalidProperty* invalidProperty = new InvalidProperty;
					Str::SPrintF( invalidProperty->text,
						"Array '%s' is too big\n"
						"(%u items of type '%s')",
						arrayType.GetTypeName(), numObjects, itemType.GetTypeName()
						);
					return invalidProperty;
				}
				else
				{
					ArrayProperty* newProperty = new ArrayProperty;
					newProperty->type = &arrayType;

					for( UINT iObject = 0; iObject < numObjects; iObject++ )
					{
						const UINT itemOffset = iObject * itemSize;
						void* itemData = mxAddByteOffset( c_cast(void*)arrayBase, itemOffset );

						AProperty* itemProperty = APropertyFactory::StaticCreateProperty( itemData, itemType );
						if( itemProperty != nil ) {
							Str::SPrintF( itemProperty->name, "[%u]", iObject );
							newProperty->kids.Add( itemProperty );
						}
					}

					return newProperty;
				}
			}
			break;

		default:
			ptERROR("CreateObjectProperties: Unknown type id (%s)\n", ETypeKind_To_Chars(typeKind));
		}
		return nil;
	}
};

static DefaultPropertyFactory gs_DefaultPropertyFactory;

APropertyFactory*	APropertyFactory::first = &gs_DefaultPropertyFactory;

AProperty* APropertyFactory::StaticCreateProperty( void* o, const mxType& type )
{
	APropertyFactory* current = APropertyFactory::first;
	while( current != nil )
	{
		AProperty* property_ = current->CreateProperty( o, type );
		if( property_ != nil ) {
			return property_;
		}
		current = current->_next;
	}
	ptERROR("APropertyFactory::StaticCreateProperty: couldn't create property for '%s'\n", type.GetTypeName());
	return nil;
}

#endif // MX_EDITOR

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
