#include "realtime_srv/math/Vector3.h"
#include "realtime_srv/math/Vector2.h"

#include <random>

using namespace realtime_srv;

float RealtimeSrvMath::GetRandomFloat()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution< float > dis(0.f, 1.f);

	//std::mt19937 gen;
	//gen.seed( std::random_device()( ) );
	//static std::uniform_real_distribution< float > dis( 0.f, 1.f ); // distribution in range [0.f, 1.f]

	return dis(gen);


	//return static_cast< float >( rand() ) / static_cast< float >( RAND_MAX );
}

bool RealtimeSrvMath::Is3DVectorEqual(const Vector3& A, const Vector3& B)
{
	return (A.X == B.X && A.Y == B.Y && A.Z == B.Z);
}

bool RealtimeSrvMath::Is2DVectorEqual(const Vector2& A, const Vector2& B)
{
	return (A.X == B.X && A.Y == B.Y);
}