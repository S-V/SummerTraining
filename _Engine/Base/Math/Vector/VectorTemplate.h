/*
=============================================================================
	File:	VectorTemplate.h
	Desc:	Templated vector classes.
=============================================================================
*/

#ifndef __MATH_VECTOR_TEMPLATE_H__
#define __MATH_VECTOR_TEMPLATE_H__



//@todo: Use Eigen.

mxSWIPED("Irrlicht");

template <class T>
class vector2d
{
public:
	//! Default constructor (null vector)
	vector2d() : x(0), y(0) {}
	//! Constructor with two different values
	vector2d(T nx, T ny) : x(nx), y(ny) {}
	//! Constructor with the same value for both members
	explicit vector2d(T n) : x(n), y(n) {}
	//! Copy constructor
	vector2d(const vector2d<T>& other) : x(other.x), y(other.y) {}

	// operators

	vector2d<T> operator-() const { return vector2d<T>(-x, -y); }

	vector2d<T>& operator=(const vector2d<T>& other) { x = other.x; y = other.y; return *this; }

	vector2d<T> operator+(const vector2d<T>& other) const { return vector2d<T>(x + other.x, y + other.y); }
	vector2d<T>& operator+=(const vector2d<T>& other) { x+=other.x; y+=other.y; return *this; }
	vector2d<T> operator+(const T v) const { return vector2d<T>(x + v, y + v); }
	vector2d<T>& operator+=(const T v) { x+=v; y+=v; return *this; }

	vector2d<T> operator-(const vector2d<T>& other) const { return vector2d<T>(x - other.x, y - other.y); }
	vector2d<T>& operator-=(const vector2d<T>& other) { x-=other.x; y-=other.y; return *this; }
	vector2d<T> operator-(const T v) const { return vector2d<T>(x - v, y - v); }
	vector2d<T>& operator-=(const T v) { x-=v; y-=v; return *this; }

	vector2d<T> operator*(const vector2d<T>& other) const { return vector2d<T>(x * other.x, y * other.y); }
	vector2d<T>& operator*=(const vector2d<T>& other) { x*=other.x; y*=other.y; return *this; }
	vector2d<T> operator*(const T v) const { return vector2d<T>(x * v, y * v); }
	vector2d<T>& operator*=(const T v) { x*=v; y*=v; return *this; }

	vector2d<T> operator/(const vector2d<T>& other) const { return vector2d<T>(x / other.x, y / other.y); }
	vector2d<T>& operator/=(const vector2d<T>& other) { x/=other.x; y/=other.y; return *this; }
	vector2d<T> operator/(const T v) const { return vector2d<T>(x / v, y / v); }
	vector2d<T>& operator/=(const T v) { x/=v; y/=v; return *this; }

	//! sort in order x, Y. Equality with rounding tolerance.
	bool operator<=(const vector2d<T>&other) const
	{
		return 	(x<other.x || core::equals(x, other.x)) ||
				(core::equals(x, other.x) && (y<other.y || core::equals(y, other.y)));
	}

	//! sort in order x, Y. Equality with rounding tolerance.
	bool operator>=(const vector2d<T>&other) const
	{
		return 	(x>other.x || core::equals(x, other.x)) ||
				(core::equals(x, other.x) && (y>other.y || core::equals(y, other.y)));
	}

	//! sort in order x, Y. Difference must be above rounding tolerance.
	bool operator<(const vector2d<T>&other) const
	{
		return 	(x<other.x && !core::equals(x, other.x)) ||
				(core::equals(x, other.x) && y<other.y && !core::equals(y, other.y));
	}

	//! sort in order x, Y. Difference must be above rounding tolerance.
	bool operator>(const vector2d<T>&other) const
	{
		return 	(x>other.x && !core::equals(x, other.x)) ||
				(core::equals(x, other.x) && y>other.y && !core::equals(y, other.y));
	}

	bool operator==(const vector2d<T>& other) const { return equals(other); }
	bool operator!=(const vector2d<T>& other) const { return !equals(other); }

	// functions

