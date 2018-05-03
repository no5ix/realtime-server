// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "DeliveryNotificationMgr.h"
#include "RealTimeSrvHelper.h"



namespace
{
	const float kDelayBeforeAckTimeout = 0.5f;
}

DeliveryNotificationMgr::DeliveryNotificationMgr( bool inShouldSendAcks, bool inShouldProcessAcks ) :
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


DeliveryNotificationMgr::~DeliveryNotificationMgr()
{
}


InFlightPacket* DeliveryNotificationMgr::WriteSequenceNumber( OutputBitStream& inOutputStream )
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

void DeliveryNotificationMgr::WriteAckData( OutputBitStream& inOutputStream )
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



bool DeliveryNotificationMgr::ProcessSequenceNumber( InputBitStream& inInputStream )
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


void DeliveryNotificationMgr::AddPendingAck( PacketSequenceNumber inSequenceNumber )
{
	if ( mPendingAcks.size() == 0 || !mPendingAcks.back().ExtendIfShould( inSequenceNumber ) )
	{
		mPendingAcks.emplace_back( inSequenceNumber );
	}
}

