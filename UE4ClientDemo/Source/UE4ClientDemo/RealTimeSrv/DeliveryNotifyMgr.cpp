// Fill out your copyright notice in the Description page of Project Settings.


#include "DeliveryNotifyMgr.h"
#include "RealTimeSrvHelper.h"



DeliveryNotifyMgr::DeliveryNotifyMgr( bool inShouldSendAcks, bool inShouldProcessAcks ) :
	mNextOutgoingSequenceNumber( 0 ),
	mNextExpectedSequenceNumber( 0 ),
	mShouldSendAcks( inShouldSendAcks ),
	mShouldProcessAcks( inShouldProcessAcks ),
	mDeliveredPacketCount( 0 ),
	mDroppedPacketCount( 0 ),
	mDispatchedPacketCount( 0 )
{
	if ( mShouldSendAcks )
	{
		mAckBitField = new AckBitField();
	}
}

DeliveryNotifyMgr::~DeliveryNotifyMgr()
{
	if ( mShouldSendAcks && mAckBitField )
	{
		delete mAckBitField;
	}
}

void DeliveryNotifyMgr::Reset()
{
	if ( mShouldSendAcks && mAckBitField )
	{
		delete mAckBitField;
		mAckBitField = new AckBitField();
	}
	mNextOutgoingSequenceNumber = 0;
	mNextExpectedSequenceNumber = 0;
	mDeliveredPacketCount = 0;
	mDroppedPacketCount = 0;
	mDispatchedPacketCount = 0;
	mInFlightPackets.clear();
}

InFlightPacket* DeliveryNotifyMgr::WriteSequenceNumber( OutputBitStream& inOutputStream )
{
	PacketSN sequenceNumber = mNextOutgoingSequenceNumber++;
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


bool DeliveryNotifyMgr::ProcessSequenceNumber( InputBitStream& inInputStream )
{
	PacketSN	sequenceNumber;
	inInputStream.Read( sequenceNumber );
	if ( RealTimeSrvHelper::SequenceGreaterThanOrEqual( sequenceNumber, mNextExpectedSequenceNumber ) )
	{
		PacketSN lastSN = mNextExpectedSequenceNumber - 1;
		mNextExpectedSequenceNumber = sequenceNumber + 1;

		if ( mShouldSendAcks )
		{
			mAckBitField->AddToAckBitField( sequenceNumber, lastSN );
		}

		return true;
	}
	else
	{
		return false;
	}

	return false;
}


