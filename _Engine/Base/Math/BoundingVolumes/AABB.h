/*
=============================================================================
	File:	AABB.h
	Desc:	Axis Aligned Bounding Box.
=============================================================================
*/

#ifndef __BOUNDING_VOLUMES_AABB_H__
#define __BOUNDING_VOLUMES_AABB_H__

#include <Base/Object/BaseType.h>

mxSWIPED("Id Software, Doom3/Prey SDK");


//
//	AABB- Axis Aligned Bounding Box.
//	
//	This AABB consists of two points ( the minima and maxima ).
//
class AABB : public CStruct {
public:
	Vec3D	mPoints[ 2 ];	// min and max points.

public:
	mxDECLARE_CLASS( AABB, CStruct );
	mxDECLARE_REFLECTION;

					AABB( void );
					explicit AABB( const Vec3D &mins, const Vec3D &maxs );
					explicit AABB( const Vec3D &point );
					explicit AABB( EInitInfinity );

	Vec3D &			GetMin();
	const Vec3D &	GetMin() const;

	Vec3D &			GetMax();
	const Vec3D &	GetMax() const;

	//// Enumeration for referring to features by name rather than by number.
	//enum
	//{
	//	MIN_POINT = 0,
	//	MAX_POINT = 1,
	//	NUM_POINTS = 2,	// NUM_POINTS is defined in shlobj.h
	//};

	const Vec3D &	operator[]( const UINT index ) const;
	Vec3D &			operator[]( const UINT index );

	AABB			operator+( const Vec3D &t ) const;				// returns translated bounds
	AABB &			operator+=( const Vec3D &t );					// translate the bounds
	
	AABB			operator*( const Matrix3 &r ) const;				// returns rotated bounds
	AABB &			operator*=( const Matrix3 &r );					// rotate the bounds
	
	AABB			operator+( const AABB &a ) const;
	AABB &			operator+=( const AABB &a );
	
	AABB			operator-( const AABB &a ) const;
	AABB &			operator-=( const AABB &a );

	bool			Compare( const AABB &a ) const;							// exact compare, no epsilon
	bool			Compare( const AABB &a, const FLOAT epsilon ) const;	// compare with epsilon
	bool			operator==(	const AABB &a ) const;						// exact compare, no epsilon
	bool			operator!=(	const AABB &a ) const;						// exact compare, no epsilon

	void			Clear( void );									// inside out bounds
	void			SetZero( void );								// single point at origin
	void			SetInfinity( void );
	void			Set( const Vec3D& minPoint, const Vec3D& maxPoint );

	Vec3D			GetCenter( void ) const;						// returns center of bounds
	FLOAT			GetRadius( void ) const;						// returns the radius relative to the bounds origin
	FLOAT			GetRadius( const Vec3D &center ) const;			// returns the radius relative to the given center
	FLOAT			GetVolume( void ) const;						// returns the volume of the bounds
	bool			IsCleared( void ) const;						// returns true if bounds are inside out

	bool			AddPoint( const Vec3D &v );						// add the point, returns true if the bounds expanded
	bool			AddBounds( const AABB &a );						// add the bounds, returns true if the bounds expanded
	AABB			Intersect( const AABB &a ) const;				// return intersection of this bounds with the given bounds
	AABB &			IntersectSelf( const AABB &a );					// intersect this bounds with the given bounds
	AABB			Expand( const FLOAT d ) const;					// return bounds expanded in all directions with the given value
	AABB &			ExpandSelf( const FLOAT d );					// expand bounds in all directions with the given value

	AABB &			ExpandSelf( const Vec3D &d );					// expand bounds in all directions by the given vector
	const Vec3D		Size( void ) const;								// Returns the extent of this box ( maxPoint - minPoint ).
	const Vec3D		GetHalfSize( void ) const;						// Returns the half extent of this box ( maxPoint - minPoint ) / 2.
	FLOAT			ShortestDistanceSquared( const Vec3D &point ) const;
	FLOAT			ShortestDistance( const Vec3D &point ) const;
	bool			Contains( const AABB &a ) const;
	UINT			GetLargestAxis( void ) const;
	Vec3D			FindEdgePoint( const Vec3D& dir ) const;
	Vec3D			FindEdgePoint( const Vec3D& start, const Vec3D& dir ) const;
	Vec3D			FindVectorToEdge( const Vec3D& dir ) const;
	Vec3D			FindVectorToEdge( const Vec3D& start, const Vec3D& dir ) const;

