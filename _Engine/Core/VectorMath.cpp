/*
=============================================================================
	File:	VectorMath.cpp
	Desc:
=============================================================================
*/

#include <Core/Core_PCH.h>
#pragma hdrstop
#include <Core/Core.h>
#include <Core/VectorMath.h>

mxBEGIN_STRUCT(AABB24)
	mxMEMBER_FIELD(min_point),
	mxMEMBER_FIELD(max_point),
mxEND_REFLECTION

void AABB24_Clear( AABB24 *aabb )
{
	aabb->min_point = Float3_Set( +BIG_NUMBER, +BIG_NUMBER, +BIG_NUMBER );
	aabb->max_point = Float3_Set( -BIG_NUMBER, -BIG_NUMBER, -BIG_NUMBER );
}
void AABB24_AddPoint( AABB24 *aabb, const Float3& p )
{
	aabb->min_point = Float3_Min( aabb->min_point, p );
	aabb->max_point = Float3_Max( aabb->max_point, p );
}
void AABB24_AddAABB( AABB24 *aabb, const AABB24& other )
{
	aabb->min_point = Float3_Min( aabb->min_point, other.min_point );
	aabb->max_point = Float3_Max( aabb->max_point, other.max_point );
}
void AABB_GetCorners( const Float3& aabbMin, const Float3& aabbMax, Float3 points[8] )
{
/*

   Y
   |  /Z
   | /
   |/_____X
 (0,0,0)

        7___________6
       /|           /
      / |          /|
     /  |         / |
    3------------2  |
    |   4________|__5
    |   /        |  /
    |  /         | /
    | /          |/
    0/___________1


	0 = ---
	1 = +--
	2 = ++-
	3 = -+-
	4 = --+
	5 = +-+
	6 = +++
	7 = -++
*/
	const Float3 aabbMinMax[2] = { aabbMin, aabbMax };

	for( UINT i = 0; i < 8; i++ )
	{
		((float*)&points[i])[0] = ((float*)&aabbMinMax[ (i^(i>>1))& 1 ])[ 0 ];
		((float*)&points[i])[1] = ((float*)&aabbMinMax[ (i>>1)    & 1 ])[ 1 ];
		((float*)&points[i])[2] = ((float*)&aabbMinMax[ (i>>2)    & 1 ])[ 2 ];
	}
}
const UINT8* AABB_GetEdgeIndices()
{
	static const UINT8 edgeIndices[ 12*2 ] =
	{
		0, 1,
		1, 2,
		2, 3,
		3, 0,

		7, 6,
		6, 5,
		5, 4,
		4, 7,

		1, 5,
		6, 2,
		3, 7,
		4, 0
	};
	return edgeIndices;
}
float AABB_GetVolume( const AABB24& aabb )
{
	Float3 size = AABB_FullSize(aabb);
	return size.x * size.y * size.z;
}
Float3 AABB_Center( const AABB24& aabb )
{
	return (aabb.min_point + aabb.max_point) * 0.5f;
}
Float3 AABB_Extent( const AABB24& aabb )
{
	return AABB_FullSize(aabb) * 0.5f;
}
Float3 AABB_FullSize( const AABB24& aabb )
{
	return aabb.max_point - aabb.min_point;
}
float AABB_ClosestDistance( const AABB24& aabb, const Float3& point )
{
	if( AABB_ContainsPoint( aabb, point ) ) {
		return 0.0f;
	}
	float distance = 0.0f;
	for( int i = 0; i < 3; i++ )
	{
		if( point[i] < aabb.min_point[i] ) {
			float delta = aabb.min_point[i] - point[i];
			distance += delta * delta;
		} else if ( point[i] > aabb.max_point[i] ) {
			float delta = point[i] - aabb.max_point[i];
			distance += delta * delta;
		}
	}
	return distance;
}
bool AABB_ContainsPoint( const AABB24& aabb, const Float3& point )
{
	return point.x >= aabb.min_point.x
		&& point.y >= aabb.min_point.y
		&& point.z >= aabb.min_point.z
		&& point.x <= aabb.max_point.x
		&& point.z <= aabb.max_point.z
		&& point.y <= aabb.max_point.y;
}
bool AABB_ContainsPoint( const AABB24& aabb, const Float3& point, float epsilon )
{
	return point.x >= aabb.min_point.x - epsilon
		&& point.y >= aabb.min_point.y - epsilon
		&& point.z >= aabb.min_point.z - epsilon
		&& point.x <= aabb.max_point.x + epsilon
		&& point.z <= aabb.max_point.z + epsilon
		&& point.y <= aabb.max_point.y + epsilon;
}
void AABB_EncodePosition( const AABB24& aabb, const Float3& point, UINT8 output[3] )
{
	mxASSERT(AABB_ContainsPoint( aabb, point ));
	const Float3 aabbSize = AABB_FullSize(aabb);
	const Float3 relativePosition = point - aabb.min_point;	// [0..aabb_size]
	mxASSERT(Float3_AllGreaterOrEqual( relativePosition, 0.0f ));
	const Float3 rescaledPosition = Float3_Multiply( relativePosition, Float3_Reciprocal(aabbSize) );// [0..1]
	const Float3 quantizedPosition = rescaledPosition * 255.0f;	// [0..255]
	output[0] = quantizedPosition.x;
	output[1] = quantizedPosition.y;
	output[2] = quantizedPosition.z;
}
void AABB_DecodePosition( const AABB24& aabb, const UINT8 input[3], Float3 *point )
{
	const Float3 aabbSize = AABB_FullSize(aabb);
	const Float3 quantizedPosition = Float3_Set(input[0],input[1],input[2]);	// [0..255]
	const Float3 rescaledPosition = quantizedPosition * (1.0f/255.0f);	// [0..1]
	const Float3 relativePosition = Float3_Multiply( rescaledPosition, aabbSize );	// [0..aabb_size]
	*point = aabb.min_point + relativePosition;
}
ATextStream & operator << ( ATextStream & log, const AABB24& bounds )
{
	log << "{min=" << bounds.min_point << ", max=" << bounds.max_point << "}";
	return log;
}

