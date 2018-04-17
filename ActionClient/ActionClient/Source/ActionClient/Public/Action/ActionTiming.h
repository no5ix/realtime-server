// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class ActionTiming
{
public:

	ActionTiming();

	void Update();

	float GetDeltaTime() const { return mDeltaTime; }

	double GetTime() const;

	static double GetPlatformTime();

	float GetTimef() const
	{
		return static_cast< float >( GetTime() );
	}

	float GetFrameStartTime() const { return mFrameStartTimef; }


	static ActionTiming sInstance;

private:
	float		mDeltaTime;

	double		mLastFrameStartTime;
	float		mFrameStartTimef;

};