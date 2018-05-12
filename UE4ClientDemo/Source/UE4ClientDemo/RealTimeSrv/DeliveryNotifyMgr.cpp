// Fill out your copyright notice in the Description page of Project Settings.


#include "DeliveryNotifyMgr.h"
#include "RealTimeSrvHelper.h"



namespace
{
	const float kDelayBeforeAckTimeout = 0.5f;
}

DeliveryNotifyMgr::DeliveryNotifyMgr( bool inShouldSendAcks, bool inShouldProcessAcks ) :
	mNextOutgoingSequenceNumber( 0 ),
	mNextExpectedSequenceNumber( 0 ),
	mShouldSendAcks( inShouldSendAcks ),
	mShouldProcessAcks( inShouldProcessAcks ),
	mDeliveredPacketCount( 0 ),
	mDroppedPacketCount( 0 ),
	mDispatchedPacketCount( 0 )
{
}


DeliveryNotifyMgr::~DeliveryNotifyMgr()
{
}


InFlightPacket* DeliveryNotifyMgr::WriteSequenceNumber( OutputBitStream& inOutputStream )
{
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

void DeliveryNotifyMgr::WriteAckData( OutputBitStream& inOutputStream )
{
	bool hasAcks = ( mPendingAcks.size() > 0 );

	inOutputStream.Write( hasAcks );
	if ( hasAcks )
	{
		mPendingAcks.front().Write( inOutputStream );
		mPendingAcks.pop_front();
	}
}

bool DeliveryNotifyMgr::ProcessSequenceNumber( InputBitStream& inInputStream )
{
	PacketSequenceNumber	sequenceNumber;
	inInputStream.Read( sequenceNumber );
	if ( RealTimeSrvHelper::SequenceGreaterThanOrEqual( sequenceNumber, mNextExpectedSequenceNumber ) )
	{
		mNextExpectedSequenceNumber = sequenceNumber + 1;

		if ( mShouldSendAcks )
		{
			AddPendingAck( sequenceNumber );
		}

		return true;
	}
	else
	{
		return false;
	}

	return false;
}


void DeliveryNotifyMgr::AddPendingAck( PacketSequenceNumber inSequenceNumber )
{
	if ( mPendingAcks.size() == 0 || !mPendingAcks.back().ExtendIfShould( inSequenceNumber ) )
	{
		mPendingAcks.emplace_back( inSequenceNumber );
	}
}