	AABB			Trasform( const Matrix4& transform ) const;
	AABB &			TrasformSelf( const Matrix4& transform );
	AABB			Translate( const Vec3D &translation ) const;	// return translated bounds
	AABB &			TranslateSelf( const Vec3D &translation );		// translate this bounds
	AABB			Rotate( const Matrix3 &rotation ) const;		// return rotated bounds
	AABB &			RotateSelf( const Matrix3 &rotation );			// rotate this bounds

	FLOAT			PlaneDistance( const Plane3D &plane ) const;
	EPlaneSide		PlaneSide( const Plane3D &plane, const FLOAT epsilon = ON_EPSILON ) const;

	bool			ContainsPoint( const Vec3D &p ) const;			// includes touching
	bool			IntersectsBounds( const AABB &a ) const;	// includes touching
	bool			LineIntersection( const Vec3D &start, const Vec3D &end ) const;
					// intersection point is start + dir * scale
	bool			RayIntersection( const Vec3D &start, const Vec3D &dir, FLOAT &scale ) const;

	void			FromSphere( const Sphere& sphere );

					// most tight bounds for the given transformed bounds
	void			FromTransformedBounds( const AABB &bounds, const Vec3D &origin, const Matrix3 &axis );
					// most tight bounds for a point set
	void			FromPoints( const Vec3D *points, const UINT numPoints );
					// most tight bounds for a translation
	void			FromPointTranslation( const Vec3D &point, const Vec3D &translation );
	void			FromBoundsTranslation( const AABB &bounds, const Vec3D &origin, const Matrix3 &axis, const Vec3D &translation );
					// most tight bounds for a rotation
	void			FromPointRotation( const Vec3D &point, const Rotation &rotation );
	void			FromBoundsRotation( const AABB &bounds, const Vec3D &origin, const Matrix3 &axis, const Rotation &rotation );

	Sphere			ToSphere( void ) const;

	void			AxisProjection( const Vec3D &dir, FLOAT &min, FLOAT &max ) const;
	void			AxisProjection( const Vec3D &origin, const Matrix3 &axis, const Vec3D &dir, FLOAT &min, FLOAT &max ) const;

	//------------------------------------------------------------------------------------
	enum EBoxVertexInfo { NUM_CORNERS = 8 };

					// Gets the eight corners of this box.
	void			GetCorners( Vec3D points[NUM_CORNERS] ) const;

					// Returns the vertex normals at each corner of this box.
	const Vec3D *	GetUnitCubeVertexNormals();

	//------------------------------------------------------------------------------------
	enum EBoxEdgeInfo { NUM_EDGES = 12 };
					// Returns the 12 edges of the box ( each edge is a pair of indices into the array of corners of this box ).
	static const UINT *	GetEdges();

	//------------------------------------------------------------------------------------

	// if you're standing in the center of a box
	// the planes' normals are directed outwards...
	enum EBoxPlaneSide
	{
		SIDE_IN_FRONT,
		SIDE_BEHIND,

		SIDE_ABOVE,
		SIDE_BENEATH,

		SIDE_LEFT,
		SIDE_RIGHT,

		NUM_SIDES
	};
	void	GetPlanes( Plane3D outPlanes[NUM_SIDES] ) const;

	//------------------------------------------------------------------------------------

	bool			IsValid() const;	// Consistency check.

public:
	static const AABB	zero_aabb;		// min = max = (0,0,0)
	static const AABB	unit_cube;		// [-0.5 .. +0.5] on each axis
	static const AABB	infinite_aabb;	// min = {-inf}, max = {+inf}
	static const AABB	invalid_aabb;	// min = {+inf}, max = {-inf}
};

mxDECLARE_POD_TYPE(AABB);

