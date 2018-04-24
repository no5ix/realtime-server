// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionInputState.h"


bool ActionInputState::Write( OutputMemoryBitStream& inOutputStream ) const
{
	inOutputStream.Write( mDesiredMoveForwardAmount );
	inOutputStream.Write( mDesiredMoveRightAmount );
	inOutputStream.Write( mDesiredTurnAmount );
	inOutputStream.Write( mDesiredLookUpAmount );

	return true;
}

bool ActionInputState::Read( InputMemoryBitStream& inInputStream )
{
	inInputStream.Read( mDesiredMoveForwardAmount );
	inInputStream.Read( mDesiredMoveRightAmount );
	inInputStream.Read( mDesiredTurnAmount );
	inInputStream.Read( mDesiredLookUpAmount );
	return true;
}
