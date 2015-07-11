#pragma once

#include <Core/VectorMath.h>

struct LightFlags {
	enum Enum {
		// Is the light enabled?
		Enabled	= BIT(0),
		// Does the light cast dynamic shadows? (Set if the light causes the affected geometry to cast shadows.)
		Shadows	= BIT(1),

		Dummy1	= BIT(2),
		Dummy2	= BIT(3),
		Dummy3	= BIT(4),
		//Dummy4	= BIT(5),
		//Dummy5	= BIT(6),
		//Dummy6	= BIT(7),
		//Dummy7	= BIT(8),
		//Dummy8	= BIT(9),
		//Dummy9	= BIT(10),
		//Dummy10	= BIT(11),

		Default	= Enabled|Shadows
	};
};
mxDECLARE_FLAGS( LightFlags::Enum, UINT32, FLightFlags );

/*
-----------------------------------------------------------------------------
	rxGlobalLight
	represents a directional (infinite) light source.
-----------------------------------------------------------------------------
*/
struct rxGlobalLight : public CStruct
{
	Float3			m_color;		// diffuse color
	//RGBAf			m_backColor;
	Float3			m_direction;	// normalized light direction in world space
	FLightFlags		m_flags;
	Float4			m_cascadeSplits;	// PSSM cascade splits
	float			m_shadowDepthBias;
	float			m_shadowFadeDistance;

public:
	mxDECLARE_CLASS( rxGlobalLight, CStruct );
	mxDECLARE_REFLECTION;
	rxGlobalLight();
};

/*
-----------------------------------------------------------------------------
	rxLocalLight

	represents a localized dynamic light source in a scene;
	this structure should be as small as possible
-----------------------------------------------------------------------------
*/
struct rxLocalLight : public CStruct
{
	//Float4	position_radius;	// xyz = light position in view-space, z = light range
	//Float4	diffuse_falloff;	// rgb = diffuse color, a = falloff
	Float3	position;	// xyz = light position in view-space, z = light range
	float	radius;
	Float3	color;	// rgb = diffuse color, a = falloff

public:
	mxDECLARE_CLASS( rxLocalLight, CStruct );
	mxDECLARE_REFLECTION;
	rxLocalLight();
};

#if 0
TPtr< WorldMatrixCM >	m_transform;	// pointer to local-to-world transform

/*
-----------------------------------------------------------------------------
	rxLocalLight

	represents a localized dynamic light source in a scene;

	this structure should be as small as possible
-----------------------------------------------------------------------------
*/
mxALIGN_16(struct rxLocalLight) : public rxLight
{
	// common light data
	Float4			m_position;		// light position in world-space
	Float4			m_diffuseColor;	// rgb - color, a - light intensity
	Float4			m_radiusInvRadius;	// x - light radius, y - inverse light range
	Float4			m_specularColor;
	// spot light data
	Float4			m_spotDirection;	// normalized axis direction in world space
	// x = cosTheta (cosine of half inner cone angle), y = cosPhi (cosine of half outer cone angle)
	Float4			m_spotAngles;

	// nudge a bit to reduce self-shadowing (we can get shadow acne on all surfaces without this bias)
	F4				m_shadowDepthBias;

	LightFlags		m_flags;
	LightType		m_lightType;	// ELightType

	// for light flares
	rxTexture::Ref	m_billboard;
	F4				m_billboardSize;

	// spot light projector
	rxTexture::Ref	m_projector;
	F4				m_projectorIntensity;

public:
	mxDECLARE_CLASS(rxLocalLight,rxLight);
	mxDECLARE_REFLECTION;

	rxLocalLight();

	bool EnclosesView( const rxSceneContext& sceneContext );

	const Vec3D& GetOrigin() const;
	void SetOrigin( const Vec3D& newPos );

	// range of influence
	FLOAT GetRadius() const;
	void SetRadius( FLOAT newRadius );

	// Direction that the light is pointing in world space.
	void SetDirection( const Vec3D& newDirection );
	const Vec3D& GetDirection() const;

	// Sets the apex angles for the spot light which determine the light's angles of illumination.

	// theta - angle, in radians, of a spotlight's inner cone - that is, the fully illuminated spotlight cone.
	// This value must be in the range from 0 through the value specified by Phi.
	//
	void SetInnerConeAngle( FLOAT theta );
	FLOAT GetInnerConeAngle() const;

	// phi - angle, in radians, defining the outer edge of the spotlight's outer cone. Points outside this cone are not lit by the spotlight.
	// This value must be between 0 and pi.
	//
	void SetOuterConeAngle( FLOAT phi );
	FLOAT GetOuterConeAngle() const;

	// Set projective texture blend factor ('factor' must be in range [0..1]).
	void SetProjectorIntensity( FLOAT factor = 0.5f );
	FLOAT GetProjectorIntensity() const;

	void SetShadowDepthBias( FLOAT f );
	FLOAT GetShadowDepthBias() const;

	FLOAT CalcBottomRadius() const;

	bool DoesCastShadows() const;
	void SetCastShadows( bool bShadows );

	bool IntersectsSphere( const Sphere& sphere ) const;

	mxIMPLEMENT_COMMON_PROPERTIES(rxLocalLight);
};

//
//	rxPointLight - is a point source.
//
//	The light has a position in space and radiates light in all directions.
//


//
//	rxSpotLight - is a spotlight source.
//
//	This light is like a point light,
//	except that the illumination is limited to a cone.
//	This light type has a direction and several other parameters
//	that determine the shape of the cone it produces.
//


/*
-----------------------------------------------------------------------------
	SDynLight

	represents a visible dynamic light source which can cast a shadow;

	this structure should be as small as possible
-----------------------------------------------------------------------------
*/
mxALIGN_16(struct SDynLight)
{
	Float4		posAndRadius;	// xyz = light position in view-space, z = light range
	Float4		diffuseAndInvRadius;	// rgb = diffuse color, a = inverse range
	Float4		falloff;	// x - falloff start, y - falloff width
	float4x4	viewProjection;
};
#endif

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
