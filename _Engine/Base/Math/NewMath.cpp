/*
=============================================================================
Useful references:

Practical Cross Platform SIMD Math
http://www.gamedev.net/page/resources/_/technical/general-programming/practical-cross-platform-simd-math-r3068

SIMDíifying multi-platform math
http://molecularmusings.wordpress.com/2011/10/18/simdifying-multi-platform-math/

http://seanmiddleditch.com/journal/2012/08/matrices-handedness-pre-and-post-multiplication-row-vs-column-major-and-notations/

Rotation About an Arbitrary Axis in 3 Dimensions, Glenn Murray, June 6, 2013
http://inside.mines.edu/~gmurray/ArbitraryAxisRotation/

avoiding trigonometry
http://www.iquilezles.org/www/articles/noacos/noacos.htm

Quaternions: How
http://physicsforgames.blogspot.ru/2010/02/quaternions.html

Useful math derivations:
http://www.euclideanspace.com/maths/

Perspective projections in LH and RH systems:
http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/perspective-projections-in-lh-and-rh-systems-r3598

—ËÒÚÂÏ‡  ÓÓ‰ËÌ‡Ú LHvsRH Yup vs Zup:
http://www.gamedev.ru/code/forum/?id=185904

@todo:
	The behaviour of Vector4_Normalize() is incorrect.
=============================================================================
*/

#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#include <Base/Math/NewMath.h>

#if MM_ENABLE_REFLECTION

mxBEGIN_STRUCT(Float2)
	mxMEMBER_FIELD(x),
	mxMEMBER_FIELD(y),
mxEND_REFLECTION

mxBEGIN_STRUCT(Float3)
	mxMEMBER_FIELD(x),
	mxMEMBER_FIELD(y),
	mxMEMBER_FIELD(z),
mxEND_REFLECTION

mxBEGIN_STRUCT(Float4)
	mxMEMBER_FIELD(x),
	mxMEMBER_FIELD(y),
	mxMEMBER_FIELD(z),
	mxMEMBER_FIELD(w),
mxEND_REFLECTION

mxBEGIN_STRUCT(Float4x4)
	mxMEMBER_FIELD(r0),
	mxMEMBER_FIELD(r1),
	mxMEMBER_FIELD(r2),
	mxMEMBER_FIELD(r3),
mxEND_REFLECTION

mxBEGIN_STRUCT(Float3x4)
	mxMEMBER_FIELD(r),
mxEND_REFLECTION

mxBEGIN_STRUCT(Half2)
	mxMEMBER_FIELD(x),
	mxMEMBER_FIELD(y),
mxEND_REFLECTION

#endif // MM_ENABLE_REFLECTION

/*
Provide functions to build both left-handed and right-handed projection matrices (both orthographic and perspective projections).
Provide functions for building view matrices using LookAt for both left-handed and right-handed coordinate systems.
This would make this library useful for both OpenGL and DirectX applications (or OpenGL applications with a left-handed coordinate system!).
*/

const bool Matrix_TryInverse( Float4x4 * m )
{
UNDONE;
return false;
}
// http://stackoverflow.com/a/9614511/4232223
// based on Laplace Expansion Theorem: http://www.geometrictools.com/Documentation/LaplaceExpansionTheorem.pdf
//
const Float4x4 Matrix_Inverse( const Float4x4& m )
{
	float s0 = m.m[0][0] * m.m[1][1] - m.m[1][0] * m.m[0][1];
	float s1 = m.m[0][0] * m.m[1][2] - m.m[1][0] * m.m[0][2];
	float s2 = m.m[0][0] * m.m[1][3] - m.m[1][0] * m.m[0][3];
	float s3 = m.m[0][1] * m.m[1][2] - m.m[1][1] * m.m[0][2];
	float s4 = m.m[0][1] * m.m[1][3] - m.m[1][1] * m.m[0][3];
	float s5 = m.m[0][2] * m.m[1][3] - m.m[1][2] * m.m[0][3];

	float c5 = m.m[2][2] * m.m[3][3] - m.m[3][2] * m.m[2][3];
	float c4 = m.m[2][1] * m.m[3][3] - m.m[3][1] * m.m[2][3];
	float c3 = m.m[2][1] * m.m[3][2] - m.m[3][1] * m.m[2][2];
	float c2 = m.m[2][0] * m.m[3][3] - m.m[3][0] * m.m[2][3];
	float c1 = m.m[2][0] * m.m[3][2] - m.m[3][0] * m.m[2][2];
	float c0 = m.m[2][0] * m.m[3][1] - m.m[3][0] * m.m[2][1];

	// Should check for 0 determinant
	float det = (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);
	mxASSERT(det != 0);
	float invdet = 1.0 / det;

	Float4x4 result;

	result.m[0][0] = ( m.m[1][1] * c5 - m.m[1][2] * c4 + m.m[1][3] * c3) * invdet;
	result.m[0][1] = (-m.m[0][1] * c5 + m.m[0][2] * c4 - m.m[0][3] * c3) * invdet;
	result.m[0][2] = ( m.m[3][1] * s5 - m.m[3][2] * s4 + m.m[3][3] * s3) * invdet;
	result.m[0][3] = (-m.m[2][1] * s5 + m.m[2][2] * s4 - m.m[2][3] * s3) * invdet;

	result.m[1][0] = (-m.m[1][0] * c5 + m.m[1][2] * c2 - m.m[1][3] * c1) * invdet;
	result.m[1][1] = ( m.m[0][0] * c5 - m.m[0][2] * c2 + m.m[0][3] * c1) * invdet;
	result.m[1][2] = (-m.m[3][0] * s5 + m.m[3][2] * s2 - m.m[3][3] * s1) * invdet;
	result.m[1][3] = ( m.m[2][0] * s5 - m.m[2][2] * s2 + m.m[2][3] * s1) * invdet;

	result.m[2][0] = ( m.m[1][0] * c4 - m.m[1][1] * c2 + m.m[1][3] * c0) * invdet;
	result.m[2][1] = (-m.m[0][0] * c4 + m.m[0][1] * c2 - m.m[0][3] * c0) * invdet;
	result.m[2][2] = ( m.m[3][0] * s4 - m.m[3][1] * s2 + m.m[3][3] * s0) * invdet;
	result.m[2][3] = (-m.m[2][0] * s4 + m.m[2][1] * s2 - m.m[2][3] * s0) * invdet;

	result.m[3][0] = (-m.m[1][0] * c3 + m.m[1][1] * c1 - m.m[1][2] * c0) * invdet;
	result.m[3][1] = ( m.m[0][0] * c3 - m.m[0][1] * c1 + m.m[0][2] * c0) * invdet;
	result.m[3][2] = (-m.m[3][0] * s3 + m.m[3][1] * s1 - m.m[3][2] * s0) * invdet;
	result.m[3][3] = ( m.m[2][0] * s3 - m.m[2][1] * s1 + m.m[2][2] * s0) * invdet;

	return result;
}
// Builds a right-handed look-at (or view) matrix.
// A view transformation matrix transforms world coordinates into the camera or view space.
// Parameters:
// 'eyePosition' - camera position
// 'cameraTarget' - camera look-at target
// 'cameraUpVector' - camera UP vector
//
const Float4x4 Matrix_LookAt( const Float3& eyePosition, const Float3& cameraTarget, const Float3& cameraUpVector )
{
    mxASSERT(!Float3_Equal(cameraUpVector, Float3_Zero()));
    mxASSERT(!Float3_IsInfinite(cameraUpVector));

	Float3	axisX;	// camera right vector
	Float3	axisY;	// camera forward vector
	Float3	axisZ;	// camera up vector

	axisY = Float3_Normalized( Float3_Subtract( cameraTarget, eyePosition ) );
	axisX = Float3_Normalized( Float3_Cross( axisY, cameraUpVector ) );
	axisZ = Float3_Normalized( Float3_Cross( axisX, axisY ) );

	float Tx = -Float3_Dot( axisX, eyePosition );
	float Ty = -Float3_Dot( axisY, eyePosition );
	float Tz = -Float3_Dot( axisZ, eyePosition );

	Float4x4	result;
	result.r0 = Float4_Set( axisX.x,	axisY.x,	axisZ.x,	0.0f );
	result.r1 = Float4_Set( axisX.y,	axisY.y,	axisZ.y,	0.0f );
	result.r2 = Float4_Set( axisX.z,	axisY.z,	axisZ.z,	0.0f );
	result.r3 = Float4_Set( Tx,		Ty,			Tz,			1.0f );
	return result;
}