	//! Checks if this vector equals the other one.
	/** Takes floating point rounding errors into account.
	\param other Vector to compare with.
	\return True if the two vector are (almost) equal, else false. */
	bool equals(const vector2d<T>& other) const
	{
		return core::equals(x, other.x) && core::equals(y, other.y);
	}

	vector2d<T>& set(T nx, T ny) {x=nx; y=ny; return *this; }
	vector2d<T>& set(const vector2d<T>& p) { x=p.x; y=p.y; return *this; }

	//! Gets the length of the vector.
	/** \return The length of the vector. */
	T getLength() const { return core::squareroot( x*x + y*y ); }

	//! Get the squared length of this vector
	/** This is useful because it is much faster than getLength().
	\return The squared length of the vector. */
	T getLengthSQ() const { return x*x + y*y; }

	//! Get the dot product of this vector with another.
	/** \param other Other vector to take dot product with.
	\return The dot product of the two vectors. */
	T dotProduct(const vector2d<T>& other) const
	{
		return x*other.x + y*other.y;
	}

	//! Gets distance from another point.
	/** Here, the vector is interpreted as a point in 2-dimensional space.
	\param other Other vector to measure from.
	\return Distance from other point. */
	T getDistanceFrom(const vector2d<T>& other) const
	{
		return vector2d<T>(x - other.x, y - other.y).getLength();
	}

	//! Returns squared distance from another point.
	/** Here, the vector is interpreted as a point in 2-dimensional space.
	\param other Other vector to measure from.
	\return Squared distance from other point. */
	T getDistanceFromSQ(const vector2d<T>& other) const
	{
		return vector2d<T>(x - other.x, y - other.y).getLengthSQ();
	}

	//! rotates the point anticlockwise around a center by an amount of degrees.
	/** \param degrees Amount of degrees to rotate by, anticlockwise.
	\param center Rotation center.
	\return This vector after transformation. */
	vector2d<T>& rotateBy(DOUBLE degrees, const vector2d<T>& center=vector2d<T>())
	{
		degrees *= DEGTORAD64;
		const DOUBLE cs = cos(degrees);
		const DOUBLE sn = sin(degrees);

		x -= center.x;
		y -= center.y;

		set((T)(x*cs - y*sn), (T)(x*sn + y*cs));

		x += center.x;
		y += center.y;
		return *this;
	}

	//! Normalize the vector.
	/** The null vector is left untouched.
	\return Reference to this vector, after normalization. */
	vector2d<T>& normalize()
	{
		FLOAT length = (FLOAT)(x*x + y*y);
		if (core::equals(length, 0.f))
			return *this;
		length = core::reciprocal_squareroot ( length );
		x = (T)(x * length);
		y = (T)(y * length);
		return *this;
	}

	//! Calculates the angle of this vector in degrees in the trigonometric sense.
	/** 0 is to the right (3 o'clock), values increase counter-clockwise.
	This method has been suggested by Pr3t3nd3r.
	\return Returns a value between 0 and 360. */
	DOUBLE getAngleTrig() const
	{
		if (y == 0)
			return x < 0 ? 180 : 0;
		else
		if (x == 0)
			return y < 0 ? 270 : 90;

		if ( y > 0)
			if (x > 0)
				return atan((irr::DOUBLE)y/(irr::DOUBLE)x) * RADTODEG64;
			else
				return 180.0-atan((irr::DOUBLE)y/-(irr::DOUBLE)x) * RADTODEG64;
		else
			if (x > 0)
				return 360.0-atan(-(irr::DOUBLE)y/(irr::DOUBLE)x) * RADTODEG64;
			else
				return 180.0+atan(-(irr::DOUBLE)y/-(irr::DOUBLE)x) * RADTODEG64;
	}

	//! Calculates the angle of this vector in degrees in the counter trigonometric sense.
	/** 0 is to the right (3 o'clock), values increase clockwise.
	\return Returns a value between 0 and 360. */
	inline DOUBLE getAngle() const
	{
		if (y == 0) // corrected thanks to a suggestion by Jox
			return x < 0 ? 180 : 0;
		else if (x == 0)
			return y < 0 ? 90 : 270;

		// don't use getLength here to avoid precision loss with s32 vectors
		DOUBLE tmp = y / sqrt((DOUBLE)(x*x + y*y));
		tmp = atan( core::squareroot(1 - tmp*tmp) / tmp) * RADTODEG64;

		if (x>0 && y>0)
			return tmp + 270;
		else
		if (x>0 && y<0)
			return tmp + 90;
		else
		if (x<0 && y<0)
			return 90 - tmp;
		else
		if (x<0 && y>0)
			return 270 - tmp;

		return tmp;
	}

