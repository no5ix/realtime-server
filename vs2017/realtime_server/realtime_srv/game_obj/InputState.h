// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>
#include "realtime_srv/math/Vector3.h"

namespace realtime_srv
{

class OutputBitStream;
class InputBitStream;

class InputState
{
public:

	InputState(
		float desiredMoveForwardAmount = 0.f,
		float desiredMoveRightAmount = 0.f,

		float desiredTurnAmountX = 0.f,
		float desiredTurnAmountY = 0.f,
		float desiredTurnAmountZ = 0.f,

		float desiredLookUpAmountX = 0.f,
		float desiredLookUpAmountY = 0.f,
		float desiredLookUpAmountZ = 0.f)
		:
		desiredMoveForwardAmount_(desiredMoveForwardAmount),
		desiredMoveRightAmount_(desiredMoveRightAmount),

		desiredTurnAmountX_(desiredTurnAmountX),
		desiredTurnAmountY_(desiredTurnAmountY),
		desiredTurnAmountZ_(desiredTurnAmountZ),

		desiredLookUpAmountX_(desiredLookUpAmountX),
		desiredLookUpAmountY_(desiredLookUpAmountY),
		desiredLookUpAmountZ_(desiredLookUpAmountZ)
	{}

	float GetDesiredMoveForwardAmount()	const { return desiredMoveForwardAmount_; }
	float GetDesiredMoveRightAmount()	const { return desiredMoveRightAmount_; }

	Vector3 GetDesiredTurnRot()	const
	{ return Vector3(desiredTurnAmountX_, desiredTurnAmountY_, desiredTurnAmountZ_); }
	Vector3 GetDesiredLookUpRot()	const
	{ return Vector3(desiredLookUpAmountX_, desiredLookUpAmountY_, desiredLookUpAmountZ_); }


	virtual bool Write(OutputBitStream& outputStream) const;
	virtual bool Read(InputBitStream& inputStream);


public:

	float	desiredMoveForwardAmount_;
	float	desiredMoveRightAmount_;

	float	desiredTurnAmountX_;
	float	desiredTurnAmountY_;
	float	desiredTurnAmountZ_;

	float desiredLookUpAmountX_;
	float desiredLookUpAmountY_;
	float desiredLookUpAmountZ_;

};
typedef std::shared_ptr<InputState> InputStatePtr;

}
