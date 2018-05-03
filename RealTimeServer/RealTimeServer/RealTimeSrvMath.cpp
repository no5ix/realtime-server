#include "RealTimeServerPCH.h"
#include "Vector2.h"
#include "Vector3.h"




#include <random>

//const Vector3 Vector3::Zero( 0.0f, 0.0f, 0.0f );
//const Vector3 Vector3::UnitX( 1.0f, 0.0f, 0.0f );
//const Vector3 Vector3::UnitY( 0.0f, 1.0f, 0.0f );
//const Vector3 Vector3::UnitZ( 0.0f, 0.0f, 1.0f );

float RealTimeSrvMath::GetRandomFloat()
{
	static std::random_device rd;
	static std::mt19937 gen( rd() );
	static std::uniform_real_distribution< float > dis( 0.f, 1.f );
	return dis( gen );
}

//Vector3 ActionServerMath::GetRandomVector( const Vector3& inMin, const Vector3& inMax )
//{
//	Vector3 r = Vector3( GetRandomFloat(), GetRandomFloat(), GetRandomFloat() );
//	return inMin + ( inMax - inMin ) * r;
//}


bool	RealTimeSrvMath::Is3DVectorEqual( const Vector3& inA, const Vector3& inB ) 
{
	return ( inA.X == inB.X && inA.Y == inB.Y && inA.Z == inB.Z );
}

bool	RealTimeSrvMath::Is2DVectorEqual( const Vector2& inA, const Vector2& inB )
{
	return ( inA.X == inB.X && inA.Y == inB.Y );
}