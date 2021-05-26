
#include "realtime_srv/game_obj/ActionList.h"


using namespace realtime_srv;



bool ActionList::AddMoveIfNew( const Action& inAction )
{
	float timeStamp = inAction.GetTimestamp();

	if ( timeStamp > mLastMoveTimestamp )
	{
		float deltaTime = mLastMoveTimestamp >= 0.f ? timeStamp - mLastMoveTimestamp : 0.f;

		mLastMoveTimestamp = timeStamp;

		actionQ_.emplace_back( inAction.GetInputState(), timeStamp, deltaTime );
		return true;
	}

	return false;
}

void	ActionList::RemovedProcessedMoves( float inLastMoveProcessedOnServerTimestamp )
{
	while ( !actionQ_.empty() && actionQ_.front().GetTimestamp() <= inLastMoveProcessedOnServerTimestamp )
	{
		actionQ_.pop_front();
	}
}
