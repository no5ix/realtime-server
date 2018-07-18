#pragma once


#include "Character.h"

using namespace realtime_srv;

class Robot : public Character
{
	virtual void AfterProcessInput() override
	{

		//curRotation_ = Vector3( 0.f,
		//	realtime_srv::RealtimeSrvMath::GetRandomFloat() * 180.f,
		//	0.f );

		//curCameraRotation_ = Vector3(
		//	realtime_srv::RealtimeSrvMath::GetRandomFloat() * 180.f,
		//	realtime_srv::RealtimeSrvMath::GetRandomFloat() * 180.f,
		//	realtime_srv::RealtimeSrvMath::GetRandomFloat() * 180.f );


		//AddActionInput( curCameraRotation_.ToQuaternion() * Vector3::Forward(),
		//	( realtime_srv::RealtimeSrvMath::GetRandomFloat() > 0.8 ? 1 : -1 ) );

		//AddActionInput( curCameraRotation_.ToQuaternion() * Vector3::Right(),
		//	( realtime_srv::RealtimeSrvMath::GetRandomFloat() > 0.8 ? 1 : -1 ) );

		//float simulateDeltaTime = 0.016f;

		//ApplyControlInputToVelocity( simulateDeltaTime );

		//const Vector3& Delta = curVelocity_ * simulateDeltaTime;
		//if ( !Delta.IsNearlyZero( 1e-6f ) )
		//{
		//	SetLocation( GetLocation() + Delta );
		//}


		realtime_srv::InputStatePtr simulateInputeState( new realtime_srv::InputState(
			//realtime_srv::RealtimeSrvMath::GetRandomFloat() > 0.8 ? 1 : -1,
			//realtime_srv::RealtimeSrvMath::GetRandomFloat() > 0.8 ? 1 : -1,
			1.f,
			1.f,

			0.f,
			//realtime_srv::RealtimeSrvMath::GetRandomFloat() * 180.f,
			50.f,
			0.f,

			//realtime_srv::RealtimeSrvMath::GetRandomFloat() * 180.f,
			//realtime_srv::RealtimeSrvMath::GetRandomFloat() * 180.f,
			//realtime_srv::RealtimeSrvMath::GetRandomFloat() * 180.f
			0.f,
			0.f,
			0.f
		) );

		float simulateDeltaTime = 0.016f;
		Character::ProcessInput( simulateDeltaTime, simulateInputeState );

		Character::AfterProcessInput();
	}


};