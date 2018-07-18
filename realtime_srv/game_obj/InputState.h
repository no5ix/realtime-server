// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

namespace realtime_srv
{

class InputState
{
public:

	InputState(
		float _DesiredMoveForwardAmount = 0.f,
		float _DesiredMoveRightAmount = 0.f,

		float _DesiredTurnAmountX = 0.f,
		float _DesiredTurnAmountY = 0.f,
		float _DesiredTurnAmountZ = 0.f,

		float _DesiredLookUpAmountX = 0.f,
		float _DesiredLookUpAmountY = 0.f,
		float _DesiredLookUpAmountZ = 0.f )
		:
		mDesiredMoveForwardAmount( _DesiredMoveForwardAmount ),
		mDesiredMoveRightAmount( _DesiredMoveRightAmount ),

		mDesiredTurnAmountX( _DesiredTurnAmountX ),
		mDesiredTurnAmountY( _DesiredTurnAmountY ),
		mDesiredTurnAmountZ( _DesiredTurnAmountZ ),

		mDesiredLookUpAmountX( _DesiredLookUpAmountX ),
		mDesiredLookUpAmountY( _DesiredLookUpAmountY ),
		mDesiredLookUpAmountZ( _DesiredLookUpAmountZ )
	{}

	float GetDesiredMoveForwardAmount()	const { return mDesiredMoveForwardAmount; }
	float GetDesiredMoveRightAmount()	const { return mDesiredMoveRightAmount; }

	Vector3 GetDesiredTurnRot()	const
	{ return Vector3( mDesiredTurnAmountX, mDesiredTurnAmountY, mDesiredTurnAmountZ ); }
	Vector3 GetDesiredLookUpRot()	const
	{ return Vector3( mDesiredLookUpAmountX, mDesiredLookUpAmountY, mDesiredLookUpAmountZ ); }


	virtual bool Write( OutputBitStream& inOutputStream ) const;
	virtual bool Read( InputBitStream& inInputStream );


public:

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
