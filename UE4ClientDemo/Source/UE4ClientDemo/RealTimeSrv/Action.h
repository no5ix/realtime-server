
#pragma once
#include "RealTimeSrvInputState.h"
#include "BitStream.h"

class Action{
public:

	Action() {}

	Action( const RealTimeSrvInputState& inInputState, float inTimestamp, float inDeltaTime ) :
		mInputState( inInputState ),
		mTimestamp( inTimestamp ),
		mDeltaTime( inDeltaTime )
	{}


	const RealTimeSrvInputState&	GetInputState()	const { return mInputState; }
	float				GetTimestamp()	const { return mTimestamp; }
	float				GetDeltaTime()	const { return mDeltaTime; }

	bool Write( OutputBitStream& inOutputStream ) const;
	bool Read( InputBitStream& inInputStream );

private:
	RealTimeSrvInputState	mInputState;
	float		mTimestamp;
	float		mDeltaTime;

};


