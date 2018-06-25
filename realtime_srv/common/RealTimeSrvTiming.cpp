#include "realtime_srv/common/RealtimeSrvShared.h"

using namespace realtime_srv;


#ifndef IS_WIN
#include <chrono>
using namespace std::chrono;
#endif

RealtimeSrvTiming	RealtimeSrvTiming::sInstance;

namespace
{
#ifdef IS_WIN
	LARGE_INTEGER sStartTime = { 0 };
#else
	high_resolution_clock::time_point sStartTime;
#endif
}

RealtimeSrvTiming::RealtimeSrvTiming()
{
#ifdef IS_WIN
	LARGE_INTEGER perfFreq;
	QueryPerformanceFrequency( &perfFreq );
	mPerfCountDuration = 1.0 / perfFreq.QuadPart;

	QueryPerformanceCounter( &sStartTime );

	mLastFrameStartTime = GetGameTimeD();
#else
	sStartTime = high_resolution_clock::now();
#endif
}

void RealtimeSrvTiming::Update()
{

	double currentTime = GetGameTimeD();

	mDeltaTime = ( float )( currentTime - mLastFrameStartTime );

	mLastFrameStartTime = currentTime;
	mFrameStartTimef = static_cast< float > ( mLastFrameStartTime );

}

double RealtimeSrvTiming::GetGameTimeD() const
{
#ifdef IS_WIN
	LARGE_INTEGER curTime, timeSinceStart;
	QueryPerformanceCounter( &curTime );

	timeSinceStart.QuadPart = curTime.QuadPart - sStartTime.QuadPart;

	return timeSinceStart.QuadPart * mPerfCountDuration;
#else
	auto now = high_resolution_clock::now();
	auto ms = duration_cast< milliseconds >( now - sStartTime ).count();
	//a little uncool to then convert into a double just to go back, but oh well.
	return static_cast< double >( ms ) / 1000;
#endif
}

