/*
=============================================================================
	File:	Rotation.h
	Desc:	Class for representing rotations around an arbitrary axis.
	Note:	Originally written by Id Software.
	Copyright (C) 2004 Id Software, Inc. ( idRotation )
=============================================================================
*/

#ifndef __MATH_ROTATION_H__
#define __MATH_ROTATION_H__
mxSWIPED("idSoftware, Doom3/Prey/Quake4 SDKs");
/*
=============================================================================

	Describes a complete rotation in degrees about an arbitrary axis.
	A local rotation matrix is stored for fast rotation of multiple points.

=============================================================================
*/



class Angles;
class idQuat;
class Matrix3;

//
//	Rotation
//
class Rotation {

	friend class Angles;
	friend class idQuat;
	friend class Matrix3;

public:
						Rotation( void );
						Rotation( const Vec3D &rotationOrigin, const Vec3D &rotationVec, const FLOAT rotationAngle );

	void				Set( const Vec3D &rotationOrigin, const Vec3D &rotationVec, const FLOAT rotationAngle );
	void				SetOrigin( const Vec3D &rotationOrigin );
	void				SetVec( const Vec3D &rotationVec );					// has to be normalized
	void				SetVec( const FLOAT x, const FLOAT y, const FLOAT z );	// has to be normalized
	void				SetAngle( const FLOAT rotationAngle );
	void				Scale( const FLOAT s );
	void				ReCalculateMatrix( void );
	const Vec3D &		GetOrigin( void ) const;
	const Vec3D &		GetVec( void ) const;
	FLOAT				GetAngle( void ) const;

	Rotation			operator-() const;										// flips rotation
	Rotation			operator*( const FLOAT s ) const;						// scale rotation
	Rotation			operator/( const FLOAT s ) const;						// scale rotation
	Rotation &			operator*=( const FLOAT s );							// scale rotation
	Rotation &			operator/=( const FLOAT s );							// scale rotation
	Vec3D				operator*( const Vec3D &v ) const;						// rotate vector

	friend Rotation		operator*( const FLOAT s, const Rotation &r );		// scale rotation
	friend Vec3D		operator*( const Vec3D &v, const Rotation &r );		// rotate vector
	friend Vec3D &		operator*=( Vec3D &v, const Rotation &r );			// rotate vector

	idQuat				ToQuat( void ) const;
	const Matrix3 &		ToMat3( void ) const;
	Matrix4				ToMat4( void ) const;
	Vec3D				ToAngularVelocity( void ) const;

	void				RotatePoint( Vec3D &point ) const;

	void				Normalize180( void );
	void				Normalize360( void );

private:
	Vec3D				origin;			// origin of rotation
	Vec3D				vec;			// normalized vector to rotate around
	FLOAT				angle;			// angle of rotation in degrees
	mutable Matrix3		axis;			// rotation axis
	mutable bool		axisValid;		// true if rotation axis is valid
};


mxFORCEINLINE Rotation::Rotation( void ) {
}

mxFORCEINLINE Rotation::Rotation( const Vec3D &rotationOrigin, const Vec3D &rotationVec, const FLOAT rotationAngle ) {
	mxASSERT( rotationVec.IsNormalized() );
	this->origin	= rotationOrigin;
	this->vec		= rotationVec;
	this->angle		= rotationAngle;
	this->axisValid = false;
}

mxFORCEINLINE void Rotation::Set( const Vec3D &rotationOrigin, const Vec3D &rotationVec, const FLOAT rotationAngle ) {
	mxASSERT( rotationVec.IsNormalized() );
	this->origin	= rotationOrigin;
	this->vec		= rotationVec;
	this->angle		= rotationAngle;
	this->axisValid = false;
}

mxFORCEINLINE void Rotation::SetOrigin( const Vec3D &rotationOrigin ) {
	origin = rotationOrigin;
}

mxFORCEINLINE void Rotation::SetVec( const Vec3D &rotationVec ) {
	vec = rotationVec;
	axisValid = false;
}

mxFORCEINLINE void Rotation::SetVec( FLOAT x, FLOAT y, FLOAT z ) {
	vec[0] = x;
	vec[1] = y;
	vec[2] = z;
	axisValid = false;
}

mxFORCEINLINE void Rotation::SetAngle( const FLOAT rotationAngle ) {
	angle = rotationAngle;
	axisValid = false;
}

mxFORCEINLINE void Rotation::Scale( const FLOAT s ) {
	angle *= s;
	axisValid = false;
}

mxFORCEINLINE void Rotation::ReCalculateMatrix( void ) {
	axisValid = false;
	ToMat3();
}

mxFORCEINLINE const Vec3D &Rotation::GetOrigin( void ) const {
	return origin;
}

mxFORCEINLINE const Vec3D &Rotation::GetVec( void ) const  {
	return vec;
}

mxFORCEINLINE FLOAT Rotation::GetAngle( void ) const  {
	return angle;
}

mxFORCEINLINE Rotation Rotation::operator-() const {
	return Rotation( origin, vec, -angle );
}

mxFORCEINLINE Rotation Rotation::operator*( const FLOAT s ) const {
	return Rotation( origin, vec, angle * s );
}

mxFORCEINLINE Rotation Rotation::operator/( const FLOAT s ) const {
	mxASSERT( s != 0.0f );
	return Rotation( origin, vec, angle / s );
}

mxFORCEINLINE Rotation &Rotation::operator*=( const FLOAT s ) {
	angle *= s;
	axisValid = false;
	return *this;
}

mxFORCEINLINE Rotation &Rotation::operator/=( const FLOAT s ) {
	mxASSERT( s != 0.0f );
	angle /= s;
	axisValid = false;
	return *this;
}

mxFORCEINLINE Vec3D Rotation::operator*( const Vec3D &v ) const {
	if ( !axisValid ) {
		ToMat3();
	}
	return ((v - origin) * axis + origin);
}

mxFORCEINLINE Rotation operator*( const FLOAT s, const Rotation &r ) {
	return r * s;
}

mxFORCEINLINE Vec3D operator*( const Vec3D &v, const Rotation &r ) {
	return r * v;
}

mxFORCEINLINE Vec3D &operator*=( Vec3D &v, const Rotation &r ) {
	v = r * v;
	return v;
}

mxFORCEINLINE void Rotation::RotatePoint( Vec3D &point ) const {
	if ( !axisValid ) {
		ToMat3();
	}
	point = ((point - origin) * axis + origin);
}



#endif /* !__MATH_ROTATION_H__ */

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
