// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionInputState.h"


bool ActionInputState::Write( OutputMemoryBitStream& inOutputStream ) const
{
	return true;
}

bool ActionInputState::Read( InputMemoryBitStream& inInputStream )
{
	return true;
}
