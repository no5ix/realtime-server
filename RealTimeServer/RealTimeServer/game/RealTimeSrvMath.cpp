#include "RealTimeSrvPCH.h"
#include "Vector2.h"
#include "Vector3.h"
#include <random>



float RealTimeSrvMath::GetRandomFloat()
{
	static std::random_device rd;
	static std::mt19937 gen( rd() );
	static std::uniform_real_distribution< float > dis( 0.f, 1.f );
	return dis( gen );
}


bool	RealTimeSrvMath::Is3DVectorEqual( const Vector3& inA, const Vector3& inB ) 
{
	return ( inA.X == inB.X && inA.Y == inB.Y && inA.Z == inB.Z );
}

bool	RealTimeSrvMath::Is2DVectorEqual( const Vector2& inA, const Vector2& inB )
{
	return ( inA.X == inB.X && inA.Y == inB.Y );
}