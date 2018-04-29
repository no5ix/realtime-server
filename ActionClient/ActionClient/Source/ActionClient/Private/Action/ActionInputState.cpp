// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionInputState.h"


bool ActionInputState::Write( OutputMemoryBitStream& inOutputStream ) const
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

bool ActionInputState::Read( InputMemoryBitStream& inInputStream )
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
