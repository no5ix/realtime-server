#pragma once

namespace realtime_srv
{
class Action
{
public:

	Action( InputState* inInputState ) : inputState_( inInputState ) {}

	Action( const InputStatePtr& inInputState, float inTimestamp, float inDeltaTime ) :
		inputState_( inInputState ),
		mTimestamp( inTimestamp ),
		mDeltaTime( inDeltaTime )
	{}


	InputStatePtr& GetInputState() { return inputState_; }
	const InputStatePtr& GetInputState() const { return inputState_; }
	float				GetTimestamp()	const { return mTimestamp; }
	float				GetDeltaTime()	const { return mDeltaTime; }

	bool Write( OutputBitStream& inOutputStream ) const;
	bool Read( InputBitStream& inInputStream );

private:
	InputStatePtr	inputState_;
	float		mTimestamp;
	float		mDeltaTime;

};

}
