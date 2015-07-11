/*
=============================================================================
	File:	MiniMath.h
	Desc:	The Mini-Math library.
	Based on XNAMath/DirectX math, GLM/Eigen and Sony's vector math.
=============================================================================
*/
#pragma once

#define MM_ENABLE_INTRINSICS	(0)
#define MM_ENABLE_REFLECTION	(1)

// overload [] and arithmetic operators for convenience
#define MM_OVERLOAD_OPERATORS	(1)

/*
immintrin.h - AVX 256 bit SIMD instruction set. (Defines __m256.)
wmmintrin.h - AVX AES instructions.
nmmintrin.h - The Core 2 Duo 4.2 SIMD instruction set.
smmintrin.h - The Core 2 Duo 4.1 SIMD instruction set.
tmmintrin.h - SSSE 3 aka SSE 3.1 SIMD instruction set.
pmmintrin.h - SSE 3 SIMD instruction set.
emmintrin.h - SSE 2 SIMD instruction set.
xmmintrin.h - SSE(1) SIMD instruction set. (Defines __m128.)
mmintrin.h - MMX instruction set. (Defines __m64.)
*/
#include <xmmintrin.h>	// SSE 1, __m128
#include <emmintrin.h>	// SSE 2, __m128i
#include <pmmintrin.h>	// SSE 3
#if defined(__SSE4_1__)
#	include <smmintrin.h>
#endif // defined(__SSE4_1__)

/*
=======================================================================
	DEFINES
=======================================================================
*/

#define mmSELECT_0         0x00000000
#define mmSELECT_1         0xFFFFFFFF

#define mmPERMUTE_0X       0x00010203
#define mmPERMUTE_0Y       0x04050607
#define mmPERMUTE_0Z       0x08090A0B
#define mmPERMUTE_0W       0x0C0D0E0F
#define mmPERMUTE_1X       0x10111213
#define mmPERMUTE_1Y       0x14151617
#define mmPERMUTE_1Z       0x18191A1B
#define mmPERMUTE_1W       0x1C1D1E1F

#define mmCRMASK_CR6       0x000000F0
#define mmCRMASK_CR6TRUE   0x00000080
#define mmCRMASK_CR6FALSE  0x00000020
#define mmCRMASK_CR6BOUNDS mmCRMASK_CR6FALSE

#define mmCACHE_LINE_SIZE  64

// Stolen from XMISNAN and XMISINF
#if !MM_ENABLE_INTRINSICS
#define mmIS_NAN(x)  ((*(UINT*)&(x) & 0x7F800000) == 0x7F800000 && (*(UINT*)&(x) & 0x7FFFFF) != 0)
#define mmIS_INF(x)  ((*(UINT*)&(x) & 0x7FFFFFFF) == 0x7F800000)
#endif

/*
=======================================================================
	LOW-LEVEL TYPES
=======================================================================
*/

typedef UINT16 Half;

// 2D Vector; 32 bit floating point components
struct Float2
{
	float	x, y;
};
// 3D Vector; 32 bit floating point components
struct Float3
{
	float	x, y, z;

#if MM_OVERLOAD_OPERATORS
	float& operator [] ( int i ) { return (&x)[i]; }
	float operator [] ( int i ) const { return (&x)[i]; }
#endif
};
// 4D Vector; 32 bit floating point components
__declspec(align(16)) struct Float4
{
	float	x, y, z, w;

#if MM_OVERLOAD_OPERATORS
	float& operator [] ( int i ) { return (&x)[i]; }
	float operator [] ( int i ) const { return (&x)[i]; }
#endif
};
// 8D Vector; 32 bit floating point components
__declspec(align(32)) struct Float8
{
	float	x, y, z, w;
	float	p, q, r, s;
};

#if MM_ENABLE_INTRINSICS

	// Define 128-bit wide 16-byte aligned hardware register type.
	typedef __m128				Vector4;

	// Microsoft recommends passing the __m128 data type by reference to functions.
	// This is the behavior of the XNA Math Library.
	//typedef const __m128&		Vector4Arg;
	// But Win32 VC++ does not support passing aligned objects by value,
	// here is a bug-report at Microsoft saying so:
	// http://connect.microsoft.com/VisualStudio/feedback/ViewFeedback.aspx?FeedbackID=334581
	typedef const __m128		Vector4Arg;

#else

	// Fall back to 128-bit wide 4D floating point vector type.
	typedef Float4				Vector4;

	// the vector parameter type (for passing vectors to functions)
	typedef const Float4 &		Vector4Arg;

	// Fall back to 256-bit wide 4D floating point vector type.
	typedef Float8				Vector8;

	// the vector parameter type (for passing vectors to functions)
	typedef const Float8 &		Vector8Arg;

#endif


union Vector4i
{
	int		i[4];
	Vector4	v;
};
union Vector4f
{
	float	f[4];
	Vector4	v;
};

struct Int2 {
	INT32	x;
	INT32	y;
};
struct Int4 {
	INT32	x;
	INT32	y;
	INT32	z;
	INT32	w;
};
struct UInt4 {
	UINT32	x;
	UINT32	y;
	UINT32	z;
	UINT32	w;
};

// 2D Vector; 16 bit floating point components
struct Half2 {
	Half	x;
	Half	y;
};

struct Half4 {
	Half	x, y, z, w;
};

// 2D Vector; 16 bit signed integer components
struct Short2 {
	INT16	x;
	INT16	y;
};
// 2D Vector; 16 bit unsigned integer components
struct UShort2 {
	UINT16	x;
	UINT16	y;
};
// 2D Vector; 16 bit signed normalized integer components
struct NShort2 {
	INT16	x;
	INT16	y;
};
// 2D Vector; 16 bit unsigned normalized integer components
struct UNShort2 {
	UINT16	x;
	UINT16	y;
};

// 4D Vector of 8-bit unsigned integer components.
// This is often used for storing RGBA colors or compressed normals.
struct UByte4 {
	union {
		struct {
			UINT8 x, y, z, w;
		};
		struct {
			UINT8 r, g, b, a;
		};
		struct {
			UINT8 c[4];
		};
		UINT32	v;
	};
};

/*
=======================================================================
	HIGH-LEVEL TYPES
=======================================================================
*/
mxSWIPED("comments from CryEngine3 SDK, Cry_Camera.h");
//
// We are using a "right-handed" coordinate system where the positive X-Axis points 
// to the right, the positive Y-Axis points away from the viewer and the positive
// Z-Axis points up. The following illustration shows our coordinate system.
//
// <PRE>
//  Z-axis                                  
//    ^                               
//    |                               
//    |   Y-axis                   
//    |  /                         
//    | /                           
//    |/                             
//    +----------------> X-axis     
// </PRE>
//
// This same system is also used in Blender/3D-Studio-MAX and CryENGINE.
//
// Vector Cross product:
// The direction of the cross product follows the right hand rule.
// Rotation direction is counter-clockwise when looking down the axis
// from the the positive end towards the origin.
//
// Matrix multiplication order: left-to-right (as in DirectX),
// i.e. scaling, followed by rotation, followed by translation:
// S * R * T.

