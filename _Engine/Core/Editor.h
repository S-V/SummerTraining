/*
=============================================================================
	File:	EditorInterface.h
	Desc:	Support for communication with the editor.
			Property interfaces are mainly used by the editor property grid.
	Note:	functions prefixed with ed* are meant to be called
			from the editor only!
=============================================================================
*/
#pragma once

class Clump;
class ObjectList;
class AssetID;

class AProperty;

#if MX_EDITOR

//
// AEditable
// base class for most editor objects, provides fast RTTI
// NOTE: this class must be abstract!
// NOTE: this class must not define any member fields!
//
struct AEditable : public AObject
{
	mxDECLARE_ABSTRACT_CLASS(AEditable,AObject);
	mxNO_REFLECTED_MEMBERS;

public:

protected:
	AEditable();
	virtual ~AEditable();
};

// properties are allocated from a contiguous buffer ('object blob') to minimize memory fragmentation
//typedef TArray< BYTE >	PropertyBuffer;
//typedef TOffset< AProperty >	PropertyOffset;
//	PropertyOffset	sibling;	// linked list of brothers and sisters
//	PropertyOffset	children;	// linked list of children

/*
-----------------------------------------------------------------------------
	AbstractProperty
	abstract base class for editable properties
-----------------------------------------------------------------------------
*/
struct AProperty : public AObject, public ReferenceCountedX
{
	typedef TRefPtr< AProperty >	Ref;
	mxDECLARE_ABSTRACT_CLASS(AProperty,AObject);
	mxNO_REFLECTED_MEMBERS;

public:
	// name of this property
	String					name;

protected:
	AProperty();
	virtual ~AProperty();
};

struct APropertyFactory : public TSinglyLinkedList< APropertyFactory >
{
	virtual AProperty* CreateProperty( void* o, const mxType& type ) = 0;

	static APropertyFactory*	first;	// linked list of factories, previous overrides the next one

	static AProperty* StaticCreateProperty( void* o, const mxType& type );
};

class BooleanProperty : public AProperty
{
	mxDECLARE_ABSTRACT_CLASS( BooleanProperty, AProperty );
	mxNO_REFLECTED_MEMBERS;
public:
	virtual bool GetValue() const = 0;
	virtual void SetValue( bool newValue ) = 0;
};

class IntegerProperty : public AProperty
{
	mxDECLARE_ABSTRACT_CLASS( IntegerProperty, AProperty );
	mxNO_REFLECTED_MEMBERS;
public:
	virtual int GetValue() const = 0;
	virtual void SetValue( int newValue ) = 0;
};

class FloatProperty : public AProperty
{
	mxDECLARE_ABSTRACT_CLASS( FloatProperty, AProperty );
	mxNO_REFLECTED_MEMBERS;
public:
	virtual double GetValue() const = 0;
	virtual void SetValue( double newValue ) = 0;
};

class EnumProperty : public AProperty
{
	mxDECLARE_CLASS( EnumProperty, AProperty );
public:
	void *						o;
	TPtr< const mxEnumType >	type;
};

class FlagsProperty : public AProperty
{
	mxDECLARE_CLASS( FlagsProperty, AProperty );
public:
	void *						o;
	TPtr< const mxFlagsType >	type;
};

class StringProperty : public AProperty
{
	mxDECLARE_ABSTRACT_CLASS( StringProperty, AProperty );
	mxNO_REFLECTED_MEMBERS;
public:
	virtual const String GetValue() const = 0;
	virtual void SetValue( const String& newValue ) = 0;
};

class PointerProperty : public AProperty
{
	mxDECLARE_ABSTRACT_CLASS( PointerProperty, AProperty );
	mxNO_REFLECTED_MEMBERS;
public:
	TPtr< const mxPointerType >	type;
};

class ClassIdProperty : public AProperty
{
	mxDECLARE_ABSTRACT_CLASS( ClassIdProperty, AProperty );
	mxNO_REFLECTED_MEMBERS;
public:
	virtual const mxClass& GetValue() const = 0;
	virtual void SetValue( const mxClass& newValue ) {};
};

class AssetIdProperty : public AProperty
{
	mxDECLARE_ABSTRACT_CLASS( AssetIdProperty, AProperty );
	mxNO_REFLECTED_MEMBERS;
public:
	virtual const AssetID& GetValue() const = 0;
	virtual void SetValue( const AssetID& newValue ) {};
};