// Builds a right-handed look-at (or view) matrix.
// View transformation matrix transforms world coordinates into camera or view space.
// 'eyePosition' - camera position
// 'lookDirection' - camera look direction
// 'upDirection' - camera UP vector
const Float4x4 Matrix_LookTo( const Float3& eyePosition, const Float3& lookDirection, const Float3& upDirection )
{
    mxASSERT(!Float3_Equal(lookDirection, Float3_Zero()));
    mxASSERT(!Float3_IsInfinite(lookDirection));
    mxASSERT(!Float3_Equal(upDirection, Float3_Zero()));
    mxASSERT(!Float3_IsInfinite(upDirection));

	Float3	axisX;	// camera right vector
	Float3	axisY;	// camera forward vector
	Float3	axisZ;	// camera up vector

	axisY = Float3_Normalized( lookDirection );

	axisX = Float3_Cross( axisY, upDirection );
	axisX = Float3_Normalized( axisX );

	axisZ = Float3_Cross( axisX, axisY );
	axisZ = Float3_Normalized( axisZ );

	Float4x4	cameraWorldMatrix;
	cameraWorldMatrix.r0 = Float4_Set( axisX, 0 );
	cameraWorldMatrix.r1 = Float4_Set( axisY, 0 );
	cameraWorldMatrix.r2 = Float4_Set( axisZ, 0 );
	cameraWorldMatrix.r3 = Float4_Set( eyePosition, 1 );

	Float4x4	result;
	result = Matrix_OrthoInverse( cameraWorldMatrix );
	return result;
}

// Constructs a symmetric view frustum using OpenGL's clip-space conventions (NDC depth in range [-1..+1]).
// FoV_Y_radians - (full) vertical field of view, in radians (e.g. PI/2 radians or 90 degrees)
//
const Float4x4 Matrix_PerspectiveOGL( float FoV_Y_radians, float aspect_ratio, float near_clip, float far_clip )
{
	float tanHalfFoV = tan( FoV_Y_radians * 0.5f );
	float scaleX = 1.0f / (aspect_ratio * tanHalfFoV);
	float scaleY = 1.0f / tanHalfFoV;

	float projA = (far_clip + near_clip) / (far_clip - near_clip);
	float projB = (-2.0f * near_clip * far_clip) / (far_clip - near_clip);

	Float4x4	result;
	result.r0 = Vector4_Set(
		scaleX,
		0.0f,
		0.0f,
		0.0f
	);
	result.r1 = Vector4_Set(
		0.0f,
		0.0f,
		projA,
		1.0f
	);
	result.r2 = Vector4_Set(
		0.0f,
		scaleY,
		0.0f,
		0.0f
	);
	result.r3 = Vector4_Set(
		0.0f,
		0.0f,
		projB,
		0.0f
	);
	return result;
}