// SRT - scale rotation translation
// SQT - scale quaternion translation

// PRY - Pitch/Roll/Yaw
// PRS - position rotation scale

// Euler angles:
// Pitch (Attitude) (Up / Down) - rotation around X axis (between -90 and 90 deg);
// Roll (Bank) (Fall over)      - rotation around Y axis (between -180 and 180 deg);
// Yaw (Heading) (Left / Right) - rotation around Z axis (yaw is also called 'heading') (between -180 and 180).

// Azimuth - horizontal angle (yaw)
// Elevation - vertical angle (pitch)
// Tilt - angle around the look direction vector (roll)

// The order of transformations is pitch first, then roll, then yaw.
// Relative to the object's local coordinate axis,
// this is equivalent to rotation around the x-axis,
// followed by rotation around the y-axis,
// followed by rotation around the z-axis.

// Angle units: radians

// In our right-handed coordinate system front faces are counter-clockwise triangles.
// That is, a triangle will be considered front-facing if its vertices are counter-clockwise.

/*
-----------------------------------------------------------------------
	4x4 matrix: sixteen 32 bit floating point components
	aligned on a 16 byte boundary and mapped to four hardware vector registers.
	Translation component is in r[3].xyz.
	NOTE: both in OpenGL and Direct3D the matrix layout in memory is the same.
	Column-major versus row-major is purely a notational convention.
-----------------------------------------------------------------------
*/
mxPREALIGN(16) struct Float4x4
{
	union {
		struct {
			Vector4	r0, r1, r2, r3;
		};
		Vector4	r[4];
		//struct
		//{
		//	float a0,  a1,   a2,  a3;	//<= column 0
		//	float a4,  a5,   a6,  a7;	//<= column 1
		//	float a8,  a9,  a10, a11;	//<= column 2
		//	float a12, a13, a14, a15;	//<= column 3
		//};// translation is (a12, a13, a14)
		//struct
		//{
		//	float m00, m01, m02, m03;	//<= column 0
		//	float m10, m11, m12, m13;	//<= column 1
		//	float m20, m21, m22, m23;	//<= column 2
		//	float m30, m31, m32, m33;	//<= column 3
		//};// translation is (m30, m31, m32)
		//float	a[16];
		float	m[4][4];
	};

#if MM_OVERLOAD_OPERATORS
	Float4& operator [] ( int row ) { return r[row]; }
	const Float4& operator [] ( int row ) const { return r[row]; }
#endif
};

//@todo: specify passing by value or register depending on the platform
#if MM_ENABLE_INTRINSICS

	typedef const Float4x4 &	Float4x4Arg;

#else

	typedef const Float4x4 &	Float4x4Arg;

#endif

/*
----------------------------------------------------------------------
	Packed matrix: the fourth row is (0,0,0,1),
	the translation is computed from r[0].w, r[1].w, r[2].w,
	the first three elements of each row form a rotation matrix
	and the last element of every row is the translation over one axis.

	Mat3x3	R;	// rotation component
	Vec3	T;	// translation component

	R[0][0], R[0][1], R[0][2], Tx[0]
	R[1][0], R[1][1], R[1][2], Ty[1]
	R[2][0], R[2][1], R[2][2], Tz[2]
----------------------------------------------------------------------
*/
mxPREALIGN(16) struct Float3x4
{
	Vector4	r[3];	// laid out as 3 rows in memory
	// r[0].xyz contains translation,
	// (r[0].x, r[1].y, r[2].z, 0f) are the third row of the full 4x4 matrix.
};

/*
----------------------------------------------------------------------
	3x3 matrix
----------------------------------------------------------------------
*/
struct Float3x3
{
	union {
		struct {
			Float3	r0, r1, r2;
		};
		Float3	r[3];
		float	m[3][3];
	};

#if MM_OVERLOAD_OPERATORS
	Float3& operator [] ( int row ) { return r[row]; }
	const Float3& operator [] ( int row ) const { return r[row]; }
#endif

};

///*
//----------------------------------------------------------------------
//	Packed matrix
//----------------------------------------------------------------------
//*/
//struct Float3x4
//{
//	Vector4	T;
//	Vector4	axisX;
//	Vector4	axisY;
//	// Z-axis is computed as cross product X-axis X Y-axis
//};

/*
=======================================================================
	LOW-LEVEL FUNCTION PROTOTYPES
=======================================================================
*/

//=====================================================================
//	SCALAR OPERATIONS
//=====================================================================

float Float_Rcp( float x );	// reciprocal = 1/x
float Float_Abs( float x );
float Float_Floor( float x );
float Float_Ceiling( float x );
float Float_Modulo( float x, float y );
int Float_Round( float x );
int Float_Truncate( float x );
float Float_Exp2f( float power );
float Float_CopySign( float magnitude, float sign );
BYTE Float_ToByte( float x );

float Float_Sqrt( float x );
float Float_InvSqrt( float x );
float Float_InvSqrtEst( float x );

void Float_SinCos( float x, float &s, float &c );
float Float_Sin( float x );
float Float_Cos( float x );
float Float_Tan( float x );
float Float_ASin( float x );
float Float_ACos( float x );
float Float_ATan( float x );
float Float_ATan2( float y, float x );

float Float_Exp( float x );
float Float_Log( float x );
float Float_Log10( float x );
float Float_Pow( float x, float y );
float Float_Lerp( float from, float to, float amount );

//=====================================================================
//	DATA CONVERSION OPERATIONS
//=====================================================================

const Float2& Float3_As_Float2( const Float3& v );

const Float4& Vector4_As_Float4( const Vector4& v );
const Float3& Vector4_As_Float3( const Vector4& v );
const Float3& Float4_As_Float3( const Float4& v );

//=====================================================================
//	LOAD OPERATIONS
//=====================================================================

const Vector4	LoadFloat3( const Float3& v, float w );

//=====================================================================
//	STORE OPERATIONS
//=====================================================================

//=====================================================================
//	GENERAL VECTOR OPERATIONS
//=====================================================================

//=====================================================================
//	2D VECTOR OPERATIONS
//=====================================================================

