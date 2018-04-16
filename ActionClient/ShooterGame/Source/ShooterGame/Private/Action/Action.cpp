// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Action.h"

//

bool Action::Write( OutputMemoryBitStream& inOutputStream ) const
{
	return true;
}

bool Action::Read( InputMemoryBitStream& inInputStream )
{
	return true;
}
