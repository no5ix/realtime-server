#include "realtime_srv/rep/BitStream.h"
#include "realtime_srv/game_obj/InputState.h"

#include "realtime_srv/game_obj/Action.h"



using namespace realtime_srv;

bool Action::Write( OutputBitStream& inOutputStream ) const
{
	inputState_->Write( inOutputStream );
	inOutputStream.Write( mTimestamp );

	return true;
}

bool Action::Read( InputBitStream& inInputStream )
{
	inputState_->Read( inInputStream );
	inInputStream.Read( mTimestamp );

	return true;
}


