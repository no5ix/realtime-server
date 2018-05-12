// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BitStream.h"
#include "RealTimeSrvHelper.h"



class AckRange
{
public:
	AckRange() : mStart( 0 ), mCount( 0 ) {}

	AckRange( PacketSequenceNumber inStart ) : mStart( inStart ), mCount( 1 ) {}

	//if this is the next in sequence, just extend the range
	inline bool ExtendIfShould( PacketSequenceNumber inSequenceNumber );

	PacketSequenceNumber	GetStart() const { return mStart; }
	uint32_t		GetCount() const { return mCount; }

	void Write( OutputBitStream& inOutputStream ) const;
	void Read( InputBitStream& inInputStream );

private:
	PacketSequenceNumber	mStart;
	uint32_t		mCount;
};

inline bool AckRange::ExtendIfShould( PacketSequenceNumber inSequenceNumber )
{
	if ( inSequenceNumber == mStart + mCount )
	{
		++mCount;
		return true;
	}
	else
	{
		return false;
	}
}