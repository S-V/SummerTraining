/*
=============================================================================
	File:	Vec4D.h
	Desc:	4D vector.
			Copyright (C) 2004 Id Software, Inc. ( idVec4 )
=============================================================================
*/

#ifndef __MATH_VEC4D_H__
#define __MATH_VEC4D_H__
mxSWIPED("idSoftware, Doom3/Prey/Quake4 SDKs");


class Angles;
class Polar3;
class Matrix3;

//
//	Vec4D
//
mxPREALIGN(16) struct Vec4D {
public:
	union {
		struct {
			FLOAT	x;
			FLOAT	y;
			FLOAT	z;
			FLOAT	w;
		};
		Vector4		q;
		FLOAT		f[4];
		struct {
			FLOAT	r;
			FLOAT	g;
			FLOAT	b;
			FLOAT	a;
		};
	};

public:
					Vec4D( void );
					explicit Vec4D( EInitZero );
					explicit Vec4D( EInitIdentity );
					explicit Vec4D( EInitInfinity );
					explicit Vec4D( const FLOAT x, const FLOAT y, const FLOAT z, const FLOAT w );
					explicit Vec4D( const Vec2D& xy, const Vec2D& zw );
					explicit Vec4D( const Vec3D& xyz, const FLOAT w );
					explicit Vec4D( const FLOAT xyzw );

	void 			Set( const FLOAT x, const FLOAT y, const FLOAT z, const FLOAT w );
	void 			Set( const Vec3D& xyz, FLOAT w );
	void			SetZero( void );

	FLOAT			operator[]( const INT index ) const;
	FLOAT &			operator[]( const INT index );
	const Vec4D		operator-() const;
	FLOAT			operator*( const Vec4D &a ) const;
	const Vec4D		operator*( const FLOAT a ) const;
	const Vec4D		operator/( const FLOAT a ) const;
	const Vec4D		operator+( const Vec4D &a ) const;
	const Vec4D		operator-( const Vec4D &a ) const;
	Vec4D &			operator+=( const Vec4D &a );
	Vec4D &			operator-=( const Vec4D &a );
	Vec4D &			operator/=( const Vec4D &a );
	Vec4D &			operator/=( const FLOAT a );
	Vec4D &			operator*=( const FLOAT a );

	friend const Vec4D	operator*( const FLOAT a, const Vec4D& b );

	bool			Compare( const Vec4D &a ) const;							// exact compare, no epsilon
	bool			Compare( const Vec4D &a, const FLOAT epsilon ) const;		// compare with epsilon
	bool			operator==(	const Vec4D &a ) const;						// exact compare, no epsilon
	bool			operator!=(	const Vec4D &a ) const;						// exact compare, no epsilon

	FLOAT			GetLength( void ) const;
	FLOAT			LengthSqr( void ) const;
	FLOAT			Normalize( void );			// returns length
	FLOAT			NormalizeFast( void );		// returns length

	INT				GetDimension( void ) const;

	const Vec2D &	ToVec2( void ) const;
	Vec2D &			ToVec2( void );
	const Vec3D &	ToVec3( void ) const;
	Vec3D &			ToVec3( void );
	const FLOAT *	ToPtr( void ) const;
	FLOAT *			ToPtr( void );

	void			Lerp( const Vec4D &v1, const Vec4D &v2, const FLOAT l );

	Vec4D & 		RotateAboutX( FLOAT angle );
	Vec4D & 		RotateAboutY( FLOAT angle );
	Vec4D & 		RotateAboutZ( FLOAT angle );
	Vec4D & 		RotateAboutAxis( FLOAT angle, const Vec3D& axis );

	template< typename TYPE >
	mxFORCEINLINE Vec4D & operator = ( const TYPE& other )
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
		return *this;
	}

public:
	static const Vec4D vec4_zero;
	static const Vec4D vec4_unit;
};

mxDECLARE_STRUCT( Vec4D );

mxFORCEINLINE Vec4D::Vec4D( void ) {
}

mxFORCEINLINE Vec4D::Vec4D( EInitZero )
{
	this->x = 0.0f;
	this->y = 0.0f;
	this->z = 0.0f;
	this->w = 0.0f;
}

mxFORCEINLINE Vec4D::Vec4D( EInitIdentity )
{
	this->x = 1.0f;
	this->y = 1.0f;
	this->z = 1.0f;
	this->w = 1.0f;
}

mxFORCEINLINE Vec4D::Vec4D( EInitInfinity )
{
	this->x = BIG_NUMBER;
	this->y = BIG_NUMBER;
	this->z = BIG_NUMBER;
	this->w = BIG_NUMBER;
}

mxFORCEINLINE Vec4D::Vec4D( const FLOAT x, const FLOAT y, const FLOAT z, const FLOAT w ) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

mxFORCEINLINE Vec4D::Vec4D( const Vec2D& xy, const Vec2D& zw ) {
	this->x = xy.x;
	this->y = xy.y;
	this->z = zw.x;
	this->w = zw.y;
}

mxFORCEINLINE Vec4D::Vec4D( const Vec3D& xyz, const FLOAT w ) {
	this->x = xyz.x;
	this->y = xyz.y;
	this->z = xyz.z;
	this->w = w;
}

mxFORCEINLINE Vec4D::Vec4D( const FLOAT xyzw ) {
	this->x = xyzw;
	this->y = xyzw;
	this->z = xyzw;
	this->w = xyzw;
}

mxFORCEINLINE void Vec4D::Set( const FLOAT x, const FLOAT y, const FLOAT z, const FLOAT w ) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