mxFORCEINLINE AABB::AABB( void ) {
}

mxFORCEINLINE AABB::AABB( const Vec3D &mins, const Vec3D &maxs ) {
	mPoints[0] = mins;
	mPoints[1] = maxs;
}

mxFORCEINLINE AABB::AABB( const Vec3D &point ) {
	mPoints[0] = point;
	mPoints[1] = point;
}

mxFORCEINLINE AABB::AABB( EInitInfinity ) {
	*this = infinite_aabb;
}

mxFORCEINLINE const Vec3D &AABB::operator[]( const UINT index ) const {
	return mPoints[index];
}

mxFORCEINLINE Vec3D &AABB::operator[]( const UINT index ) {
	return mPoints[index];
}

mxFORCEINLINE AABB AABB::operator+( const Vec3D &t ) const {
	return AABB( mPoints[0] + t, mPoints[1] + t );
}

mxFORCEINLINE AABB &AABB::operator+=( const Vec3D &t ) {
	mPoints[0] += t;
	mPoints[1] += t;
	return *this;
}

mxFORCEINLINE AABB AABB::operator*( const Matrix3 &r ) const {
	AABB bounds;
	bounds.FromTransformedBounds( *this, Vec3D::vec3_zero, r );
	return bounds;
}

mxFORCEINLINE AABB &AABB::operator*=( const Matrix3 &r ) {
	this->FromTransformedBounds( *this, Vec3D::vec3_zero, r );
	return *this;
}

mxFORCEINLINE AABB AABB::operator+( const AABB &a ) const {
	AABB newBounds;
	newBounds = *this;
	newBounds.AddBounds( a );
	return newBounds;
}

mxFORCEINLINE AABB &AABB::operator+=( const AABB &a ) {
	AABB::AddBounds( a );
	return *this;
}

mxFORCEINLINE AABB AABB::operator-( const AABB &a ) const
{
	mxASSERT( mPoints[1][0] - mPoints[0][0] > a.mPoints[1][0] - a.mPoints[0][0] &&
			mPoints[1][1] - mPoints[0][1] > a.mPoints[1][1] - a.mPoints[0][1] &&
			mPoints[1][2] - mPoints[0][2] > a.mPoints[1][2] - a.mPoints[0][2] );

	return AABB( Vec3D( mPoints[0][0] + a.mPoints[1][0], mPoints[0][1] + a.mPoints[1][1], mPoints[0][2] + a.mPoints[1][2] ),
				Vec3D( mPoints[1][0] + a.mPoints[0][0], mPoints[1][1] + a.mPoints[0][1], mPoints[1][2] + a.mPoints[0][2] ) );
}

mxFORCEINLINE AABB &AABB::operator-=( const AABB &a ) {
	mxASSERT( mPoints[1][0] - mPoints[0][0] > a.mPoints[1][0] - a.mPoints[0][0] &&
			mPoints[1][1] - mPoints[0][1] > a.mPoints[1][1] - a.mPoints[0][1] &&
			mPoints[1][2] - mPoints[0][2] > a.mPoints[1][2] - a.mPoints[0][2] );

	mPoints[0] += a.mPoints[1];
	mPoints[1] += a.mPoints[0];
	return *this;
}

mxFORCEINLINE bool AABB::Compare( const AABB &a ) const
{
	return ( mPoints[0].Compare( a.mPoints[0] ) && mPoints[1].Compare( a.mPoints[1] ) );
}

mxFORCEINLINE bool AABB::Compare( const AABB &a, const FLOAT epsilon ) const
{
	return ( mPoints[0].Compare( a.mPoints[0], epsilon ) && mPoints[1].Compare( a.mPoints[1], epsilon ) );
}

mxFORCEINLINE bool AABB::operator==( const AABB &a ) const {
	return Compare( a );
}

mxFORCEINLINE bool AABB::operator!=( const AABB &a ) const {
	return !Compare( a );
}

mxFORCEINLINE void AABB::Clear( void ) {
	mPoints[0][0] = mPoints[0][1] = mPoints[0][2] = +BIG_NUMBER;
	mPoints[1][0] = mPoints[1][1] = mPoints[1][2] = -BIG_NUMBER;
}

