#pragma once


#include "Character.h"

using namespace realtime_srv;

class Robot : public Character
{
public:

	Robot() : simulateInputeState_(new realtime_srv::InputState)
	{
		SetLocation(
			RealtimeSrvMath::GetRandomFloat() * 600.f,
			RealtimeSrvMath::GetRandomFloat() * 500.f,
			0.f);
	}

protected:

	// of course u can set location & rotation directly...XD
	virtual void AfterProcessInput() override
	{
		MakeSimulativeInput();

		Character::ProcessInput(kSimulateDeltaTime_, simulateInputeState_);

		Character::AfterProcessInput();
	}

	void MoveInCircles()
	{
		if (kYaw_ > 0)
			kYaw_ -= 1.6f;
		else
			kYaw_ = 360.f;
	}

	void MakeSimulativeInput()
	{
		MoveInCircles();

		simulateInputeState_->desiredMoveForwardAmount_ = 1.f;
		simulateInputeState_->desiredMoveRightAmount_ = 1.f;

		simulateInputeState_->desiredTurnAmountX_ = 0.f;
		simulateInputeState_->desiredTurnAmountY_ = kYaw_;
		simulateInputeState_->desiredTurnAmountZ_ = 0.f;

		simulateInputeState_->desiredLookUpAmountX_ = 0.f;
		simulateInputeState_->desiredLookUpAmountY_ = kYaw_;
		simulateInputeState_->desiredLookUpAmountZ_ = 0.f;
	}


protected:

	static float kYaw_;
	static const float kSimulateDeltaTime_;
	realtime_srv::InputStatePtr simulateInputeState_;

};

float Robot::kYaw_ = RealtimeSrvMath::GetRandomFloat() * 360.f;
const float Robot::kSimulateDeltaTime_ = 0.033f;
