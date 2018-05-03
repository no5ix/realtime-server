// Fill out your copyright notice in the Description page of Project Settings.


#include "RealTimeSrvTiming.h"

RealTimeSrvTiming RealTimeSrvTiming::sInstance;

namespace
{
	double kStartTime = 0;
}

RealTimeSrvTiming::RealTimeSrvTiming()
{
	kStartTime = GetPlatformTime();
	mLastFrameStartTime = GetGameTimeD();
}

void RealTimeSrvTiming::Update()
{

	double currentTime = GetGameTimeD();

	mDeltaTime = ( float )( currentTime - mLastFrameStartTime );

	mLastFrameStartTime = currentTime;
	mFrameStartTimef = static_cast< float > ( mLastFrameStartTime );

}

double RealTimeSrvTiming::GetGameTimeD() const
{
	double currentTime = GetPlatformTime();
	return currentTime - kStartTime;
}

double RealTimeSrvTiming::GetPlatformTime()
{
	return FPlatformTime::Seconds();
}
