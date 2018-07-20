#pragma once
#include <memory>
#include "realtime_srv/game_obj/InputState.h"


namespace realtime_srv
{

class InputState;
class OutputBitStream;
class InputBitStream;

class Action
{
public:

	Action( InputState* inInputState ) : inputState_( inInputState ) {}

	Action( const std::shared_ptr<InputState>& inInputState, float inTimestamp, float inDeltaTime ) :
		inputState_( inInputState ),
		mTimestamp( inTimestamp ),
		mDeltaTime( inDeltaTime )
	{}


	std::shared_ptr<InputState>& GetInputState() { return inputState_; }
	const std::shared_ptr<InputState>& GetInputState() const { return inputState_; }
	float				GetTimestamp()	const { return mTimestamp; }
	float				GetDeltaTime()	const { return mDeltaTime; }

	bool Write( OutputBitStream& inOutputStream ) const;
	bool Read( InputBitStream& inInputStream );

private:
	std::shared_ptr<InputState>	inputState_;
	float		mTimestamp;
	float		mDeltaTime;

};

}
