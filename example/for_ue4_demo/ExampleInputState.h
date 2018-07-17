// Fill out your copyright notice in the Description page of Project Settings.
#include <realtime_srv/game_obj/InputState.h>

#pragma once


class ExampleInputState : public realtime_srv::InputState
{
public:

	ExampleInputState() :
		realtime_srv::InputState(),

		mDesiredTurnRateAmount( 0.f ),
		mDesiredLookUpRateAmount( 0.f ),

		mDesiredOnStartJumpAmount( 0.f ),
		mDesiredOnStopJumpAmount( 0.f ),

		mDesiredMoveUpAmount( 0.f ),

		mIsShooting( false )
	{}

	float GetDesiredTurnRateAmount()	const { return mDesiredTurnRateAmount; }
	float GetDesiredLookUpRateAmount()	const { return mDesiredLookUpRateAmount; }

	float GetDesiredOnStartJumpAmount()	const { return mDesiredOnStartJumpAmount; }
	float GetDesiredOnStopJumpAmount()	const { return mDesiredOnStopJumpAmount; }

	float GetDesiredOnMoveUpAmount()	const { return mDesiredMoveUpAmount; }

	bool  IsShooting()					const { return mIsShooting; }

	virtual bool Write( realtime_srv::OutputBitStream& inOutputStream ) const override
	{ return realtime_srv::InputState::Write( inOutputStream ); }

	virtual bool Read( realtime_srv::InputBitStream& inInputStream ) override
	{ return realtime_srv::InputState::Read( inInputStream ); }

protected:

	float	mDesiredTurnRateAmount;
	float	mDesiredLookUpRateAmount;

	float	mDesiredOnStartJumpAmount;
	float	mDesiredOnStopJumpAmount;

	float	mDesiredMoveUpAmount;

	bool	mIsShooting;
};
typedef std::shared_ptr<ExampleInputState> ExampleInputStatePtr;
