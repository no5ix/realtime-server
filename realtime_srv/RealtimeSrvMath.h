#pragma once

namespace realtime_srv
{
class Vector3;
class Vector2;

namespace RealtimeSrvMath
{
float GetRandomFloat();

bool	Is3DVectorEqual( const Vector3& inA, const Vector3& inB );

bool	Is2DVectorEqual( const Vector2& inA, const Vector2& inB );

inline float ToDegrees( float inRadians )
{
	return inRadians * 180.0f / PI;
}

/** Computes absolute value in a generic way */
template< class T >
static inline T Abs( const T A )
{
	return ( A >= ( T )0 ) ? A : -A;
}

static inline float Sqrt( float Value ) { return sqrtf( Value ); }

/** Multiples value by itself */
template< class T >
static inline T Square( const T A )
{
	return A*A;
}

/** Clamps X to be between Min and Max, inclusive */
template< class T >
static inline T Clamp( const T X, const T Min, const T Max )
{
	return X < Min ? Min : X < Max ? X : Max;
}

/** Returns higher value in a generic way */
template< class T >
static inline T Max( const T A, const T B )
{
	return ( A >= B ) ? A : B;
}

static inline float InvSqrt( float number )
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y = number;
	i = *( long * )&y;
	i = 0x5f3759df - ( i >> 1 );
	y = *( float * )&i;
	y = y * ( threehalfs - ( x2 * y * y ) );

	return y;
}

}

}