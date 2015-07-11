inline const Vector4 LoadFloat3( const Float3& v, float w )
{
	Vector4	result;
	result.x = v.x;
	result.y = v.y;
	result.z = v.z;
	result.w = w;
	return result;
}

inline const Float2 Float2_Zero()
{
	Float2 result;
	result.x = 0.0f;
	result.y = 0.0f;
	return result;
}
inline const Float2 Float2_Set( float x, float y )
{
	Float2 result;
	result.x = x;
	result.y = y;
	return result;
}
inline const Float2 Float2_Scale( const Float2& v, float s )
{
	Float2 result;
	result.x = v.x * s;
	result.y = v.y * s;
	return result;
}
inline const float Float2_Length( const Float2& v )
{
	return Float_Sqrt( Float2_Dot( v, v ) );
}
inline const float Float2_Dot( const Float2& a, const Float2& b )
{
	return (a.x * b.x) + (a.y * b.y);
}
// Linearly interpolates one vector to another.
// Precise method which guarantees from = to when amount = 1.
inline const Float2 Float2_Lerp( const Float2& from, const Float2& to, float amount )
{
	if( amount <= 0.0f ) {
		return from;
	} else if( amount >= 1.0f ) {
		return to;
	} else {
		return Float2_Lerp01( from, to, amount );
	}
}
inline const Float2 Float2_Lerp01( const Float2& from, const Float2& to, float amount )
{
	mxASSERT(amount >= 0.0f && amount <= 1.0f);
	return from + ( to - from ) * amount;
}

inline const Float3 Float3_Zero()
{
	Float3 result;
	result.x = 0.0f;
	result.y = 0.0f;
	result.z = 0.0f;
	return result;
}
inline const Float3 Float3_Replicate( float value )
{
	Float3 result;
	result.x = value;
	result.y = value;
	result.z = value;
	return result;
}
inline const Float3 Float3_Set( float x, float y, float z )
{
	Float3 result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}
inline const Float3 Float3_Scale( const Float3& v, float s )
{
	Float3 result;
	result.x = v.x * s;
	result.y = v.y * s;
	result.z = v.z * s;
	return result;
}
inline const float Float3_LengthSquared( const Float3& v )
{
	return Float3_Dot( v, v );
}
inline const float Float3_Length( const Float3& v )
{
	return Float_Sqrt( Float3_LengthSquared( v ) );
}
inline const Float3 Float3_Normalized( const Float3& v )
{
	float length = Float3_LengthSquared( v );
	// Prevent divide by zero.
	if( length != 0.0f ) {
		length = Float_InvSqrt( length );
	}
	return Float3_Scale( v, length );
}
inline const Float3 Float3_Abs( const Float3& x )
{
	Float3 result;
	result.x = Float_Abs( x.x );
	result.y = Float_Abs( x.y );
	result.z = Float_Abs( x.z );
	return result;
}
inline const Float3 Float3_Negate( const Float3& x )
{
	Float3 result;
	result.x = -x.x;
	result.y = -x.y;
	result.z = -x.z;
	return result;
}
inline const Float3 Float3_Reciprocal( const Float3& v )
{
	Float3 result = { Float_Rcp(v.x), Float_Rcp(v.y), Float_Rcp(v.z) };
	return result;
}
inline const bool Float3_IsInfinite( const Float3& v )
{
	return mmIS_INF(v.x) || mmIS_INF(v.y) || mmIS_INF(v.z);
}

inline const Float3 Float3_Min( const Float3& a, const Float3& b )
{
	Float3 result;
	result.x = minf( a.x, b.x );
	result.y = minf( a.y, b.y );
	result.z = minf( a.z, b.z );
	return result;
}
inline const Float3 Float3_Max( const Float3& a, const Float3& b )
{
	Float3 result;
	result.x = maxf( a.x, b.x );
	result.y = maxf( a.y, b.y );
	result.z = maxf( a.z, b.z );
	return result;
}
inline const Float3 Float3_Add( const Float3& a, const Float3& b )
{
	Float3 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}
inline const Float3 Float3_Subtract( const Float3& a, const Float3& b )
{
	Float3 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}
