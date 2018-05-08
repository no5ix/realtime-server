// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>
#include "Engine/World.h"

class RealTimeSrvTiming
{
public:


	static void StaticInit( UWorld * inWorld ) 
	{ 
		sInstance.reset( new RealTimeSrvTiming() ); 	
		check( sInstance );
		if ( sInstance )
		{
			sInstance->mWorld = inWorld;
		}
	}

	void Update( float inCurGameTime, float inDeltaTime );

	float GetDeltaTime() const { return mDeltaTime; }

	//double GetGameTimeD() const;

	//static double GetUETime();

	float GetCurrentGameTime() const
	{
		//return static_cast< float >( GetGameTimeD() );
		return mWorld->GetTimeSeconds();
	}

	float GetFrameStartTime() const { return mFrameStartTimef; }


	static std::unique_ptr< RealTimeSrvTiming > sInstance;

private:
	RealTimeSrvTiming() {}

private:
	float		mDeltaTime;

	double		mLastFrameStartTime;
	float		mFrameStartTimef;

	UWorld*		mWorld;

};