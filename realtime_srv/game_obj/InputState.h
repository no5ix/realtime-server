// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

namespace realtime_srv
{

class InputState
{
public:

	InputState() :
		mDesiredMoveForwardAmount( 0.f ),
		mDesiredMoveRightAmount( 0.f ),

		mDesiredTurnAmountX( 0.f ),
		mDesiredTurnAmountY( 0.f ),
		mDesiredTurnAmountZ( 0.f ),

		mDesiredLookUpAmountX( 0.f ),
		mDesiredLookUpAmountY( 0.f ),
		mDesiredLookUpAmountZ( 0.f )
	{}

	float GetDesiredMoveForwardAmount()	const { return mDesiredMoveForwardAmount; }
	float GetDesiredMoveRightAmount()	const { return mDesiredMoveRightAmount; }

	Vector3 GetDesiredTurnRot()	const
	{ return Vector3( mDesiredTurnAmountX, mDesiredTurnAmountY, mDesiredTurnAmountZ ); }
	Vector3 GetDesiredLookUpRot()	const
	{ return Vector3( mDesiredLookUpAmountX, mDesiredLookUpAmountY, mDesiredLookUpAmountZ ); }


	virtual bool Write( OutputBitStream& inOutputStream ) const;
	virtual bool Read( InputBitStream& inInputStream );

protected:

	float	mDesiredMoveForwardAmount;
	float	mDesiredMoveRightAmount;

	float	mDesiredTurnAmountX;
	float	mDesiredTurnAmountY;
	float	mDesiredTurnAmountZ;

	float mDesiredLookUpAmountX;
	float mDesiredLookUpAmountY;
	float mDesiredLookUpAmountZ;

};
typedef std::shared_ptr<InputState> InputStatePtr;

}
