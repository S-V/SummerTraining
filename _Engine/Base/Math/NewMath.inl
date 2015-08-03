inline float Float_Rcp( float x )
{
	return 1.0f / x;
}
inline float Float_Abs( float x )
{
	return ::fabs( x );
}
inline float Float_Floor( float x )
{
	return ::floorf( x );
}
inline float Float_Ceiling( float x )
{
	return ::ceilf( x );
}
// Returns the floating-point remainder of x/y (rounded towards zero):
// fmod = x - floor(x/y) * y
inline float Float_Modulo( float x, float y )
{
	return ::fmod( x, y );
}
inline int Float_Round( float x )
{
	return (int) x;
}
// Computes 2 raised to the given power n.
inline float Float_Exp2f( float power )
{
	return ::pow( 2.0f, power );
}
// Produces a value with the magnitude of 'magnitude' and the sign of 'sign'
inline float Float_CopySign( float magnitude, float sign )
{
	magnitude = Float_Abs( magnitude );
	return sign > 0.0f ? magnitude : -magnitude;
}
inline int Float_Truncate( float x )
{
	return (int)::floorf( x );
}
mxSWIPED("Doom3 BFG edition");
inline BYTE Float_ToByte( float f )
{
	// If a converted result is negative the value (0) is returned and if the
	// converted result is larger than the maximum byte the value (255) is returned.
	__m128 x = _mm_load_ss( &f );
	x = _mm_max_ss( x, g_SIMD_Zero );
	x = _mm_min_ss( x, g_SIMD_255 );
	return static_cast<BYTE>( _mm_cvttss_si32( x ) );
}
inline float Float_Sqrt( float x )
{
	return ::sqrtf( x );
}
inline float Float_InvSqrt( float x )
{
	return 1.0f / ::sqrtf( x );
}
inline float Float_InvSqrtEst( float x )
{
	return 1.0f / ::sqrtf( x );
}

inline void Float_SinCos( float x, float &s, float &c )
{
	s = sinf(x);
	c = cosf(x);
}
inline float Float_Sin( float x )
{
	return sinf(x);
}
inline float Float_Cos( float x )
{
	return cosf(x);
}
inline float Float_Tan( float x )
{
	return tanf(x);
}
// The asin function returns the arcsine of x in the range –Pi/2 to Pi/2 radians.
inline float Float_ASin( float x )
{
	return asinf(x);
}
// The acos function returns the arccosine of x in the range 0 to Pi radians.
inline float Float_ACos( float x )
{
	return acosf(x);
}
// atan returns the arctangent of x in the range –Pi/2 to Pi/2 radians.
inline float Float_ATan( float x )
{
	return atanf(x);
}
// atan2 returns the arctangent of y/x in the range –Pi to Pi radians.
// For any real number (e.g., floating point) arguments x and y not both equal to zero,
// atan2(y, x) is the angle in radians between the positive x-axis of a plane
// and the point given by the coordinates (x, y) on it.
// The angle is positive for counter-clockwise angles (upper half-plane, y > 0),
// and negative for clockwise angles (lower half-plane, y < 0).
inline float Float_ATan2( float y, float x )
{
	return ::atan2f( y, x );
}

inline float Float_Exp( float x )
{
	return expf(x);
}
inline float Float_Log( float x )
{
	return logf(x);
}
inline float Float_Log10( float x )
{
	return log10f(x);
}
inline float Float_Pow( float x, float y )
{
	return ::powf( x, y );
}
inline float Float_Lerp( float from, float to, float amount )
{
	// Precise method which guarantees v = v1 when t = 1.
	return from * (1.0f - amount) + to * amount;
}
inline const Float2& Float3_As_Float2( const Float3& v )
{
	return *reinterpret_cast< const Float2* >( &v );
}
inline const Float4& Vector4_As_Float4( const Vector4& v )
{
	return *reinterpret_cast< const Float4* >( &v );
}
inline const Float3& Vector4_As_Float3( const Vector4& v )
{
	return *reinterpret_cast< const Float3* >( &v );
}
inline const Float3& Float4_As_Float3( const Float4& v )
{
	return *reinterpret_cast< const Float3* >( &v );
}

template< class XYZ >
static inline const Float3 Float3_From_XYZ( const XYZ& xyz )
{
	Float3 result = { xyz.x, xyz.y, xyz.z };
	return result;
}

inline const bool Float3_Equal( const Float3& a, const Float3& b )
{
	return ( a.x == b.x && a.y == b.y && a.z == b.z );
}

inline const bool Float3_Equal( const Float3& a, const Float3& b, const float epsilon )
{
	if( fabs( a.x - b.x ) > epsilon ) {
		return false;
	}
	if( fabs( a.y - b.y ) > epsilon ) {
		return false;
	}
	if( fabs( a.z - b.z ) > epsilon ) {
		return false;
	}
	return true;
}

