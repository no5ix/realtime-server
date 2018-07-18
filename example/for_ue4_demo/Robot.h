#pragma once


#include "Character.h"

using namespace realtime_srv;

class Robot : public Character
{
public:
	Robot()
	{
		SetLocation(
			RealtimeSrvMath::GetRandomFloat() * 600.f,
			RealtimeSrvMath::GetRandomFloat() * 500.f,
			1000.f );
	}

protected:
	virtual void AfterProcessInput() override
	{
		static float moveDirection = 1.f;
		static float yaw = RealtimeSrvMath::GetRandomFloat() * 360.f;
		if ( yaw > 0 )
		{
			moveDirection = 1.f;
			yaw -= 0.6f;
			if ( yaw > 180.f )
			{
				moveDirection = -1.f;
			}
		}
		else
		{
			yaw = 360.f;
			moveDirection = 0.f;
		}
		realtime_srv::InputStatePtr simulateInputeState( new realtime_srv::InputState(
			moveDirection,
			moveDirection,

			0.f,
			yaw,
			0.f,

			0.f,
			yaw,
			0.f
		) );

		float simulateDeltaTime = 0.016f;
		Character::ProcessInput( simulateDeltaTime, simulateInputeState );

		Character::AfterProcessInput();
	}


};