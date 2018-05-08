// Fill out your copyright notice in the Description page of Project Settings.


#include "RealTimeSrvTiming.h"
#include "RealTimeSrvHelper.h"

std::unique_ptr< RealTimeSrvTiming >	RealTimeSrvTiming::sInstance;

//namespace
//{
//	double kStartTime = 0;
//}

//RealTimeSrvTiming::RealTimeSrvTiming()
//{
	//kStartTime = GetUETime();
	//mLastFrameStartTime = GetGameTimeD();
//}

void RealTimeSrvTiming::Update(float inCurGameTime, float inDeltaTime)
{

	//double currentTime = GetGameTimeD();

	//mDeltaTime = ( float )( currentTime - mLastFrameStartTime );

	//mLastFrameStartTime = currentTime;
	//mFrameStartTimef = static_cast< float > ( mLastFrameStartTime );

	//A_LOG_1( "$$$$$$$$$$$$$$$ RealTimeSrvTiming::Update $$$$$$$$$$$$$$$$$$$$" );
	//A_LOG_N( "inDeltaTime = ", inDeltaTime );
	//A_LOG_N( "inCurGameTime = ", inCurGameTime );
	//A_LOG_N( "mLastFrameStartTime = ", mLastFrameStartTime );
	//A_LOG_N( "inCurGameTime - mLastFrameStartTime = ", inCurGameTime - mLastFrameStartTime );
	//A_LOG_1( "$$$$$$$$$$$$$$$$ RealTimeSrvTiming::Update $$$$$$$$$$$$$$$$$$$" );


	double currentTime = inCurGameTime;

	mDeltaTime = inDeltaTime;

	mLastFrameStartTime = currentTime;
	mFrameStartTimef = mLastFrameStartTime;

}

//double RealTimeSrvTiming::GetGameTimeD() const
//{
//	double currentTime = GetUETime();
//	return currentTime - kStartTime;
//}
//
//double RealTimeSrvTiming::GetUETime()
//{
//	return FPlatformTime::Seconds();
//}
