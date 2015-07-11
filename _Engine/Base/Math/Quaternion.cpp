/*
=============================================================================
	File:	Quaternion.cpp
	Desc:	Quaternion class.
	Note:	Originally written by Id Software.
	Copyright (C) 2004 Id Software, Inc. ( idQuat )
=============================================================================
*/

#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#include <Base/Math/Math.h>

//mxBEGIN_STRUCT(idQuat)
//	mxMEMBER_FIELD(x)
//	mxMEMBER_FIELD(y)
//	mxMEMBER_FIELD(z)
//	mxMEMBER_FIELD(w)
//mxEND_REFLECTION

const idQuat	idQuat::quat_zero		( 0.0f, 0.0f, 0.0f, 0.0f );
const idQuat	idQuat::quat_identity	( 0.0f, 0.0f, 0.0f, 1.0f );

/*
=====================
idQuat::ToRotation
=====================
*/
Rotation idQuat::ToRotation( void ) const {
	Vec3D vec;
	FLOAT angle;

	vec.x = x;
	vec.y = y;
	vec.z = z;
	angle = Float_ACos( w );
	if ( angle == 0.0f ) {
		vec.Set( 0.0f, 0.0f, 1.0f );
	} else {
		//vec *= (1.0f / sin( angle ));
		vec.Normalize();
		vec.FixDegenerateNormal();
		angle *= 2.0f * mxRAD2DEG;
	}
	return Rotation( Vec3D::vec3_zero, vec, angle );
}

/*
=====================
idQuat::ToMat3
=====================
*/
Matrix3 idQuat::ToMat3( void ) const
{
	FLOAT	wx, wy, wz;
	FLOAT	xx, yy, yz;
	FLOAT	xy, xz, zz;
	FLOAT	x2, y2, z2;

	x2 = x + x;
	y2 = y + y;
	z2 = z + z;

	xx = x * x2;
	xy = x * y2;
	xz = x * z2;

	yy = y * y2;
	yz = y * z2;
	zz = z * z2;

	wx = w * x2;
	wy = w * y2;
	wz = w * z2;

	mxOPTIMIZE("make this 'inline' (?) and help RVO.");

	Matrix3	mat;

	mat[ 0 ][ 0 ] = 1.0f - ( yy + zz );
	mat[ 0 ][ 1 ] = xy - wz;
	mat[ 0 ][ 2 ] = xz + wy;

	mat[ 1 ][ 0 ] = xy + wz;
	mat[ 1 ][ 1 ] = 1.0f - ( xx + zz );
	mat[ 1 ][ 2 ] = yz - wx;

	mat[ 2 ][ 0 ] = xz - wy;
	mat[ 2 ][ 1 ] = yz + wx;
	mat[ 2 ][ 2 ] = 1.0f - ( xx + yy );

	return mat;
}

/*
=====================
idQuat::ToMat4
=====================
*/
Matrix4 idQuat::ToMat4( void ) const
{
	mxOPTIMIZE("expand the matrix multiplication by hand");
	return this->ToMat3().ToMat4();
}

/*
=====================
idQuat::ToCQuat
=====================
*/
CQuat idQuat::ToCQuat( void ) const {
	if ( w < 0.0f ) {
		return CQuat( -x, -y, -z );
	}
	return CQuat( x, y, z );
}

/*
============
idQuat::ToAngularVelocity
============
*/
Vec3D idQuat::ToAngularVelocity( void ) const {
	Vec3D vec;

	vec.x = x;
	vec.y = y;
	vec.z = z;
	vec.Normalize();
	return vec * Float_ACos( w );
}

/*
=============
idQuat::ToString
=============
*/
//const char *idQuat::ToString( INT precision ) const {
//	return idStr::FloatArrayToString( ToPtr(), GetDimension(), precision );
//}

/*
=====================
idQuat::Slerp

Spherical linear interpolation between two quaternions.
=====================
*/
idQuat &idQuat::Slerp( const idQuat &from, const idQuat &to, FLOAT t )
{
	idQuat	temp;
	FLOAT	omega, cosom, sinom, scale0, scale1;

	if ( t <= 0.0f ) {
		*this = from;
		return *this;
	}

	if ( t >= 1.0f ) {
		*this = to;
		return *this;
	}

	if ( from == to ) {
		*this = to;
		return *this;
	}

	cosom = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;
	if ( cosom < 0.0f ) {
		temp = -to;
		cosom = -cosom;
	} else {
		temp = to;
	}

	if ( ( 1.0f - cosom ) > 1e-6f ) {
#if 0
		omega = acos( cosom );
		sinom = 1.0f / sin( omega );
		scale0 = sin( ( 1.0f - t ) * omega ) * sinom;
		scale1 = sin( t * omega ) * sinom;
#else
		scale0 = 1.0f - cosom * cosom;
		sinom = Float_InvSqrt( scale0 );
		omega = Math::ATan16( scale0 * sinom, cosom );
		scale0 = Math::Sin16( ( 1.0f - t ) * omega ) * sinom;
		scale1 = Math::Sin16( t * omega ) * sinom;
#endif
	} else {
		scale0 = 1.0f - t;
		scale1 = t;
	}

	*this = ( scale0 * from ) + ( scale1 * temp );
	return *this;
}

/*
=====================
idQuat::IsNormalized
=====================
*/
bool idQuat::IsNormalized( const FLOAT tolerance ) const
{
	return (fabs( this->GetLength() - 1.0f ) <= tolerance);
}

/*
=============
CQuat::ToRotation
=============
*/
Rotation CQuat::ToRotation( void ) const {
	return this->ToQuat().ToRotation();
}

/*
=============
CQuat::ToMat3
=============
*/
Matrix3 CQuat::ToMat3( void ) const {
	return this->ToQuat().ToMat3();
}

/*
=============
CQuat::ToMat4
=============
*/
Matrix4 CQuat::ToMat4( void ) const {
	return this->ToQuat().ToMat4();
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
