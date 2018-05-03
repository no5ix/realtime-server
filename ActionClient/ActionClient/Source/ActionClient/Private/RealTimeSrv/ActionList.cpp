// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionList.h"


const Action& ActionList::AddAction( const RealTimeSrvInputState& inInputState, float inTimestamp )
{
	//first move has 0 time. it's okay, it only happens once
	float deltaTime = mLastActionTimestamp >= 0.f ? inTimestamp - mLastActionTimestamp : 0.f;

	mActions.emplace_back( inInputState, inTimestamp, deltaTime );

	mLastActionTimestamp = inTimestamp;

	return mActions.back();
}

bool ActionList::AddActionIfNew( const Action& inAction )
{
	//we might have already received this move in another packet ( since we're sending the same move in multiple packets )
	//so make sure it's new...

	//adjust the deltatime and then place!
	float timeStamp = inAction.GetTimestamp();

	if (timeStamp > mLastActionTimestamp)
	{
		float deltaTime = mLastActionTimestamp >= 0.f ? timeStamp - mLastActionTimestamp : 0.f;

		mLastActionTimestamp = timeStamp;

		mActions.emplace_back( inAction.GetInputState(), timeStamp, deltaTime );
		return true;
	}

	return false;
}

void	ActionList::RemovedProcessedActions( float inLastActionProcessedOnServerTimestamp )
{
	while (!mActions.empty() && mActions.front().GetTimestamp() <= inLastActionProcessedOnServerTimestamp)
	{
		mActions.pop_front();
	}
}
