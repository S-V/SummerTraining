/*
=============================================================================
	File:	VectorMath.h
	Desc:	
	ToDo:	cleanup and move code to other place
=============================================================================
*/
#pragma once

#include <Base/Math/Math.h>
#include <Base/Util/Color.h>
#include <Core/Input.h>

#define mmFloat3_Forward	(*(const Float3*)&g_MMIdentityR1)
#define mmFloat3_Right		(*(const Float3*)&g_MMIdentityR0)
#define mmFloat3_Up			(*(const Float3*)&g_MMIdentityR2)

// Axis Aligned Bounding Box represented by two extreme (corner) points (minima and maxima).
struct AABB24
{
	Float3	min_point;	//12 minimum corner of the box
	Float3	max_point;	//12 maximum corner of the box
};
mxDECLARE_STRUCT(AABB24);
mxDECLARE_POD_TYPE(AABB24);

void AABB24_Clear( AABB24 *aabb );
void AABB24_AddPoint( AABB24 *aabb, const Float3& p );
void AABB24_AddAABB( AABB24 *aabb, const AABB24& other );
// Gets the eight corners of this box.
void AABB_GetCorners( const Float3& aabbMin, const Float3& aabbMax, Float3 points[8] );
// Returns the 12 edges of the box ( each edge is a pair of indices into the array of corners of this box ).
const UINT8* AABB_GetEdgeIndices();
float AABB_GetVolume( const AABB24& aabb );
Float3 AABB_Center( const AABB24& aabb );
Float3 AABB_Extent( const AABB24& aabb );	// returns half size
Float3 AABB_FullSize( const AABB24& aabb );
float AABB_ClosestDistance( const AABB24& aabb, const Float3& point );
bool AABB_ContainsPoint( const AABB24& aabb, const Float3& point );// includes touching
bool AABB_ContainsPoint( const AABB24& aabb, const Float3& point, float epsilon );// includes touching
void AABB_EncodePosition( const AABB24& aabb, const Float3& point, UINT8 output[3] );
void AABB_DecodePosition( const AABB24& aabb, const UINT8 input[3], Float3 *point );

ATextStream & operator << ( ATextStream & log, const AABB24& bounds );

struct Sphere16 : public CStruct
{
	Float3	center;
	float	radius;
public:
	mxDECLARE_CLASS(Sphere16,CStruct);
	mxDECLARE_REFLECTION;
};
// makes an inside-out sphere
void Sphere16_Clear( Sphere16 *sphere );
void Sphere16_From_AABB( const AABB24& aabb, Sphere16 *sphere );

// Cohen–Sutherland clipping algorithm
enum EClipCodes
{
	CLIP_NEG_X = 0,
	CLIP_POS_X = 1,
	CLIP_NEG_Y = 2,
	CLIP_POS_Y = 3,
	CLIP_NEG_Z = 4,
	CLIP_POS_Z = 5,
};

// projection matrix transforms vertices from world space to normalized clip space
// where only points that fulfill the following are inside the view frustum:
// -|w| <= x <= |w|
// -|w| <= y <= |w|
// -|w| <= z <= |w|
// tests each axis against W and returns the corresponding clip mask (with bits from EClipCodes)
template< class VECTOR >
static inline
UINT ClipMask( VECTOR & v )
{
#if 0
	UINT cmask = 0;
	if( v.p.w - v.p.x < 0.0f ) cmask |= BIT( CLIP_POS_X );
	if( v.p.x + v.p.w < 0.0f ) cmask |= BIT( CLIP_NEG_X );
	if( v.p.w - v.p.y < 0.0f ) cmask |= BIT( CLIP_POS_Y );
	if( v.p.y + v.p.w < 0.0f ) cmask |= BIT( CLIP_NEG_Y );
	if( v.p.w - v.p.z < 0.0f ) cmask |= BIT( CLIP_POS_Z );
	if( v.p.z + v.p.w < 0.0f ) cmask |= BIT( CLIP_NEG_Z );
	return cmask;
#else
	return
		(( v.x < -v.w ) << CLIP_NEG_X)|
		(( v.x >  v.w ) << CLIP_POS_X)|
		(( v.y < -v.w ) << CLIP_NEG_Y)|
		(( v.y >  v.w ) << CLIP_POS_Y)|
		(( v.z < -v.w ) << CLIP_NEG_Z)|
		(( v.z >  v.z ) << CLIP_POS_Z);
#endif
}

/*
-----------------------------------------------------------------------------
	ArcBall
	Arcball is a method to manipulate and rotate objects in 3D intuitively.
	The motivation behind the trackball (aka arcball) is to provide an intuitive user interface for complex 3D
	object rotation via a simple, virtual sphere - the screen space analogy to the familiar input device bearing
	the same name.
	The sphere is a good choice for a virtual trackball because it makes a good enclosure for most any object;
	and its surface is smooth and continuous, which is important in the generation of smooth rotations in response
	to smooth mouse movements.
	Any smooth, continuous shape, however, could be used, so long as points on its surface can be generated
	in a consistent way.
-----------------------------------------------------------------------------
*/
class ArcBall : public CStruct {
public:
	mxDECLARE_CLASS(ArcBall,CStruct);
	mxDECLARE_REFLECTION;
	ArcBall();
	void Initialize( int width, int height );	// must be done after resizing the viewport
	void BeginDrag( int x, int y );	// start the rotation (pass current mouse position)
	void MoveDrag( int x, int y );	// continue the rotation (pass current mouse position)
	void EndDrag();					// stop the rotation 
	Vector4 GetQuaternion() const;
	Float4x4 GetRotationMatrix() const;
private:
	Float3 GetArcballVector( int screenX, int screenY ) const;
private:
	Float3		m_downPt;	// saved click vector (starting point of rotation arc)
	Float3		m_currPt;	// saved click vector (current point of rotation arc)
	float		m_scaleX;	// for normalizing screen-space X coordinate
	float		m_scaleY;	// for normalizing screen-space Y coordinate
	//float		m_radius;	// arc ball's radius in screen coords (default = 1.0f)
	bool		m_isDown;	// is the user dragging the mouse?
	Vector4		m_qDown;	// Quaternion before button down
	Vector4		m_qNow;		// Composite quaternion for current drag
};

String256 MatrixToString( const Float4x4& M );

/*
-----------------------------------------------------------------------------
	View3D
	contains common view parameters
-----------------------------------------------------------------------------
*/
struct View3D : public CStruct {
	// Basis vectors, expressed in world space:
	// X(right), Y(up) and Z(lookDir) vectors, must be orthonormal (they form the camera coordinate system).
	Float3	right;			//12 Right direction.
	Float3	up;				//12 Up direction.
	Float3	look;			//12 Look direction.
	Float3	origin;			//12 Eye position - Location of the camera, expressed in world space.
	float	near_clip;		//4 Near clipping plane.
	float	far_clip;		//4 Far clipping plane.
	float	halfFoVY;		//4 Vertical field of view angle, in radians (default = PI/4).
	float	aspect_ratio;	//4 projection ratio (ratio between width and height of view surface)
	// 64 bytes
public:
	mxDECLARE_CLASS(View3D,CStruct);
	mxDECLARE_REFLECTION;
	View3D();
};
mxDECLARE_POD_TYPE(View3D);

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
