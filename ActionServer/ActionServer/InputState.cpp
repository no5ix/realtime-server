#include "ActionServerShared.h"



bool InputState::Write( OutputMemoryBitStream& inOutputStream ) const
{
	inOutputStream.Write( mDesiredMoveForwardAmount );
	inOutputStream.Write( mDesiredMoveRightAmount );
	inOutputStream.Write( mDesiredTurnAmount );
	inOutputStream.Write( mDesiredLookUpAmount );

	return true;
}

bool InputState::Read( InputMemoryBitStream& inInputStream )
{
	inInputStream.Read( mDesiredMoveForwardAmount );
	inInputStream.Read( mDesiredMoveRightAmount );
	inInputStream.Read( mDesiredTurnAmount );
	inInputStream.Read( mDesiredLookUpAmount );
	return true;
}
