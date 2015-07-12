/*
=============================================================================
	File:	Core.h
	Desc:	
=============================================================================
*/
#pragma once

// Upper design limits
enum {
	MAX_ENTITIES = 16384
};

mxDECLARE_32BIT_HANDLE(HEntity);
mxREFLECT_AS_BUILT_IN_INTEGER(HEntity);

class Entity : public CStruct
{
	EntityComponent *	m_components;	// linked list of components
public:
	mxDECLARE_CLASS(Entity,CStruct);
	mxDECLARE_REFLECTION;
	Entity();
	~Entity();
	ERet AddComponent( EntityComponent* newComponent );
	EntityComponent* FindComponentOfType( const mxClass* baseType );
};

struct EntityComponent : public CStruct, public TSinglyLinkedList< EntityComponent >
{
	TPtr< Entity >			m_owner;
	TPtr< const mxClass >	m_class;
public:
	mxDECLARE_CLASS(EntityComponent,CStruct);
	mxDECLARE_REFLECTION;
	EntityComponent();
	~EntityComponent();
	const mxClass* GetType() const;
};

namespace EntitySystem
{
	//
}//namespace EntitySystem

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
