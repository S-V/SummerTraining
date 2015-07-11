/*
=============================================================================
	File:	Matrix_Common.h
	Desc:	Common stuff used by matrix classes.
=============================================================================
*/

#ifndef __MATH_MATRIX_COMMON_H__
#define __MATH_MATRIX_COMMON_H__
mxSWIPED("idSoftware, Doom3/Prey/Quake4 SDKs");
/*
=============================================================================

  Matrix classes, all matrices are row-major except Matrix3.

=============================================================================
*/



const DOUBLE MATRIX_INVERSE_EPSILON	= 1e-14;
const DOUBLE MATRIX_EPSILON			= 1e-6;

class Angles;
class idQuat;
class CQuat;
class Rotation;

class Plane3D;

class Matrix2;
class Matrix3;
class Matrix4;



#endif /* !__MATH_MATRIX_COMMON_H__ */

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