inline const bool Float4_Equal( const Float4& a, const Float4& b )
{
	return ( a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w );
}

inline const bool Float4_Equal( const Float4& a, const Float4& b, const float epsilon )
{
	if( fabs( a.x - b.x ) > epsilon ) {
		return false;
	}
	if( fabs( a.y - b.y ) > epsilon ) {
		return false;
	}
	if( fabs( a.z - b.z ) > epsilon ) {
		return false;
	}
	if( fabs( a.w - b.w ) > epsilon ) {
		return false;
	}
	return true;
}

inline const Float4 Plane_FromPointNormal( const Float3& point, const Float3& normal )
{
	mxASSERT(Float3_IsNormalized(normal));
	Float4 plane = { normal.x, normal.y, normal.z, Float3_Dot( normal, point ) * -1.0f };
	return plane;
}
inline const Float3 Plane_CalculateNormal( const Float4& plane )
{
	const Float3 N = *reinterpret_cast< const Float3* >( &plane );
	return Float3_Normalized( N );
}
inline const Float3& Plane_GetNormal( const Float4& plane )
{
	const Float3& N = *reinterpret_cast< const Float3* >( &plane );
	mxASSERT(Float3_IsNormalized( N ));
	return N;
}
inline const float Plane_PointDistance( const Float4& plane, const Float3& point )
{
	return plane.x * point.x + plane.y * point.y + plane.z * point.z + plane.w;
}
inline const Float4 Plane_Translate( const Float4& plane, const Float3& translation )
{
	Float4 result = plane;
	result.w = -Float3_Dot( Plane_GetNormal(plane), translation );
	return result;
}
inline const Half2 Float2_To_Half2( const Float2& xy )
{
	Half2 result = { Float_To_Half(xy.x), Float_To_Half(xy.y) };
	return result;
}
inline const Float2 Half2_To_Float2( const Half2& xy )
{
	Float2 result = { Half_To_Float(xy.x), Half_To_Float(xy.y) };
	return result;
}

// NOTE: operator overloads should be defined only for 'non-vectorizable' types (Float2, Float3).
// NOTE: operator overloads must be unambiguous!

#if MM_OVERLOAD_OPERATORS

inline const Float2 operator + ( const Float2& a, const Float2& b ) {
	Float2 result = { a.x + b.x, a.y + b.y };
	return result;
}
inline const Float2 operator - ( const Float2& a, const Float2& b ) {
	Float2 result = { a.x - b.x, a.y - b.y };
	return result;
}
inline const Float2 operator * ( const Float2& v, const float scale ) {
	Float2 result = { v.x * scale, v.y * scale };
	return result;
}
inline const Float2 operator / ( const Float2& v, const float scale ) {
	float inverse = Float_Rcp( scale );
	Float2 result = { v.x * inverse, v.y * inverse };
	return result;
}

inline const Float3 operator + ( const Float3& a, const Float3& b ) {
	Float3 result = { a.x + b.x, a.y + b.y, a.z + b.z };
	return result;
}
inline const Float3 operator - ( const Float3& a, const Float3& b ) {
	Float3 result = { a.x - b.x, a.y - b.y, a.z - b.z };
	return result;
}
inline const Float3 operator * ( const Float3& v, const float scale ) {
	Float3 result = { v.x * scale, v.y * scale, v.z * scale };
	return result;
}
inline const Float3 operator / ( const Float3& v, const float scale ) {
	float inverse = Float_Rcp( scale );
	Float3 result = { v.x * inverse, v.y * inverse, v.z * inverse };
	return result;
}

inline Float3& operator += ( Float3& a, const Float3& b ) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}
inline Float3& operator -= ( Float3& a, const Float3& b ) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}
inline Float3& operator *= ( Float3& v, const float scale ) {
	v.x *= scale;
	v.y *= scale;
	v.z *= scale;
	return v;
}
inline Float3& operator /= ( Float3& v, const float scale ) {
	float inverse = Float_Rcp( scale );
	v.x *= inverse;
	v.y *= inverse;
	v.z *= inverse;
	return v;
}

inline const Float4 operator + ( const Float4& a, const Float4& b ) {
	Float4 result = {
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
		a.w + b.w
	};
	return result;
}
inline const Float4 operator - ( const Float4& a, const Float4& b ) {
	Float4 result = {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z,
		a.w - b.w
	};
	return result;
}

