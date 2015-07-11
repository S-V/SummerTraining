/*
=============================================================================
	File:	Core.cpp
	Desc:	
=============================================================================
*/
#include <Core/Core_PCH.h>
#pragma hdrstop
#include <Core/Core.h>
#include <Core/EntitySystem.h>

mxDEFINE_CLASS(Entity);
mxBEGIN_REFLECTION(Entity)
	mxMEMBER_FIELD(m_components),
mxEND_REFLECTION;
Entity::Entity()
{
	m_components = NULL;
}
Entity::~Entity()
{
}
ERet Entity::AddComponent( EntityComponent* newComponent )
{
	chkRET_X_IF_NOT( newComponent->m_owner == NULL, ERR_INVALID_PARAMETER );
	chkRET_X_IF_NOT( this->FindComponentOfType(newComponent->m_class) == NULL, ERR_SUCH_OBJECT_ALREADY_EXISTS );
	newComponent->AppendSelfToList( &m_components );
	return ALL_OK;
}
EntityComponent* Entity::FindComponentOfType( const mxClass* baseType )
{
	EntityComponent* current = m_components;
	while( current )
	{
		if( current->m_class->IsDerivedFrom( *baseType ) ) {
			return current;
		}
		current = current->_next;
	}
	return NULL;
}

mxDEFINE_CLASS(EntityComponent);
mxBEGIN_REFLECTION(EntityComponent)
	mxMEMBER_FIELD(_next),
	mxMEMBER_FIELD(m_owner),
	mxMEMBER_FIELD(m_class),
mxEND_REFLECTION;
EntityComponent::EntityComponent()
{
}
EntityComponent::~EntityComponent()
{
}
const mxClass* EntityComponent::GetType() const
{
	return m_class;
}

namespace EntitySystem
{
	//
}//namespace EntitySystem

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