inline const Float3 Float3_Multiply( const Float3& a, const Float3& b )
{
	Float3 result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return result;
}
// Returns the dot product (aka 'scalar product') of a and b.
inline const float Float3_Dot( const Float3& a, const Float3& b )
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}
// The direction of the cross product follows the right hand rule.
//definition: http://www.euclideanspace.com/maths/algebra/vectors/vecAlgebra/cross/index.htm
inline const Float3 Float3_Cross( const Float3& a, const Float3& b )
{
	Float3 result;
	result.x = (a.y * b.z) - (a.z * b.y);
	result.y = (a.z * b.x) - (a.x * b.z);
	result.z = (a.x * b.y) - (a.y * b.x);
	return result;
}
// Finds a vector orthogonal to the given one.
// see: http://lolengine.net/blog/2013/09/21/picking-orthogonal-vector-combing-coconuts
inline const Float3 Float3_FindOrthogonalTo( const Float3& v )
{
	// Always works if the input is non-zero.
	// Doesn't require the input to be normalized.
	// Doesn't normalize the output.
	return Float_Abs(v.x) > Float_Abs(v.z) ?
		Float3_Set(-v.y, v.x, 0.0f) : Float3_Set(0.0f, -v.z, v.y);
}
// Returns the component-wise result of x * a + y * (1.0 - a),
// i.e., the linear blend of x and y using vector a.
// NOTE:
// The value for a is not restricted to the range [0, 1].
inline const Float3 Float3_LerpFast( const Float3& x, const Float3& y, float a )
{
	float b = 1.0f - a;
	Float3 result;
	result.x = (x.x * a) + (y.x * b);
	result.y = (x.y * a) + (y.y * b);
	result.z = (x.z * a) + (y.z * b);
	return result;
}
// Linearly interpolates one vector to another.
// Precise method which guarantees from = to when amount = 1.
inline const Float3 Float3_Lerp( const Float3& from, const Float3& to, float amount )
{
	if( amount <= 0.0f ) {
		return from;
	} else if( amount >= 1.0f ) {
		return to;
	} else {
		return Float3_Lerp01( from, to, amount );
	}
}
inline const Float3 Float3_Lerp01( const Float3& from, const Float3& to, float amount )
{
	mxASSERT(amount >= 0.0f && amount <= 1.0f);
	return Float3_Add( from, Float3_Scale( Float3_Subtract(to, from), amount ) );
}
inline const bool Float3_IsNormalized( const Float3& v, float epsilon )
{
	return Float_Abs(Float3_Length(v) - 1.0f) < epsilon;
}
inline const bool Float3_AllGreaterOrEqual(  const Float3& xyz, float value )
{
	return xyz.x >= value && xyz.y >= value && xyz.z >= value;
}

inline const Float4 Float4_Zero()
{
	Float4 result = { 0.0f, 0.0f, 0.0f, 0.0f };
	return result;
}

inline const Float4 Float4_Set( const Float3& xyz, float w )
{
	Float4 result;
	result.x = xyz.x;
	result.y = xyz.y;
	result.z = xyz.z;
	result.w = w;
	return result;
}
inline const Float4 Float4_Set( float x, float y, float z, float w )
{
	Float4 result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
	return result;
}
inline const Float4 Float4_Replicate( float value )
{
	Float4 result;
	result.x = value;
	result.y = value;
	result.z = value;
	result.w = value;
	return result;
}

inline const Vector4 Vector4_Zero()
{
	Vector4 result;
	result.x = 0.0f;
	result.y = 0.0f;
	result.z = 0.0f;
	result.w = 0.0f;
	return result;
}
inline const Vector4 Vector4_Replicate( float value )
{
	Vector4 result;
	result.x = value;
	result.y = value;
	result.z = value;
	result.w = value;
	return result;
}
inline const Vector4 Vector4_Set( const Float3& xyz, float w )
{
	Vector4 result;
	result.x = xyz.x;
	result.y = xyz.y;
	result.z = xyz.z;
	result.w = w;
	return result;
}
inline const Vector4 Vector4_Set( float x, float y, float z, float w )
{
	Vector4 result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
	return result;
}
inline const float Vector4_Get_X( Vector4Arg v )
{
	return v.x;
}
inline const float Vector4_Get_Y( Vector4Arg v )
{
	return v.y;
}
inline const float Vector4_Get_Z( Vector4Arg v )
{
	return v.z;
}
inline const float Vector4_Get_W( Vector4Arg v )
{
	return v.w;
}

// Replicate the X component of the vector.
inline const Vector4 Vector4_SplatX( Vector4Arg v )
{
	Vector4 result;
	result.x = v.x;
	result.y = v.x;
	result.z = v.x;
	result.w = v.x;
	return result;
}

