// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "DeliveryNotificationManager.h"
#include "ActionHelper.h"



namespace
{
	const float kDelayBeforeAckTimeout = 0.5f;
}

DeliveryNotificationManager::DeliveryNotificationManager( bool inShouldSendAcks, bool inShouldProcessAcks ) :
	mNextOutgoingSequenceNumber( 0 ),
	mNextExpectedSequenceNumber( 0 ),
	//everybody starts at 0...
	mShouldSendAcks( inShouldSendAcks ),
	mShouldProcessAcks( inShouldProcessAcks ),
	mDeliveredPacketCount( 0 ),
	mDroppedPacketCount( 0 ),
	mDispatchedPacketCount( 0 )
{
}


DeliveryNotificationManager::~DeliveryNotificationManager()
{
}


InFlightPacket* DeliveryNotificationManager::WriteSequenceNumber( OutputMemoryBitStream& inOutputStream )
{
	//write the sequence number, but also create an inflight packet for this...
	PacketSequenceNumber sequenceNumber = mNextOutgoingSequenceNumber++;
	inOutputStream.Write( sequenceNumber );

	++mDispatchedPacketCount;

	if ( mShouldProcessAcks ) 
	{
		mInFlightPackets.emplace_back( sequenceNumber );

		return &mInFlightPackets.back();
	}
	else
	{
		return nullptr;
	}
}

void DeliveryNotificationManager::WriteAckData( OutputMemoryBitStream& inOutputStream )
{
	bool hasAcks = ( mPendingAcks.size() > 0 );

	inOutputStream.Write( hasAcks );
	if ( hasAcks )
	{
		//note, we could write all the acks
		mPendingAcks.front().Write( inOutputStream );
		mPendingAcks.pop_front();
	}
}



bool DeliveryNotificationManager::ProcessSequenceNumber( InputMemoryBitStream& inInputStream )
{
	PacketSequenceNumber	sequenceNumber;

	inInputStream.Read( sequenceNumber );
	if ( sequenceNumber == mNextExpectedSequenceNumber )
	{
		mNextExpectedSequenceNumber = sequenceNumber + 1;
		if ( mShouldSendAcks )
		{
			AddPendingAck( sequenceNumber );
		}
		return true;
	}
	else if ( sequenceNumber < mNextExpectedSequenceNumber )
	{
		return false;
	}
	else if ( sequenceNumber > mNextExpectedSequenceNumber )
	{
		mNextExpectedSequenceNumber = sequenceNumber + 1;
		if ( mShouldSendAcks )
		{
			AddPendingAck( sequenceNumber );
		}
		return true;
	}

	return false;
}


void DeliveryNotificationManager::AddPendingAck( PacketSequenceNumber inSequenceNumber )
{
	if ( mPendingAcks.size() == 0 || !mPendingAcks.back().ExtendIfShould( inSequenceNumber ) )
	{
		mPendingAcks.emplace_back( inSequenceNumber );
	}
}