mxFORCEINLINE void AABB::SetZero( void ) {
	mPoints[0][0] = mPoints[0][1] = mPoints[0][2] =
	mPoints[1][0] = mPoints[1][1] = mPoints[1][2] = 0;
}

mxFORCEINLINE void AABB::SetInfinity( void ) {
	mPoints[0][0] = mPoints[0][1] = mPoints[0][2] = -BIG_NUMBER;
	mPoints[1][0] = mPoints[1][1] = mPoints[1][2] = +BIG_NUMBER;
}

mxFORCEINLINE void AABB::Set( const Vec3D& minPoint, const Vec3D& maxPoint )
{
	mPoints[0] = minPoint;
	mPoints[1] = maxPoint;
}

mxFORCEINLINE Vec3D AABB::GetCenter( void ) const
{
	return Vec3D(
		( mPoints[1][0] + mPoints[0][0] ) * 0.5f,
		( mPoints[1][1] + mPoints[0][1] ) * 0.5f,
		( mPoints[1][2] + mPoints[0][2] ) * 0.5f
	);
}

mxFORCEINLINE FLOAT AABB::GetVolume( void ) const
{
	if ( mPoints[0][0] >= mPoints[1][0] || mPoints[0][1] >= mPoints[1][1] || mPoints[0][2] >= mPoints[1][2] ) {
		return 0.0f;
	}
	return ( ( mPoints[1][0] - mPoints[0][0] ) * ( mPoints[1][1] - mPoints[0][1] ) * ( mPoints[1][2] - mPoints[0][2] ) );
}

mxFORCEINLINE bool AABB::IsCleared( void ) const {
	return mPoints[0][0] > mPoints[1][0];
}

mxFORCEINLINE bool AABB::AddPoint( const Vec3D &v ) {
	bool expanded = false;
	if ( v[0] < mPoints[0][0]) {
		mPoints[0][0] = v[0];
		expanded = true;
	}
	if ( v[0] > mPoints[1][0]) {
		mPoints[1][0] = v[0];
		expanded = true;
	}
	if ( v[1] < mPoints[0][1] ) {
		mPoints[0][1] = v[1];
		expanded = true;
	}
	if ( v[1] > mPoints[1][1]) {
		mPoints[1][1] = v[1];
		expanded = true;
	}
	if ( v[2] < mPoints[0][2] ) {
		mPoints[0][2] = v[2];
		expanded = true;
	}
	if ( v[2] > mPoints[1][2]) {
		mPoints[1][2] = v[2];
		expanded = true;
	}
	return expanded;
}

mxFORCEINLINE bool AABB::AddBounds( const AABB &a ) {
	bool expanded = false;
	if ( a.mPoints[0][0] < mPoints[0][0] ) {
		mPoints[0][0] = a.mPoints[0][0];
		expanded = true;
	}
	if ( a.mPoints[0][1] < mPoints[0][1] ) {
		mPoints[0][1] = a.mPoints[0][1];
		expanded = true;
	}
	if ( a.mPoints[0][2] < mPoints[0][2] ) {
		mPoints[0][2] = a.mPoints[0][2];
		expanded = true;
	}
	if ( a.mPoints[1][0] > mPoints[1][0] ) {
		mPoints[1][0] = a.mPoints[1][0];
		expanded = true;
	}
	if ( a.mPoints[1][1] > mPoints[1][1] ) {
		mPoints[1][1] = a.mPoints[1][1];
		expanded = true;
	}
	if ( a.mPoints[1][2] > mPoints[1][2] ) {
		mPoints[1][2] = a.mPoints[1][2];
		expanded = true;
	}
	return expanded;
}

