#pragma once

namespace ActionServerMath
{
	//const float PI = 3.1415926535f;
	float GetRandomFloat();

	//Vector3 GetRandomVector( const Vector3& inMin, const Vector3& inMax );

	//inline bool	Is2DVectorEqual( const Vector3& inA, const Vector3& inB )
	//{
	//	return ( inA.mX == inB.mX && inA.mY == inB.mY );
	//}

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


	static inline float InvSqrt( float number  )
	{
		long i;
		float x2, y;
		const float threehalfs = 1.5F;

		x2 = number * 0.5F;
		y = number;
		i = *( long * )&y;                       // evil floating point bit level hacking（对浮点数的邪恶位元hack）
		i = 0x5f3759df - ( i >> 1 );               // what the fuck?（这他妈的是怎么回事？）
		y = *( float * )&i;
		y = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration （第一次迭代）
													//      y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed（第二次迭代，可以删除）

		return y;
	}

}

