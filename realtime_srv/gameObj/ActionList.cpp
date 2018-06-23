#include "realtime_srv/common/RealtimeSrvShared.h"




const Action& ActionList::AddMove( const InputState& inInputState, float inTimestamp )
{
	float deltaTime = mLastMoveTimestamp >= 0.f ? inTimestamp - mLastMoveTimestamp : 0.f;

	mMoves.emplace_back( inInputState, inTimestamp, deltaTime );

	mLastMoveTimestamp = inTimestamp;

	return mMoves.back();
}

bool ActionList::AddMoveIfNew( const Action& inMove )
{
	float timeStamp = inMove.GetTimestamp();

	if ( timeStamp > mLastMoveTimestamp )
	{
		float deltaTime = mLastMoveTimestamp >= 0.f ? timeStamp - mLastMoveTimestamp : 0.f;

		mLastMoveTimestamp = timeStamp;

		mMoves.emplace_back( inMove.GetInputState(), timeStamp, deltaTime );
		return true;
	}

	return false;
}

void	ActionList::RemovedProcessedMoves( float inLastMoveProcessedOnServerTimestamp )
{
	while (!mMoves.empty() && mMoves.front().GetTimestamp() <= inLastMoveProcessedOnServerTimestamp)
	{
		mMoves.pop_front();
	}
}