mxFORCEINLINE AABB AABB::Intersect( const AABB &a ) const {
	AABB n;
	n.mPoints[0][0] = ( a.mPoints[0][0] > mPoints[0][0] ) ? a.mPoints[0][0] : mPoints[0][0];
	n.mPoints[0][1] = ( a.mPoints[0][1] > mPoints[0][1] ) ? a.mPoints[0][1] : mPoints[0][1];
	n.mPoints[0][2] = ( a.mPoints[0][2] > mPoints[0][2] ) ? a.mPoints[0][2] : mPoints[0][2];
	n.mPoints[1][0] = ( a.mPoints[1][0] < mPoints[1][0] ) ? a.mPoints[1][0] : mPoints[1][0];
	n.mPoints[1][1] = ( a.mPoints[1][1] < mPoints[1][1] ) ? a.mPoints[1][1] : mPoints[1][1];
	n.mPoints[1][2] = ( a.mPoints[1][2] < mPoints[1][2] ) ? a.mPoints[1][2] : mPoints[1][2];
	return n;
}

mxFORCEINLINE AABB &AABB::IntersectSelf( const AABB &a )
{
	if ( a.mPoints[0][0] > mPoints[0][0] ) {
		mPoints[0][0] = a.mPoints[0][0];
	}
	if ( a.mPoints[0][1] > mPoints[0][1] ) {
		mPoints[0][1] = a.mPoints[0][1];
	}
	if ( a.mPoints[0][2] > mPoints[0][2] ) {
		mPoints[0][2] = a.mPoints[0][2];
	}
	if ( a.mPoints[1][0] < mPoints[1][0] ) {
		mPoints[1][0] = a.mPoints[1][0];
	}
	if ( a.mPoints[1][1] < mPoints[1][1] ) {
		mPoints[1][1] = a.mPoints[1][1];
	}
	if ( a.mPoints[1][2] < mPoints[1][2] ) {
		mPoints[1][2] = a.mPoints[1][2];
	}
	return *this;
}

mxFORCEINLINE AABB AABB::Expand( const FLOAT d ) const
{
	return AABB( Vec3D( mPoints[0][0] - d, mPoints[0][1] - d, mPoints[0][2] - d ),
				Vec3D( mPoints[1][0] + d, mPoints[1][1] + d, mPoints[1][2] + d ) );
}

mxFORCEINLINE AABB &AABB::ExpandSelf( const FLOAT d ) {
	mPoints[0][0] -= d;
	mPoints[0][1] -= d;
	mPoints[0][2] -= d;
	mPoints[1][0] += d;
	mPoints[1][1] += d;
	mPoints[1][2] += d;
	return *this;
}

mxFORCEINLINE AABB &AABB::ExpandSelf( const Vec3D &d ) 
{
	mPoints[0][0] -= d[0];
	mPoints[0][1] -= d[1];
	mPoints[0][2] -= d[2];
	mPoints[1][0] += d[0];
	mPoints[1][1] += d[1];
	mPoints[1][2] += d[2];
	return( *this );
}

mxFORCEINLINE const Vec3D AABB::Size( void ) const 
{
	return( mPoints[1] - mPoints[0] );
}

mxFORCEINLINE const Vec3D AABB::GetHalfSize( void ) const 
{
	return this->Size() * 0.5f;
}

mxFORCEINLINE bool AABB::Contains( const AABB &a ) const
{
	for( UINT i = 0; i < 3; i++ ) {
		if( a[1][i] > mPoints[1][i] ) {
			return( false );
		}
		if( a[0][i] < mPoints[0][i] ) {
			return( false );
		}
	}

	return( true );
}

mxFORCEINLINE UINT AABB::GetLargestAxis( void ) const
{
	Vec3D work = mPoints[1] - mPoints[0];
	UINT axis = 0;

	if( work[1] > work[0] )
	{
		axis = 1;
	}
	if( work[2] > work[axis] )
	{
		axis = 2;
	}
	return( axis );
}

