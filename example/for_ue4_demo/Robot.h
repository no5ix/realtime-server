#pragma once


#include "Character.h"

using namespace realtime_srv;

class Robot : public Character
{
public:

	Robot() : simulateInputeState_( new realtime_srv::InputState )
	{
		SetLocation(
			RealtimeSrvMath::GetRandomFloat() * 600.f,
			RealtimeSrvMath::GetRandomFloat() * 500.f,
			0.f );
	}

protected:

	// of course u can set location & rotation directly...XD
	virtual void AfterProcessInput() override
	{
		MakeSimulativeInput();

		Character::ProcessInput( kSimulateDeltaTime_, simulateInputeState_ );

		Character::AfterProcessInput();
	}

	void MoveInCircles()
	{
		if ( kYaw_ > 0 )
			kYaw_ -= 1.6f;
		else
			kYaw_ = 360.f;
	}

	void MakeSimulativeInput()
	{
		MoveInCircles();

		simulateInputeState_->mDesiredMoveForwardAmount = 1.f;
		simulateInputeState_->mDesiredMoveRightAmount = 1.f;

		simulateInputeState_->mDesiredTurnAmountX = 0.f;
		simulateInputeState_->mDesiredTurnAmountY = kYaw_;
		simulateInputeState_->mDesiredTurnAmountZ = 0.f;

		simulateInputeState_->mDesiredLookUpAmountX = 0.f;
		simulateInputeState_->mDesiredLookUpAmountY = kYaw_;
		simulateInputeState_->mDesiredLookUpAmountZ = 0.f;
	}


protected:

	static float kYaw_;
	static const float kSimulateDeltaTime_;
	realtime_srv::InputStatePtr simulateInputeState_;

};

float Robot::kYaw_ = RealtimeSrvMath::GetRandomFloat() * 360.f;
const float Robot::kSimulateDeltaTime_ = 0.033f;
