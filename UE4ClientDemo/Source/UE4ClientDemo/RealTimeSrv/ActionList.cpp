// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionList.h"
#include "RealTimeSrvTiming.h"


const Action& ActionList::AddAction( const RealTimeSrvInputState& inInputState )
{
	//float deltaTime = mLastActionTimestamp >= 0.f ? inDeltaTime - mLastActionTimestamp : 0.f;


	mLastActionTimestamp = RealTimeSrvTiming::sInstance->GetFrameStartTime();

	mActions.emplace_back( 
		inInputState, 
		mLastActionTimestamp, 
		RealTimeSrvTiming::sInstance->GetDeltaTime()
	);

	return mActions.back();
}

void	ActionList::RemovedProcessedActions( float inLastActionProcessedOnServerTimestamp )
{
	while (!mActions.empty() && mActions.front().GetTimestamp() <= inLastActionProcessedOnServerTimestamp)
	{
		mActions.pop_front();
	}
}