const Float2	Float2_Zero();
const Float2 	Float2_Set( float x, float y );
const Float2 	Float2_Scale( const Float2& v, float s );
const float 	Float2_Length( const Float2& v );
const float		Float2_Dot( const Float2& a, const Float2& b );
const Float2	Float2_Lerp( const Float2& from, const Float2& to, float amount );
const Float2	Float2_Lerp01( const Float2& from, const Float2& to, float amount );

const Half2		Float2_To_Half2( const Float2& xy );

const Float2	Half2_To_Float2( const Half2& xy );

//=====================================================================
//	3D VECTOR OPERATIONS
//=====================================================================

const Float3	Float3_Zero();
const Float3 	Float3_Replicate( float value );
const Float3 	Float3_Set( float x, float y, float z );
const Float3	Float3_Scale( const Float3& v, float s );
const float 	Float3_LengthSquared( const Float3& v );
const float 	Float3_Length( const Float3& v );
const Float3 	Float3_Normalized( const Float3& v );
const Float3 	Float3_Abs( const Float3& v );
const Float3 	Float3_Negate( const Float3& v );
const Float3 	Float3_Reciprocal( const Float3& v );
const bool		Float3_IsInfinite( const Float3& v );
const bool		Float3_Equal( const Float3& a, const Float3& b );
const bool		Float3_Equal( const Float3& a, const Float3& b, const float epsilon );
const Float3 	Float3_Min( const Float3& a, const Float3& b );
const Float3 	Float3_Max( const Float3& a, const Float3& b );
const Float3 	Float3_Add( const Float3& a, const Float3& b );
const Float3 	Float3_Subtract( const Float3& a, const Float3& b );
const Float3 	Float3_Multiply( const Float3& a, const Float3& b );
const float		Float3_Dot( const Float3& a, const Float3& b );
const Float3	Float3_Cross( const Float3& a, const Float3& b );
const Float3	Float3_FindOrthogonalTo( const Float3& v );
const Float3	Float3_LerpFast( const Float3& x, const Float3& y, float a );
const Float3	Float3_Lerp( const Float3& from, const Float3& to, float amount );
const Float3	Float3_Lerp01( const Float3& from, const Float3& to, float amount );
const bool 		Float3_IsNormalized( const Float3& v, float epsilon = 1e-5f );
const bool 		Float3_AllGreaterOrEqual(  const Float3& xyz, float value );

//=====================================================================
//	4D VECTOR OPERATIONS
//=====================================================================

const Float4 	Float4_Zero();
const Float4 	Float4_Set( const Float3& xyz, float w );
const Float4 	Float4_Set( float x, float y, float z, float w );
const Float4	Float4_Replicate( float value );
const bool		Float4_Equal( const Float4& a, const Float4& b );
const bool		Float4_Equal( const Float4& a, const Float4& b, const float epsilon );

const Vector4	Vector4_Zero();
const Vector4	Vector4_Replicate( float value );
const Vector4	Vector4_Set( const Float3& xyz, float w );
const Vector4	Vector4_Set( float x, float y, float z, float w );
const float		Vector4_Get_X( Vector4Arg v );
const float		Vector4_Get_Y( Vector4Arg v );
const float 	Vector4_Get_Z( Vector4Arg v );
const float 	Vector4_Get_W( Vector4Arg v );
const Vector4 	Vector4_SplatX( Vector4Arg v );
const Vector4 	Vector4_SplatY( Vector4Arg v );
const Vector4 	Vector4_SplatZ( Vector4Arg v );
const Vector4 	Vector4_SplatW( Vector4Arg v );
const bool 		Vector4_IsNaN( Vector4Arg v );
const bool 		Vector4_IsInfinite( Vector4Arg v );
const bool 		Vector4_Equal( Vector4Arg a, Vector4Arg b );
const bool 		Vector4_NotEqual( Vector4Arg a, Vector4Arg b );
const float		Vector4_LengthSquared( Vector4Arg v );
const Vector4	Vector4_LengthSquaredV( Vector4Arg v );
const Vector4	Vector4_LengthV( Vector4Arg v );
const Vector4	Vector4_SqrtV( Vector4Arg v );
const Vector4	Vector4_ReciprocalSqrtV( Vector4Arg v );
const Vector4 	Vector4_ReciprocalLengthEstV( Vector4Arg v );
const Vector4 	Vector4_ReciprocalLengthV( Vector4Arg v );
const Vector4 	Vector4_Normalized( Vector4Arg v );
const Vector4	Vector4_Negate( Vector4Arg v );
const Vector4	Vector4_Scale( Vector4Arg v, float s );
const Vector4 	Vector4_Add( Vector4Arg a, Vector4Arg b );
const Vector4 	Vector4_Subtract( Vector4Arg a, Vector4Arg b );
const Vector4 	Vector4_Multiply( Vector4Arg a, Vector4Arg b );
const Vector4 	Vector4_MultiplyAdd( Vector4Arg a, Vector4Arg b, Vector4Arg c );
const float		Vector4_Dot( Vector4Arg a, Vector4Arg b );
const Vector4	Vector4_DotV( Vector4Arg a, Vector4Arg b );
void			Vector4_SinCos( Vector4Arg v, Vector4 *s, Vector4 *c );

//=====================================================================
//	QUATERNION OPERATIONS
//	Quaternion: q = u*sin(a/2) + cos(a/2),
//	where u - a unit vector (axis of rotation), a - angle of rotation;
//	if N - (normalized) axis vector, then Q = ( N*sin(a/2), cos(a/2) )
//	A quaternion only represents a rotation if it is normalized.
//	If it is not normalized, then there is also a uniform scale that accompanies the rotation.
//=====================================================================

const bool      Quaternion_Equal( Vector4Arg a, Vector4Arg b );
const bool      Quaternion_NotEqual( Vector4Arg a, Vector4Arg b );
const bool      Quaternion_IsNaN( Vector4Arg q );
const bool      Quaternion_IsInfinite( Vector4Arg q );
const bool      Quaternion_IsIdentity( Vector4Arg q );

