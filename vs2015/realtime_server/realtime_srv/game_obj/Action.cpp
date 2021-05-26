#include "realtime_srv/rep/BitStream.h"
#include "realtime_srv/game_obj/InputState.h"

#include "realtime_srv/game_obj/Action.h"



using namespace realtime_srv;

bool Action::Write(OutputBitStream& outputStream) const
{
	inputState_->Write(outputStream);
	outputStream.Write(timestamp_);

	return true;
}

bool Action::Read(InputBitStream& inputStream)
{
	inputState_->Read(inputStream);
	inputStream.Read(timestamp_);

	return true;
}


