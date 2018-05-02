#pragma once

class Action
{
public:

	Action() {}

	Action( const InputState& inInputState, float inTimestamp, float inDeltaTime ) :
		mInputState( inInputState ),
		mTimestamp( inTimestamp ),
		mDeltaTime( inDeltaTime )
	{}


	const InputState&	GetInputState()	const { return mInputState; }
	float				GetTimestamp()	const { return mTimestamp; }
	float				GetDeltaTime()	const { return mDeltaTime; }

	bool Write( OutputBitStream& inOutputStream ) const;
	bool Read( InputBitStream& inInputStream );

private:
	InputState	mInputState;
	float		mTimestamp;
	float		mDeltaTime;

};