// Constructs a symmetric view frustum using Direct3D's clip-space conventions (NDC depth in range [0..+1]).
// FoV_Y_radians - (full) vertical field of view, in radians (e.g. PI/2 radians or 90 degrees)
//
const Float4x4 Matrix_PerspectiveD3D( float FoV_Y_radians, float aspect_ratio, float near_clip, float far_clip )
{
	mxASSERT(aspect_ratio != 0);
	mxASSERT(near_clip != far_clip);

	float tanHalfFoV = tan( FoV_Y_radians * 0.5f );
	float scaleY = 1.0f / tanHalfFoV;
	float scaleX = scaleY / aspect_ratio;
	float projA = far_clip / (far_clip - near_clip);
	float projB = -near_clip * projA;

	Float4x4	result;
	result.r0 = Vector4_Set(
		scaleX,
		0.0f,
		0.0f,
		0.0f
	);
	result.r1 = Vector4_Set(
		0.0f,
		0.0f,
		projA,
		1.0f
	);
	result.r2 = Vector4_Set(
		0.0f,
		scaleY,
		0.0f,
		0.0f
	);
	result.r3 = Vector4_Set(
		0.0f,
		0.0f,
		projB,
		0.0f
	);
	return result;
}
/*
Multiplication by our projection matrix (NOTE: z corresponds to height, y - to depth):

            | H   0   0   0 |
[x y z w] * | 0   0   A   1 | = | (x*H)  (z*V)  (y*A + w*B) (y) |
            | 0   V   0   0 |
            | 0   0   B   0 |
*/
/*
Multiplication by inverse projection matrix (P^-1):

            | 1/H   0   0    0  |
[x y z w] * |  0    0  1/V   0  | = | (x/H)   (w)   (y/V)  (z/B - w*A/B) |
            |  0    0   0   1/B |
            |  0    1   0  -A/B |
*/
void Matrix_ProjectionAndInverse_D3D(
									 float FoV_Y_radians, float aspect_ratio, float near_clip, float far_clip,
									 Float4x4 *projectionMatrix, Float4x4 *inverseProjectionMatrix
									 )
{
	mxASSERT(aspect_ratio != 0);
	mxASSERT(near_clip != far_clip);

	const double tanHalfFoV = tan( (double)FoV_Y_radians * 0.5 );
	const double V = 1.0f / tanHalfFoV;
	const double H = V / aspect_ratio;
	const double A = far_clip / (far_clip - near_clip);
	const double B = -near_clip * A;

	projectionMatrix->r0 = Vector4_Set( H, 0, 0, 0 );
	projectionMatrix->r1 = Vector4_Set( 0, 0, A, 1 );
	projectionMatrix->r2 = Vector4_Set( 0, V, 0, 0 );
	projectionMatrix->r3 = Vector4_Set( 0, 0, B, 0 );

	//@todo: rearrange using algebraic rules for better precision
	inverseProjectionMatrix->r0 = Vector4_Set( 1/H, 0,  0,  0  );
	inverseProjectionMatrix->r1 = Vector4_Set(  0,  0, 1/V, 0  );
	inverseProjectionMatrix->r2 = Vector4_Set(  0,  0,  0, 1/B );
	inverseProjectionMatrix->r3 = Vector4_Set(  0,  1,  0,-A/B );
}

// http://en.wikipedia.org/wiki/Orthographic_projection
// In our coordinate system, the projection matrix
// transform X coordinate from [left, right] => [-1..+1]
// transform Z coordinate from [bottom, top] => [-1..+1]
// transform Y coordinate from [ near, far ] => [-1..+1] if OpenGL or [0..1] if Direct3D
//
const Float4x4 	Matrix_OrthographicD3D( float width, float height, float near_clip, float far_clip )
{
	mxASSERT(width != 0);
	mxASSERT(height != 0);
	mxASSERT(near_clip != far_clip);

	Float4x4	result;
	result.r0 = Vector4_Set(
		2.0f / width,		// Z: [-W/2..+W/2] => [-1..+1]
		0.0f,
		0.0f,
		0.0f
	);
	result.r1 = Vector4_Set(
		0.0f,
		1.0f / (far_clip - near_clip),	// Y: [N..F] => [0..1]
		0.0f,
		0.0f
	);
	result.r2 = Vector4_Set(
		0.0f,
		0.0f,
		2.0f / height,		// Z: [-H/2..+H/2] => [-1..+1]
		0.0f
	);
	result.r3 = Vector4_Set(
		0.0f,
		near_clip / (near_clip - far_clip),
		0.0f,		
		1.0f
	);
	return result;
}
// http://www.codeguru.com/cpp/misc/misc/graphics/article.php/c10123/Deriving-Projection-Matrices.htm#page-2
const Float4x4 Matrix_OrthographicD3D( float left, float right, float bottom, float top, float near_clip, float far_clip )
{
	float R_plus_L = left + right;
	float R_minus_L = left - right;
	float T_minus_B = top - bottom;
	float F_minus_N = far_clip - near_clip;

	Float4x4	result;


	// first translate x, y and z to center around the origin
	Float4x4 T = Matrix_Translation(
		-R_plus_L/R_minus_L,
		-near_clip / F_minus_N,
		-(top + bottom) / T_minus_B
	);
	// then scale x and z into [-1..+1] range, y into [0..1] range
	Float4x4 S = Matrix_Scaling(
		2.0f / R_minus_L,
		1.0f / F_minus_N,
		2.0f / T_minus_B
	);
	result = Matrix_Multiply( T, S );
	result = Matrix_Transpose(result);

#if 0
	result.r0 = Vector4_Set(
		2.0f / R_minus_L,
		0.0f,
		0.0f,
		0.0f
	);
	result.r1 = Vector4_Set(
		0.0f,
		2.0f / T_minus_B,
		0.0f,
		0.0f
	);
	result.r2 = Vector4_Set(
		0.0f,
		0.0f,
		1.0f / F_minus_N,
		0.0f
	);
	result.r3 = Vector4_Set(
		-R_plus_L/R_minus_L,
		-(top + bottom) / T_minus_B,
		-near_clip / F_minus_N,
		1.0f
	);
#endif

/*
2/(r-l)      0            0           0
0            2/(t-b)      0           0
0            0            1/(zf-zn)   0
(l+r)/(l-r)  (t+b)/(b-t)  zn/(zn-zf)  1
*/

#if 0
	result.r0 = Vector4_Set(
		2.0f / R_minus_L,
		0.0f,
		0.0f,
		0.0f
	);
	result.r2 = Vector4_Set(
		0.0f,
		2.0f / T_minus_B,
		0.0f,
		0.0f
	);
	result.r1 = Vector4_Set(
		0.0f,
		0.0f,
		1.0f / F_minus_N,
		0.0f
	);
	result.r3 = Vector4_Set(
		-R_plus_L/R_minus_L,
		-(top + bottom) / T_minus_B,
		-near_clip / F_minus_N,
		1.0f
	);
#endif
	return result;
}