	//! Calculates the angle between this vector and another one in degree.
	/** \param b Other vector to test with.
	\return Returns a value between 0 and 90. */
	inline DOUBLE getAngleWith(const vector2d<T>& b) const
	{
		DOUBLE tmp = x*b.x + y*b.y;

		if (tmp == 0.0)
			return 90.0;

		tmp = tmp / core::squareroot((DOUBLE)((x*x + y*y) * (b.x*b.x + b.y*b.y)));
		if (tmp < 0.0)
			tmp = -tmp;

		return atan(sqrt(1 - tmp*tmp) / tmp) * RADTODEG64;
	}

	//! Returns if this vector interpreted as a point is on a line between two other points.
	/** It is assumed that the point is on the line.
	\param begin Beginning vector to compare between.
	\param end Ending vector to compare between.
	\return True if this vector is between begin and end, false if not. */
	bool isBetweenPoints(const vector2d<T>& begin, const vector2d<T>& end) const
	{
		if (begin.x != end.x)
		{
			return ((begin.x <= x && x <= end.x) ||
				(begin.x >= x && x >= end.x));
		}
		else
		{
			return ((begin.y <= y && y <= end.y) ||
				(begin.y >= y && y >= end.y));
		}
	}

	//! Creates an interpolated vector between this vector and another vector.
	/** \param other The other vector to interpolate with.
	\param d Interpolation value between 0.0f (all the other vector) and 1.0f (all this vector).
	Note that this is the opposite direction of interpolation to getInterpolated_quadratic()
	\return An interpolated vector.  This vector is not modified. */
	vector2d<T> getInterpolated(const vector2d<T>& other, DOUBLE d) const
	{
		DOUBLE inv = 1.0f - d;
		return vector2d<T>((T)(other.x*inv + x*d), (T)(other.y*inv + y*d));
	}

	//! Creates a quadratically interpolated vector between this and two other vectors.
	/** \param v2 Second vector to interpolate with.
	\param v3 Third vector to interpolate with (maximum at 1.0f)
	\param d Interpolation value between 0.0f (all this vector) and 1.0f (all the 3rd vector).
	Note that this is the opposite direction of interpolation to getInterpolated() and interpolate()
	\return An interpolated vector. This vector is not modified. */
	vector2d<T> getInterpolated_quadratic(const vector2d<T>& v2, const vector2d<T>& v3, DOUBLE d) const
	{
		// this*(1-d)*(1-d) + 2 * v2 * (1-d) + v3 * d * d;
		const DOUBLE inv = 1.0f - d;
		const DOUBLE mul0 = inv * inv;
		const DOUBLE mul1 = 2.0f * d * inv;
		const DOUBLE mul2 = d * d;

		return vector2d<T> ( (T)(x * mul0 + v2.x * mul1 + v3.x * mul2),
					(T)(y * mul0 + v2.y * mul1 + v3.y * mul2));
	}

	//! Sets this vector to the linearly interpolated vector between a and b.
	/** \param a first vector to interpolate with, maximum at 1.0f
	\param b second vector to interpolate with, maximum at 0.0f
	\param d Interpolation value between 0.0f (all vector b) and 1.0f (all vector a)
	Note that this is the opposite direction of interpolation to getInterpolated_quadratic()
	*/
	vector2d<T>& interpolate(const vector2d<T>& a, const vector2d<T>& b, DOUBLE d)
	{
		x = (T)((DOUBLE)b.x + ( ( a.x - b.x ) * d ));
		y = (T)((DOUBLE)b.y + ( ( a.y - b.y ) * d ));
		return *this;
	}

	//! X coordinate of vector.
	T x;

	//! Y coordinate of vector.
	T y;
};



#endif /* !__MATH_VECTOR_TEMPLATE_H__ */

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