// property that consists of one or more child properties
class APropertyList : public AProperty
{
	mxDECLARE_ABSTRACT_CLASS( APropertyList, AProperty );
	mxNO_REFLECTED_MEMBERS;
public:
	virtual UINT NumKids() const = 0;
	virtual AProperty* GetKidAt( UINT i ) = 0;
};
struct PropertyList : public APropertyList
{
	TArray< AProperty::Ref >	m_kids;
public:
	mxDECLARE_CLASS( PropertyList, APropertyList );
	mxNO_REFLECTED_MEMBERS;
	virtual UINT NumKids() const { return m_kids.Num(); }
	virtual AProperty* GetKidAt( UINT i ) { return m_kids.At( i ); }
};

// e.g. class structure
class AggregateProperty : public APropertyList
{
	mxDECLARE_CLASS( AggregateProperty, APropertyList );
	mxNO_REFLECTED_MEMBERS;
public:
	TPtr< const mxClass >	type;
	TArray< AProperty::Ref > kids;
public:
	AggregateProperty();
	~AggregateProperty();
	virtual UINT NumKids() const override { return kids.Num(); }
	virtual AProperty* GetKidAt( UINT i ) override { return kids.At( i ); }
};

class UserPointerProperty : public AProperty
{
	mxDECLARE_CLASS( UserPointerProperty, AProperty );
	mxNO_REFLECTED_MEMBERS;
public:
	TPtr< const mxUserPointerType >	type;
};

class BlobProperty : public AProperty
{
	mxDECLARE_CLASS( BlobProperty, AProperty );
	mxNO_REFLECTED_MEMBERS;
public:
	TPtr< const mxBlobType >	type;
	void *						blob;
};

class ArrayProperty : public APropertyList
{
	mxDECLARE_ABSTRACT_CLASS( ArrayProperty, APropertyList );
	mxNO_REFLECTED_MEMBERS;
public:
	TPtr< const mxArray >	type;
	TArray< AProperty::Ref >	kids;
public:
	virtual UINT NumKids() const { return kids.Num(); }
	virtual AProperty* GetKidAt( UINT i ) { return kids[i]; }
};

template
<
	class BASE_PROPERTY,	// where BASE_PROPERTY : ... : AProperty
	typename VAL_TYPE,
	typename RET_TYPE = const VAL_TYPE&,
	typename ARG_TYPE = const VAL_TYPE&,
	class MIXIN = EmptyClass
>
struct TGetSetProperty : public BASE_PROPERTY, public MIXIN
{
	VAL_TYPE *	m_object;

public:
	typedef BASE_PROPERTY Super;

	TGetSetProperty( VAL_TYPE* address = NULL )
	{
		m_object = address;
	}
	bool SetPointer( VAL_TYPE* address )
	{
		chkRET_FALSE_IF_NIL(address);
		m_object = address;
		return true;
	}
	//--- BASE_PROPERTY
	virtual RET_TYPE GetValue() const override
	{
		mxASSERT_PTR(m_object);
		return TPODCast< VAL_TYPE >::GetConst( m_object );
	}
	virtual void SetValue( ARG_TYPE newValue ) override
	{
		mxASSERT_PTR(m_object);
		TPODCast< VAL_TYPE >::Set( m_object, 0, newValue );
	}
};

// for showing error message
class InvalidProperty : public AProperty
{
	mxDECLARE_CLASS( InvalidProperty, AProperty );
public:
	String	text;
};

//---------------------------------------------------------------------------

/*
-Make an abstract “EditAction” class, that represents a single action taken by the user to edit the level.  Give it virtual methods for “Undo” and “Redo”
-Make a few classes that inherit from EditAction and implement Undo and Redo for a specific action (moving an object, rotating an object, adding/removing an object, changing a Property, etc.)
-MapEditorLevel class has two stacks of EditAction’s: an Undo stack and a Redo stack.  When the user performs an action, it’s pushed onto the Undo stack and the Redo stack is cleared.
 When the user does an Undo, the Undo stack is popped, Undo is called on that EditAction, and the action is pushed onto the Redo stack.
 When the user does a Redo, the Redo stack is popped, Redo is called, and the action is pushed onto the Undo stack.
*/
class EditorAction
{
public:
	virtual void Undo() = 0;
	virtual void Redo() = 0;
};

#endif // MX_EDITOR

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
