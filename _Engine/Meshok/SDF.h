// Signed Distance Field.
#pragma once

// useful links:
// modeling with distance functions:
// http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm

template< class DENSITY_SOURCE >	// where DENSITY_SOURCE has "GetDensity( float x, float y, float z ) : float"
Float3 EstimateNormal( const DENSITY_SOURCE& field, float _x, float _y, float _z, float D = 1e-3f )
{
#if 0
	float	d0	= field.GetDistanceAt( Float3_Set( _x, _y, _z ) );
	float	Nx	= field.GetDistanceAt( Float3_Set( _x + D, _y, _z ) ) - d0;
	float	Ny	= field.GetDistanceAt( Float3_Set( _x, _y + D, _z ) ) - d0;
	float	Nz	= field.GetDistanceAt( Float3_Set( _x, _y, _z + D ) ) - d0;
#else
	const Float3 P = Float3_Set( _x, _y, _z );
	const float Nx = field.GetDistanceAt(P + Float3_Set(D, 0.f, 0.f)) - field.GetDistanceAt(P - Float3_Set(D, 0.f, 0.f));
	const float Ny = field.GetDistanceAt(P + Float3_Set(0.f, D, 0.f)) - field.GetDistanceAt(P - Float3_Set(0.f, D, 0.f));
	const float Nz = field.GetDistanceAt(P + Float3_Set(0.f, 0.f, D)) - field.GetDistanceAt(P - Float3_Set(0.f, 0.f, D));
#endif
	Float3	N = Float3_Normalized( Float3_Set( Nx, Ny, Nz ) );
	mxASSERT(Float3_IsNormalized(N));
	return	N;
}

// distance field / density source
struct AVolume
{
	// estimates the density at the given point
	// positive if outside, negative if inside
	virtual float GetDistanceAt( const Float3& _position ) const = 0;

public:	// The following functions can be overridden for better efficiency:

	// estimates the gradient at the given point (points outward)
	virtual Float3 GetNormalAt( const Float3& _position ) const;

	struct Sample {
		Float3	normal;	// normal points outward
		float	distance;// positive if outside, negative if inside
	};
	// estimates the density and gradient at the given point
	virtual Sample SampleAt( const Float3& _position ) const;

	virtual bool IntersectsLine(
		const Float3& start, const Float3& end,
		Float3 &point, Float3 &normal
		) const;

	static inline bool IsSolid( float distance ) { return distance <= 0.0f; }

protected:
	virtual ~AVolume() {}
};

struct SphereSDF : public AVolume
{
	Float3	center;
	float	radius;
public:
	SphereSDF()
	{
		center = Float3_Zero();
		radius = 1.0f;
	}
	// returns <0 if the point is inside the solid
	virtual float GetDistanceAt( const Float3& _position ) const override
	{
		Float3	v = Float3_Subtract( _position, center );
		float	l = Float3_Length( v );
		float	d = l - radius;
		return	d;
	}
	virtual Float3 GetNormalAt( const Float3& _position ) const override
	{
		return Float3_Normalized( _position - center );
	}
	virtual Sample SampleAt( const Float3& _position ) const override
	{
		Float3	v = Float3_Subtract( _position, center );
		float	l = Float3_Length( v );
		float	d = l - radius;

		Sample	result;
		result.distance = d;
		result.normal = v / l;
		return result;
	}
	virtual bool IntersectsLine(
		const Float3& start, const Float3& end,
		Float3 &point, Float3 &normal
	) const;
};

struct HalfSpaceSDF : public AVolume
{
	Float4	plane;
public:
	HalfSpaceSDF()
	{
		plane = Float4_Zero();
	}
	Float3 GetPlaneNormal() const
	{
		return Float3_Normalized(Float3_Set(plane.x, plane.y, plane.z));
	}
	virtual float GetDistanceAt( const Float3& _position ) const override
	{
		return Plane_PointDistance( plane, _position );
	}
	virtual Float3 GetNormalAt( const Float3& _position ) const override
	{
		mxASSERT(Float3_IsNormalized( Float4_As_Float3( plane ) ));
		return Float4_As_Float3( plane );
	}
	virtual Sample SampleAt( const Float3& _position ) const override
	{
		mxASSERT(Float3_IsNormalized( Float4_As_Float3( plane ) ));
		Sample	result;
		result.distance = Plane_PointDistance( plane, _position );
		result.normal = Float4_As_Float3( plane );
		return result;
	}
};

struct AxisAlignedBoxSDF : public AVolume
{
	Float3	size;	// full size
public:
	AxisAlignedBoxSDF()
	{
		size = Float3_Replicate(1);
	}
	virtual float GetDistanceAt( const Float3& _position ) const override;
};

struct TorusSDF : public AVolume
{
	Float3	center;
	Float2	T;	// x = upper diameter, y = torus thickness
public:
	TorusSDF()
	{
		center = Float3_Zero();
		T = Float2_Set(1.0f, 1.0f);
	}
	virtual float GetDistanceAt( const Float3& _position ) const override
	{
		Float3 localPosition = _position - center;
		Float2 q = Float2_Set(
			Float2_Length(Float2_Set(localPosition.x,localPosition.z)) - T.x,
			_position.y
		);
		return Float2_Length(q) - T.y;
	}
};

struct TransformSDF : public AVolume
{
	AVolume *	op;
	Float4x4	xform;
public:
	TransformSDF()
	{
		op = nil;
		xform = Matrix_Identity();
	}
	virtual float GetDistanceAt( const Float3& _position ) const override
	{
		Float3 transformedPosition = Matrix_TransformPoint( xform, _position );
		return op->GetDistanceAt( transformedPosition );
	}
};



// A - B
struct CSGSubtraction : AVolume
{
	AVolume *	opA;
	AVolume *	opB;
public:
	virtual float GetDistanceAt( const Float3& _position ) const override;
	virtual Sample SampleAt( const Float3& _position ) const override;
};

// A + B
struct CSGUnion : AVolume
{
	AVolume *	opA;
	AVolume *	opB;
public:
	virtual float GetDistanceAt( const Float3& _position ) const override;
	virtual Sample SampleAt( const Float3& _position ) const override;
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
