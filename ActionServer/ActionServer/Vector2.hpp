
#pragma once

#define _USE_MATH_DEFINES
#include <math.h>


struct Vector2
{
	union
	{
		struct
		{
			double X;
			double Y;
		};
		double data[2];
	};


	/**
	* Constructors.
	*/
	inline Vector2();
	inline Vector2(double data[]);
	inline Vector2(double value);
	inline Vector2(double x, double y);


	/**
	* Constants for common vectors.
	*/
	static inline Vector2 Zero();
	static inline Vector2 One();
	static inline Vector2 Right();
	static inline Vector2 Left();
	static inline Vector2 Up();
	static inline Vector2 Down();


	/**
	* Returns the angle between two vectors in radians.
	* @param a: The first vector.
	* @param b: The second vector.
	* @return: A scalar value.
	*/
	static inline double Angle(Vector2 a, Vector2 b);

	/**
	* Returns a vector with its magnitude clamped to maxLength.
	* @param vector: The target vector.
	* @param maxLength: The maximum length of the return vector.
	* @return: A new vector.
	*/
	static inline Vector2 ClampMagnitude(Vector2 vector, double maxLength);

	/**
	* Returns the component of a in the direction of b (scalar projection).
	* @param a: The target vector.
	* @param b: The vector being compared against.
	* @return: A scalar value.
	*/
	static inline double Component(Vector2 a, Vector2 b);

	/**
	* Returns the distance between a and b.
	* @param a: The first point.
	* @param b: The second point.
	* @return: A scalar value.
	*/
	static inline double Distance(Vector2 a, Vector2 b);

	/**
	* Returns the dot product of two vectors.
	* @param lhs: The left side of the multiplication.
	* @param rhs: The right side of the multiplication.
	* @return: A scalar value.
	*/
	static inline double Dot(Vector2 lhs, Vector2 rhs);

	/**
	* Converts a polar representation of a vector into cartesian
	* coordinates.
	* @param rad: The magnitude of the vector.
	* @param theta: The angle from the X axis.
	* @return: A new vector.
	*/
	static inline Vector2 FromPolar(double rad, double theta);

	/**
	* Returns a vector linearly interpolated between a and b, moving along
	* a straight line. The vector is clamped to never go beyond the end points.
	* @param a: The starting point.
	* @param b: The ending point.
	* @param t: The interpolation value [0-1].
	* @return: A new vector.
	*/
	static inline Vector2 Lerp(Vector2 a, Vector2 b, double t);

	/**
	* Returns a vector linearly interpolated between a and b, moving along
	* a straight line.
	* @param a: The starting point.
	* @param b: The ending point.
	* @param t: The interpolation value [0-1] (no actual bounds).
	* @return: A new vector.
	*/
	static inline Vector2 LerpUnclamped(Vector2 a, Vector2 b, double t);

	/**
	* Returns the magnitude of a vector.
	* @param v: The vector in question.
	* @return: A scalar value.
	*/
	static inline double Magnitude(Vector2 v);

	/**
	* Returns a vector made from the largest components of two other vectors.
	* @param a: The first vector.
	* @param b: The second vector.
	* @return: A new vector.
	*/
	static inline Vector2 Max(Vector2 a, Vector2 b);

	/**
	* Returns a vector made from the smallest components of two other vectors.
	* @param a: The first vector.
	* @param b: The second vector.
	* @return: A new vector.
	*/
	static inline Vector2 Min(Vector2 a, Vector2 b);

	/**
	* Returns a vector "maxDistanceDelta" units closer to the target. This
	* interpolation is in a straight line, and will not overshoot.
	* @param current: The current position.
	* @param target: The destination position.
	* @param maxDistanceDelta: The maximum distance to move.
	* @return: A new vector.
	*/
	static inline Vector2 MoveTowards(Vector2 current, Vector2 target,
		double maxDistanceDelta);

	/**
	* Returns a new vector with magnitude of one.
	* @param v: The vector in question.
	* @return: A new vector.
	*/
	static inline Vector2 Normalized(Vector2 v);

	/**
	* Creates a new coordinate system out of the two vectors.
	* Normalizes "normal" and normalizes "tangent" and makes it orthogonal to
	* "normal"..
	* @param normal: A reference to the first axis vector.
	* @param tangent: A reference to the second axis vector.
	*/
	static inline void OrthoNormalize(Vector2 &normal, Vector2 &tangent);

