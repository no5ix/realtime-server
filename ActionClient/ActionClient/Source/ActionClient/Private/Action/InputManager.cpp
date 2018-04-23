// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
//#include "Action.h"
#include "InputManager.h"
#include "ActionTiming.h"

std::unique_ptr< InputManager >	InputManager::sInstance;


namespace
{
	float kTimeBetweenInputSamples = 0.03f;
}


void InputManager::StaticInit()
{
	sInstance.reset( new InputManager() );
}


InputManager::InputManager() :
	mNextTimeToSampleInput( 0.f ),
	mPendingAction( nullptr )
{

}

void InputManager::HandleInput( EInputAction inInputAction, float inValue )
{
	switch (inInputAction)
	{
	case EIA_MoveForward:
		mCurrentState.mDesiredMoveForwardAmount = inValue;
		break;

	case EIA_MoveRight:
		mCurrentState.mDesiredMoveRightAmount = inValue;
		break;

	case EIA_Turn:
		mCurrentState.mDesiredTurnAmount = inValue;
		break;

	case EIA_LookUp:
		mCurrentState.mDesiredLookUpAmount = inValue;
		break;

	case EIA_MoveUp:
		mCurrentState.mDesiredMoveUpAmount = inValue;
		break;

	case EIA_LookUpRate:
		mCurrentState.mDesiredLookUpRateAmount = inValue;
		break;
	case EIA_TurnRate:
		mCurrentState.mDesiredTurnRateAmount = inValue;
		break;
	case EIA_OnStartJump:
		mCurrentState.mDesiredOnStartJumpAmount = inValue;
		break;
	case EIA_OnStopJump:
		mCurrentState.mDesiredOnStopJumpAmount = inValue;
		break;
	}
}



const Action& InputManager::SampleInputAsAction()
{
	return mActionList.AddAction( GetState(), ActionTiming::sInstance.GetFrameStartTime() );
}

bool InputManager::IsTimeToSampleInput()
{
	float time = ActionTiming::sInstance.GetFrameStartTime();
	if (time > mNextTimeToSampleInput)
	{
		mNextTimeToSampleInput = mNextTimeToSampleInput + kTimeBetweenInputSamples;
		return true;
	}

	return false;

	//return true;
}

void InputManager::Update()
{
	if (IsTimeToSampleInput())
	{
		mPendingAction = &SampleInputAsAction();
	}
}