mxDEFINE_CLASS(Sphere16);
mxBEGIN_REFLECTION(Sphere16)
	mxMEMBER_FIELD(center),
	mxMEMBER_FIELD(radius),
mxEND_REFLECTION

void Sphere16_Clear( Sphere16 *sphere )
{
	sphere->center = Float3_Zero();
	sphere->radius = -1.0f;
}
void Sphere16_From_AABB( const AABB24& aabb, Sphere16 *sphere )
{
	sphere->center = AABB_Center(aabb);
	sphere->radius = Float3_Length(AABB_Extent(aabb));
}

#if 0
/*
-----------------------------------------------------------------------------
	rxAABB
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS( rxAABB );
mxBEGIN_REFLECTION( rxAABB )
	mxMEMBER_FIELD( m_corner ),
	mxMEMBER_FIELD( m_extent ),
mxEND_REFLECTION
#endif

/*
-----------------------------------------------------------------------------
	ArcBall
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS(ArcBall);
mxBEGIN_REFLECTION(ArcBall)
	mxMEMBER_FIELD(m_scaleX),
	mxMEMBER_FIELD(m_scaleY),
	//mxMEMBER_FIELD(m_radius),
	mxMEMBER_FIELD(m_qDown),
	mxMEMBER_FIELD(m_qNow),
mxEND_REFLECTION

ArcBall::ArcBall()
{
	m_downPt = Float3_Zero();
	m_currPt = Float3_Zero();
	m_scaleX = 1.0f;
	m_scaleY = 1.0f;
	//m_radius = 1.0f;
	m_isDown = false;
	m_qDown = Quaternion_Identity();
	m_qNow = Quaternion_Identity();
}

void ArcBall::Initialize( int width, int height )
{
	m_scaleX = 2.0f / width;
	m_scaleY = 2.0f / height;
}

void ArcBall::BeginDrag( int x, int y )
{
	m_qDown = m_qNow;
	m_downPt = GetArcballVector( x, y );
	m_isDown = true;
}

static Vector4 QuatFromBallPoints( const Float3& vFrom, const Float3& vTo )
{
	float	fDot = Float3_Dot( vFrom, vTo );
	Float3	vPart = Float3_Cross( vFrom, vTo );
	Vector4	result;
	result.x = vPart.x;
	result.y = vPart.y;
	result.z = vPart.z;
	result.w = fDot;
	result = Quaternion_Normalize(result);
	return result;
}

void ArcBall::MoveDrag( int x, int y )
{
	if( m_isDown )
	{
		m_currPt = GetArcballVector( x, y );
		const Vector4 qCurr = QuatFromBallPoints( m_downPt, m_currPt );
		m_qNow = Quaternion_Multiply( m_qDown, qCurr );	// qDown -> qCurr
		m_qNow = Quaternion_Normalize( m_qNow );
	}
}

void ArcBall::EndDrag()
{
	m_isDown = false;
}

Vector4 ArcBall::GetQuaternion() const
{
	return m_qNow;
}

Float4x4 ArcBall::GetRotationMatrix() const
{
	return Matrix_FromQuaternion(m_qNow);
}

// The ArcBall works by mapping click coordinates in a window
// directly into the ArcBall's sphere coordinates, as if it were directly in front of you.
// To accomplish this, first we simply scale down the mouse coordinates
// from the range of [0...width), [0...height) to [-1...1], [1...-1].
//ArcBall sphere constants:
//Diameter is       2.0f
//Radius is         1.0f
//Radius squared is 1.0f

/**
 * Get a normalized vector from the center of the virtual ball O to a
 * point P on the virtual ball surface, such that P is aligned on
 * screen's (X,Y) coordinates.  If (X,Y) is too far away from the
 * sphere, return the nearest point on the virtual ball surface.
 */
