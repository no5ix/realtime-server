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

	Action(InputState* inputState) : inputState_(inputState) {}

	Action(const std::shared_ptr<InputState>& inputState, float timestamp, float deltaTime) :
		inputState_(inputState),
		timestamp_(timestamp),
		deltaTime_(deltaTime)
	{}


	std::shared_ptr<InputState>& GetInputState() { return inputState_; }
	const std::shared_ptr<InputState>& GetInputState() const { return inputState_; }
	float				GetTimestamp()	const { return timestamp_; }
	float				GetDeltaTime()	const { return deltaTime_; }

	bool Write(OutputBitStream& outputStream) const;
	bool Read(InputBitStream& inputStream);

private:
	std::shared_ptr<InputState>	inputState_;
	float		timestamp_;
	float		deltaTime_;

};

}
