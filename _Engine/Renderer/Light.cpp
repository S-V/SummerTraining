#include "Renderer/Renderer_PCH.h"
#pragma hdrstop
#include <Renderer/Light.h>

mxBEGIN_FLAGS( FLightFlags )
mxREFLECT_BIT( Enabled, LightFlags::Enabled ),
mxREFLECT_BIT( Shadows, LightFlags::Shadows ),
mxREFLECT_BIT( Dummy1, LightFlags::Dummy1 ),
mxREFLECT_BIT( Dummy2, LightFlags::Dummy2 ),
mxREFLECT_BIT( Dummy3, LightFlags::Dummy3 ),
//mxREFLECT_BIT( LightFlags, Dummy4 ),
//mxREFLECT_BIT( LightFlags, Dummy5 ),
//mxREFLECT_BIT( LightFlags, Dummy6 ),
//mxREFLECT_BIT( LightFlags, Dummy7 ),
//mxREFLECT_BIT( LightFlags, Dummy8 ),
//mxREFLECT_BIT( LightFlags, Dummy9 ),
//mxREFLECT_BIT( LightFlags, Dummy10 ),
mxEND_FLAGS

mxDEFINE_CLASS( rxGlobalLight );
mxBEGIN_REFLECTION( rxGlobalLight )
	mxMEMBER_FIELD(m_color),
	mxMEMBER_FIELD(m_direction),
	mxMEMBER_FIELD(m_flags),
	mxMEMBER_FIELD(m_cascadeSplits),
	mxMEMBER_FIELD(m_shadowDepthBias),
	mxMEMBER_FIELD(m_shadowFadeDistance),	
mxEND_REFLECTION

rxGlobalLight::rxGlobalLight()
{
	m_color = Float3_Replicate(1);
	m_direction = Float3_Set(0, 0, -1);	// down

	//m_cascadeSplits.Set( 0.125f, 0.25f, 0.5f, 1.0f );
	m_cascadeSplits = Float4_Set( 0.05f, 0.2f, 0.5f, 1.0f );

	m_shadowDepthBias = 0.001f;
	m_shadowFadeDistance = 500.0f;

	m_flags = LightFlags::Default;
}

