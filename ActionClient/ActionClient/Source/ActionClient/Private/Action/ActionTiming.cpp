// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionTiming.h"

ActionTiming ActionTiming::sInstance;

namespace
{
	double kStartTime = 0;
}

ActionTiming::ActionTiming()
{
	kStartTime = GetPlatformTime();
	mLastFrameStartTime = GetTime();
}

void ActionTiming::Update()
{

	double currentTime = GetTime();

	mDeltaTime = ( float )( currentTime - mLastFrameStartTime );

	mLastFrameStartTime = currentTime;
	mFrameStartTimef = static_cast< float > ( mLastFrameStartTime );

}

double ActionTiming::GetTime() const
{
	double currentTime = GetPlatformTime();
	return currentTime - kStartTime;
}

double ActionTiming::GetPlatformTime()
{
	return FPlatformTime::Seconds();
}