// Replicate the Y component of the vector.
inline const Vector4 Vector4_SplatY( Vector4Arg v )
{
	Vector4 result;
	result.x = v.y;
	result.y = v.y;
	result.z = v.y;
	result.w = v.y;
	return result;
}

// Replicate the Z component of the vector.
inline const Vector4 Vector4_SplatZ( Vector4Arg v )
{
	Vector4 result;
	result.x = v.z;
	result.y = v.z;
	result.z = v.z;
	result.w = v.z;
	return result;
}

// Replicate the W component of the vector.
inline const Vector4 Vector4_SplatW( Vector4Arg v )
{
	Vector4 result;
	result.x = v.w;
	result.y = v.w;
	result.z = v.w;
	result.w = v.w;
	return result;
}
inline const float Vector4_LengthSquared( Vector4Arg v )
{
	return Vector4_Dot( v, v );
}
inline const Vector4 Vector4_LengthSquaredV( Vector4Arg v )
{
	return Vector4_DotV( v, v );
}
inline const Vector4 Vector4_LengthV( Vector4Arg v )
{
	return Vector4_SqrtV( Vector4_LengthSquaredV( v ) );
}
inline const Vector4 Vector4_SqrtV( Vector4Arg v )
{
	mxTODO(:);
    // if (x == +Infinity)  sqrt(x) = +Infinity
    // if (x == +0.0f)      sqrt(x) = +0.0f
    // if (x == -0.0f)      sqrt(x) = -0.0f
    // if (x < 0.0f)        sqrt(x) = QNaN
	Vector4 result;
	result.x = sqrtf( v.x );
	result.y = sqrtf( v.y );
	result.z = sqrtf( v.z );
	result.w = sqrtf( v.w );
	return result;
}
inline const Vector4 Vector4_ReciprocalSqrtV( Vector4Arg v )
{
	mxTODO(:);
    // if (x == +Infinity)  rsqrt(x) = 0
    // if (x == +0.0f)      rsqrt(x) = +Infinity
    // if (x == -0.0f)      rsqrt(x) = -Infinity
    // if (x < 0.0f)        rsqrt(x) = QNaN
	Vector4 result;
	result.x = 1.0f / sqrtf( v.x );
	result.y = 1.0f / sqrtf( v.y );
	result.z = 1.0f / sqrtf( v.z );
	result.w = 1.0f / sqrtf( v.w );
	return result;
}
inline const Vector4 Vector4_ReciprocalLengthV( Vector4Arg v )
{
	float length = Vector4_LengthSquared( v );
	// Prevent divide by zero.
	if( length > 0.0f ) {
		length = 1.0f/sqrtf(length);
	}
	return Vector4_Replicate(length);
	//return Vector4_ReciprocalSqrtV( Vector4_LengthSquaredV( v ) );
}
inline const Vector4 Vector4_Normalized( Vector4Arg v )
{
	return Vector4_Multiply( Vector4_ReciprocalLengthV( v ), v );
}
inline const Vector4 Vector4_Negate( Vector4Arg v )
{
	Vector4	result;
	result.x = -v.x;
	result.y = -v.y;
	result.z = -v.z;
	result.w = -v.w;
	return result;
}
inline const Vector4 Vector4_Scale( Vector4Arg v, float s )
{
	Vector4	result;
	result.x = v.x * s;
	result.y = v.y * s;
	result.z = v.z * s;
	result.w = v.w * s;
	return result;
}
inline const Vector4 Vector4_Add( Vector4Arg a, Vector4Arg b )
{
	Vector4	result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	result.w = a.w + b.w;
	return result;
}
inline const Vector4 Vector4_Subtract( Vector4Arg a, Vector4Arg b )
{
	Vector4	result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	result.w = a.w - b.w;
	return result;
}
// Computes component-wise multiplication.
inline const Vector4 Vector4_Multiply( Vector4Arg a, Vector4Arg b )
{
	Vector4	result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	result.w = a.w * b.w;
	return result;
}
// Computes a * b + c.
inline const Vector4 Vector4_MultiplyAdd( Vector4Arg a, Vector4Arg b, Vector4Arg c )
{
	Vector4	result;
	result.x = a.x * b.x + c.x;
	result.y = a.y * b.y + c.y;
	result.z = a.z * b.z + c.z;
	result.w = a.w * b.w + c.w;
	return result;
}