	/**
	* Returns the vector projection of a onto b.
	* @param a: The target vector.
	* @param b: The vector being projected onto.
	* @return: A new vector.
	*/
	static inline Vector2 Project(Vector2 a, Vector2 b);

	/**
	* Returns a vector reflected about the provided line.
	* This behaves as if there is a plane with the line as its normal, and the
	* vector comes in and bounces off this plane.
	* @param vector: The vector traveling inward at the imaginary plane.
	* @param line: The line about which to reflect.
	* @return: A new vector pointing outward from the imaginary plane.
	*/
	static inline Vector2 Reflect(Vector2 vector, Vector2 line);

	/**
	* Returns the vector rejection of a on b.
	* @param a: The target vector.
	* @param b: The vector being projected onto.
	* @return: A new vector.
	*/
	static inline Vector2 Reject(Vector2 a, Vector2 b);

	/**
	* Rotates vector "current" towards vector "target" by "maxRadiansDelta".
	* This treats the vectors as directions and will linearly interpolate
	* between their magnitudes by "maxMagnitudeDelta". This function does not
	* overshoot. If a negative delta is supplied, it will rotate away from
	* "target" until it is pointing the opposite direction, but will not
	* overshoot that either.
	* @param current: The starting direction.
	* @param target: The destination direction.
	* @param maxRadiansDelta: The maximum number of radians to rotate.
	* @param maxMagnitudeDelta: The maximum delta for magnitude interpolation.
	* @return: A new vector.
	*/
	static inline Vector2 RotateTowards(Vector2 current, Vector2 target,
		double maxRadiansDelta,
		double maxMagnitudeDelta);

	/**
	* Multiplies two vectors component-wise.
	* @param a: The lhs of the multiplication.
	* @param b: The rhs of the multiplication.
	* @return: A new vector.
	*/
	static inline Vector2 Scale(Vector2 a, Vector2 b);

	/**
	* Returns a vector rotated towards b from a by the percent t.
	* Since interpolation is done spherically, the vector moves at a constant
	* angular velocity. This rotation is clamped to 0 <= t <= 1.
	* @param a: The starting direction.
	* @param b: The ending direction.
	* @param t: The interpolation value [0-1].
	*/
	static inline Vector2 Slerp(Vector2 a, Vector2 b, double t);

	/**
	* Returns a vector rotated towards b from a by the percent t.
	* Since interpolation is done spherically, the vector moves at a constant
	* angular velocity. This rotation is unclamped.
	* @param a: The starting direction.
	* @param b: The ending direction.
	* @param t: The interpolation value [0-1].
	*/
	static inline Vector2 SlerpUnclamped(Vector2 a, Vector2 b, double t);

	/**
	* Returns the squared magnitude of a vector.
	* This is useful when comparing relative lengths, where the exact length
	* is not important, and much time can be saved by not calculating the
	* square root.
	* @param v: The vector in question.
	* @return: A scalar value.
	*/
	static inline double SqrMagnitude(Vector2 v);

	/**
	* Calculates the polar coordinate space representation of a vector.
	* @param vector: The vector to convert.
	* @param rad: The magnitude of the vector.
	* @param theta: The angle from the X axis.
	*/
	static inline void ToPolar(Vector2 vector, double &rad, double &theta);


	/**
	* Operator overloading.
	*/
	inline struct Vector2& operator+=(const double rhs);
	inline struct Vector2& operator-=(const double rhs);
	inline struct Vector2& operator*=(const double rhs);
	inline struct Vector2& operator/=(const double rhs);
	inline struct Vector2& operator+=(const Vector2 rhs);
	inline struct Vector2& operator-=(const Vector2 rhs);
};

inline Vector2 operator-(Vector2 rhs);
inline Vector2 operator+(Vector2 lhs, const double rhs);
inline Vector2 operator-(Vector2 lhs, const double rhs);
inline Vector2 operator*(Vector2 lhs, const double rhs);
inline Vector2 operator/(Vector2 lhs, const double rhs);
inline Vector2 operator+(const double lhs, Vector2 rhs);
inline Vector2 operator-(const double lhs, Vector2 rhs);
inline Vector2 operator*(const double lhs, Vector2 rhs);
inline Vector2 operator/(const double lhs, Vector2 rhs);
inline Vector2 operator+(Vector2 lhs, const Vector2 rhs);
inline Vector2 operator-(Vector2 lhs, const Vector2 rhs);
inline bool operator==(const Vector2 lhs, const Vector2 rhs);
inline bool operator!=(const Vector2 lhs, const Vector2 rhs);



