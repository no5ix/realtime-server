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

	float GetDeltaTime() const { return deltaTime_; }

	double GetGameTimeD() const;

	float GetCurrentGameTime() const
	{
		return static_cast<float>(GetGameTimeD());
	}

	float GetFrameStartTime() const { return frameStartTimef_; }


	static RealtimeSrvTiming sInst;

private:
	float		deltaTime_;
	uint64_t	deltaTick_;

	double		lastFrameStartTime_;
	float		frameStartTimef_;
	double		perfCountDuration_;

#ifdef IS_LINUX
	muduo::Timestamp sStartTime;
#endif // IS_LINUX
};
}