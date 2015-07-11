// Signed Distance Field.
#include "stdafx.h"
#pragma hdrstop
#include <Meshok/Meshok.h>
#include <Meshok/VoxelEngine.h>

Float3 AVolume::GetNormalAt( const Float3& _position ) const
{
	return EstimateNormal( *this, _position.x, _position.y, _position.z );
}
AVolume::Sample AVolume::SampleAt( const Float3& _position ) const
{
	Sample	result;
	result.normal = this->GetNormalAt(_position);
	result.distance = this->GetDistanceAt(_position);
	return result;
}

// see: Numerical Methods for Ray Tracing Implicitly Defined Surfaces
// http://graphics.williams.edu/courses/cs371/f14/reading/implicit.pdf
Float3 FindIntersectionLinearSearch(const Float3& p0, const Float3& p1, const AVolume* vol)
{
	// approximate the zero crossing by finding the min value along the edge
	float minValue = 100000.f;
	float t = 0.f;
	float currentT = 0.f;
	const int steps = 8;
	const float increment = 1.f / (float)steps;
	while (currentT <= 1.f)
	{
		const Float3 p = p0 + ((p1 - p0) * currentT);
		const float density = Float_Abs(vol->SampleAt(p).distance);
		if (density < minValue)
		{
			minValue = density;
			t = currentT;
		}

		currentT += increment;
	}

	return p0 + ((p1 - p0) * t);
}
#if 0

bool FindIntersectionBinarySearch( const AVolume* vol, const Float3& _start, const Float3& _end, Float3 &point )
{
	Float3 start = _start;
	Float3 end = _end;
	Point startP = vol->Sample( start );
	Point endP = vol->Sample( end );
	if( Sign(startP.density) == Sign(endP.density) ) {
		return false;
	}

//point=ApproximateZeroCrossingPosition(start,end,vol);
//	normal = vol->Sample(point).normal;
	//return true;

	float error = 1e-3f;
	int iteration = 0;
	float density = 0.0f;
	int maxIterations = 16;

	Point midP;

	for(;;)
	{
		Float3 mid = (start + end) * 0.5f;

		startP = vol->Sample( start );
		endP = vol->Sample( end );
		midP = vol->Sample( mid );

		float dStart = startP.density;
		float dMid = midP.density;
		float dEnd = endP.density;		

		mxASSERT(Sign(dStart) != Sign(dEnd));

//LogStream(LL_Info)<<"iter="<<iteration<<", start="<<start<<", mid="<<mid<<", end="<<end<<", dStart="<<dStart<<", dMid="<<dMid<<", dEnd="<<dEnd;

		if( Sign(dStart) != Sign(dMid) )
		{
			end = mid;
		}
		else //if( Sign(dMid) != Sign(dEnd) )
		{
			start = mid;
		}

		point = mid;
		density = (startP.density + endP.density) * 0.5f;
		normal = (startP.normal + endP.normal) * 0.5f;
		iteration++;

		float d0 = Abs(startP.density);
		float d1 = Abs(endP.density);
		float diff = Abs( d0 + d1 );

		if( diff <= error ) {
			break;
		}

//DBGOUT("iter[%d]: dStart=%f, dEnd=%f\n", iteration, startP.density, endP.density);

		//if( startP.density < 0.0f ) {
		//	end = mid;
		//} else if( endP.density < 0.0f ) {
		//	start = mid;
		//}

		if( iteration >= maxIterations ) {
			LogStream(LL_Info)<<"Too many iter: diff="<<diff;
			break;
		}
	}
}
#endif
// a simple ray-marching algorithm that finds zero-crossings of the given implicit surface
bool AVolume::IntersectsLine( const Float3& _start, const Float3& _end, Float3 &point, Float3 &normal ) const
{
	Float3 start = _start;
	Float3 end = _end;
	Sample startP = this->SampleAt( start );
	Sample endP = this->SampleAt( end );
	if( Sign(startP.distance) == Sign(endP.distance) ) {
		return false;
	}

point=FindIntersectionLinearSearch(start,end,this);
//	normal = this->SampleAt(point).normal;

normal = EstimateNormal(*this,point.x,point.y,point.z);

	return true;

#if 0
	float error = 1e-3f;
	int iteration = 0;
	float density = 0.0f;
	int maxIterations = 16;

	Point midP;

	for(;;)
	{
		Float3 mid = (start + end) * 0.5f;

		startP = this->Sample( start );
		endP = this->Sample( end );
		midP = this->Sample( mid );

		float dStart = startP.density;
		float dMid = midP.density;
		float dEnd = endP.density;		

		mxASSERT(Sign(dStart) != Sign(dEnd));

//LogStream(LL_Info)<<"iter="<<iteration<<", start="<<start<<", mid="<<mid<<", end="<<end<<", dStart="<<dStart<<", dMid="<<dMid<<", dEnd="<<dEnd;

		if( Sign(dStart) != Sign(dMid) )
		{
			end = mid;
		}
		else //if( Sign(dMid) != Sign(dEnd) )
		{
			start = mid;
		}

		point = mid;
		density = (startP.density + endP.density) * 0.5f;
		normal = (startP.normal + endP.normal) * 0.5f;
		iteration++;

		float d0 = Abs(startP.density);
		float d1 = Abs(endP.density);
		float diff = Abs( d0 + d1 );

		if( diff <= error ) {
			break;
		}

//DBGOUT("iter[%d]: dStart=%f, dEnd=%f\n", iteration, startP.density, endP.density);

		//if( startP.density < 0.0f ) {
		//	end = mid;
		//} else if( endP.density < 0.0f ) {
		//	start = mid;
		//}

		if( iteration >= maxIterations ) {
			LogStream(LL_Info)<<"Too many iter: diff="<<diff;
			break;
		}
	}
	return true;
#endif
}