/*******************************************************************************
* Implementation
*/

Vector2::Vector2() : X(0), Y(0) {}
Vector2::Vector2(double data[]) : X(data[0]), Y(data[1]) {}
Vector2::Vector2(double value) : X(value), Y(value) {}
Vector2::Vector2(double x, double y) : X(x), Y(y) {}


Vector2 Vector2::Zero() { return Vector2(0, 0); }
Vector2 Vector2::One() { return Vector2(1, 1); }
Vector2 Vector2::Right() { return Vector2(1, 0); }
Vector2 Vector2::Left() { return Vector2(-1, 0); }
Vector2 Vector2::Up() { return Vector2(0, 1); }
Vector2 Vector2::Down() { return Vector2(0, -1); }


double Vector2::Angle(Vector2 a, Vector2 b)
{
	double v = Dot(a, b) / (Magnitude(a) * Magnitude(b));
	v = fmax(v, -1.0);
	v = fmin(v, 1.0);
	return acos(v);
}

Vector2 Vector2::ClampMagnitude(Vector2 vector, double maxLength)
{
	double length = Magnitude(vector);
	if (length > maxLength)
		vector *= maxLength / length;
	return vector;
}

double Vector2::Component(Vector2 a, Vector2 b)
{
	return Dot(a, b) / Magnitude(b);
}

double Vector2::Distance(Vector2 a, Vector2 b)
{
	return Vector2::Magnitude(a - b);
}

double Vector2::Dot(Vector2 lhs, Vector2 rhs)
{
	return lhs.X * rhs.X + lhs.Y * rhs.Y;
}

Vector2 Vector2::FromPolar(double rad, double theta)
{
	Vector2 v;
	v.X = rad * cos(theta);
	v.Y = rad * sin(theta);
	return v;
}

Vector2 Vector2::Lerp(Vector2 a, Vector2 b, double t)
{
	if (t < 0) return a;
	else if (t > 1) return b;
	return LerpUnclamped(a, b, t);
}

Vector2 Vector2::LerpUnclamped(Vector2 a, Vector2 b, double t)
{
	return (b - a) * t + a;
}

double Vector2::Magnitude(Vector2 v)
{
	return sqrt(SqrMagnitude(v));
}

Vector2 Vector2::Max(Vector2 a, Vector2 b)
{
	double x = a.X > b.X ? a.X : b.X;
	double y = a.Y > b.Y ? a.Y : b.Y;
	return Vector2(x, y);
}

Vector2 Vector2::Min(Vector2 a, Vector2 b)
{
	double x = a.X > b.X ? b.X : a.X;
	double y = a.Y > b.Y ? b.Y : a.Y;
	return Vector2(x, y);
}

Vector2 Vector2::MoveTowards(Vector2 current, Vector2 target,
	double maxDistanceDelta)
{
	Vector2 d = target - current;
	double m = Magnitude(d);
	if (m < maxDistanceDelta || m == 0)
		return target;
	return current + (d * maxDistanceDelta / m);
}

Vector2 Vector2::Normalized(Vector2 v)
{
	double mag = Magnitude(v);
	if (mag == 0)
		return Vector2::Zero();
	return v / mag;
}

void Vector2::OrthoNormalize(Vector2 &normal, Vector2 &tangent)
{
	normal = Normalized(normal);
	tangent = Reject(tangent, normal);
	tangent = Normalized(tangent);
}

Vector2 Vector2::Project(Vector2 a, Vector2 b)
{
	double m = Magnitude(b);
	return Dot(a, b) / (m * m) * b;
}

Vector2 Vector2::Reflect(Vector2 vector, Vector2 planeNormal)
{
	return vector - 2 * Project(vector, planeNormal);
}

Vector2 Vector2::Reject(Vector2 a, Vector2 b)
{
	return a - Project(a, b);
}

