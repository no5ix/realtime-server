#include "RealTimeSrvPCH.h"



#if !_WIN32
#include <chrono>
using namespace std::chrono;
#endif

RealTimeSrvTiming	RealTimeSrvTiming::sInstance;

namespace
{
#if _WIN32
	LARGE_INTEGER sStartTime = { 0 };
#else
	high_resolution_clock::time_point sStartTime;
#endif
}

RealTimeSrvTiming::RealTimeSrvTiming()
{
#if _WIN32
	LARGE_INTEGER perfFreq;
	QueryPerformanceFrequency( &perfFreq );
	mPerfCountDuration = 1.0 / perfFreq.QuadPart;

	QueryPerformanceCounter( &sStartTime );

	mLastFrameStartTime = GetGameTimeD();
#else
	sStartTime = high_resolution_clock::now();
#endif
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
#if _WIN32
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