mxFORCEINLINE AABB AABB::Trasform( const Matrix4& transform ) const
{
#if 1 // correct method ( if the matrix is row-major, includes only translation and rotation )
	Vec3D center( transform.TransformPoint( GetCenter() ) );

	Vec3D halfExtent( this->GetHalfSize() );

	Matrix4  mat;

	mat[0][0] = fabs( transform[0][0] );
	mat[0][1] = fabs( transform[0][1] );
	mat[0][2] = fabs( transform[0][2] );
	mat[0][3] = 0.0f;

	mat[1][0] = fabs( transform[1][0] );
	mat[1][1] = fabs( transform[1][1] );
	mat[1][2] = fabs( transform[1][2] );
	mat[1][3] = 0.0f;

	mat[2][0] = fabs( transform[2][0] );
	mat[2][1] = fabs( transform[2][1] );
	mat[2][2] = fabs( transform[2][2] );
	mat[2][3] = 0.0f;

	mat[3][0] = 0.0f;
	mat[3][1] = 0.0f; 
	mat[3][2] = 0.0f;
	mat[3][3] = 1.0f;

	halfExtent = mat.TransformNormal( halfExtent );

	return AABB( center - halfExtent, center + halfExtent );

#else // this is said to be faster, but it has branches :

	// Start with translation component.
	Vec3D min, max;	// <= The points of the resulting AABB.
	min = max = transform.GetTranslation();

	// Do rotation.
	// Find extreme points by considering product of min and max with each component of the matrix.
	for ( UINT j = 0; j < 3; j++ )
	{
		for ( UINT i = 0; i < 3; i++ )
		{
			FLOAT  a = transform[i][j] * mPoints[0][i];
			FLOAT  b = transform[i][j] * mPoints[1][i];
			
			if ( a < b ) {
				min[j] += a;
				max[j] += b;
			} else {
				min[j] += b;
				max[j] += a;
			}
		}
	}

	return AABB( min, max );
#endif
}

mxFORCEINLINE AABB &AABB::TrasformSelf( const Matrix4& transform )
{
#if 1 // correct method ( if the matrix is row-major, includes only translation and rotation )
	Vec3D center( transform.TransformPoint( this->GetCenter() ) );

	Vec3D halfExtent( this->GetHalfSize() );

	Matrix4  mat;

	mat[0][0] = fabs( transform[0][0] );
	mat[0][1] = fabs( transform[0][1] );
	mat[0][2] = fabs( transform[0][2] );
	mat[0][3] = 0.0f;

	mat[1][0] = fabs( transform[1][0] );
	mat[1][1] = fabs( transform[1][1] );
	mat[1][2] = fabs( transform[1][2] );
	mat[1][3] = 0.0f;

	mat[2][0] = fabs( transform[2][0] );
	mat[2][1] = fabs( transform[2][1] );
	mat[2][2] = fabs( transform[2][2] );
	mat[2][3] = 0.0f;

	mat[3][0] = 0.0f;
	mat[3][1] = 0.0f; 
	mat[3][2] = 0.0f;
	mat[3][3] = 1.0f;

	halfExtent = mat.TransformNormal( halfExtent );

	mPoints[ 0 ] = center - halfExtent;
	mPoints[ 1 ] = center + halfExtent;

#else // this is said to be faster, but it has branches :

	// Store old points.
	Vec3D  minPoint( mPoints[0] );
	Vec3D  maxPoint( mPoints[1] );

	// Start with translation component.
	mPoints[0] = mPoints[1] = transform.GetTranslation();

	// Do rotation.
	// Find extreme points by considering product of minPoint and maxPoint with each component of the matrix.
	for ( UINT j = 0; j < 3; j++ )
	{
		for ( UINT i = 0; i < 3; i++ )
		{
			FLOAT  a = transform[i][j] * minPoint[i];
			FLOAT  b = transform[i][j] * maxPoint[i];
			
			if ( a < b ) {
				mPoints[0][j] += a;
				mPoints[1][j] += b;
			} else {
				mPoints[0][j] += b;
				mPoints[1][j] += a;
			}
		}
	}

#endif
	return *this;
}

mxFORCEINLINE AABB AABB::Translate( const Vec3D &translation ) const {
	return AABB( mPoints[0] + translation, mPoints[1] + translation );
}

mxFORCEINLINE AABB &AABB::TranslateSelf( const Vec3D &translation ) {
	mPoints[0] += translation;
	mPoints[1] += translation;
	return *this;
}