#if 0

// Creates a transformation matrix from my engine's space to OpenGL space:
// rotates PI/2 clockwise around X axis.
void MakeMatrix_MySpace_To_OpenGL( glm::mat4 & M )// right-handed to left-handed (OpenGL)
{
	/*
	Assuming column-major scheme...
	1  0  0  0
	0  0  1  0
	0 -1  0  0
	0  0  0  1
	...This matrix will set the y value to the z value, and the z value to -y.
	*/
	M[0][0] =  1.0f;	M[0][1] =  0.0f;	M[0][2] =  0.0f;	M[0][3] =  0.0f;
	M[1][0] =  0.0f;	M[1][1] =  0.0f;	M[1][2] =  1.0f;	M[1][3] =  0.0f;
	M[2][0] =  0.0f;	M[2][1] = -1.0f;	M[2][2] =  0.0f;	M[2][3] =  0.0f;
	M[3][0] =  0.0f;	M[3][1] =  0.0f;	M[3][2] =  0.0f;	M[3][3] =  1.0f;
}

// Creates a transformation matrix from my engine's space to OpenGL space:
// rotates PI/2 counterclockwise around
void MakeMatrix_OpenGL_To_MySpace( glm::mat4 & M )
{
	/*
	Assuming column-major scheme...
	1  0  0  0
	0  0 -1  0
	0  1  0  0
	0  0  0  1
	...This matrix will set the y value to the -z value, and the z value to y.
	*/
	M[0][0] =  1.0f;	M[0][1] =  0.0f;	M[0][2] =  0.0f;	M[0][3] =  0.0f;
	M[1][0] =  0.0f;	M[1][1] =  0.0f;	M[1][2] = -1.0f;	M[1][3] =  0.0f;
	M[2][0] =  0.0f;	M[2][1] =  1.0f;	M[2][2] =  0.0f;	M[2][3] =  0.0f;
	M[3][0] =  0.0f;	M[3][1] =  0.0f;	M[3][2] =  0.0f;	M[3][3] =  1.0f;
}

const Float4x4 Matrix_PerspectiveOGL( float fovyRadians, float aspect, float zNear, float zFar )
{
	Float4x4	result;
	float tanHalfFovy = tan(fovyRadians / float(2));
	result.c[0] = Vector4_Set( 1.0f / (aspect * tanHalfFovy), 0.0f, 0.0f, 0.0f );
	result.c[1] = Vector4_Set( 0.0f, 1.0f / tanHalfFovy, 0.0f, 0.0f );
	result.c[2] = Vector4_Set( 0.0f, 0.0f, -(zFar + zNear) / (zFar - zNear), -1.0f );
	result.c[3] = Vector4_Set( 0.0f, 0.0f, -(2.0f * zFar * zNear) / (zFar - zNear), 0.0f );
	return result;
}
// This code snippet implements:  D3DXMatrixPerspectiveFovLH : http://msdn.microsoft.com/en-us/library/windows/desktop/bb205350(v=vs.85).aspx
inline void vp_matrixPerspectiveFovLH(float matrix[16],float fov,float aspectRatio,float zn,float zf)
{
	float yScale = 1/tanf(fov/2);
	float xScale = yScale / aspectRatio;

	matrix[0]  = xScale;
	matrix[1] = 0;
	matrix[2] = 0;
	matrix[3] = 0;

	matrix[4] = 0;
	matrix[5]  = yScale;
	matrix[6] = 0;
	matrix[7] = 0;

	matrix[8] = 0;
	matrix[9] = 0;
	matrix[10] = zf / (zf-zn);
	matrix[11] = 1;

	matrix[12] = 0;
	matrix[13] = 0;
	matrix[14] = -zn*zf/(zf-zn);
	matrix[15] = 0;
}
const Float4x4 Matrix_PerspectiveD3D( float fovyRadians, float aspect, float zNear, float zFar )
{
	const float H = 1.0f / Tan( fovyRadians * 0.5f );
	const float V = H / aspect;
	const float q = zFar / (zFar-zNear);

	Float4x4	result;

	result.m00 = H;		result.m01 = 0.0f;		result.m02 = 0.0f;	result.m03 = 0.0f;
	result.m10 = 0.0f;	result.m11 = q;			result.m12 = 0.0f;	result.m13 = 1.0f;
	result.m20 = 0.0f;	result.m21 = 0.0f;		result.m22 = V;		result.m23 = 0.0f;
	result.m30 = 0.0f;	result.m31 = -zNear*q;	result.m32 = 0.0f;	result.m33 = 0.0f;

	//result.a[0 ]	= xScale;
	//result.a[1 ]	= 0.0f;
	//result.a[2 ]	= 0.0f;
	//result.a[3 ]	= 0.0f;

	//result.a[4 ] 	= 0.0f;
	//result.a[5 ] 	= 0.0f;
	//result.a[6 ] 	= q;
	//result.a[7 ] 	= 1.0f;

	//result.a[8 ]	= 0.0f;
	//result.a[9 ]	= yScale;
	//result.a[10]	= 0.0f;
	//result.a[11]	= 0.0f;

	//result.a[12] 	= 0.0f;
	//result.a[13] 	= 0.0f;
	//result.a[14] 	= -zNear*q;
	//result.a[15] 	= 0.0f;

	return result;
}
#endif

