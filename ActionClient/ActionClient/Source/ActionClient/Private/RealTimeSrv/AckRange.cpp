// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "AckRange.h"


void AckRange::Write( OutputBitStream& inOutputStream ) const
{
	inOutputStream.Write( mStart );
	bool hasCount = mCount > 1;
	inOutputStream.Write( hasCount );
	if ( hasCount )
	{
		//most you can ack is 255...
		uint32_t countMinusOne = mCount - 1;
		uint8_t countToAck = countMinusOne > 255 ? 255 : static_cast< uint8_t >( countMinusOne );
		inOutputStream.Write( countToAck );
	}
}

void AckRange::Read( InputBitStream& inInputStream )
{
	inInputStream.Read( mStart );
	bool hasCount;
	inInputStream.Read( hasCount );
	if ( hasCount )
	{
		uint8_t countMinusOne;
		inInputStream.Read( countMinusOne );
		mCount = countMinusOne + 1;
	}
	else
	{
		//default!
		mCount = 1;
	}
}