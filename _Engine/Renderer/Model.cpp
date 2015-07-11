/*
=============================================================================
	Graphics model used for rendering.
=============================================================================
*/
#include "Renderer/Renderer_PCH.h"
#pragma hdrstop
#include <Renderer/Model.h>

/*
-----------------------------------------------------------------------------
	rxModel
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS( rxModel );
mxBEGIN_REFLECTION( rxModel )
	mxMEMBER_FIELD( m_mesh ),
	mxMEMBER_FIELD( m_proxy ),
	mxMEMBER_FIELD( m_batches ),
	mxMEMBER_FIELD( m_transform ),
	mxMEMBER_FIELD( m_boneMatrices ),
mxEND_REFLECTION
rxModel::rxModel()
{
}
rxModel* rxModel::Create( rxMesh* mesh, Clump* clump )
{
	rxModel* model = clump->New< rxModel >();
	rxProxy* proxy = clump->New< rxProxy >();
	Float3x4* mTRS = clump->New< Float3x4 >();

	model->m_mesh = mesh;
	model->m_proxy = proxy;
	model->m_transform = mTRS;

	Sphere16_From_AABB(mesh->m_bounds, &proxy->m_bv);
	proxy->m_model = model;
	proxy->m_transform = mTRS;

	*mTRS = Float3x4_Identity();

	return model;
}
void* rxModel::Load( Assets::LoadContext2 & context )
{
	Clump* clump = context.clump;
//	return Create( mesh, clump );
UNDONE;
	//rxModel* model = clump->New< rxModel >();
	//rxProxy* proxy = clump->New< rxProxy >();
	//Float3x4* mTRS = clump->New< Float3x4 >();
#if 0
	model->m_mesh = ?;
	model->m_proxy = proxy;
	model->m_transform = mTRS;

	proxy->m_bv = ?;
	proxy->m_model = model;
	proxy->m_transform = mTRS;
#endif

	//return model;
	return NULL;
}
ERet rxModel::Online( Assets::LoadContext2 & context )
{
	return ALL_OK;
}
void rxModel::Offline( Assets::LoadContext2 & context )
{

}
void rxModel::Destruct( Assets::LoadContext2 & context )
{

}

/*
-----------------------------------------------------------------------------
	rxProxy
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS( rxProxy );
mxBEGIN_REFLECTION( rxProxy )
	mxMEMBER_FIELD( m_bv ),
	mxMEMBER_FIELD( m_model ),
	mxMEMBER_FIELD( m_transform ),
mxEND_REFLECTION
rxProxy::rxProxy()
{
	Sphere16_Clear( &m_bv );
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
