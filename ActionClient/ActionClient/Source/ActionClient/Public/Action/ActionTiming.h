// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class ActionTiming
{
public:

	ActionTiming();

	void Update();

	float GetDeltaTime() const { return mDeltaTime; }

	double GetGameTimeD() const;

	static double GetPlatformTime();

	float GetCurrentGameTime() const
	{
		return static_cast< float >( GetGameTimeD() );
	}

	float GetFrameStartTime() const { return mFrameStartTimef; }


	static ActionTiming sInstance;

private:
	float		mDeltaTime;

	double		mLastFrameStartTime;
	float		mFrameStartTimef;

};