Vector2 Vector2::RotateTowards(Vector2 current, Vector2 target,
	double maxRadiansDelta,
	double maxMagnitudeDelta)
{
	double magCur = Magnitude(current);
	double magTar = Magnitude(target);
	double newMag = magCur + maxMagnitudeDelta *
		((magTar > magCur) - (magCur > magTar));
	newMag = fmin(newMag, fmax(magCur, magTar));
	newMag = fmax(newMag, fmin(magCur, magTar));

	double totalAngle = Angle(current, target) - maxRadiansDelta;
	if (totalAngle <= 0)
		return Normalized(target) * newMag;
	else if (totalAngle >= M_PI)
		return Normalized(-target) * newMag;

	double axis = current.X * target.Y - current.Y * target.X;
	axis = axis / fabs(axis);
	if (!(1 - fabs(axis) < 0.00001))
		axis = 1;
	current = Normalized(current);
	Vector2 newVector = current * cos(maxRadiansDelta) +
		Vector2(-current.Y, current.X) * sin(maxRadiansDelta) * axis;
	return newVector * newMag;
}

Vector2 Vector2::Scale(Vector2 a, Vector2 b)
{
	return Vector2(a.X * b.X, a.Y * b.Y);
}

Vector2 Vector2::Slerp(Vector2 a, Vector2 b, double t)
{
	if (t < 0) return a;
	else if (t > 1) return b;
	return SlerpUnclamped(a, b, t);
}

Vector2 Vector2::SlerpUnclamped(Vector2 a, Vector2 b, double t)
{
	double magA = Magnitude(a);
	double magB = Magnitude(b);
	a /= magA;
	b /= magB;
	double dot = Dot(a, b);
	dot = fmax(dot, -1.0);
	dot = fmin(dot, 1.0);
	double theta = acos(dot) * t;
	Vector2 relativeVec = Normalized(b - a * dot);
	Vector2 newVec = a * cos(theta) + relativeVec * sin(theta);
	return newVec * (magA + (magB - magA) * t);
}

double Vector2::SqrMagnitude(Vector2 v)
{
	return v.X * v.X + v.Y * v.Y;
}

void Vector2::ToPolar(Vector2 vector, double &rad, double &theta)
{
	rad = Magnitude(vector);
	theta = atan2(vector.Y, vector.X);
}


struct Vector2& Vector2::operator+=(const double rhs)
{
	X += rhs;
	Y += rhs;
	return *this;
}

struct Vector2& Vector2::operator-=(const double rhs)
{
	X -= rhs;
	Y -= rhs;
	return *this;
}

struct Vector2& Vector2::operator*=(const double rhs)
{
	X *= rhs;
	Y *= rhs;
	return *this;
}

struct Vector2& Vector2::operator/=(const double rhs)
{
	X /= rhs;
	Y /= rhs;
	return *this;
}

struct Vector2& Vector2::operator+=(const Vector2 rhs)
{
	X += rhs.X;
	Y += rhs.Y;
	return *this;
}

struct Vector2& Vector2::operator-=(const Vector2 rhs)
{
	X -= rhs.X;
	Y -= rhs.Y;
	return *this;
}

Vector2 operator-(Vector2 rhs) { return rhs * -1; }
Vector2 operator+(Vector2 lhs, const double rhs) { return lhs += rhs; }
Vector2 operator-(Vector2 lhs, const double rhs) { return lhs -= rhs; }
Vector2 operator*(Vector2 lhs, const double rhs) { return lhs *= rhs; }
Vector2 operator/(Vector2 lhs, const double rhs) { return lhs /= rhs; }
Vector2 operator+(const double lhs, Vector2 rhs) { return rhs += lhs; }
Vector2 operator-(const double lhs, Vector2 rhs) { return rhs -= lhs; }
Vector2 operator*(const double lhs, Vector2 rhs) { return rhs *= lhs; }
Vector2 operator/(const double lhs, Vector2 rhs) { return rhs /= lhs; }
Vector2 operator+(Vector2 lhs, const Vector2 rhs) { return lhs += rhs; }
Vector2 operator-(Vector2 lhs, const Vector2 rhs) { return lhs -= rhs; }

bool operator==(const Vector2 lhs, const Vector2 rhs)
{
	return lhs.X == rhs.X && lhs.Y == rhs.Y;
}

bool operator!=(const Vector2 lhs, const Vector2 rhs)
{
	return !(lhs == rhs);
}
