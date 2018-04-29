


#include "ActionServerShared.h"


bool InputState::Write( OutputMemoryBitStream& inOutputStream ) const
{
	inOutputStream.Write( mDesiredMoveForwardAmount );
	inOutputStream.Write( mDesiredMoveRightAmount );


	inOutputStream.Write( mDesiredTurnAmountX );
	inOutputStream.Write( mDesiredTurnAmountY );
	inOutputStream.Write( mDesiredTurnAmountZ );

	inOutputStream.Write( mDesiredLookUpAmountX );
	inOutputStream.Write( mDesiredLookUpAmountY );
	inOutputStream.Write( mDesiredLookUpAmountZ );


	return true;
}

bool InputState::Read( InputMemoryBitStream& inInputStream )
{
	inInputStream.Read( mDesiredMoveForwardAmount );
	inInputStream.Read( mDesiredMoveRightAmount );


	inInputStream.Read( mDesiredTurnAmountX );
	inInputStream.Read( mDesiredTurnAmountY );
	inInputStream.Read( mDesiredTurnAmountZ );

	inInputStream.Read( mDesiredLookUpAmountX );
	inInputStream.Read( mDesiredLookUpAmountY );
	inInputStream.Read( mDesiredLookUpAmountZ );

	return true;
}
