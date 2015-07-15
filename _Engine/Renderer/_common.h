// This is a shared header file included by both host application (C++) code and shader code (HLSL).

#ifdef __cplusplus
#	define DECLARE_CB( cbName, slot )	enum { cbName##_Index = slot }; __declspec(align(16)) struct cbName
#	define SAMPLER_USES_SLOT( samplerName, slot )	enum { samplerName##_Index = slot };
#	define CPP_CODE( code )		code
#	define HLSL_CODE( code )
#	define cbuffer	mxPREALIGN(16) struct
#	define PACK_MATRIX
#else // if HLSL
#	define DECLARE_CB( cbName, slot )	cbuffer cbName : register( b##slot )
#	define SAMPLER_USES_SLOT( samplerName, slot )	SamplerState samplerName : register( s##slot )
#	define CPP_CODE( code )
#	define HLSL_CODE( code )	code
#	define PACK_MATRIX	column_major
#endif

#define PACKOFFSET( byteOffset )	HLSL_CODE( : packoffset( byteOffset ) )
#define SEMANTIC( X )				HLSL_CODE( : X )

#ifndef __cplusplus
#	define CB_USES_SLOT( cbName, slot )	cbuffer cbName : register( b##slot )
#endif

#ifdef __cplusplus
	typedef Float4x4	float4x4;
	typedef Float4		float4;
	typedef Float3		float3;
#endif

// Shader constant registers that are reserved by the engine.

/*===============================================
		Global shader constants.
===============================================*/

// per-frame constants
DECLARE_CB( G_PerFrame, 0 )
{
	//float4	screenSize_invSize;	// xy - backbuffer dimensions, zw - inverse viewport size
	// x - globalTimeInSeconds;	// Time parameter, in seconds. This keeps increasing.
/*	
	float	g_time;
	float	g_invTime;
	float	g_deltaTime;
	float	g_invDeltaTime;
*/
	float4	g_timeParams;
/*
	// Depth Of Field

	// x - Focal plane distance, y - Near blur plane distance,
	// z - Far blur plane distance, w - Far blur limit [0,1]
	// g_fFocalPlaneDistance, g_fNearBlurPlaneDistance, g_fFarBlurPlaneDistance, g_fFarBlurLimit
	float4	DoF_params;

	// High Dynamic Range Rendering
	// g_fExposure, g_fBloomScale, g_fInvContrast
	float4	HDR_params;

	// Environment

	float4	ambientColor;

	// Subsurface scattering
	// x - SSS width,
	float4	SSS_params;
*/
};

// per-camera constants
DECLARE_CB( G_PerCamera, 1 )
{
	PACK_MATRIX float4x4	g_viewMatrix;
	PACK_MATRIX float4x4	g_viewProjectionMatrix;
	PACK_MATRIX float4x4	g_inverseViewMatrix;
	PACK_MATRIX float4x4	g_projectionMatrix;
	PACK_MATRIX float4x4	g_inverseProjectionMatrix;
	PACK_MATRIX float4x4	g_inverseViewProjectionMatrix;

	// x = tan( 0.5 * horizFOV), y = tan( 0.5 * vertFOV ), z = 1/y, w = 1
	float4		g_tanHalfFoV;	// used for restoring view-space position

	// Values used to linearize the Z buffer (http://www.humus.name/temp/Linearize%20depth.txt)
	// x = 1-far/near
	// y = far/near
	// z = x/far
	// w = y/far
	float4	g_ZBufferParams;

	//depthClipPlanes
	// x = nearZ, y = farZ, z = invNearZ, w = invFarZ
	float4	g_ZBufferParams2;
	
	// x = H, y = V, z = A, w = B
	float4 g_ProjParams;
	
	// x = 1/H
	// y = 1/V
	// z = 1/B
	// w = -A/B
	float4 g_ProjParams2;

	float3	g_WorldSpaceCameraPos;

	// frustum corners in world-space are used for restoring world-space pixel positions)
	//float4	g_frustumCornerVS_FarTopLeft;
	//float4	g_frustumCornerVS_FarTopRight;
	//float4	g_frustumCornerVS_FarBottomLeft;
	//float4	g_frustumCornerVS_FarBottomRight;
};

// per-object constants
DECLARE_CB( G_PerObject, 2 )
{
	PACK_MATRIX float4x4	g_worldMatrix;	// object-to-world transformation matrix
	PACK_MATRIX float4x4	g_worldViewMatrix;
	PACK_MATRIX float4x4	g_worldViewProjectionMatrix;

	//PACK_MATRIX float4x4	g_worldMatrixIT;		// transpose of the inverse of the world matrix
	//PACK_MATRIX float4x4	g_worldViewMatrixIT;	// transpose of the inverse of the world-view matrix
};