mxFORCEINLINE AABB AABB::Rotate( const Matrix3 &rotation ) const {
	AABB bounds;
	bounds.FromTransformedBounds( *this, Vec3D::vec3_zero, rotation );
	return bounds;
}

mxFORCEINLINE AABB &AABB::RotateSelf( const Matrix3 &rotation ) {
	this->FromTransformedBounds( *this, Vec3D::vec3_zero, rotation );
	return *this;
}

mxFORCEINLINE bool AABB::ContainsPoint( const Vec3D &p ) const {
	if ( p[0] < mPoints[0][0] || p[1] < mPoints[0][1] || p[2] < mPoints[0][2]
		|| p[0] > mPoints[1][0] || p[1] > mPoints[1][1] || p[2] > mPoints[1][2] ) {
		return false;
	}
	return true;
}

mxFORCEINLINE bool AABB::IntersectsBounds( const AABB &a ) const {
	if ( a.mPoints[1][0] < mPoints[0][0] || a.mPoints[1][1] < mPoints[0][1] || a.mPoints[1][2] < mPoints[0][2]
		|| a.mPoints[0][0] > mPoints[1][0] || a.mPoints[0][1] > mPoints[1][1] || a.mPoints[0][2] > mPoints[1][2] ) {
		return false;
	}
	return true;
}

mxINLINE Sphere AABB::ToSphere( void ) const {
#if 0
	Sphere sphere;
	sphere.SetOrigin( this->GetCenter() );
	sphere.SetRadius( (mPoints[1] - sphere.GetOrigin()).Length() );
	return sphere;
#else
	return Sphere( (mPoints[0] + mPoints[1]) * 0.5f,
				( (mPoints[1] - mPoints[0]) * 0.5f ).GetLength() );
#endif
}

mxFORCEINLINE void AABB::AxisProjection( const Vec3D &dir, FLOAT &min, FLOAT &max ) const {
	FLOAT d1, d2;
	Vec3D center, extents;

	center = ( mPoints[0] + mPoints[1] ) * 0.5f;
	extents = mPoints[1] - center;

	d1 = dir * center;
	d2 = fabs( extents[0] * dir[0] ) +
			fabs( extents[1] * dir[1] ) +
				fabs( extents[2] * dir[2] );

	min = d1 - d2;
	max = d1 + d2;
}

mxFORCEINLINE void AABB::AxisProjection( const Vec3D &origin, const Matrix3 &axis, const Vec3D &dir, FLOAT &min, FLOAT &max ) const {
	FLOAT d1, d2;
	Vec3D center, extents;

	center = ( mPoints[0] + mPoints[1] ) * 0.5f;
	extents = mPoints[1] - center;
	center = origin + center * axis;

	d1 = dir * center;
	d2 = fabs( extents[0] * ( dir * axis[0] ) ) +
			fabs( extents[1] * ( dir * axis[1] ) ) +
				fabs( extents[2] * ( dir * axis[2] ) );

	min = d1 - d2;
	max = d1 + d2;
}

mxFORCEINLINE Vec3D & AABB::GetMin() {
	return mPoints[ 0 ];
}

mxFORCEINLINE const Vec3D & AABB::GetMin() const {
	return mPoints[ 0 ];
}

mxFORCEINLINE Vec3D & AABB::GetMax() {
	return mPoints[ 1 ];
}

mxFORCEINLINE const Vec3D & AABB::GetMax() const {
	return mPoints[ 1 ];
}

mxFORCEINLINE bool AABB::IsValid() const
{
	return (mPoints[0].x <= mPoints[1].x)
		&& (mPoints[0].y <= mPoints[1].y)
		&& (mPoints[0].z <= mPoints[1].z)

		&& mPoints[0].IsValid()
		&& mPoints[1].IsValid();
}

mxFORCEINLINE void AABB::FromSphere( const Sphere& sphere )
{
	Vec3D  v( sphere.GetRadius(), sphere.GetRadius(), sphere.GetRadius() );
	mPoints[0] = sphere.GetOrigin() - v;
	mPoints[1] = sphere.GetOrigin() + v;
}



#endif // ! __BOUNDING_VOLUMES_AABB_H__

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