bool SphereSDF::IntersectsLine(
	const Float3& start, const Float3& end,
	Float3 &point, Float3 &normal
) const
{
	return AVolume::IntersectsLine(start,end,point,normal);
#if 0
	mxSWIPED("idLib");
	//Returns true if the ray intersects the sphere.
	//The ray can intersect the sphere in both directions from the start point.
	//If start is inside the sphere then scale1 < 0 and scale2 > 0.
	//intersection points are (start + dir * scale1) and (start + dir * scale2)

	Float3 dir = Float3_Normalized(end - start);

	double a, b, c, d, sqrtd;

	Float3 p = start - center;

	a = dir * dir;
	b = dir * p;
	c = p * p - radius * radius;
	d = b * b - c * a;

	if ( d < 0.0f ) {
		return false;	// A negative discriminant corresponds to ray missing sphere.
	}

	sqrtd = Float_Sqrt( d );
	a = 1.0f / a;

	float scale1 = ( -b + sqrtd ) * a;
	float scale2 = ( -b - sqrtd ) * a;

	point = start + dir * scale1;
	normal = Float3_Normalized(point - center);

	return true;

#if 0
	Float3 r, s, e;
	FLOAT a;

	s = start - origin;
	e = end - origin;
	r = e - s;
	a = -s * r;
	if ( a <= 0 ) {
		return ( s * s < radius * radius );
	}
	else if ( a >= r * r ) {
		return ( e * e < radius * radius );
	}
	else {
		r = s + ( a / ( r * r ) ) * r;
		return ( r * r < radius * radius );
	}
#endif
#endif
}

float AxisAlignedBoxSDF::GetDistanceAt( const Float3& _position ) const
{
	Float3 halfSize = size * 0.5f;
	//AABB24 aabb = { halfSize * -1.0f, halfSize * +1.0f };
	//return AABB_ClosestDistance(aabb,_position);
	return Float3_Length(
			Float3_Max(
				Float3_Abs(_position) - size
				,
				Float3_Zero()
			)
		);
}

float CSGSubtraction::GetDistanceAt( const Float3& _position ) const
{
	const float dA = opA->GetDistanceAt(_position);
	const float dB = opB->GetDistanceAt(_position);
	// if point B is outside, return A
	return ( dB > 0.0f ) ? dA : -dB;
}
AVolume::Sample CSGSubtraction::SampleAt( const Float3& _position ) const
{
	const Sample pA = opA->SampleAt(_position);
	const Sample pB = opB->SampleAt(_position);

	// if point B is outside
	if( pB.distance > 0.0f )
	{
		return pA;
	}
	else
	{
		Sample result = { Float3_Negate(pB.normal), -pB.distance };
		return result;
	}
}

float CSGUnion::GetDistanceAt( const Float3& _position ) const
{
	const float dA = opA->GetDistanceAt(_position);
	const float dB = opB->GetDistanceAt(_position);

	return ( dA < dB ) ? dA : dB;
}
AVolume::Sample CSGUnion::SampleAt( const Float3& _position ) const
{
	const Sample pA = opA->SampleAt(_position);
	const Sample pB = opB->SampleAt(_position);

	// if point B is outside
	if( pA.distance < pB.distance )
	{
		return pA;
	}
	else
	{
		return pB;
	}
}

// 3D ray / AABox intersection by Jeroen Baert
// http://gamedev.stackexchange.com/a/24464
bool intersectRayAABox2(
						const Float3& origin, const Float3& direction,
						const AABB24& box,
						int& tnear, int& tfar
						)
{
    Float3 T_1, T_2; // vectors to hold the T-values for every direction
    double t_near = -DBL_MAX; // maximums defined in float.h
    double t_far = DBL_MAX;

    for (int i = 0; i < 3; i++){ //we test slabs in every direction
        if (direction[i] == 0){ // ray parallel to planes in this direction
            if ((origin[i] < box.min_point[i]) || (origin[i] > box.max_point[i])) {
                return false; // parallel AND outside box : no intersection possible
            }
        } else { // ray not parallel to planes in this direction
            T_1[i] = (box.min_point[i] - origin[i]) / direction[i];
            T_2[i] = (box.max_point[i] - origin[i]) / direction[i];

            if(T_1[i] > T_2[i]){ // we want T_1 to hold values for intersection with near plane
                TSwap(T_1,T_2);
            }
            if (T_1[i] > t_near){
                t_near = T_1[i];
            }
            if (T_2[i] < t_far){
                t_far = T_2[i];
            }
            if( (t_near > t_far) || (t_far < 0) ){
                return false;
            }
        }
    }
    tnear = t_near; tfar = t_far; // put return values in place
    return true; // if we made it here, there was an intersection - YAY
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