// Returns the dot product (aka 'scalar product') of a and b.
inline const float Vector4_Dot( Vector4Arg a,  Vector4Arg b )
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

inline const Vector4 Vector4_DotV( Vector4Arg a, Vector4Arg b )
{
	return Vector4_Replicate(Vector4_Dot(a, b));
}

// Returns the cross product (aka 'vector product') of a and b.
// This operation is defined in respect to a right-handed coordinate system,
// i.e. the direction of the resulting vector can be found by the "Right Hand Rule").
inline const Vector4 Vector4_Cross3( Vector4Arg a,  Vector4Arg b )
{
	Vector4 result;
	result.x = (a.y * b.z) - (a.z * b.y);
	result.y = (a.z * b.x) - (a.x * b.z);
	result.z = (a.x * b.y) - (a.y * b.x);
	result.w = 0.0f;
	return result;
}

inline const bool Vector4_IsNaN( Vector4Arg v )
{
	return mmIS_NAN(v.x) || mmIS_NAN(v.y) || mmIS_NAN(v.z) || mmIS_NAN(v.w);
}
inline const bool Vector4_IsInfinite( Vector4Arg v )
{
	return mmIS_INF(v.x) || mmIS_INF(v.y) || mmIS_INF(v.z) || mmIS_INF(v.w);
}
inline const bool Vector4_Equal( Vector4Arg a, Vector4Arg b )
{
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}
inline const bool Vector4_NotEqual( Vector4Arg a, Vector4Arg b )
{
	return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

inline const bool Quaternion_Equal( Vector4Arg a, Vector4Arg b )
{
	return Vector4_Equal( a, b );
}
inline const bool Quaternion_NotEqual( Vector4Arg a, Vector4Arg b )
{
	return Vector4_NotEqual( a, b );
}
inline const bool Quaternion_IsNaN( Vector4Arg q )
{
	return Vector4_IsNaN( q );
}
inline const bool Quaternion_IsInfinite( Vector4Arg q )
{
	return Vector4_IsInfinite( q );
}
inline const bool Quaternion_IsIdentity( Vector4Arg q )
{
	return Vector4_Equal( q, g_MMIdentityR3.v );
}
inline const Vector4 Quaternion_Identity()
{
	return g_MMIdentityR3.v;
}
inline const Vector4 Quaternion_Dot( Vector4Arg a, Vector4Arg b )
{
	return Vector4_DotV( a, b );
}
// Rotation 'a' is followed by 'b'
inline const Vector4 Quaternion_Multiply( Vector4Arg a, Vector4Arg b )
{
	Vector4 result;
	result.x = (b.w * a.x) + (b.x * a.w) + (b.y * a.z) - (b.z * a.y);
	result.y = (b.w * a.y) - (b.x * a.z) + (b.y * a.w) + (b.z * a.x);
	result.z = (b.w * a.z) + (b.x * a.y) - (b.y * a.x) + (b.z * a.w);
	result.w = (b.w * a.w) - (b.x * a.x) - (b.y * a.y) - (b.z * a.z);
	return result;
}
inline const Vector4 Quaternion_Normalize( Vector4Arg q )
{
	return Vector4_Normalized( q );
}
inline const Vector4 Quaternion_Conjugate( Vector4Arg q )
{
	Vector4 result;
	result.x = -q.x;
	result.y = -q.y;
	result.z = -q.z;
	result.w = q.w;
	return result;
}

inline const Float4x4 Matrix_Identity()
{
	Float4x4	result;
	result.r0 = Vector4_Set( 1.0f, 0.0f, 0.0f, 0.0f );
	result.r1 = Vector4_Set( 0.0f, 1.0f, 0.0f, 0.0f );
	result.r2 = Vector4_Set( 0.0f, 0.0f, 1.0f, 0.0f );
	result.r3 = Vector4_Set( 0.0f, 0.0f, 0.0f, 1.0f );
	return result;
}
inline const Float4x4 Matrix_Scaling( float x, float y, float z )
{
	Float4x4	result;
	result.r0 = Vector4_Set( x,    0.0f, 0.0f, 0.0f );
	result.r1 = Vector4_Set( 0.0f, y,    0.0f, 0.0f );
	result.r2 = Vector4_Set( 0.0f, 0.0f, z,    0.0f );
	result.r3 = Vector4_Set( 0.0f, 0.0f, 0.0f, 1.0f );
	return result;
}
inline const Float4x4 Matrix_Translation( float x, float y, float z )
{
	Float4x4	result;
	result.r0 = Vector4_Set( 1.0f, 0.0f, 0.0f, 0.0f );
	result.r1 = Vector4_Set( 0.0f, 1.0f, 0.0f, 0.0f );
	result.r2 = Vector4_Set( 0.0f, 0.0f, 1.0f, 0.0f );
	result.r3 = Vector4_Set( x,    y,    z,    1.0f );
	return result;
}
inline const Float4x4 Matrix_Translation( Vector4Arg xyz )
{
	Float4x4	result;
	result.r0 = Vector4_Set( 1.0f,  0.0f,  0.0f,  0.0f );
	result.r1 = Vector4_Set( 0.0f,  1.0f,  0.0f,  0.0f );
	result.r2 = Vector4_Set( 0.0f,  0.0f,  1.0f,  0.0f );
	result.r3 = Vector4_Set( xyz.x, xyz.y, xyz.z, 1.0f );
	return result;
}

// Transforms a 4D point by a 4x4 matrix.
inline const Vector4 Matrix_Transform( const Float4x4& m, Vector4Arg p )
{
	Vector4	result;
	result.x = (m.r0.x * p.x) + (m.r1.x * p.y) + (m.r2.x * p.z) + (m.r3.x * p.w);
	result.y = (m.r0.y * p.x) + (m.r1.y * p.y) + (m.r2.y * p.z) + (m.r3.y * p.w);
	result.z = (m.r0.z * p.x) + (m.r1.z * p.y) + (m.r2.z * p.z) + (m.r3.z * p.w);
	result.w = (m.r0.w * p.x) + (m.r1.w * p.y) + (m.r2.w * p.z) + (m.r3.w * p.w);
	return result;
}

// Transforms a 3D point (W=1) by a 4x4 matrix.
inline const Vector4 Matrix_Transform3( const Float4x4& m, Vector4Arg p )
{
	Vector4	result;
	result.x = (m.r0.x * p.x) + (m.r1.x * p.y) + (m.r2.x * p.z) + (m.r3.x);
	result.y = (m.r0.y * p.x) + (m.r1.y * p.y) + (m.r2.y * p.z) + (m.r3.y);
	result.z = (m.r0.z * p.x) + (m.r1.z * p.y) + (m.r2.z * p.z) + (m.r3.z);
	result.w = (m.r0.w * p.x) + (m.r1.w * p.y) + (m.r2.w * p.z) + (m.r3.w);
	return result;
}

// Transforms a 3D direction (W=0) by a 4x4 matrix.
inline const Vector4 Matrix_TransformNormal3( const Float4x4& m, Vector4Arg p )
{
	Vector4	result;
	result.x = (m.r0.x * p.x) + (m.r1.x * p.y) + (m.r2.x * p.z);
	result.y = (m.r0.y * p.x) + (m.r1.y * p.y) + (m.r2.y * p.z);
	result.z = (m.r0.z * p.x) + (m.r1.z * p.y) + (m.r2.z * p.z);
	result.w = (m.r0.w * p.x) + (m.r1.w * p.y) + (m.r2.w * p.z);
	return result;
}

// Performs a 4x4 matrix multiply by a 4x4 matrix.
// NOTE: Matrix multiplication order: left-to-right (as in DirectX), i.e. the transform 'b' is applied after 'a'.
inline const Float4x4 Matrix_Multiply( const Float4x4& a, const Float4x4& b )
{
	Float4x4	result;
	result.r0 = Matrix_Transform( b, a.r0 );
	result.r1 = Matrix_Transform( b, a.r1 );
	result.r2 = Matrix_Transform( b, a.r2 );
	result.r3 = Matrix_Transform( b, a.r3 );
	return result;
}

inline const Float4x4 Matrix_Transpose( const Float4x4& m )
{
	Float4x4	result;
	result.r0 = Vector4_Set( m.r0.x, m.r1.x, m.r2.x, m.r3.x );
	result.r1 = Vector4_Set( m.r0.y, m.r1.y, m.r2.y, m.r3.y );
	result.r2 = Vector4_Set( m.r0.z, m.r1.z, m.r2.z, m.r3.z );
	result.r3 = Vector4_Set( m.r0.w, m.r1.w, m.r2.w, m.r3.w );
	return result;
}
