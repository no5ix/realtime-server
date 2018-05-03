// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "RealTimeSrvHelper.h"
#include "RealTimeSrvTiming.h"


class WeightedTimedMovingAverage
{
public:

	WeightedTimedMovingAverage( float inDuration = 5.f ) :
		mDuration( inDuration ),
		mValue( 0.f )
	{
		mTimeLastEntryMade = RealTimeSrvTiming::sInstance.GetCurrentGameTime();
	}

	void UpdatePerSecond( float inValue )
	{
		float time = RealTimeSrvTiming::sInstance.GetCurrentGameTime();
		float timeSinceLastEntry = time - mTimeLastEntryMade;

		float valueOverTime = inValue / timeSinceLastEntry;

		//now update our value by whatever amount of the duration that was..
		float fractionOfDuration = ( timeSinceLastEntry / mDuration );
		if ( fractionOfDuration > 1.f ) { fractionOfDuration = 1.f; }

		mValue = mValue * ( 1.f - fractionOfDuration ) + valueOverTime * fractionOfDuration;

		mTimeLastEntryMade = time;
	}

	void Update( float inValue )
	{
		float time = RealTimeSrvTiming::sInstance.GetCurrentGameTime();
		float timeSinceLastEntry = time - mTimeLastEntryMade;

		//now update our value by whatever amount of the duration that was..
		float fractionOfDuration = ( timeSinceLastEntry / mDuration );
		if ( fractionOfDuration > 1.f ) { fractionOfDuration = 1.f; }

		//A_LOG_M( "fractionOfDuration = %f", fractionOfDuration );

		mValue = mValue * ( 1.f - fractionOfDuration ) + inValue * fractionOfDuration;

		mTimeLastEntryMade = time;
	}

	float GetValue() const { return mValue; }

private:

	float			mTimeLastEntryMade;
	float			mValue;
	float			mDuration;

};