const Vector4	Quaternion_Identity();
const Vector4	Quaternion_Dot( Vector4Arg a, Vector4Arg b );
const Vector4	Quaternion_Multiply( Vector4Arg a, Vector4Arg b );
const Vector4	Quaternion_LengthSq( Vector4Arg q );
const Vector4	Quaternion_ReciprocalLength( Vector4Arg q );
const Vector4	Quaternion_Length( Vector4Arg q );
const Vector4	Quaternion_NormalizeEst( Vector4Arg q );
const Vector4	Quaternion_Normalize( Vector4Arg q );
const Vector4	Quaternion_Conjugate( Vector4Arg q );
const Vector4	Quaternion_Inverse( Vector4Arg q );
const Vector4	Quaternion_Ln( Vector4Arg q );
const Vector4	Quaternion_Exp( Vector4Arg q );
const Vector4	Quaternion_Slerp( Vector4Arg Q0, Vector4Arg a, float t );
const Vector4	Quaternion_SlerpV( Vector4Arg Q0, Vector4Arg a, Vector4Arg T );
const Vector4	Quaternion_Squad( Vector4Arg Q0, Vector4Arg a, Vector4Arg b, Vector4Arg Q3, float t );
const Vector4	Quaternion_SquadV( Vector4Arg Q0, Vector4Arg a, Vector4Arg b, Vector4Arg Q3, Vector4Arg T );
void			Quaternion_SquadSetup(_Out_ Vector4* pA, _Out_ Vector4* pB, _Out_ Vector4* pC, Vector4Arg Q0, Vector4Arg a, Vector4Arg b, Vector4Arg Q3 );
const Vector4	Quaternion_BaryCentric( Vector4Arg Q0, Vector4Arg a, Vector4Arg b, float f, float g );
const Vector4	Quaternion_BaryCentricV( Vector4Arg Q0, Vector4Arg a, Vector4Arg b, Vector4Arg F, Vector4Arg G );

const Vector4	Quaternion_RotationPitchRollYaw( float pitch, float roll, float yaw );
const Vector4	Quaternion_RotationRollPitchYawFromVector( Vector4Arg Angles );
const Vector4	Quaternion_RotationNormal( Vector4Arg normalAxis, float angle );
const Vector4	Quaternion_RotationAxis( Vector4Arg axis, float angle );
const Vector4	Quaternion_RotationMatrix(Float4x4Arg M );

void			Quaternion_ToAxisAngle(_Out_ Vector4* pAxis, _Out_ float* pAngle, Vector4Arg q );

//=====================================================================
//	DUAL QUATERNIONS
//=====================================================================

const Vector8	DualQuaternion_Identity();

//=====================================================================
//	MATRIX OPERATIONS
//=====================================================================

const Float4x4 	Matrix_Identity();
const Float4x4 	Matrix_Scaling( float x, float y, float z );
const Float4x4 	Matrix_Translation( float x, float y, float z );
const Float4x4 	Matrix_Translation( Vector4Arg xyz );
const Vector4	Matrix_Transform( const Float4x4& m, Vector4Arg p );
const Vector4	Matrix_Transform3( const Float4x4& m, Vector4Arg p );
const Vector4	Matrix_TransformNormal3( const Float4x4& m, Vector4Arg p );
const Float4x4 	Matrix_Multiply( const Float4x4& a, const Float4x4& b );
const Float4x4 	Matrix_Transpose( const Float4x4& m );
const bool 		Matrix_TryInverse( Float4x4 * m );
const Float4x4 	Matrix_Inverse( const Float4x4& m );
const Float4x4 	Matrix_OrthoInverse( const Float4x4& m );
const Float4x4	ProjectionMatrix_Inverse( const Float4x4& m );
const Float4x4 	Matrix_LookAt( const Float3& eyePosition, const Float3& cameraTarget, const Float3& cameraUpVector );
const Float4x4 	Matrix_LookTo( const Float3& eyePosition, const Float3& lookDirection, const Float3& upDirection );
const Float4x4 	Matrix_PerspectiveOGL( float FoV_Y_radians, float aspect_ratio, float near_clip, float far_clip );
const Float4x4 	Matrix_PerspectiveD3D( float FoV_Y_radians, float aspect_ratio, float near_clip, float far_clip );
const Float4x4 	Matrix_OrthographicD3D( float width, float height, float near_clip, float far_clip );
const Float4x4 	Matrix_OrthographicD3D( float left, float right, float bottom, float top, float near_clip, float far_clip );
const Float4x4	Matrix_BuildTransform( const Float3& translation, const Float4& rotation, const Float3& scaling );
const Float4x4	Matrix_BuildTransform( const Float3& translation, const Float4& rotation );
const Float4	Matrix_Project( const Float4x4& m, const Float4& p );
const bool		Matrix_Equal( const Float4x4& A, const Float4x4& B );
const bool		Matrix_Equal( const Float4x4& A, const Float4x4& B, const float epsilon );

const Float4x4	Matrix_RotationX( float angle );
const Float4x4	Matrix_RotationY( float angle );
const Float4x4	Matrix_RotationZ( float angle );
const Float4x4	Matrix_RotationRollPitchYaw( float pitch, float yaw, float roll );
const Float4x4	Matrix_RotationRollPitchYawFromVector( Vector4Arg angles);
const Float4x4	Matrix_RotationNormal( Vector4Arg normalAxis, float angle );
const Float4x4	Matrix_RotationAxis( Vector4Arg axis, float angle );

const Float4x4	Matrix_FromQuaternion( Vector4Arg q );
const Float4x4	Matrix_FromRotationMatrix( const Float3x3& m );
const Float4x4	Matrix_FromAxes( const Float3& axisX, const Float3& axisY, const Float3& axisZ );
const Float4x4	Matrix_From_Float3x4( const Float3x4& m );

const Float3x3 	Float3x3_Transpose( const Float3x3& m );
const Float3x3 	Float3x3_Multiply( const Float3x3& a, const Float3x3& b );
const Float3	Float3x3_Transform( const Float3x3& m, const Float3& p );
const Float3x3	Float3x3_FromQuaternion( Vector4Arg q );
const Float3x3	Float3x3_GetSkewSymmetricMatrix( const Float3& v );
const Float3x3	Float3x3_RotationX( float angle );
const Float3x3	Float3x3_RotationY( float angle );
const Float3x3	Float3x3_RotationZ( float angle );
const Float3x3	Float3x3_RotationPitchRollYaw( float pitch, float roll, float yaw );
const Float3x3	Float3x3_RotationNormal( const Float3& normalAxis, float angle );
const Float3	Float3x3_ExtractPitchYawRoll( const Float3x3& m );

const Float3	Matrix_GetTranslation( const Float4x4& m );
const Float3	Matrix_TransformPoint( const Float4x4& m, const Float3& p );
const Float3	Matrix_TransformNormal( const Float4x4& m, const Float3& v );

const Float3x4	Float3x4_Identity();
const Float3x4	Float3x4_Pack( const Float4x4& m );
const Float4x4	Float3x4_Unpack( const Float3x4& m );
const Float3	Float3x4_GetTranslation( const Float3x4& m );