Float3 ArcBall::GetArcballVector( int screenX, int screenY ) const
{
	//Adjust point coords and scale down to range of [-1 ... 1]
	float x = 1.0f - (screenX * m_scaleX);
	float y = 1.0f - (screenY * m_scaleY);

	mxASSERT(x >= -1.0f && x <= 1.0f);
	mxASSERT(y >= -1.0f && y <= 1.0f);

	float lengthSquared = x * x + y * y;

	// NOTE: y and z axes are swapped according to our coordinate system:

	//If the point is mapped outside of the sphere... (length_squared > radius_squared)
	if( lengthSquared > 1.0f ) {
		//Compute a normalizing factor (radius / sqrt(length))
		float norm = Float_InvSqrt(lengthSquared);
		//Return the "normalized" vector, a point on the sphere
		return Float3_Set( x*norm, 0.0f, y*norm );
	} else {
		//Return a vector to a point mapped inside the sphere sqrt(radius squared - length)
		return Float3_Set( x, Float_Sqrt(1.0f - lengthSquared), y );
	}
}

String256 MatrixToString( const Float4x4& M )
{
	String256	result;
	UNDONE;
#if 0
	const glm::vec4 col0 = M[0];
	const glm::vec4 col1 = M[1];
	const glm::vec4 col2 = M[2];
	const glm::vec4 col3 = M[3];
	//NOTE: this prints the matrix in row-major format:
	str::SPrintF(result,
		"%.3f, %.3f, %.3f, %.3f,\n"
		"%.3f, %.3f, %.3f, %.3f,\n"
		"%.3f, %.3f, %.3f, %.3f,\n"
		"%.3f, %.3f, %.3f, %.3f\n",
		col0[0], col0[1], col0[2], col0[3],
		col1[0], col1[1], col1[2], col1[3],
		col2[0], col2[1], col2[2], col2[3],
		col3[0], col3[1], col3[2], col3[3]
	);
#endif
	return result;
}

/*
-----------------------------------------------------------------------------
	View3D
-----------------------------------------------------------------------------
*/
#define rxDEFAULT_ASPECT_RATIO		1.333333F

#define rxCAMERA_DEFAULT_NEAR_PLANE		1.0f
#define rxCAMERA_DEFAULT_FAR_PLANE		5000.0f
#define rxCAMERA_WIDESCREEN_ASPECT		(16.0f/9.0f)
#define rxCAMERA_DEFAULT_ASPECT			(rxDEFAULT_ASPECT_RATIO)
#define rxCAMERA_DEFAULT_FIELD_OF_VIEW	(mxPI_DIV_4)

mxDEFINE_CLASS(View3D);
mxBEGIN_REFLECTION(View3D)
	mxMEMBER_FIELD( right ),
	mxMEMBER_FIELD( up ),
	mxMEMBER_FIELD( look ),
	mxMEMBER_FIELD( origin ),
	mxMEMBER_FIELD( near_clip ),
	mxMEMBER_FIELD( far_clip ),
	mxMEMBER_FIELD( halfFoVY ),
	mxMEMBER_FIELD( aspect_ratio ),
mxEND_REFLECTION;
View3D::View3D()
{
	right	= Float3_Set( 1.0f, 0.0f, 0.0f );
	up		= Float3_Set( 0.0f, 1.0f, 0.0f );
	look	= Float3_Set( 0.0f, 0.0f, 1.0f );
	origin	= Float3_Set( 0.0f, 0.0f, 0.0f );
	near_clip	= 0.1f;
	far_clip	= 10000.0f;
	halfFoVY	= rxCAMERA_DEFAULT_FIELD_OF_VIEW;
	aspect_ratio = rxDEFAULT_ASPECT_RATIO;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
