// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
//#include "Action.h"
#include "InputManager.h"
#include "ActionTiming.h"
#include "ActionHelper.h"

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

void InputManager::HandleTurnInput( EInputAction inTurnInputAction, float inX /*Pitch*/, float inY /*Yaw*/, float inZ /*Roll*/)
{

	switch ( inTurnInputAction )
	{
	case EIA_Turn:
		mCurrentState.mDesiredTurnAmountX = inX;
		mCurrentState.mDesiredTurnAmountY = inY;
		mCurrentState.mDesiredTurnAmountZ = inZ;
		break;

	case EIA_LookUp:
		mCurrentState.mDesiredLookUpAmountX = inX;
		mCurrentState.mDesiredLookUpAmountY = inY;
		mCurrentState.mDesiredLookUpAmountZ = inZ;
		break;

	//case EIA_LookUpRate:
	//	mCurrentState.mDesiredLookUpRateAmount = inValue;
	//	break;
	//case EIA_TurnRate:
	//	mCurrentState.mDesiredTurnRateAmount = inValue;
	//	break;
	}
}

void InputManager::HandleMoveInput( EInputAction inMoveInputAction, float inValue )
{
	switch (inMoveInputAction)
	{
	case EIA_MoveForward:
		mCurrentState.mDesiredMoveForwardAmount = inValue;
		break;

	case EIA_MoveRight:
		mCurrentState.mDesiredMoveRightAmount = inValue;
		break;


	case EIA_MoveUp:
		mCurrentState.mDesiredMoveUpAmount = inValue;
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

	//const Action& testAction = mActionList.AddAction( GetState(), ActionTiming::sInstance.GetFrameStartTime() );
	//A_LOG_N( "mActionList.GetActionCount() : ", float( mActionList.GetActionCount() ) );
	//return testAction;
}

bool InputManager::IsTimeToSampleInput()
{
	//float time = ActionTiming::sInstance.GetFrameStartTime();
	//if (time > mNextTimeToSampleInput)
	//{
	//	mNextTimeToSampleInput = mNextTimeToSampleInput + kTimeBetweenInputSamples;
	//	return true;
	//}

	//return false;

	return true;
}

void InputManager::Update()
{
	if (IsTimeToSampleInput())
	{
		mPendingAction = &SampleInputAsAction();
	}
}