/*
-----------------------------------------------------------------------------
	rxLocalLight
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS(rxLocalLight);
mxBEGIN_REFLECTION(rxLocalLight)
	//mxMEMBER_FIELD( position_radius ),
	//mxMEMBER_FIELD( diffuse_falloff ),
	mxMEMBER_FIELD( position ),
	mxMEMBER_FIELD( radius ),
	mxMEMBER_FIELD( color ),
mxEND_REFLECTION
rxLocalLight::rxLocalLight()
{
	//position_radius = Float4_Set(0,0,0,1);
	//diffuse_falloff = Float4_Set(1,1,1,1);
	position = Float3_Set(0,0,0);
	radius = 1.0f;
	color = Float3_Set(1,1,1);
}

#if 0
//---------------------------------------------------------------------------

static
mxFORCEINLINE
FASTBOOL Point_Light_Encloses_Eye( const rxLocalLight& light, const rxSceneContext& view )
{
	mxOPTIMIZE("vectorize:");
#if 0
	FLOAT distanceSqr = (light.GetOrigin() - view.GetOrigin()).LengthSqr();
	return distanceSqr < squaref( light.GetRadius() + view.nearZ + 0.15f );
#else
	//FLOAT distance = (light.GetOrigin() - view.GetOrigin()).LengthFast();
	//return distance < light.GetRadius() + view.nearZ;
	Float4	lightVec = XMVectorSubtract( light.m_position, view.invViewMatrix.r[3] );
	Float4	lightVecSq = XMVector3LengthSq( lightVec );
	float	distanceSq = XMVectorGetX( lightVecSq );
	return distanceSq < squaref( light.GetRadius() + view.nearClip );
#endif
}

static
mxFORCEINLINE
FASTBOOL Spot_Light_EnclosesView( const rxLocalLight& light, const rxSceneContext& view )
{
#if 0

	Vec3D  origin( this->GetOrigin() );	// cone origin

	Vec3D  dir( GetDirection() );	// direction of cone axis
	//mxASSERT( dir.IsNormalized() );

	FLOAT t = dir * ( point - origin );

	if ( t < 0 || t > GetRange() ) {
		return FALSE;
	}

	FLOAT r = t * ( bottomRadius * GetInvRange() );	// cone radius at closest point

	// squared distance from the point to the cone axis
	FLOAT distSq = (( origin + dir * t ) - point).LengthSqr();

	return ( distSq < squaref(r) );

#else

	Vec3D  origin( light.GetOrigin() );

	Vec3D  d( view.GetEyePosition() - origin );

	FLOAT sqrLength = d.LengthSqr();

	d *= InvSqrt( sqrLength );

	FLOAT x = d * light.GetDirection();

	FLOAT cosPhi = as_vec4(light.m_spotAngles).y;

	return ( x > 0.0f )
		&& ( x >= cosPhi )
		&& ( sqrLength < squaref(light.GetRadius()) );

#endif
}

/*
-----------------------------------------------------------------------------
	rxLocalLight
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS(rxLocalLight);
mxBEGIN_REFLECTION(rxLocalLight)
	mxMEMBER_FIELD( m_position )
	mxMEMBER_FIELD( m_diffuseColor )
	mxMEMBER_FIELD( m_radiusInvRadius )
	mxMEMBER_FIELD( m_specularColor )

	mxMEMBER_FIELD( m_spotDirection )
	mxMEMBER_FIELD( m_spotAngles )

	mxMEMBER_FIELD( m_shadowDepthBias )

	mxMEMBER_FIELD( m_flags )
	mxMEMBER_FIELD( m_lightType )

	mxMEMBER_FIELD( m_billboard )
	mxMEMBER_FIELD( m_billboardSize )

	mxMEMBER_FIELD2( m_projector,	Projector_Texture,	Field_NoDefaultInit )
	mxMEMBER_FIELD( m_projectorIntensity )
mxEND_REFLECTION


rxLocalLight::rxLocalLight()
{
	m_position = g_XMIdentityR3;
	m_diffuseColor = XMVectorSet( 1.0f, 1.0f, 1.0f, 1.0f );
	m_radiusInvRadius = XMVectorSet( 1.0f, 1.0f, 0.0f, 0.0f );
	m_specularColor = g_XMZero;

	m_spotDirection = XMVectorSet( 0.0f, -1.0f, 0.0f, 0.0f );
	this->SetOuterConeAngle(90.0f);
	this->SetInnerConeAngle(45.0f);

	//m_shadowDepthBias = -0.0005f;
	m_shadowDepthBias = -0.0001f;	//<- for lights with large radius

	m_flags = LF_All;
	m_lightType = ELightType::Light_Point;

	m_billboardSize = 1.0f;
	m_projectorIntensity = 0.5f;
}

bool rxLocalLight::EnclosesView( const rxSceneContext& sceneContext )
{
	switch( m_lightType )
	{
	case Light_Point :
		return Point_Light_Encloses_Eye( *this, sceneContext );

	case Light_Spot :
		return Spot_Light_EnclosesView( *this, sceneContext );

	default:
		Unreachable;
	}
	return false;
}

const Vec3D& rxLocalLight::GetOrigin() const
{
	return as_vec3(m_position);
}

void rxLocalLight::SetOrigin( const Vec3D& newPos )
{
	m_position = vec_load1(newPos);
}

FLOAT rxLocalLight::GetRadius() const
{
	return XMVectorGetX(m_radiusInvRadius);
}

void rxLocalLight::SetRadius( FLOAT newRadius )
{
	mxASSERT(newRadius > 0.0f);
	VRET_IF_NOT(newRadius > 0.0f);
	const FLOAT invRadius = 1.0f / newRadius;
	m_radiusInvRadius = XMVectorSet( newRadius, invRadius, 0.0f, 0.0f );
}

void rxLocalLight::SetDirection( const Vec3D& newDirection )
{
	m_spotDirection = vec_load0(newDirection);
}

const Vec3D& rxLocalLight::GetDirection() const
{
	return as_vec3(m_spotDirection);
}

void rxLocalLight::SetInnerConeAngle( FLOAT theta )
{
	theta = DEG2RAD(theta);
	mxASSERT2( 0.0f < theta && theta < this->GetOuterConeAngle(), "spotlight inner angle must be in range [0..outerAngle]" );
	FLOAT cosTheta = Cos( theta * 0.5f );
	as_vec4(m_spotAngles).x = cosTheta;
}

FLOAT rxLocalLight::GetInnerConeAngle() const
{
	FLOAT cosTheta = as_vec4( m_spotAngles ).x;
	FLOAT innerAngle = ACos(cosTheta) * 2.0f;
	innerAngle = RAD2DEG(innerAngle);
	return innerAngle;
}

void rxLocalLight::SetOuterConeAngle( FLOAT phi )
{
	phi = DEG2RAD(phi);
	mxASSERT2( 0.0f < phi && phi < MX_PI, "spotlight outer angle must be in range [0..PI]" );
	FLOAT cosPhi = Cos( phi * 0.5f );
	as_vec4(m_spotAngles).y = cosPhi;

	FLOAT	innerAngle = this->GetInnerConeAngle();
	innerAngle = minf(innerAngle,RAD2DEG(phi-0.001f));
	this->SetInnerConeAngle(innerAngle);
}

FLOAT rxLocalLight::GetOuterConeAngle() const
{
	FLOAT cosTheta = as_vec4( m_spotAngles ).y;
	FLOAT outerAngle = 2.0f * ACos( cosTheta );
	outerAngle = RAD2DEG(outerAngle);
	return outerAngle;
}

FLOAT rxLocalLight::CalcBottomRadius() const
{
	FLOAT cosPhi = as_vec4( m_spotAngles ).y;
	FLOAT halfOuterAngle = ACos( cosPhi );
	FLOAT bottomRadius = this->GetRadius() * Tan( halfOuterAngle );
	return bottomRadius;
}

void rxLocalLight::SetProjectorIntensity( FLOAT factor )
{
	m_projectorIntensity = factor;
}

FLOAT rxLocalLight::GetProjectorIntensity() const
{
	return m_projectorIntensity;
}

void rxLocalLight::SetShadowDepthBias( FLOAT f )
{
	m_shadowDepthBias = f;
}

FLOAT rxLocalLight::GetShadowDepthBias() const
{
	return m_shadowDepthBias;
}

bool rxLocalLight::DoesCastShadows() const
{
	return m_flags & LF_Shadows;
}

void rxLocalLight::SetCastShadows( bool bShadows )
{
	setbit_cond( m_flags.m_bitmask, bShadows, LF_Shadows );
}

static
bool Point_Light_Intersects_Sphere( const rxLocalLight& light, const Sphere& sphere )
{
	// LengthSqr( centerA - centerB ) <= Sqr(radiusA + radiusB)
	return (as_vec3(light.m_position) - sphere.Center).LengthSqr() <= squaref(light.GetRadius() + sphere.Radius);
}

static
bool Spot_Light_Intersects_Sphere( const rxLocalLight& light, const Sphere& sphere )
{
	return Point_Light_Intersects_Sphere(light,sphere);
}

bool rxLocalLight::IntersectsSphere( const Sphere& sphere ) const
{
	switch( m_lightType )
	{
	case Light_Point :
		return Point_Light_Intersects_Sphere( *this, sphere );

	case Light_Spot :
		return Spot_Light_Intersects_Sphere( *this, sphere );

	default:
		Unreachable;
	}
	return false;
}
#endif

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
