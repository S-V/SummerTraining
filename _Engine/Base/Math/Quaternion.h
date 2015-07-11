/*
=============================================================================
	File:	Quaternion.h
	Desc:	Quaternion class.
	Note:	Originally written by Id Software.
	Copyright (C) 2004 Id Software, Inc. ( idQuat )
=============================================================================
*/

#ifndef __MATH_QUATERNION_H__
#define __MATH_QUATERNION_H__
mxSWIPED("idSoftware, Doom3/Prey SDK");
/*
=============================================================================

	Quaternion

=============================================================================
*/



class Vec3D;
class Vec4D;
class Angles;
class Rotation;
class Matrix3;
class Matrix4;
class CQuat;

//
//	idQuat
//
mxPREALIGN(16) struct idQuat {
public:
	union {
		struct {
			FLOAT	x;
			FLOAT	y;
			FLOAT	z;
			FLOAT	w;
		};
		Vector4		quad;
	};

public:
					idQuat( void );
					idQuat( FLOAT x, FLOAT y, FLOAT z, FLOAT w );
					idQuat( const Vec3D& axis, FLOAT angle );
					explicit idQuat( EInitIdentity );

	void 			Set( FLOAT x, FLOAT y, FLOAT z, FLOAT w );
	void			SetZero();
	void			SetIdentity();

	FLOAT			operator[]( INT index ) const;
	FLOAT &			operator[]( INT index );
	idQuat			operator-() const;
	idQuat &			operator=( const idQuat &a );
	idQuat			operator+( const idQuat &a ) const;
	idQuat &			operator+=( const idQuat &a );
	idQuat			operator-( const idQuat &a ) const;
	idQuat &			operator-=( const idQuat &a );
	idQuat			operator*( const idQuat &a ) const;
	Vec3D			operator*( const Vec3D &a ) const;
	idQuat			operator*( FLOAT a ) const;
	idQuat &			operator*=( const idQuat &a );
	idQuat &			operator*=( FLOAT a );

	friend idQuat		operator*( const FLOAT a, const idQuat &b );
	friend Vec3D	operator*( const Vec3D &a, const idQuat &b );

	bool			Compare( const idQuat &a ) const;						// exact compare, no epsilon
	bool			Compare( const idQuat &a, const FLOAT epsilon ) const;	// compare with epsilon
	bool			operator==(	const idQuat &a ) const;					// exact compare, no epsilon
	bool			operator!=(	const idQuat &a ) const;					// exact compare, no epsilon

	idQuat			Inverse( void ) const;
	FLOAT			GetLength( void ) const;
	FLOAT			LengthSq( void ) const;
	idQuat &			Normalize( void );
	void			NormalizeFast( void );	// unsafe

	FLOAT			CalcW( void ) const;
	INT				GetDimension( void ) const;

	Rotation		ToRotation( void ) const;
	Matrix3			ToMat3( void ) const;	// NOTE: this quaternion must be normalized.
	Matrix4			ToMat4( void ) const;	// NOTE: this quaternion must be normalized.
	CQuat			ToCQuat( void ) const;
	Vec3D			ToAngularVelocity( void ) const;
	const FLOAT *	ToPtr( void ) const;
	FLOAT *			ToPtr( void );

	const Vec4D &	ToVec4( void ) const;
	Vec4D &			ToVec4( void );

					// Spherical interpolation for two quaternions.
	idQuat &			Slerp( const idQuat &from, const idQuat &to, FLOAT t );

					// Constructs a direction vector from this quaternion.
	Vec3D			ToDirection() const;

	// Testing & Debugging.

	bool	IsNormalized( const FLOAT tolerance = 0.0f ) const;

public:
	static const idQuat	quat_zero;		// Addition identity quaternion.
	static const idQuat	quat_identity;	// Multiplication identity quaternion.

mxTODO(" add quaternions, representing common rotations (e.g. 'static idQuat::quat_rotXY90')");
};

//mxDECLARE_STRUCT( idQuat );

mxFORCEINLINE idQuat::idQuat( void ) {
}

mxFORCEINLINE idQuat::idQuat( FLOAT x, FLOAT y, FLOAT z, FLOAT w ) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

