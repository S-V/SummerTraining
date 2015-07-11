/*
=============================================================================
	Graphics model used for rendering.
=============================================================================
*/
#pragma once

#include <Core/VectorMath.h>
#include <Renderer/Material.h>
#include <Renderer/Mesh.h>

class rxProxy;

struct ModelHeader_t
{
	//AssetID
};

mxTODO("use this instead of Float3x4");
struct rxTransform : public CStruct
{
	Float4	translation;
	Float4	orientation;
};

/*
-----------------------------------------------------------------------------
	rxModel is a graphics model used for rendering;
	it basically represents an instance of a renderable mesh in a scene.
	several models can share one mesh.
-----------------------------------------------------------------------------
*/
struct rxModel : public CStruct
{
	TPtr< rxMesh >			m_mesh;		// mesh for rendering
	TPtr< rxProxy >			m_proxy;	// proxy for coarse culling
	TBuffer< rxMaterial* >	m_batches;	// pointers to graphics materials
	TPtr< Float3x4 >	m_transform;	// pointer to local-to-world transform
	TBuffer< Float4x4 >	m_boneMatrices;
	//24/48
public:
	mxDECLARE_CLASS( rxModel, CStruct );
	mxDECLARE_REFLECTION;
	rxModel();

	static rxModel* Create( rxMesh* mesh, Clump* clump );

	static void* Load( Assets::LoadContext2 & context );
	static ERet Online( Assets::LoadContext2 & context );
	static void Offline( Assets::LoadContext2 & context );
	static void Destruct( Assets::LoadContext2 & context );
};

/*
-----------------------------------------------------------------------------
	rxProxy

	used mainly for coarse visibility tests (view frustum culling),
	'broadphase proxy' in physics engine terms
-----------------------------------------------------------------------------
*/
struct rxProxy : public CStruct
{
	Sphere16				m_bv;		// 24 bounding volume in local space
	TPtr< rxModel >			m_model;	// the client object (could use a void* with type ID)
	TPtr< Float3x4 >		m_transform;// local-to-world transform
	// 32/40
public:
	mxDECLARE_CLASS( rxProxy, CStruct );
	mxDECLARE_REFLECTION;
	rxProxy();
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