//=====================================================================
//	PLANE OPERATIONS
//=====================================================================
const Float4 Plane_FromPointNormal( const Float3& point, const Float3& normal );
const Float3 Plane_CalculateNormal( const Float4& plane );
const Float3 Plane_GetNormal( const Float4& plane );
const float Plane_PointDistance( const Float4& plane, const Float3& point );

//=====================================================================
//	COLOR OPERATIONS
//=====================================================================

//=====================================================================
//	MISCELLANEOUS OPERATIONS
//=====================================================================

float Get_Field_of_View_X_Degrees( float FoV_Y_degrees, float width, float height );
float Get_Field_of_View_Y_Degrees( float FoV_X_degrees, float width, float height );

const Float4x4 	Matrix_BuildView(
								 const Float3& rightDirection,
								 const Float3& lookDirection,
								 const Float3& upDirection,
								 const Float3& eyePosition
								 );

/*
-----------------------------------------------------------------------
	These functions can be used for compressing floating-point normal vectors.

	Quote from 'Rubicon':
	http://www.gamedev.net/topic/564892-rebuilding-normal-from-2-components-with-cross-product/
	Trust me, 8 bits per component is plenty,
	especially if it gets your whole vertex to 32 bytes or lower.
	I've used it for years and nobody's ever even noticed
	- you get a perfect spread of lighting across a ball at any magnification you like
	- don't forget its interpolated per pixel
	- all this decompression happens at the VS level, not the PS.

	Excerpt from ShaderX:
	Normalized normals or tangents are usually stored as three floats (D3DVSDT_FLOAT3), but
	this precision is not needed. Each component has a range of –1.0 to 1.0 (due to normalization),
	ideal for quantization. We don’t need any integer components, so we can devote all bits to the
	fractional scale and a sign bit. Biasing the floats makes them fit nicely into an unsigned byte.
	For most normals, 8 bits will be enough (that’s 16.7 million values over the surface of a
	unit sphere). Using D3DVSDT_UBYTE4, we have to change the vertex shader to multiply by
	1.0/127.5 and subtract 1.0 to return to signed floats. I’ll come back to the optimizations that
	you may have noticed, but for now, I’ll accept the one cycle cost for a reduction in memory
	and bandwidth saving of one-fourth for normals and tangents.
	Quantization vertex shader example:
	; v1 = normal in range 0 to 255 (integer only)
	; c0 = <1.0/127.5, -1.0, ???? , ????>
	mad r0, v1, c0.xxxx, c0.yyyy ; multiply compressed normal by 1/127.5, subtract 1

	Also, see:
	http://flohofwoe.blogspot.com/2008/03/vertex-component-packing.html
-----------------------------------------------------------------------
*/

// [0..1]f => [0..255]i
static inline UINT8 _NormalizedFloatToUInt8( float f ) {
	return f * 255.0f;
}
// [0..255]i => [0..1]f
static inline float _UInt8ToNormalizedFloat( UINT8 c ) {
	return c * 1.0f/255.0f;
}

// [-1..+1] float => [0..255] int
static inline UINT8 _NormalToUInt8( float f ) {
	// scale and bias: ((f + 1.0f) * 0.5f) * 255.0f
	return Float_Truncate( f * 127.5f + 127.5f );
}
// [0..255] int -> [-1..+1] float
static inline float _UInt8ToNormal( UINT8 c ) {
	return (c * 1.0f / 127.5f) - 1.0f;
}

inline UByte4 PackNormal( float x, float y, float z ) {
	UByte4 packed = { _NormalToUInt8(x), _NormalToUInt8(y), _NormalToUInt8(z) };
	return packed;
}
inline Float3 UnpackNormal( const UByte4& v ) {
	Float3 N = { _UInt8ToNormal(v.x), _UInt8ToNormal(v.y), _UInt8ToNormal(v.z) };
	return N;
}

inline UByte4 PackNormal( const Float3& n ) { return PackNormal(n.x, n.y, n.z); }

mxSWIPED("Doom3 BFG edition");

// GPU half-float bit patterns
#define HF_MANTISSA(x)	(x&1023)
#define HF_EXP(x)		((x&32767)>>10)
#define HF_SIGN(x)		((x&32768)?-1:1)

inline float Half_To_Float( Half x ) {
	int e = HF_EXP( x );
	int m = HF_MANTISSA( x );
	int s = HF_SIGN( x );

	if ( 0 < e && e < 31 ) {
		return s * powf( 2.0f, ( e - 15.0f ) ) * ( 1 + m / 1024.0f );
	} else if ( m == 0 ) {
        return s * 0.0f;
	}
    return s * powf( 2.0f, -14.0f ) * ( m / 1024.0f );
}

inline Half Float_To_Half( float a ) {
	unsigned int f = *(unsigned *)( &a );
	unsigned int signbit  = ( f & 0x80000000 ) >> 16;
	int exponent = ( ( f & 0x7F800000 ) >> 23 ) - 112;
	unsigned int mantissa = ( f & 0x007FFFFF );

	if ( exponent <= 0 ) {
		return 0;
	}
	if ( exponent > 30 ) {
		return (Half)( signbit | 0x7BFF );
	}

	return (Half)( signbit | ( exponent << 10 ) | ( mantissa >> 13 ) );
}

/*
=======================================================================
	GLOBALS
=======================================================================
*/

// The purpose of the following global constants is to prevent redundant 
// reloading of the constants when they are referenced by more than one
// separate inline math routine called within the same function.  Declaring
// a constant locally within a routine is sufficient to prevent redundant
// reloads of that constant when that single routine is called multiple
// times in a function, but if the constant is used (and declared) in a 
// separate math routine it would be reloaded.

#define mmGLOBALCONST extern const __declspec(selectany)

mmGLOBALCONST __m128 g_SIMD_Zero	= { 0.0f, 0.0f, 0.0f, 0.0f };
mmGLOBALCONST __m128 g_SIMD_255	= { 255.0f, 255.0f, 255.0f, 255.0f };