/*
// these constants are set upon application startup
// and change only during engine resets
DECLARE_CB( cbNeverChanges, 0 )
{
	float2	g_screenSize;	// xy - backbuffer dimensions
	float2	g_invScreenSize;	// pixel or texel size?
};
*/

struct DirectionalLight
{
	float3 direction SEMANTIC(LightDirection);
	CPP_CODE(float pad0;)
	float3 color SEMANTIC(LightColor);
	CPP_CODE(float pad1;)
};

struct PointLight
{
	float4 Position_InverseRadius;	// SEMANTIC(LightPosition)
	float4 Color_Radius;
};

#ifndef __cplusplus

/*===============================================
		Samplers
===============================================*/

SamplerState pointSampler		< string DefaultValue = "PointSampler"; >;
SamplerState trilinearSampler	< string DefaultValue = "TrilinearSampler"; >;

/*
SamplerState pointSampler : register(s0)		< string DefaultValue = "PointSampler"; >;
SamplerState bilinearSampler : register(s1)		< string DefaultValue = "BilinearSampler"; >;
SamplerState anisotropicSampler : register(s2)	< string DefaultValue = "AnisotropicSampler"; >;

SamplerState pointClampSampler : register(s3);
SamplerState linearClampSampler : register(s4);
*/
SamplerState albedoMapSampler	< string DefaultValue = "DiffuseMapSampler"; >;
SamplerState detailMapSampler;
SamplerState normalMapSampler;
SamplerState specularMapSampler;
SamplerState attenuationSampler;
SamplerState cubeMapSampler;

SamplerComparisonState	shadowMapSampler;
SamplerComparisonState	shadowMapPCFSampler;
SamplerComparisonState	shadowMapPCFBilinearSampler;

/*===============================================
		Functions
===============================================*/

/**
 * Aligns the clip space position so that it can be used as a texture coordinate.
 */
float2 ClipPosToTexCoords( in float2 clipPosition )
{
	// flip Y axis, [-1..+1] -> [0..1]
	//return float2( 0.5f, -0.5f ) * clipPosition + 0.5f;
	return float2(clipPosition.x, -clipPosition.y) * 0.5f + 0.5f;
}

/**
 * Aligns the [0,1] UV to match the view within the backbuffer.
 */
float2 TexCoordsToClipPos( in float2 texCoords )
{
	// [0..1] -> [-1..+1], flip Y axis
	return texCoords * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f );
}

/**
 * aligns the clip space position so that it can be used as a texture coordinate
 * to properly align in screen space
 */
float4 ScreenAlignedPosition( float4 screenPosition )
{
	float4 result = screenPosition;
	result.xy = (screenPosition.xy / screenPosition.w) * 0.5f + 0.5f;
	result.y = 1.0f - result.y;
	return result;
}

/*
// Z buffer to linear 0..1 depth (0 at eye, 1 at far plane)
inline float Linear01Depth( float z )
{
	float A = g_ZBufferParams.x;	// 1-far/near
	float B = g_ZBufferParams.y;	// far/near
	return 1.0 / ( A * z + B );
	//ProjectionA = FarClipDistance / (FarClipDistance - NearClipDistance);
	//ProjectionB = (-FarClipDistance * NearClipDistance) / (FarClipDistance - NearClipDistance);
	//float linearDepth = ProjectionB / (depth - ProjectionA);
}
// Z buffer to linear depth
inline float LinearEyeDepth( float z )
{
	return 1.0 / (g_ZBufferParams.z * z + g_ZBufferParams.w);
}

// Z buffer to linear 0..1 depth (0 at eye, 1 at far plane)
float HardwareDepthToLinear( float z )
{
	float near = g_ZBufferParams2.x;
	float far = g_ZBufferParams2.y;
	float A = far / (far - near);
	float B = (-far * near) / (far - near);
	float depth = B / (z - A);
	return depth;
}
*/
// Converts hardware depth buffer value (Z / W) into (1 / W).
// This is used for restoring view-space depth and position.
float HardwareDepthToInverseW( float z )
{
	return 1 / (z * g_inverseProjectionMatrix._m32 + g_inverseProjectionMatrix._m33);
}
#endif


// Preprocessor clean-up.

#ifdef __cplusplus
#	undef row_major
#	undef cbuffer
#	undef DECLARE_CB
#endif
