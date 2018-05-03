// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "Action.h"

//

bool Action::Write( OutputBitStream& inOutputStream ) const
{
	mInputState.Write( inOutputStream );
	inOutputStream.Write( mTimestamp );

	return true;
}

bool Action::Read( InputBitStream& inInputStream )
{
	mInputState.Read( inInputStream );
	inInputStream.Read( mTimestamp );

	return true;
}