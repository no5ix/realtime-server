// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <deque>
#include "BitStream.h"
#include "InFlightPacket.h"
#include "RealTimeSrvHelper.h"
#include "AckBitField.h"

class DeliveryNotifyMgr
{
public:


	DeliveryNotifyMgr( bool inShouldSendAcks, bool inShouldProcessAcks );
	~DeliveryNotifyMgr();

	inline	InFlightPacket*		WriteState( OutputBitStream& inOutputStream );
 	inline bool ReadAndProcessState( InputBitStream& inInputStream );

	uint32_t			GetDroppedPacketCount()		const { return mDroppedPacketCount; }
	uint32_t			GetDeliveredPacketCount()	const { return mDeliveredPacketCount; }
	uint32_t			GetDispatchedPacketCount()	const { return mDispatchedPacketCount; }

	const std::deque< InFlightPacket >&	GetInFlightPackets()	const { return mInFlightPackets; }

private:

	InFlightPacket*		WriteSequenceNumber( OutputBitStream& inOutputStream );

 	bool ProcessSequenceNumber( InputBitStream& inInputStream );

	PacketSN	mNextOutgoingSequenceNumber;
	PacketSN	mNextExpectedSequenceNumber;

	std::deque< InFlightPacket >	mInFlightPackets;

	bool					mShouldSendAcks;
	bool					mShouldProcessAcks;

	uint32_t		mDeliveredPacketCount;
	uint32_t		mDroppedPacketCount;
	uint32_t		mDispatchedPacketCount;

protected:
	AckBitField*			mAckBitField;

};


inline InFlightPacket* DeliveryNotifyMgr::WriteState( OutputBitStream& inOutputStream )
{
	InFlightPacket* toRet = WriteSequenceNumber( inOutputStream );
	if ( mShouldSendAcks )
	{
		mAckBitField->Write( inOutputStream );
	}
	return toRet;
}

inline bool DeliveryNotifyMgr::ReadAndProcessState( InputBitStream& inInputStream )
{
	bool toRet = ProcessSequenceNumber( inInputStream );
	//if ( mShouldProcessAcks )
	//{
	//	ProcessAckBitField( inInputStream );
	//}
	return toRet;
}