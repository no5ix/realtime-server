// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <memory>
#include "ActionInputState.h"
#include "Action.h"
#include "ActionList.h"

class InputManager
{
public:
	enum EInputAction
	{
		EIA_MoveForward,
		EIA_MoveRight,
		EIA_MoveUp,
		EIA_Turn,
		EIA_TurnRate,
		EIA_LookUp,
		EIA_LookUpRate,
		EIA_OnStartJump,
		EIA_OnStopJump
	};
public:


	static void StaticInit();
	static std::unique_ptr< InputManager >	sInstance;

	void HandleMoveInput( EInputAction inMoveInputAction, float inValue );
	void HandleTurnInput( EInputAction inTurnInputAction, float inX /*Pitch*/, float inY /*Yaw*/, float inZ /*Roll*/ );

	const ActionInputState& GetState()	const { return mCurrentState; }

	ActionList&			GetActionList() { return mActionList; }

	const Action*			GetAndClearPendingAction() { auto toRet = mPendingAction; mPendingAction = nullptr; return toRet; }

	void				Update();

private:
	const Action*		mPendingAction;
	ActionInputState mCurrentState;

	InputManager();

	bool				IsTimeToSampleInput();
	const Action&			SampleInputAsAction();

	ActionList		mActionList;
	float			mNextTimeToSampleInput;
};