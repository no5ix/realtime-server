//#pragma once


//class InputState
//{
//public:
//
//	InputState() :
//		mDesiredRightAmount( 0 ),
//		mDesiredLeftAmount( 0 ),
//		mDesiredForwardAmount( 0 ),
//		mDesiredBackAmount( 0 ),
//		mIsShooting( false )
//	{}
//
//	float GetDesiredHorizontalDelta()	const { return mDesiredRightAmount - mDesiredLeftAmount; }
//	float GetDesiredVerticalDelta()		const { return mDesiredForwardAmount - mDesiredBackAmount; }
//	bool  IsShooting()					const { return mIsShooting; }
//
//	bool Write( OutputMemoryBitStream& inOutputStream ) const;
//	bool Read( InputMemoryBitStream& inInputStream );
//
//private:
//	friend class InputManager;
//
//	float	mDesiredRightAmount, mDesiredLeftAmount;
//	float	mDesiredForwardAmount, mDesiredBackAmount;
//	bool	mIsShooting;
//};


// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


class InputState
{
public:

	InputState() :
		mDesiredMoveForwardAmount( 0 ),
		mDesiredMoveRightAmount( 0 ),
		mDesiredLookUpAmount( 0 ),
		mDesiredTurnAmount( 0 ),
		mDesiredTurnRateAmount( 0 ),
		mDesiredLookUpRateAmount( 0 ),
		mDesiredOnStartJumpAmount( 0 ),
		mDesiredOnStopJumpAmount( 0 ),
		mDesiredMoveUpAmount( 0 ),
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