mxFORCEINLINE idQuat::idQuat( const Vec3D& axis, FLOAT angle )
{
	mxASSERT( axis.IsNormalized() );
	FLOAT halfAngle = angle * 0.5f;
	FLOAT sine, cosine;
	Float_SinCos( halfAngle, sine, cosine );

	this->x = sine * axis.x;
	this->y = sine * axis.y;
	this->z = sine * axis.z;
	this->w = cosine;
}

mxFORCEINLINE idQuat::idQuat( EInitIdentity )
{
	this->x = 0.0f;
	this->y = 0.0f;
	this->z = 0.0f;
	this->w = 1.0f;
}

mxFORCEINLINE FLOAT idQuat::operator[]( INT index ) const {
	mxASSERT( ( index >= 0 ) && ( index < 4 ) );
	return ( &x )[ index ];
}

mxFORCEINLINE FLOAT& idQuat::operator[]( INT index ) {
	mxASSERT( ( index >= 0 ) && ( index < 4 ) );
	return ( &x )[ index ];
}

mxFORCEINLINE idQuat idQuat::operator-() const {
	return idQuat( -x, -y, -z, -w );
}

mxFORCEINLINE idQuat &idQuat::operator=( const idQuat &a ) {
	x = a.x;
	y = a.y;
	z = a.z;
	w = a.w;

	return *this;
}

mxFORCEINLINE idQuat idQuat::operator+( const idQuat &a ) const {
	return idQuat( x + a.x, y + a.y, z + a.z, w + a.w );
}

mxFORCEINLINE idQuat& idQuat::operator+=( const idQuat &a ) {
	x += a.x;
	y += a.y;
	z += a.z;
	w += a.w;

	return *this;
}

mxFORCEINLINE idQuat idQuat::operator-( const idQuat &a ) const {
	return idQuat( x - a.x, y - a.y, z - a.z, w - a.w );
}

mxFORCEINLINE idQuat& idQuat::operator-=( const idQuat &a ) {
	x -= a.x;
	y -= a.y;
	z -= a.z;
	w -= a.w;

	return *this;
}

mxFORCEINLINE idQuat idQuat::operator*( const idQuat &a ) const {
	return idQuat(	w*a.x + x*a.w + y*a.z - z*a.y,
					w*a.y + y*a.w + z*a.x - x*a.z,
					w*a.z + z*a.w + x*a.y - y*a.x,
					w*a.w - x*a.x - y*a.y - z*a.z );
}

mxFORCEINLINE Vec3D idQuat::operator*( const Vec3D &a ) const {
#if 0
	mxOPTIMIZE( is it faster to do the conversion to a 3x3 matrix and multiply the vector by this 3x3 matrix? )
	return ( ToMat3() * a );
#else
	// result = this->Inverse() * idQuat( a.x, a.y, a.z, 0.0f ) * (*this)
	FLOAT xxzz = x*x - z*z;
	FLOAT wwyy = w*w - y*y;

	FLOAT xw2 = x*w*2.0f;
	FLOAT xy2 = x*y*2.0f;
	FLOAT xz2 = x*z*2.0f;
	FLOAT yw2 = y*w*2.0f;
	FLOAT yz2 = y*z*2.0f;
	FLOAT zw2 = z*w*2.0f;

	return Vec3D(
		(xxzz + wwyy)*a.x		+ (xy2 + zw2)*a.y		+ (xz2 - yw2)*a.z,
		(xy2 - zw2)*a.x			+ (y*y+w*w-x*x-z*z)*a.y	+ (yz2 + xw2)*a.z,
		(xz2 + yw2)*a.x			+ (yz2 - xw2)*a.y		+ (wwyy - xxzz)*a.z
	);
#endif
}

mxFORCEINLINE idQuat idQuat::operator*( FLOAT a ) const {
	return idQuat( x * a, y * a, z * a, w * a );
}

mxFORCEINLINE idQuat operator*( const FLOAT a, const idQuat &b ) {
	return b * a;
}

mxFORCEINLINE Vec3D operator*( const Vec3D &a, const idQuat &b ) {
	return b * a;
}

mxFORCEINLINE idQuat& idQuat::operator*=( const idQuat &a ) {
	*this = *this * a;

	return *this;
}

mxFORCEINLINE idQuat& idQuat::operator*=( FLOAT a ) {
	x *= a;
	y *= a;
	z *= a;
	w *= a;

	return *this;
}

