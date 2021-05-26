#pragma once

#include <realtime_srv/game_obj/InputState.h>



class ExampleInputState : public realtime_srv::InputState
{
public:

	ExampleInputState() :
		desiredTurnRateAmount_(0.f),
		desiredLookUpRateAmount_(0.f),

		desiredOnStartJumpAmount_(0.f),
		desiredOnStopJumpAmount_(0.f),

		desiredMoveUpAmount_(0.f),

		isShooting_(false)
	{}

	float GetDesiredTurnRateAmount()	const { return desiredTurnRateAmount_; }
	float GetDesiredLookUpRateAmount()	const { return desiredLookUpRateAmount_; }

	float GetDesiredOnStartJumpAmount()	const { return desiredOnStartJumpAmount_; }
	float GetDesiredOnStopJumpAmount()	const { return desiredOnStopJumpAmount_; }

	float GetDesiredOnMoveUpAmount()	const { return desiredMoveUpAmount_; }

	bool  IsShooting()					const { return isShooting_; }

	virtual bool Write(realtime_srv::OutputBitStream& outputStream) const override
	{ return realtime_srv::InputState::Write(outputStream); }

	virtual bool Read(realtime_srv::InputBitStream& inputStream) override
	{ return realtime_srv::InputState::Read(inputStream); }

protected:

	float	desiredTurnRateAmount_;
	float	desiredLookUpRateAmount_;

	float	desiredOnStartJumpAmount_;
	float	desiredOnStopJumpAmount_;

	float	desiredMoveUpAmount_;

	bool	isShooting_;
};
typedef std::shared_ptr<ExampleInputState> ExampleInputStatePtr;
