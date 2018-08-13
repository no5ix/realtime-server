#include "realtime_srv/common/RealtimeSrvShared.h"

#ifndef IS_WIN
#include <chrono>
using namespace std::chrono;
#endif // IS_WIN

#ifdef IS_LINUX
using namespace muduo;
#endif // IS_LINUX


using namespace realtime_srv;



RealtimeSrvTiming	RealtimeSrvTiming::sInst;

namespace
{
#ifdef IS_WIN
LARGE_INTEGER sStartTime = { 0 };
#else
#ifdef IS_LINUX
//muduo::Timestamp sStartTime;
#else
high_resolution_clock::time_point sStartTime;
#endif // IS_LINUX
#endif
}

RealtimeSrvTiming::RealtimeSrvTiming()
#ifdef IS_LINUX
	: sStartTime(muduo::Timestamp::now())
#endif
{
#ifdef IS_WIN
	LARGE_INTEGER perfFreq;
	QueryPerformanceFrequency(&perfFreq);
	perfCountDuration_ = 1.0 / perfFreq.QuadPart;

	QueryPerformanceCounter(&sStartTime);

	lastFrameStartTime_ = GetGameTimeD();
#else
#ifndef IS_LINUX
	sStartTime = high_resolution_clock::now();
#endif
#endif
}

void RealtimeSrvTiming::Update()
{

	double currentTime = GetGameTimeD();

	deltaTime_ = (float)(currentTime - lastFrameStartTime_);

	lastFrameStartTime_ = currentTime;
	frameStartTimef_ = static_cast<float> (lastFrameStartTime_);

}

double RealtimeSrvTiming::GetGameTimeD() const
{
#ifdef IS_WIN
	LARGE_INTEGER curTime, timeSinceStart;
	QueryPerformanceCounter(&curTime);
	timeSinceStart.QuadPart = curTime.QuadPart - sStartTime.QuadPart;
	return timeSinceStart.QuadPart * perfCountDuration_;

#else

#ifdef IS_LINUX

	return muduo::timeDifference(muduo::Timestamp::now(), sStartTime);

#else

	auto now = high_resolution_clock::now();
	auto ms = duration_cast<milliseconds>(now - sStartTime).count();
	//a little uncool to then convert into a double just to go back, but oh well.
	return static_cast<double>(ms) / 1000;

#endif

#endif
}