mxFORCEINLINE bool idQuat::Compare( const idQuat &a ) const {
	return ( ( x == a.x ) && ( y == a.y ) && ( z == a.z ) && ( w == a.w ) );
}

mxFORCEINLINE bool idQuat::Compare( const idQuat &a, const FLOAT epsilon ) const {
	if ( fabs( x - a.x ) > epsilon ) {
		return false;
	}
	if ( fabs( y - a.y ) > epsilon ) {
		return false;
	}
	if ( fabs( z - a.z ) > epsilon ) {
		return false;
	}
	if ( fabs( w - a.w ) > epsilon ) {
		return false;
	}
	return true;
}

mxFORCEINLINE bool idQuat::operator==( const idQuat &a ) const {
	return this->Compare( a );
}

mxFORCEINLINE bool idQuat::operator!=( const idQuat &a ) const {
	return !this->Compare( a );
}

mxFORCEINLINE void idQuat::Set( FLOAT x, FLOAT y, FLOAT z, FLOAT w ) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

mxFORCEINLINE void idQuat::SetZero() {
	*this = idQuat::quat_zero;
}

mxFORCEINLINE void idQuat::SetIdentity() {
	*this = quat_identity;
}

mxFORCEINLINE idQuat idQuat::Inverse( void ) const {
	return idQuat( -x, -y, -z, w );
}

mxFORCEINLINE FLOAT idQuat::LengthSq( void ) const {
	return ( x * x + y * y + z * z + w * w );
}

mxFORCEINLINE FLOAT idQuat::GetLength( void ) const {
	FLOAT len;

	len = x * x + y * y + z * z + w * w;
	return Float_Sqrt( len );
}

mxFORCEINLINE idQuat& idQuat::Normalize( void )
{
	FLOAT len;
	FLOAT ilength;

	len = this->GetLength();
	if ( len ) {
		ilength = 1.0f / len;
		x *= ilength;
		y *= ilength;
		z *= ilength;
		w *= ilength;
	}
	return *this;
}

mxFORCEINLINE void idQuat::NormalizeFast( void )
{
	FLOAT invLength = Float_InvSqrtEst( this->LengthSq() );
	x *= invLength;
	y *= invLength;
	z *= invLength;
	w *= invLength;
}

mxFORCEINLINE FLOAT idQuat::CalcW( void ) const {
	// take the absolute value because floating point rounding may cause the dot of x,y,z to be larger than 1
	return Float_Sqrt( fabs( 1.0f - ( x * x + y * y + z * z ) ) );
}

mxFORCEINLINE INT idQuat::GetDimension( void ) const {
	return 4;
}

mxFORCEINLINE const FLOAT *idQuat::ToPtr( void ) const {
	return &x;
}

mxFORCEINLINE FLOAT *idQuat::ToPtr( void ) {
	return &x;
}

mxFORCEINLINE Vec3D idQuat::ToDirection() const
{
	FLOAT xz = x * z;
	FLOAT yw = y * w;
	FLOAT yz = y * z;
	FLOAT xw = x * w;
	FLOAT xx = x * x;
	FLOAT yy = y * y;

	return Vec3D(
		2.0f * ( xz - yw ),
		2.0f * ( yz + xw ),
		1.0f - 2.0f * ( xx + yy )
	);
}

mxFORCEINLINE const Vec4D &idQuat::ToVec4( void ) const {
	return *reinterpret_cast<const Vec4D *>(&x);
}

mxFORCEINLINE Vec4D &idQuat::ToVec4( void ) {
	return *reinterpret_cast<Vec4D *>(&x);
}


//===========================================================================

// Multiply two quaternions.
//
mxFORCEINLINE idQuat MulQuatQuat( const idQuat& q1, const idQuat& q2 )
{
	return idQuat(
		q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
		q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z,
		q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x,
		q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z );
}

// Multiply a quaternion with a vector.
//
mxFORCEINLINE idQuat MulQuatVec3( const idQuat& q, const Vec3D& w )
{
	return idQuat(
		q.w * w.x + q.y * w.z - q.z * w.y,
		q.w * w.y + q.z * w.x - q.x * w.z,
		q.w * w.z + q.x * w.y - q.y * w.x,
		-q.x * w.x - q.y * w.y - q.z * w.z );
}