inline Float4& operator += ( Float4& a, const Float4& b ) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
	return a;
}
inline Float4& operator -= ( Float4& a, const Float4& b ) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= b.w;
	return a;
}
inline Float4& operator *= ( Float4& v, const float scale ) {
	v.x *= scale;
	v.y *= scale;
	v.z *= scale;
	v.w *= scale;
	return v;
}
inline Float4& operator /= ( Float4& v, const float scale ) {
	float inverse = Float_Rcp( scale );
	v.x *= inverse;
	v.y *= inverse;
	v.z *= inverse;
	v.w *= inverse;
	return v;
}

// Dot product.
inline const float operator * ( const Float3& a, const Float3& b ) {
	return Float3_Dot( a, b );
}
// Cross product (also called 'Wedge product').
inline const Float3 operator ^ ( const Float3& a, const Float3& b ) {
	return Float3_Cross( a, b );
}

inline const Float4x4 operator * ( const Float4x4& a, const Float4x4& b ) {
	return Matrix_Multiply( a, b );
}

inline const Float4x4 operator + ( const Float4x4& a, const Float4x4& b )
{
	Float4x4 result;
	result.r0 = a.r0 + b.r0;
	result.r1 = a.r1 + b.r1;
	result.r2 = a.r2 + b.r2;
	result.r3 = a.r3 + b.r3;
	return result;
}
inline const Float4x4 operator - ( const Float4x4& a, const Float4x4& b )
{
	Float4x4 result;
	result.r0 = a.r0 - b.r0;
	result.r1 = a.r1 - b.r1;
	result.r2 = a.r2 - b.r2;
	result.r3 = a.r3 - b.r3;
	return result;
}
inline const Float4x4 operator * ( const Float4x4& m, const float scale ) {
	Float4x4 result;
	result.r0 = Vector4_Scale( m.r0, scale );
	result.r1 = Vector4_Scale( m.r1, scale );
	result.r2 = Vector4_Scale( m.r2, scale );
	result.r3 = Vector4_Scale( m.r3, scale );
	return result;
}

// Compute the inverse of a 4x4 matrix, which is expected to be an affine matrix with an orthogonal upper-left 3x3 submatrix.
// This can be used to achieve better performance (and precision, too)
// than a general inverse when the specified 4x4 matrix meets the given restrictions.
// 
inline const Float4x4 Matrix_OrthoInverse( const Float4x4& m )
{
	// Transpose the left-upper 3x3 matrix (invert the rotation part).
	Vector4	axisX = Vector4_Set( m.r0.x, m.r1.x, m.r2.x, 0.0f );
	Vector4	axisY = Vector4_Set( m.r0.y, m.r1.y, m.r2.y, 0.0f );
	Vector4	axisZ = Vector4_Set( m.r0.z, m.r1.z, m.r2.z, 0.0f );

	// Compute the (negative) translation component.
	float Tx = -Vector4_Dot( m.r0, m.r3 );
	float Ty = -Vector4_Dot( m.r1, m.r3 );
	float Tz = -Vector4_Dot( m.r2, m.r3 );

	Float4x4	result;
	result.r0 = axisX;
	result.r1 = axisY;
	result.r2 = axisZ;
	result.r3 = Vector4_Set( Tx, Ty, Tz, 1.0f );
	return result;
}
// calculated with (matrix in row-major format):
// http://www.quickmath.com/webMathematica3/quickmath/matrices/inverse/basic.jsp#c=inverse_matricesinverse&v1=W%2C0%2C0%2C0%0A0%2C0%2CA%2C1%0A0%2CH%2C0%2C0%0A0%2C0%2CB%2C0%0A
inline const Float4x4 ProjectionMatrix_Inverse( const Float4x4& m )
{
	const float H = m.r0.x;	// 1 / tan( FoVy / 2 ) / aspect_ratio
	const float V = m.r2.y;	// 1 / tan( FoVy / 2 )
	const float A = m.r1.z;	// far / (far - near)
	const float B = m.r3.z;	// -near * far / (far - near)

	Float4x4	result;
	result.r0 = Vector4_Set( 1.0f / H,	0.0f,	0.0f,		0.0f );
	result.r1 = Vector4_Set( 0.0f,		0.0f,	1.0f / V,	0.0f );
	result.r2 = Vector4_Set( 0.0f,		0.0f,	0.0f,		1.0f / B );
	result.r3 = Vector4_Set( 0.0f,		1.0f,	0.0f,		-A / B );
	return result;
}
/*
Multiplication by inverse projection matrix (P^-1):

            | 1/H   0   0    0  |
[x y z w] * |  0    0  1/V   0  | = | (x/H)   (w)   (y/V)  (z/B - w*A/B) |
            |  0    0   0   1/B |
            |  0    1   0  -A/B |
*/

#endif // MM_OVERLOAD_OPERATORS