const Float4x4 Matrix_BuildTransform( const Float3& translation, const Float4& rotation, const Float3& scaling )
{
	Float4x4	RotM = Matrix_FromQuaternion( rotation );

	Float4x4	result;
	result.r0 = Vector4_Set( RotM.r0.x * scaling.x, RotM.r0.y, RotM.r0.z, 0.0f );
	result.r1 = Vector4_Set( RotM.r1.x, RotM.r1.y * scaling.y, RotM.r1.z, 0.0f );
	result.r2 = Vector4_Set( RotM.r2.x, RotM.r2.y, RotM.r2.z * scaling.z, 0.0f );
	result.r3 = Vector4_Set( translation.x, translation.y, translation.z, 1.0f );
	return result;
}
const Float4x4 Matrix_BuildTransform( const Float3& translation, const Float4& rotation )
{
	Float4x4	result;
	result = Matrix_FromQuaternion( rotation );
	result.r3 = Vector4_Set( translation.x, translation.y, translation.z, 1.0f );
	return result;
}
// exact compare, no epsilon
const bool Matrix_Equal( const Float4x4& A, const Float4x4& B )
{
	const float* pA = reinterpret_cast< const float* >( &A );
	const float* pB = reinterpret_cast< const float* >( &B );

	for( int i = 0; i < 16; i++ )
	{
		if( pA[i] != pB[i] ) {
			return false;
		}
	}
	return true;
}
// compare with epsilon
const bool Matrix_Equal( const Float4x4& A, const Float4x4& B, const float epsilon )
{
	const float* pA = reinterpret_cast< const float* >( &A );
	const float* pB = reinterpret_cast< const float* >( &B );

	for( int i = 0; i < 16; i++ )
	{
		if( fabs( pA[i] - pB[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

// [ X, Y, Z, W ] => projection transform => homogeneous divide => [Px, Py, Pz, W ]
const Float4 Matrix_Project( const Float4x4& m, const Float4& p )
{
	Float4 pointH = Matrix_Transform( m, p );
	float invW = Float_Rcp( pointH.w );
	Float4	result;
	result.x = pointH.x * invW;
	result.y = pointH.y * invW;
	result.z = pointH.z * invW;
	result.w = pointH.w;
	return result;
}

const Float4x4 Matrix_RotationX( float angle )
{
	Float4x4 M;

	FLOAT fSinAngle;
	FLOAT fCosAngle;
	Float_SinCos(angle, fSinAngle, fCosAngle);

	M.m[0][0] = 1.0f;
	M.m[0][1] = 0.0f;
	M.m[0][2] = 0.0f;
	M.m[0][3] = 0.0f;

	M.m[1][0] = 0.0f;
	M.m[1][1] = fCosAngle;
	M.m[1][2] = fSinAngle;
	M.m[1][3] = 0.0f;

	M.m[2][0] = 0.0f;
	M.m[2][1] = -fSinAngle;
	M.m[2][2] = fCosAngle;
	M.m[2][3] = 0.0f;

	M.m[3][0] = 0.0f;
	M.m[3][1] = 0.0f;
	M.m[3][2] = 0.0f;
	M.m[3][3] = 1.0f;
	return M;
}
const Float4x4 Matrix_RotationY( float angle )
{
	Float4x4 M;

	FLOAT fSinAngle;
	FLOAT fCosAngle;
	Float_SinCos(angle, fSinAngle, fCosAngle);

    M.m[0][0] = fCosAngle;
    M.m[0][1] = 0.0f;
    M.m[0][2] = -fSinAngle;
    M.m[0][3] = 0.0f;

    M.m[1][0] = 0.0f;
    M.m[1][1] = 1.0f;
    M.m[1][2] = 0.0f;
    M.m[1][3] = 0.0f;

    M.m[2][0] = fSinAngle;
    M.m[2][1] = 0.0f;
    M.m[2][2] = fCosAngle;
    M.m[2][3] = 0.0f;

    M.m[3][0] = 0.0f;
    M.m[3][1] = 0.0f;
    M.m[3][2] = 0.0f;
    M.m[3][3] = 1.0f;
    return M;
}
const Float4x4 Matrix_RotationZ( float angle )
{
	Float4x4 M;

	FLOAT fSinAngle;
	FLOAT fCosAngle;
	Float_SinCos(angle, fSinAngle, fCosAngle);

	M.m[0][0] = fCosAngle;
	M.m[0][1] = fSinAngle;
	M.m[0][2] = 0.0f;
	M.m[0][3] = 0.0f;

	M.m[1][0] = -fSinAngle;
	M.m[1][1] = fCosAngle;
	M.m[1][2] = 0.0f;
	M.m[1][3] = 0.0f;

	M.m[2][0] = 0.0f;
	M.m[2][1] = 0.0f;
	M.m[2][2] = 1.0f;
	M.m[2][3] = 0.0f;

	M.m[3][0] = 0.0f;
	M.m[3][1] = 0.0f;
	M.m[3][2] = 0.0f;
	M.m[3][3] = 1.0f;
	return M;
}
mxUNDONE
//const Float4x4	Matrix_RotationRollPitchYaw( float pitch, float yaw, float roll );
//const Float4x4	Matrix_RotationRollPitchYawFromVector( Vector4Arg angles);
//const Float4x4	Matrix_RotationNormal( Vector4Arg normalAxis, float angle )
//{
//
//}
//const Float4x4	Matrix_RotationAxis( Vector4Arg axis, float angle );

// Builds a rotation matrix from the given (unit-length) quaternion.
const Float4x4 Matrix_FromQuaternion( Vector4Arg q )
{
	const Float3x3	rotationMatrix = Float3x3_FromQuaternion( q );
	return Matrix_FromRotationMatrix( rotationMatrix );
}

const Float4x4 Matrix_FromRotationMatrix( const Float3x3& m )
{
	Float4x4	result;
	result.r0 = Vector4_Set( m.r0.x, m.r0.y, m.r0.z, 0.0f );
	result.r1 = Vector4_Set( m.r1.x, m.r1.y, m.r1.z, 0.0f );
	result.r2 = Vector4_Set( m.r2.x, m.r2.y, m.r2.z, 0.0f );
	result.r3 = g_MMIdentityR3.v;
	return result;
}

const Float4x4 Matrix_FromAxes( const Float3& axisX, const Float3& axisY, const Float3& axisZ )
{
	Float4x4	result;
	result.r0 = Vector4_Set( axisX.x, axisX.y, axisX.z, 0.0f );
	result.r1 = Vector4_Set( axisY.x, axisY.y, axisY.z, 0.0f );
	result.r2 = Vector4_Set( axisZ.x, axisZ.y, axisZ.z, 0.0f );
	result.r3 = g_MMIdentityR3.v;
	return result;
}

const Float4x4 Matrix_From_Float3x4( const Float3x4& m )
{
	Float4x4	result;
	result.r0 = Vector4_Set( m.r[0].x, m.r[1].x, m.r[2].x, 0.0f );
	result.r1 = Vector4_Set( m.r[0].y, m.r[1].y, m.r[2].y, 0.0f );
	result.r2 = Vector4_Set( m.r[0].z, m.r[1].z, m.r[2].z, 0.0f );
	result.r3 = Vector4_Set( m.r[0].w, m.r[1].w, m.r[2].w, 1.0f );
	return result;
}

const Float3x3 Float3x3_Transpose( const Float3x3& m )
{
	Float3x3	result;
	result.r0 = Float3_Set( m.r0.x, m.r1.x, m.r2.x );
	result.r1 = Float3_Set( m.r0.y, m.r1.y, m.r2.y );
	result.r2 = Float3_Set( m.r0.z, m.r1.z, m.r2.z );
	return result;
}
const Float3x3 Float3x3_Multiply( const Float3x3& a, const Float3x3& b )
{
	Float3x3	result;
	result.r0 = Float3x3_Transform( b, a.r0 );
	result.r1 = Float3x3_Transform( b, a.r1 );
	result.r2 = Float3x3_Transform( b, a.r2 );
	return result;
}

const Float3 Float3x3_Transform( const Float3x3& m, const Float3& p )
{
	Float3	result;
	result.x = (m.r0.x * p.x) + (m.r1.x * p.y) + (m.r2.x * p.z);
	result.y = (m.r0.y * p.x) + (m.r1.y * p.y) + (m.r2.y * p.z);
	result.z = (m.r0.z * p.x) + (m.r1.z * p.y) + (m.r2.z * p.z);
	return result;
}

// returns a 3x3 skew symmetric matrix which is equivalent to the cross product
const Float3x3 Float3x3_GetSkewSymmetricMatrix( const Float3& v )
{
	Float3x3	result;
	result.r0 = Float3_Set(  0,   -v.z,  v.y );
	result.r1 = Float3_Set(  v.z,  0,   -v.x );
	result.r2 = Float3_Set( -v.y,  v.x,  0   );
	return result;
}

const Float3x3 Float3x3_RotationX( float angle )
{
	float s, c;
	Float_SinCos(angle, s, c);

	Float3x3 M;
	M.r0.x = 1.0f;
	M.r0.y = 0.0f;
	M.r0.z = 0.0f;

	M.r1.x = 0.0f;
	M.r1.y = c;
	M.r1.z = s;

	M.r2.x = 0.0f;
	M.r2.y = -s;
	M.r2.z = c;
	return M;
}
const Float3x3 Float3x3_RotationY( float angle )
{
	float s, c;
	Float_SinCos(angle, s, c);

	Float3x3 M;
	M.r0.x = c;
	M.r0.y = 0.0f;
	M.r0.z = -s;

	M.r1.x = 0.0f;
	M.r1.y = 1.0f;
	M.r1.z = 0.0f;

	M.r2.x = s;
	M.r2.y = 0.0f;
	M.r2.z = c;
	return M;
}
const Float3x3 Float3x3_RotationZ( float angle )
{
	float s, c;
	Float_SinCos(angle, s, c);

	Float3x3 M;
	M.r0.x = c;
	M.r0.y = s;
	M.r0.z = 0.0f;

	M.r1.x = -s;
	M.r1.y = c;
	M.r1.z = 0.0f;

	M.r2.x = 0.0f;
	M.r2.y = 0.0f;
	M.r2.z = 1.0f;
	return M;
}

const Float3x3 Float3x3_RotationPitchRollYaw( float pitch, float roll, float yaw )
{
UNDONE;//THIS MATRIX IS INCORRECT!
	float sr, sp, sy, cr, cp, cy;

	Float_SinCos( pitch, sp, cp );
	Float_SinCos( roll, sr, cr );
	Float_SinCos( yaw, sy, cy );

	Float3x3 M;
	M.r0 = Float3_Set( cp * cy, cp * sy, -sp );
	M.r1 = Float3_Set( sr * sp * cy + cr * -sy, sr * sp * sy + cr * cy, sr * cp );
	M.r2 = Float3_Set( cr * sp * cy + -sr * -sy, cr * sp * sy + -sr * cy, cr * cp );
	return M;
}

// Constructs a rotation matrix from the given (unit-length) quaternion.
const Float3x3 Float3x3_FromQuaternion( Vector4Arg q )
{
#if 1
	mxSWIPED("Sony math library");
    const float qx = q.x;
    const float qy = q.y;
    const float qz = q.z;
    const float qw = q.w;
    const float qx2 = ( qx + qx );
    const float qy2 = ( qy + qy );
    const float qz2 = ( qz + qz );
    const float qxqx2 = ( qx * qx2 );
    const float qxqy2 = ( qx * qy2 );
    const float qxqz2 = ( qx * qz2 );
    const float qxqw2 = ( qw * qx2 );
    const float qyqy2 = ( qy * qy2 );
    const float qyqz2 = ( qy * qz2 );
    const float qyqw2 = ( qw * qy2 );
    const float qzqz2 = ( qz * qz2 );
    const float qzqw2 = ( qw * qz2 );

	Float3x3	result;
    result.r0 = Float3_Set( ( ( 1.0f - qyqy2 ) - qzqz2 ), ( qxqy2 + qzqw2 ), ( qxqz2 - qyqw2 ) );
    result.r1 = Float3_Set( ( qxqy2 - qzqw2 ), ( ( 1.0f - qxqx2 ) - qzqz2 ), ( qyqz2 + qxqw2 ) );
    result.r2 = Float3_Set( ( qxqz2 + qyqw2 ), ( qyqz2 - qxqw2 ), ( ( 1.0f - qxqx2 ) - qyqy2 ) );
	return result;
#else
	Float3x3	result;
	result.r0 = Float3_Set(
		1 - 2 * q.y * q.y - 2 * q.z * q.z,
		2 * q.x * q.y + 2 * q.w * q.z,
		2 * q.x * q.z - 2 * q.w * q.y
		);
	result.r1 = Float3_Set(
		2 * q.x * q.y - 2 * q.w * q.z,
		1 - 2 * q.x * q.x - 2 * q.z * q.z,
		2 * q.y * q.z + 2 * q.w * q.x
		);
	result.r2 = Float3_Set(
		2 * q.x * q.z + 2 * q.w * q.y,
		2 * q.y * q.z - 2 * q.w * q.x,
		1 - 2 * q.x * q.x - 2 * q.y * q.y
		);
	return result;
#endif
}

// Builds a matrix that rotates around an arbitrary normal vector.
const Float3x3 Float3x3_RotationNormal( const Float3& axis, float angle )
{
	mxASSERT(Float3_IsNormalized(axis));
	//derivation:
	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/angleToMatrix/
	// Rodrigues' rotation formula
	// http://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula

	float s, c;
	Float_SinCos( angle, s, c );

	float t = 1.0 - c;

	mxSWIPED("Sony math library");
    float xy = ( axis.x * axis.y );
    float yz = ( axis.y * axis.z );
    float zx = ( axis.z * axis.x );

	Float3x3	result;
	result.r0 = Float3_Set( ( ( axis.x * axis.x ) * t ) + c, ( xy * t ) + ( axis.z * s ), ( zx * t ) - ( axis.y * s ) );
    result.r1 = Float3_Set( ( xy * t ) - ( axis.z * s ), ( ( axis.y * axis.y ) * t ) + c, ( yz * t ) + ( axis.x * s ) );
    result.r2 = Float3_Set( ( zx * t ) + ( axis.y * s ), ( yz * t ) - ( axis.x * s ), ( ( axis.z * axis.z ) * t ) + c );
	return result;
}

// extracts Euler angles from the given rotation matrix
// http://nghiaho.com/?page_id=846
const Float3 Float3x3_ExtractPitchYawRoll( const Float3x3& m )
{
	float x = Float_ATan2(m.m[2][1], m.m[2][2]);
	float y = Float_ATan2(-m.m[2][0], Float_Sqrt(Square(m.m[2][1]) + Square(m.m[2][2])));
	float z = Float_ATan2(m.m[1][0], m.m[0][0]);
	return Float3_Set(x,y,z);
}

const Float3 Matrix_GetTranslation( const Float4x4& m )
{
	return Vector4_As_Float3( m.r3 );
}
// Transforms a 3D point (W=1) by a 4x4 matrix.
const Float3 Matrix_TransformPoint( const Float4x4& m, const Float3& p )
{
	Float3	result;
	result.x = (m.r0.x * p.x) + (m.r1.x * p.y) + (m.r2.x * p.z) + (m.r3.x);
	result.y = (m.r0.y * p.x) + (m.r1.y * p.y) + (m.r2.y * p.z) + (m.r3.y);
	result.z = (m.r0.z * p.x) + (m.r1.z * p.y) + (m.r2.z * p.z) + (m.r3.z);
	return result;
}
// Transforms a 3D direction (W=0) by a 4x4 matrix.
const Float3 Matrix_TransformNormal( const Float4x4& m, const Float3& v )
{
	Float3	result;
	result.x = (m.r0.x * v.x) + (m.r1.x * v.y) + (m.r2.x * v.z);
	result.y = (m.r0.y * v.x) + (m.r1.y * v.y) + (m.r2.y * v.z);
	result.z = (m.r0.z * v.x) + (m.r1.z * v.y) + (m.r2.z * v.z);
	return result;
}

const Float3x4 Float3x4_Identity()
{
	Float3x4	result;
	result = Float3x4_Pack( Matrix_Identity() );
	return result;
}
const Float3x4 Float3x4_Pack( const Float4x4& m )
{
	mxASSERT( m.r0.w == 0.0f );
	mxASSERT( m.r1.w == 0.0f );
	mxASSERT( m.r2.w == 0.0f );
	mxASSERT( m.r3.w == 1.0f );

	Float3x4	result;

	result.r[0] = m.r3;
	result.r[1] = m.r0;
	result.r[2] = m.r1;

	result.r[0].w = m.r2.x;
	result.r[1].w = m.r2.y;
	result.r[2].w = m.r2.z;

	return result;
}
const Float4x4 Float3x4_Unpack( const Float3x4& m )
{
	Float4x4 result;

	result.r[0] = m.r[1];
	result.r[1] = m.r[2];
	result.r[2] = Vector4_Set( m.r[0].w, m.r[1].w, m.r[2].w, 0.0f );
	result.r[3] = m.r[0];

	result.r[0].w = 0.0f;
	result.r[1].w = 0.0f;
	result.r[3].w = 1.0f;

	return result;
}
const Float3 Float3x4_GetTranslation( const Float3x4& m )
{
	return Vector4_As_Float3( m.r[0] );
}

// Field-of-View calculation utility, adapted from: http://gamedev.stackexchange.com/a/53604/15235
// This helps to find an aspect-adjusted horizontal FOV that maintains vertical FOV irrespective of resolution and aspect ratio.
// fov_y = CalcFovY( playerAdjustableFOV, 4, 3 ); // this is your base aspect that adjusted FOV should be relative to
// fov_x = CalcFovX( fov_y, width, height ); // this is your actual window width and height

float Get_Field_of_View_X_Degrees( float FoV_Y_degrees, float width, float height )
{
	float   a;
	float   y;

	FoV_Y_degrees = clampf(FoV_Y_degrees, 1.0f, 179.0f);

	y = height / tanf( DEG2RAD(FoV_Y_degrees * 0.5f) );
	a = atanf( width / y );
	a = RAD2DEG( a ) * 2.0f;

	return a;
}

float Get_Field_of_View_Y_Degrees( float FoV_X_degrees, float width, float height )
{
	float   a;
	float   x;

	FoV_X_degrees = clampf(FoV_X_degrees, 1.0f, 179.0f);

	x = width / tanf( DEG2RAD(FoV_X_degrees * 0.5f));
	a = atanf( height / x );
	a = RAD2DEG( a ) * 2.0f;

	return a;
}

const Float4x4 	Matrix_BuildView(
								 const Float3& rightDirection,
								 const Float3& lookDirection,
								 const Float3& upDirection,
								 const Float3& eyePosition
								 )
{
	mxASSERT(Float3_IsNormalized(rightDirection));
	mxASSERT(Float3_IsNormalized(lookDirection));
	mxASSERT(Float3_IsNormalized(upDirection));
	Float4x4	cameraWorldMatrix;
	cameraWorldMatrix.r0 = LoadFloat3( rightDirection, 0.0f );	// X
	cameraWorldMatrix.r1 = LoadFloat3( lookDirection,  0.0f );	// Y
	cameraWorldMatrix.r2 = LoadFloat3( upDirection,    0.0f );	// Z
	cameraWorldMatrix.r3 = LoadFloat3( eyePosition,    1.0f );	// T
	return Matrix_OrthoInverse( cameraWorldMatrix );
}

void Vector4_SinCos( Vector4Arg v, Vector4 *s, Vector4 *c )
{
	Float_SinCos( v.x, s->x, c->x );
	Float_SinCos( v.y, s->y, c->y );
	Float_SinCos( v.z, s->z, c->z );
	Float_SinCos( v.w, s->w, c->w );
}

const Vector4 Quaternion_Inverse( Vector4Arg q )
{
	return Vector4_Multiply( Quaternion_Conjugate( q ), Vector4_ReciprocalLengthV( q ) );
}
// Spherical linear interpolation between two unit (normalized) quaternions (aka SLERP).
// NOTE:
// Interpolates along the shortest path between orientations.
// Does not clamp t between 0 and 1.
const Vector4 Quaternion_Slerp( Vector4Arg q0, Vector4Arg q1, float t )
{
    Float4 start;
	// Compute "cosine of angle between quaternions" using dot product
	float cosTheta = Vector4_Dot( q0, q1 );
	// If cosTheta < 0, the interpolation will take the long way around the sphere. 
	// To fix this, one quat must be negated.
    if( cosTheta < 0.0f ) {
		// Two quaternions q and -q represent the same rotation, but may produce different slerp.
		// We chose q or -q to rotate using the acute angle.
		cosTheta = -cosTheta;
        start = Vector4_Negate( q0 );
    } else {
        start = q0;
    }
	// Interpolation = A*[sin((1-Elapsed) * Angle)/sin(Angle)] + B * [sin (Elapsed * Angle) / sin(Angle)]
	float scale0, scale1;
    if( cosTheta < mxSLERP_DELTA ) {
		// Essential Mathematics, page 467
        float angle = acosf( cosTheta );
		// Compute inverse of denominator, so we only have to divide once
		float recipSinAngle = ( 1.0f / sinf( angle ) );
		// Compute interpolation parameters
        scale0 = sinf( ( ( 1.0f - t ) * angle ) ) * recipSinAngle;
        scale1 = sinf( ( t * angle ) ) * recipSinAngle;
    } else {
		// Perform a linear interpolation when cosTheta is close to 1
		// to avoid side effect of sin(angle) becoming a zero denominator.
        scale0 = 1.0f - t;
        scale1 = t;
    }
	// Interpolate and return new quaternion
    return Vector4_Add( Vector4_Scale( start, scale0 ), Vector4_Scale( q1, scale1 ) );
}
// Constructs a quaternion from the Euler angles.
const Vector4 Quaternion_RotationPitchRollYaw( float pitch, float roll, float yaw )
{
#if 0
	Float3 s, c;	// the sine and the cosine of the half angle
	Float_SinCos( pitch * 0.5f, s.x, c.x );
	Float_SinCos( roll * 0.5f, s.y, c.y );
	Float_SinCos( yaw * 0.5f, s.z, c.z );

	Vector4 result;
	result.w = c.x * c.y * c.z + s.x * s.y * s.z;
	result.x = s.x * c.y * c.z - c.x * s.y * s.z;
	result.y = c.x * s.y * c.z + s.x * c.y * s.z;
	result.z = c.x * c.y * s.z - s.x * s.y * c.z;
	return result;
#else
	double heading = pitch, attitude = roll, bank = yaw;
    // Assuming the angles are in radians.
    double c1 = cos(heading/2);
    double s1 = sin(heading/2);
    double c2 = cos(attitude/2);
    double s2 = sin(attitude/2);
    double c3 = cos(bank/2);
    double s3 = sin(bank/2);
    double c1c2 = c1*c2;
    double s1s2 = s1*s2;
	Vector4 result;
    result.w =c1c2*c3 - s1s2*s3;
  	result.x =c1c2*s3 + s1s2*c3;
	result.y =s1*c2*c3 + c1*s2*s3;
	result.z =c1*s2*c3 - s1*c2*s3;
	return result;
#endif
}
// Constructs a quaternion from the (normalized) rotation axis and the rotation angle.
// 'normalAxis' - normalized axis of rotation
// 'angle' - rotation angle, in radians
const Vector4 Quaternion_RotationNormal( Vector4Arg normalAxis, float angle )
{
	// See: http://lolengine.net/blog/2013/09/18/beautiful-maths-quaternion-from-vectors
	float s, c;	// the sine and the cosine of the half angle
	Float_SinCos( angle * 0.5f, s, c );

	Vector4 result;
	result.x = normalAxis.x * s;
	result.y = normalAxis.y * s;
	result.z = normalAxis.z * s;
	result.w = c;
	return result;
}

ATextStream & operator << ( ATextStream & log, const Float2& v )
{
	log.PrintF("(%.3f, %.3f)",v.x,v.y);
	return log;
}
ATextStream & operator << ( ATextStream & log, const Float3& v )
{
	log.PrintF("(%.3f, %.3f, %.3f)",v.x,v.y,v.z);
	return log;
}
ATextStream & operator << ( ATextStream & log, const Float4& v )
{
	log.PrintF("(%.3f, %.3f, %.3f, %.3f)",v.x,v.y,v.z,v.w);
	return log;
}
ATextStream & operator << ( ATextStream & log, const Float4x4& m )
{
	log << m.r0 << "\n";
	log << m.r1 << "\n";
	log << m.r2 << "\n";
	log << m.r3 << "\n";
	return log;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