mmGLOBALCONST Vector4f g_MMSinCoefficients0    = {1.0f, -0.166666667f, 8.333333333e-3f, -1.984126984e-4f};
mmGLOBALCONST Vector4f g_MMSinCoefficients1    = {2.755731922e-6f, -2.505210839e-8f, 1.605904384e-10f, -7.647163732e-13f};
mmGLOBALCONST Vector4f g_MMSinCoefficients2    = {2.811457254e-15f, -8.220635247e-18f, 1.957294106e-20f, -3.868170171e-23f};
mmGLOBALCONST Vector4f g_MMCosCoefficients0    = {1.0f, -0.5f, 4.166666667e-2f, -1.388888889e-3f};
mmGLOBALCONST Vector4f g_MMCosCoefficients1    = {2.480158730e-5f, -2.755731922e-7f, 2.087675699e-9f, -1.147074560e-11f};
mmGLOBALCONST Vector4f g_MMCosCoefficients2    = {4.779477332e-14f, -1.561920697e-16f, 4.110317623e-19f, -8.896791392e-22f};
mmGLOBALCONST Vector4f g_MMTanCoefficients0    = {1.0f, 0.333333333f, 0.133333333f, 5.396825397e-2f};
mmGLOBALCONST Vector4f g_MMTanCoefficients1    = {2.186948854e-2f, 8.863235530e-3f, 3.592128167e-3f, 1.455834485e-3f};
mmGLOBALCONST Vector4f g_MMTanCoefficients2    = {5.900274264e-4f, 2.391290764e-4f, 9.691537707e-5f, 3.927832950e-5f};
mmGLOBALCONST Vector4f g_MMASinCoefficients0   = {-0.05806367563904f, -0.41861972469416f, 0.22480114791621f, 2.17337241360606f};
mmGLOBALCONST Vector4f g_MMASinCoefficients1   = {0.61657275907170f, 4.29696498283455f, -1.18942822255452f, -6.53784832094831f};
mmGLOBALCONST Vector4f g_MMASinCoefficients2   = {-1.36926553863413f, -4.48179294237210f, 1.41810672941833f, 5.48179257935713f};
mmGLOBALCONST Vector4f g_MMATanCoefficients0   = {1.0f, 0.333333334f, 0.2f, 0.142857143f};
mmGLOBALCONST Vector4f g_MMATanCoefficients1   = {1.111111111e-1f, 9.090909091e-2f, 7.692307692e-2f, 6.666666667e-2f};
mmGLOBALCONST Vector4f g_MMATanCoefficients2   = {5.882352941e-2f, 5.263157895e-2f, 4.761904762e-2f, 4.347826087e-2f};
mmGLOBALCONST Vector4f g_MMSinEstCoefficients  = {1.0f, -1.66521856991541e-1f, 8.199913018755e-3f, -1.61475937228e-4f};
mmGLOBALCONST Vector4f g_MMCosEstCoefficients  = {1.0f, -4.95348008918096e-1f, 3.878259962881e-2f, -9.24587976263e-4f};
mmGLOBALCONST Vector4f g_MMTanEstCoefficients  = {2.484f, -1.954923183e-1f, 2.467401101f, mxINV_PI};
mmGLOBALCONST Vector4f g_MMATanEstCoefficients = {7.689891418951e-1f, 1.104742493348f, 8.661844266006e-1f, mxHALF_PI};
mmGLOBALCONST Vector4f g_MMASinEstCoefficients = {-1.36178272886711f, 2.37949493464538f, -8.08228565650486e-1f, 2.78440142746736e-1f};
mmGLOBALCONST Vector4f g_MMASinEstConstants    = {1.00000011921f, mxHALF_PI, 0.0f, 0.0f};
mmGLOBALCONST Vector4f g_MMPiConstants0        = {mxPI, mxTWO_PI, mxINV_PI, mxINV_TWOPI};
mmGLOBALCONST Vector4f g_MMIdentityR0          = {1.0f, 0.0f, 0.0f, 0.0f};
mmGLOBALCONST Vector4f g_MMIdentityR1          = {0.0f, 1.0f, 0.0f, 0.0f};
mmGLOBALCONST Vector4f g_MMIdentityR2          = {0.0f, 0.0f, 1.0f, 0.0f};
mmGLOBALCONST Vector4f g_MMIdentityR3          = {0.0f, 0.0f, 0.0f, 1.0f};
mmGLOBALCONST Vector4f g_MMNegIdentityR0       = {-1.0f,0.0f, 0.0f, 0.0f};
mmGLOBALCONST Vector4f g_MMNegIdentityR1       = {0.0f,-1.0f, 0.0f, 0.0f};
mmGLOBALCONST Vector4f g_MMNegIdentityR2       = {0.0f, 0.0f,-1.0f, 0.0f};
mmGLOBALCONST Vector4f g_MMNegIdentityR3       = {0.0f, 0.0f, 0.0f,-1.0f};
mmGLOBALCONST Vector4i g_MMNegativeZero      = {0x80000000, 0x80000000, 0x80000000, 0x80000000};
mmGLOBALCONST Vector4i g_MMNegate3           = {0x80000000, 0x80000000, 0x80000000, 0x00000000};
mmGLOBALCONST Vector4i g_MMMask3             = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000};
mmGLOBALCONST Vector4i g_MMMaskX             = {0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000};
mmGLOBALCONST Vector4i g_MMMaskY             = {0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000};
mmGLOBALCONST Vector4i g_MMMaskZ             = {0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000};
mmGLOBALCONST Vector4i g_MMMaskW             = {0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF};
mmGLOBALCONST Vector4f g_MMOne               = { 1.0f, 1.0f, 1.0f, 1.0f};
mmGLOBALCONST Vector4f g_MMOne3              = { 1.0f, 1.0f, 1.0f, 0.0f};
mmGLOBALCONST Vector4f g_MMZero              = { 0.0f, 0.0f, 0.0f, 0.0f};
mmGLOBALCONST Vector4f g_MMNegativeOne       = {-1.0f,-1.0f,-1.0f,-1.0f};
mmGLOBALCONST Vector4f g_MMOneHalf           = { 0.5f, 0.5f, 0.5f, 0.5f};
mmGLOBALCONST Vector4f g_MMNegativeOneHalf   = {-0.5f,-0.5f,-0.5f,-0.5f};
mmGLOBALCONST Vector4f g_MMNegativeTwoPi     = {-mxTWO_PI, -mxTWO_PI, -mxTWO_PI, -mxTWO_PI};
mmGLOBALCONST Vector4f g_MMNegativePi        = {-mxPI, -mxPI, -mxPI, -mxPI};
mmGLOBALCONST Vector4f g_MMHalfPi            = {mxHALF_PI, mxHALF_PI, mxHALF_PI, mxHALF_PI};
mmGLOBALCONST Vector4f g_MMPi                = {mxPI, mxPI, mxPI, mxPI};
mmGLOBALCONST Vector4f g_MMReciprocalPi      = {mxINV_PI, mxINV_PI, mxINV_PI, mxINV_PI};
mmGLOBALCONST Vector4f g_MMTwoPi             = {mxTWO_PI, mxTWO_PI, mxTWO_PI, mxTWO_PI};
mmGLOBALCONST Vector4f g_MMReciprocalTwoPi   = {mxINV_TWOPI, mxINV_TWOPI, mxINV_TWOPI, mxINV_TWOPI};
mmGLOBALCONST Vector4f g_MMEpsilon           = {1.192092896e-7f, 1.192092896e-7f, 1.192092896e-7f, 1.192092896e-7f};
mmGLOBALCONST Vector4i g_MMInfinity          = {0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000};
mmGLOBALCONST Vector4i g_MMQNaN              = {0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000};
mmGLOBALCONST Vector4i g_MMQNaNTest          = {0x007FFFFF, 0x007FFFFF, 0x007FFFFF, 0x007FFFFF};
mmGLOBALCONST Vector4i g_MMAbsMask           = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};
mmGLOBALCONST Vector4i g_MMFltMin            = {0x00800000, 0x00800000, 0x00800000, 0x00800000};
mmGLOBALCONST Vector4i g_MMFltMax            = {0x7F7FFFFF, 0x7F7FFFFF, 0x7F7FFFFF, 0x7F7FFFFF};
mmGLOBALCONST Vector4i g_MMNegOneMask		= {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
mmGLOBALCONST Vector4i g_MMMaskA8R8G8B8      = {0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000};
mmGLOBALCONST Vector4i g_MMFlipA8R8G8B8      = {0x00000000, 0x00000000, 0x00000000, 0x80000000};
mmGLOBALCONST Vector4f g_MMFixAA8R8G8B8      = {0.0f,0.0f,0.0f,(float)(0x80000000U)};
mmGLOBALCONST Vector4f g_MMNormalizeA8R8G8B8 = {1.0f/(255.0f*(float)(0x10000)),1.0f/(255.0f*(float)(0x100)),1.0f/255.0f,1.0f/(255.0f*(float)(0x1000000))};
mmGLOBALCONST Vector4i g_MMMaskA2B10G10R10   = {0x000003FF, 0x000FFC00, 0x3FF00000, 0xC0000000};
mmGLOBALCONST Vector4i g_MMFlipA2B10G10R10   = {0x00000200, 0x00080000, 0x20000000, 0x80000000};
mmGLOBALCONST Vector4f g_MMFixAA2B10G10R10   = {-512.0f,-512.0f*(float)(0x400),-512.0f*(float)(0x100000),(float)(0x80000000U)};
mmGLOBALCONST Vector4f g_MMNormalizeA2B10G10R10 = {1.0f/511.0f,1.0f/(511.0f*(float)(0x400)),1.0f/(511.0f*(float)(0x100000)),1.0f/(3.0f*(float)(0x40000000))};
mmGLOBALCONST Vector4i g_MMMaskX16Y16        = {0x0000FFFF, 0xFFFF0000, 0x00000000, 0x00000000};
mmGLOBALCONST Vector4i g_MMFlipX16Y16        = {0x00008000, 0x00000000, 0x00000000, 0x00000000};
mmGLOBALCONST Vector4f g_MMFixX16Y16         = {-32768.0f,0.0f,0.0f,0.0f};
mmGLOBALCONST Vector4f g_MMNormalizeX16Y16   = {1.0f/32767.0f,1.0f/(32767.0f*65536.0f),0.0f,0.0f};
mmGLOBALCONST Vector4i g_MMMaskX16Y16Z16W16  = {0x0000FFFF, 0x0000FFFF, 0xFFFF0000, 0xFFFF0000};
mmGLOBALCONST Vector4i g_MMFlipX16Y16Z16W16  = {0x00008000, 0x00008000, 0x00000000, 0x00000000};
mmGLOBALCONST Vector4f g_MMFixX16Y16Z16W16   = {-32768.0f,-32768.0f,0.0f,0.0f};
mmGLOBALCONST Vector4f g_MMNormalizeX16Y16Z16W16 = {1.0f/32767.0f,1.0f/32767.0f,1.0f/(32767.0f*65536.0f),1.0f/(32767.0f*65536.0f)};
mmGLOBALCONST Vector4f g_MMNoFraction        = {8388608.0f,8388608.0f,8388608.0f,8388608.0f};
mmGLOBALCONST Vector4i g_MMMaskByte          = {0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF};
mmGLOBALCONST Vector4f g_MMNegateX           = {-1.0f, 1.0f, 1.0f, 1.0f};
mmGLOBALCONST Vector4f g_MMNegateY           = { 1.0f,-1.0f, 1.0f, 1.0f};
mmGLOBALCONST Vector4f g_MMNegateZ           = { 1.0f, 1.0f,-1.0f, 1.0f};
mmGLOBALCONST Vector4f g_MMNegateW           = { 1.0f, 1.0f, 1.0f,-1.0f};
mmGLOBALCONST Vector4i g_MMSelect0101        = {mmSELECT_0, mmSELECT_1, mmSELECT_0, mmSELECT_1};
mmGLOBALCONST Vector4i g_MMSelect1010        = {mmSELECT_1, mmSELECT_0, mmSELECT_1, mmSELECT_0};
mmGLOBALCONST Vector4i g_MMOneHalfMinusEpsilon = { 0x3EFFFFFD, 0x3EFFFFFD, 0x3EFFFFFD, 0x3EFFFFFD};
mmGLOBALCONST Vector4i g_MMSelect1000        = {mmSELECT_1, mmSELECT_0, mmSELECT_0, mmSELECT_0};
mmGLOBALCONST Vector4i g_MMSelect1100        = {mmSELECT_1, mmSELECT_1, mmSELECT_0, mmSELECT_0};
mmGLOBALCONST Vector4i g_MMSelect1110        = {mmSELECT_1, mmSELECT_1, mmSELECT_1, mmSELECT_0};
mmGLOBALCONST Vector4i g_MMSwizzleXYXY       = {mmPERMUTE_0X, mmPERMUTE_0Y, mmPERMUTE_0X, mmPERMUTE_0Y};
mmGLOBALCONST Vector4i g_MMSwizzleXYZX       = {mmPERMUTE_0X, mmPERMUTE_0Y, mmPERMUTE_0Z, mmPERMUTE_0X};
mmGLOBALCONST Vector4i g_MMSwizzleYXZW       = {mmPERMUTE_0Y, mmPERMUTE_0X, mmPERMUTE_0Z, mmPERMUTE_0W};
mmGLOBALCONST Vector4i g_MMSwizzleYZXW       = {mmPERMUTE_0Y, mmPERMUTE_0Z, mmPERMUTE_0X, mmPERMUTE_0W};
mmGLOBALCONST Vector4i g_MMSwizzleZXYW       = {mmPERMUTE_0Z, mmPERMUTE_0X, mmPERMUTE_0Y, mmPERMUTE_0W};
mmGLOBALCONST Vector4i g_MMPermute0X0Y1X1Y   = {mmPERMUTE_0X, mmPERMUTE_0Y, mmPERMUTE_1X, mmPERMUTE_1Y};
mmGLOBALCONST Vector4i g_MMPermute0Z0W1Z1W   = {mmPERMUTE_0Z, mmPERMUTE_0W, mmPERMUTE_1Z, mmPERMUTE_1W};
mmGLOBALCONST Vector4f g_MMFixupY16          = {1.0f,1.0f/65536.0f,0.0f,0.0f};
mmGLOBALCONST Vector4f g_MMFixupY16W16       = {1.0f,1.0f,1.0f/65536.0f,1.0f/65536.0f};
mmGLOBALCONST Vector4i g_MMFlipY             = {0,0x80000000,0,0};
mmGLOBALCONST Vector4i g_MMFlipZ             = {0,0,0x80000000,0};
mmGLOBALCONST Vector4i g_MMFlipW             = {0,0,0,0x80000000};
mmGLOBALCONST Vector4i g_MMFlipYZ            = {0,0x80000000,0x80000000,0};
mmGLOBALCONST Vector4i g_MMFlipZW            = {0,0,0x80000000,0x80000000};
mmGLOBALCONST Vector4i g_MMFlipYW            = {0,0x80000000,0,0x80000000};
mmGLOBALCONST Vector4i g_MMMaskHenD3         = {0x7FF,0x7ff<<11,0x3FF<<22,0};
mmGLOBALCONST Vector4i g_MMMaskDHen3         = {0x3FF,0x7ff<<10,0x7FF<<21,0};
mmGLOBALCONST Vector4f g_MMAddUHenD3         = {0,0,32768.0f*65536.0f,0};
mmGLOBALCONST Vector4f g_MMAddHenD3          = {-1024.0f,-1024.0f*2048.0f,0,0};
mmGLOBALCONST Vector4f g_MMAddDHen3          = {-512.0f,-1024.0f*1024.0f,0,0};
mmGLOBALCONST Vector4f g_MMMulHenD3          = {1.0f,1.0f/2048.0f,1.0f/(2048.0f*2048.0f),0};
mmGLOBALCONST Vector4f g_MMMulDHen3          = {1.0f,1.0f/1024.0f,1.0f/(1024.0f*2048.0f),0};
mmGLOBALCONST Vector4i g_MMXorHenD3          = {0x400,0x400<<11,0,0};
mmGLOBALCONST Vector4i g_MMXorDHen3          = {0x200,0x400<<10,0,0};
mmGLOBALCONST Vector4i g_MMMaskIco4          = {0xFFFFF,0xFFFFF000,0xFFFFF,0xF0000000};
mmGLOBALCONST Vector4i g_MMXorXIco4          = {0x80000,0,0x80000,0x80000000};
mmGLOBALCONST Vector4i g_MMXorIco4           = {0x80000,0,0x80000,0};
mmGLOBALCONST Vector4f g_MMAddXIco4          = {-8.0f*65536.0f,0,-8.0f*65536.0f,32768.0f*65536.0f};
mmGLOBALCONST Vector4f g_MMAddUIco4          = {0,32768.0f*65536.0f,0,32768.0f*65536.0f};
mmGLOBALCONST Vector4f g_MMAddIco4           = {-8.0f*65536.0f,0,-8.0f*65536.0f,0};
mmGLOBALCONST Vector4f g_MMMulIco4           = {1.0f,1.0f/4096.0f,1.0f,1.0f/(4096.0f*65536.0f)};
mmGLOBALCONST Vector4i g_MMMaskDec4          = {0x3FF,0x3FF<<10,0x3FF<<20,0x3<<30};
mmGLOBALCONST Vector4i g_MMXorDec4           = {0x200,0x200<<10,0x200<<20,0};
mmGLOBALCONST Vector4f g_MMAddUDec4          = {0,0,0,32768.0f*65536.0f};
mmGLOBALCONST Vector4f g_MMAddDec4           = {-512.0f,-512.0f*1024.0f,-512.0f*1024.0f*1024.0f,0};
mmGLOBALCONST Vector4f g_MMMulDec4           = {1.0f,1.0f/1024.0f,1.0f/(1024.0f*1024.0f),1.0f/(1024.0f*1024.0f*1024.0f)};
mmGLOBALCONST Vector4i g_MMMaskByte4         = {0xFF,0xFF00,0xFF0000,0xFF000000};
mmGLOBALCONST Vector4i g_MMXorByte4          = {0x80,0x8000,0x800000,0x00000000};
mmGLOBALCONST Vector4f g_MMAddByte4          = {-128.0f,-128.0f*256.0f,-128.0f*65536.0f,0};

mmGLOBALCONST Float4 g_Float4_UnitX	= { 1.0f, 0.0f, 0.0f, 0.0f };
mmGLOBALCONST Float4 g_Float4_UnitY	= { 0.0f, 1.0f, 0.0f, 0.0f };
mmGLOBALCONST Float4 g_Float4_UnitZ	= { 0.0f, 0.0f, 1.0f, 0.0f };

#define Float4_Forward	g_Float4_UnitY
#define Float4_Right	g_Float4_UnitX
#define Float4_Up		g_Float4_UnitZ

#include "NewMath.inl"

#if MM_ENABLE_INTRINSICS
	#include "NewMathSSE.inl"
#else
	#include "NewMathFPU.inl"
#endif

/*
=======================================================================
	REFLECTION
=======================================================================
*/
#if MM_ENABLE_REFLECTION

#include <Base/Object/Reflection.h>
#include <Base/Object/StructDescriptor.h>

mxDECLARE_STRUCT(Float2);
mxDECLARE_STRUCT(Float3);
mxDECLARE_STRUCT(Float4);
mxDECLARE_STRUCT(Float4x4);
mxDECLARE_STRUCT(Float3x4);

mxDECLARE_POD_TYPE(Float2);
mxDECLARE_POD_TYPE(Float3);
mxDECLARE_POD_TYPE(Float4);
mxDECLARE_POD_TYPE(Float4x4);
mxDECLARE_POD_TYPE(Float3x4);

mxDECLARE_STRUCT(Half2);

#endif // MM_ENABLE_REFLECTION

/*
=======================================================================
	LOGGING
=======================================================================
*/
class ATextStream;

ATextStream & operator << ( ATextStream & log, const Float2& v );
ATextStream & operator << ( ATextStream & log, const Float3& v );
ATextStream & operator << ( ATextStream & log, const Float4& v );
ATextStream & operator << ( ATextStream & log, const Vector4& v );
ATextStream & operator << ( ATextStream & log, const Float4x4& m );

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
