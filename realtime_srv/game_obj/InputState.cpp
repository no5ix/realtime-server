#include "realtime_srv/rep/BitStream.h"
#include "realtime_srv/game_obj/InputState.h"


using namespace realtime_srv;

bool InputState::Write(OutputBitStream& outputStream) const
{
	outputStream.Write(desiredMoveForwardAmount_);
	outputStream.Write(desiredMoveRightAmount_);


	outputStream.Write(desiredTurnAmountX_);
	outputStream.Write(desiredTurnAmountY_);
	outputStream.Write(desiredTurnAmountZ_);

	outputStream.Write(desiredLookUpAmountX_);
	outputStream.Write(desiredLookUpAmountY_);
	outputStream.Write(desiredLookUpAmountZ_);


	return true;
}

bool InputState::Read(InputBitStream& inputStream)
{
	inputStream.Read(desiredMoveForwardAmount_);
	inputStream.Read(desiredMoveRightAmount_);


	inputStream.Read(desiredTurnAmountX_);
	inputStream.Read(desiredTurnAmountY_);
	inputStream.Read(desiredTurnAmountZ_);

	inputStream.Read(desiredLookUpAmountX_);
	inputStream.Read(desiredLookUpAmountY_);
	inputStream.Read(desiredLookUpAmountZ_);

	return true;
}
