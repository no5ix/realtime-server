#include "realtime_srv/common/RealtimeSrvShared.h"
#include "Vector2.h"
#include "Vector3.h"
#include <random>



float RealtimeSrvMath::GetRandomFloat()
{
	static std::random_device rd;
	static std::mt19937 gen( rd() );
	static std::uniform_real_distribution< float > dis( 0.f, 1.f );

	//std::mt19937 gen;
	//gen.seed( std::random_device()( ) );
	//static std::uniform_real_distribution< float > dis( 0.f, 1.f ); // distribution in range [0.f, 1.f]

	return dis( gen );


	//return static_cast< float >( rand() ) / static_cast< float >( RAND_MAX );
}

bool	RealtimeSrvMath::Is3DVectorEqual( const Vector3& inA, const Vector3& inB )
{
	return ( inA.X == inB.X && inA.Y == inB.Y && inA.Z == inB.Z );
}

bool	RealtimeSrvMath::Is2DVectorEqual( const Vector2& inA, const Vector2& inB )
{
	return ( inA.X == inB.X && inA.Y == inB.Y );
}