mxFORCEINLINE void Vec4D::Set( const Vec3D& xyz, FLOAT w )
{
	this->x = xyz.x;
	this->y = xyz.y;
	this->z = xyz.z;
	this->w = w;
}

mxFORCEINLINE void Vec4D::SetZero( void ) {
	x = y = z = w = 0.0f;
}

mxFORCEINLINE FLOAT Vec4D::operator[]( INT index ) const {
	return ( &x )[ index ];
}

mxFORCEINLINE FLOAT& Vec4D::operator[]( INT index ) {
	return ( &x )[ index ];
}

mxFORCEINLINE const Vec4D Vec4D::operator-() const {
	return Vec4D( -x, -y, -z, -w );
}

mxFORCEINLINE const Vec4D Vec4D::operator-( const Vec4D &a ) const {
	return Vec4D( x - a.x, y - a.y, z - a.z, w - a.w );
}

mxFORCEINLINE FLOAT Vec4D::operator*( const Vec4D &a ) const {
	return x * a.x + y * a.y + z * a.z + w * a.w;
}

mxFORCEINLINE const Vec4D Vec4D::operator*( const FLOAT a ) const {
	return Vec4D( x * a, y * a, z * a, w * a );
}

mxFORCEINLINE const Vec4D Vec4D::operator/( const FLOAT a ) const {
	FLOAT inva = 1.0f / a;
	return Vec4D( x * inva, y * inva, z * inva, w * inva );
}

mxFORCEINLINE const Vec4D operator*( const FLOAT a, const Vec4D& b ) {
	return Vec4D( b.x * a, b.y * a, b.z * a, b.w * a );
}

mxFORCEINLINE const Vec4D Vec4D::operator+( const Vec4D &a ) const {
	return Vec4D( x + a.x, y + a.y, z + a.z, w + a.w );
}

mxFORCEINLINE Vec4D &Vec4D::operator+=( const Vec4D &a ) {
	x += a.x;
	y += a.y;
	z += a.z;
	w += a.w;

	return *this;
}

mxFORCEINLINE Vec4D &Vec4D::operator/=( const Vec4D &a ) {
	x /= a.x;
	y /= a.y;
	z /= a.z;
	w /= a.w;

	return *this;
}

mxFORCEINLINE Vec4D &Vec4D::operator/=( const FLOAT a ) {
	FLOAT inva = 1.0f / a;
	x *= inva;
	y *= inva;
	z *= inva;
	w *= inva;

	return *this;
}

mxFORCEINLINE Vec4D &Vec4D::operator-=( const Vec4D &a ) {
	x -= a.x;
	y -= a.y;
	z -= a.z;
	w -= a.w;

	return *this;
}

mxFORCEINLINE Vec4D &Vec4D::operator*=( const FLOAT a ) {
	x *= a;
	y *= a;
	z *= a;
	w *= a;

	return *this;
}

mxFORCEINLINE bool Vec4D::Compare( const Vec4D &a ) const {
	return ( ( x == a.x ) && ( y == a.y ) && ( z == a.z ) && w == a.w );
}

mxFORCEINLINE bool Vec4D::Compare( const Vec4D &a, const FLOAT epsilon ) const {
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

mxFORCEINLINE bool Vec4D::operator==( const Vec4D &a ) const {
	return Compare( a );
}

mxFORCEINLINE bool Vec4D::operator!=( const Vec4D &a ) const {
	return !Compare( a );
}

mxFORCEINLINE FLOAT Vec4D::GetLength( void ) const {
	return ( FLOAT )Float_Sqrt( x * x + y * y + z * z + w * w );
}

mxFORCEINLINE FLOAT Vec4D::LengthSqr( void ) const {
	return ( x * x + y * y + z * z + w * w );
}

mxFORCEINLINE FLOAT Vec4D::Normalize( void ) {
	FLOAT sqrLength, invLength;

	sqrLength = x * x + y * y + z * z + w * w;
	invLength = Float_InvSqrt( sqrLength );
	x *= invLength;
	y *= invLength;
	z *= invLength;
	w *= invLength;
	return invLength * sqrLength;
}

mxFORCEINLINE FLOAT Vec4D::NormalizeFast( void ) {
	FLOAT sqrLength, invLength;

	sqrLength = x * x + y * y + z * z + w * w;
	invLength = Float_InvSqrtEst( sqrLength );
	x *= invLength;
	y *= invLength;
	z *= invLength;
	w *= invLength;
	return invLength * sqrLength;
}

mxFORCEINLINE INT Vec4D::GetDimension( void ) const {
	return 4;
}

mxFORCEINLINE const Vec2D &Vec4D::ToVec2( void ) const {
	return *reinterpret_cast<const Vec2D *>(this);
}

mxFORCEINLINE Vec2D &Vec4D::ToVec2( void ) {
	return *reinterpret_cast<Vec2D *>(this);
}

mxFORCEINLINE const Vec3D &Vec4D::ToVec3( void ) const {
	return *reinterpret_cast<const Vec3D *>(this);
}

mxFORCEINLINE Vec3D &Vec4D::ToVec3( void ) {
	return *reinterpret_cast<Vec3D *>(this);
}

mxFORCEINLINE const FLOAT *Vec4D::ToPtr( void ) const {
	return &x;
}

mxFORCEINLINE FLOAT *Vec4D::ToPtr( void ) {
	return &x;
}



#endif /* !__MATH_VEC4D_H__ */

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
