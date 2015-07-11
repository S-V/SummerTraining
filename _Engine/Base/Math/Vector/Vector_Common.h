/*
=============================================================================
	File:	Vector_Common.h
	Desc:	Common stuff used by vector classes.
=============================================================================
*/

#ifndef __MATH_VECTOR_COMMON_H__
#define __MATH_VECTOR_COMMON_H__
mxSWIPED("idSoftware, Doom3/Prey/Quake4 SDKs");


// This value controls vector normalization, vector comparison routines, etc.
const FLOAT VECTOR_EPSILON = 0.001f;

//
//	EVectorComponent - For referring to the components of a vector by name rather than by an integer.
//
enum EVectorComponent
{
	Vector_X = 0,
	Vector_Y = 1,
	Vector_Z = 2,
	Vector_W = 3,
};

enum EAxisType
{
	AXIS_X = 0,
	AXIS_Y = 1,
	AXIS_Z = 2,
	AXIS_MAX
};

//
//	EViewAxes
//
enum EViewAxes
{
	VA_Right	= Vector_X,
	VA_Up		= Vector_Y,	// 'upward'
	VA_LookAt	= Vector_Z,	// Look direction vector ('forward').
};

#define	DEFAULT_LOOK_VECTOR		Vec3D( 0.0f, 0.0f, 1.0f )
#define	WORLD_RIGHT_VECTOR		Vec3D( 1.0f, 0.0f, 0.0f )
#define	WORLD_UP_VECTOR			Vec3D( 0.0f, 1.0f, 0.0f )

#define	UNTRANSFORMED_NORMAL	DEFAULT_LOOK_VECTOR

#endif /* !__MATH_VECTOR_COMMON_H__ */

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
