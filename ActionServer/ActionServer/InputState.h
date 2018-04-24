
#pragma once


class InputState
{
public:

	InputState() :
		mDesiredMoveForwardAmount( 0.f ),
		mDesiredMoveRightAmount( 0.f ),
		mDesiredLookUpAmount( 0.f ),
		mDesiredTurnAmount( 0.f ),
		mDesiredTurnRateAmount( 0.f ),
		mDesiredLookUpRateAmount( 0.f ),
		mDesiredOnStartJumpAmount( 0.f ),
		mDesiredOnStopJumpAmount( 0.f ),
		mDesiredMoveUpAmount( 0.f ),
		mIsShooting( false )
	{}

	float GetDesiredMoveForwardAmount()	const { return mDesiredMoveForwardAmount; }
	float GetDesiredMoveRightAmount()	const { return mDesiredMoveRightAmount; }

	float GetDesiredTurnAmount()	const { return mDesiredTurnAmount; }
	float GetDesiredLookUpAmount()	const { return mDesiredLookUpAmount; }

	float GetDesiredTurnRateAmount()	const { return mDesiredTurnRateAmount; }
	float GetDesiredLookUpRateAmount()	const { return mDesiredLookUpRateAmount; }

	float GetDesiredOnStartJumpAmount()	const { return mDesiredOnStartJumpAmount; }
	float GetDesiredOnStopJumpAmount()	const { return mDesiredOnStopJumpAmount; }

	float GetDesiredOnMoveUpAmount()	const { return mDesiredMoveUpAmount; }

	bool  IsShooting()					const { return mIsShooting; }

	bool Write( OutputMemoryBitStream& inOutputStream ) const;
	bool Read( InputMemoryBitStream& inInputStream );

private:

	float	mDesiredTurnAmount;
	float   mDesiredLookUpAmount;

	float	mDesiredMoveRightAmount;
	float	mDesiredMoveForwardAmount;

	float	mDesiredTurnRateAmount;
	float	mDesiredLookUpRateAmount;

	float	mDesiredOnStartJumpAmount;
	float	mDesiredOnStopJumpAmount;

	float	mDesiredMoveUpAmount;

	bool	mIsShooting;
};