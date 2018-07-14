#pragma once

#ifdef IS_LINUX
#include <muduo/base/Timestamp.h>
#endif // IS_LINUX

namespace realtime_srv
{
class RealtimeSrvTiming
{
public:

	RealtimeSrvTiming();

	void Update();

	float GetDeltaTime() const { return mDeltaTime; }

	double GetGameTimeD() const;

	float GetCurrentGameTime() const
	{
		return static_cast< float >( GetGameTimeD() );
	}

	float GetFrameStartTime() const { return mFrameStartTimef; }


	static RealtimeSrvTiming sInst;

private:
	float		mDeltaTime;
	uint64_t	mDeltaTick;

	double		mLastFrameStartTime;
	float		mFrameStartTimef;
	double		mPerfCountDuration;

#ifdef IS_LINUX
	muduo::Timestamp sStartTime;
#endif // IS_LINUX
};
}