// Multiply a vector with a quaternion.
//
mxFORCEINLINE idQuat MulVec3Quat( const Vec3D& w, const idQuat& q )
{
	return idQuat(
		w.x * q.w + w.y * q.z - w.z * q.y,
		w.y * q.w + w.z * q.x - w.x * q.z,
		w.z * q.w + w.x * q.y - w.y * q.x,
		-w.x * q.x - w.y * q.y - w.z * q.z );
}

/*
=============================================================================

	Compressed quaternion

=============================================================================
*/

class CQuat {
public:
	FLOAT			x;
	FLOAT			y;
	FLOAT			z;

public:
					CQuat( void );
					CQuat( FLOAT x, FLOAT y, FLOAT z );

	void 			Set( FLOAT x, FLOAT y, FLOAT z );

	FLOAT			operator[]( INT index ) const;
	FLOAT &			operator[]( INT index );

	bool			Compare( const CQuat &a ) const;						// exact compare, no epsilon
	bool			Compare( const CQuat &a, const FLOAT epsilon ) const;	// compare with epsilon
	bool			operator==(	const CQuat &a ) const;					// exact compare, no epsilon
	bool			operator!=(	const CQuat &a ) const;					// exact compare, no epsilon

	INT				GetDimension( void ) const;

	Rotation		ToRotation( void ) const;
	Matrix3			ToMat3( void ) const;
	Matrix4			ToMat4( void ) const;
	idQuat			ToQuat( void ) const;
	const FLOAT *	ToPtr( void ) const;
	FLOAT *			ToPtr( void );
};

mxFORCEINLINE CQuat::CQuat( void ) {
}

mxFORCEINLINE CQuat::CQuat( FLOAT x, FLOAT y, FLOAT z ) {
	this->x = x;
	this->y = y;
	this->z = z;
}

mxFORCEINLINE void CQuat::Set( FLOAT x, FLOAT y, FLOAT z ) {
	this->x = x;
	this->y = y;
	this->z = z;
}

mxFORCEINLINE FLOAT CQuat::operator[]( INT index ) const {
	mxASSERT( ( index >= 0 ) && ( index < 3 ) );
	return ( &x )[ index ];
}

mxFORCEINLINE FLOAT& CQuat::operator[]( INT index ) {
	mxASSERT( ( index >= 0 ) && ( index < 3 ) );
	return ( &x )[ index ];
}

mxFORCEINLINE bool CQuat::Compare( const CQuat &a ) const {
	return ( ( x == a.x ) && ( y == a.y ) && ( z == a.z ) );
}

mxFORCEINLINE bool CQuat::Compare( const CQuat &a, const FLOAT epsilon ) const {
	if ( fabs( x - a.x ) > epsilon ) {
		return false;
	}
	if ( fabs( y - a.y ) > epsilon ) {
		return false;
	}
	if ( fabs( z - a.z ) > epsilon ) {
		return false;
	}
	return true;
}

mxFORCEINLINE bool CQuat::operator==( const CQuat &a ) const {
	return Compare( a );
}

mxFORCEINLINE bool CQuat::operator!=( const CQuat &a ) const {
	return !Compare( a );
}

mxFORCEINLINE INT CQuat::GetDimension( void ) const {
	return 3;
}

mxFORCEINLINE idQuat CQuat::ToQuat( void ) const
{
	// Take the absolute value because floating point rounding may cause the dot of x,y,z to be larger than 1.
	return idQuat( x, y, z, Float_Sqrt( fabs( 1.0f - ( x * x + y * y + z * z ) ) ) );
}

mxFORCEINLINE const FLOAT *CQuat::ToPtr( void ) const {
	return &x;
}

mxFORCEINLINE FLOAT *CQuat::ToPtr( void ) {
	return &x;
}

mxSWIPED("DXSDK, DXUT, CD3DArcBall");
mxFORCEINLINE
idQuat QuatFromBallPoints( const Vec3D& vFrom, const Vec3D& vTo )
{
	float	fDot = Dot( vFrom, vTo );
	Vec3D	vPart = Cross( vFrom, vTo ).GetNormalized();
	return idQuat( vPart.x, vPart.y, vPart.z, fDot );
}



#endif /* !__MATH_QUATERNION_H__ */

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
