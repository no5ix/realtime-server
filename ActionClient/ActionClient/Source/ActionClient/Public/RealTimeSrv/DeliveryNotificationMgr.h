// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <deque>
#include "BitStream.h"
#include "AckRange.h"
#include "InFlightPacket.h"


class DeliveryNotificationMgr
{
public:


	DeliveryNotificationMgr( bool inShouldSendAcks, bool inShouldProcessAcks );
	~DeliveryNotificationMgr();

	inline	InFlightPacket*		WriteState( OutputBitStream& inOutputStream );
 	inline	bool				ReadAndProcessState( InputBitStream& inInputStream );

	//void				ProcessTimedOutPackets();

	uint32_t			GetDroppedPacketCount()		const { return mDroppedPacketCount; }
	uint32_t			GetDeliveredPacketCount()	const { return mDeliveredPacketCount; }
	uint32_t			GetDispatchedPacketCount()	const { return mDispatchedPacketCount; }

	const std::deque< InFlightPacket >&	GetInFlightPackets()	const { return mInFlightPackets; }

private:



	InFlightPacket*		WriteSequenceNumber( OutputBitStream& inOutputStream );
	void				WriteAckData( OutputBitStream& inOutputStream );

 	//returns wether to drop the packet- if sequence number is too low!
 	bool				ProcessSequenceNumber( InputBitStream& inInputStream );
 	//void				ProcessAcks( InputMemoryBitStream& inInputStream );
 
 
 	void				AddPendingAck( PacketSequenceNumber inSequenceNumber );
 	//void				HandlePacketDeliveryFailure( const InFlightPacket& inFlightPacket );
 	//void				HandlePacketDeliverySuccess( const InFlightPacket& inFlightPacket );

	PacketSequenceNumber	mNextOutgoingSequenceNumber;
	PacketSequenceNumber	mNextExpectedSequenceNumber;

	std::deque< InFlightPacket >	mInFlightPackets;
	std::deque< AckRange >		mPendingAcks;

	bool					mShouldSendAcks;
	bool					mShouldProcessAcks;

	uint32_t		mDeliveredPacketCount;
	uint32_t		mDroppedPacketCount;
	uint32_t		mDispatchedPacketCount;

};


inline InFlightPacket* DeliveryNotificationMgr::WriteState( OutputBitStream& inOutputStream )
{
	InFlightPacket* toRet = WriteSequenceNumber( inOutputStream );
	if ( mShouldSendAcks )
	{
		WriteAckData( inOutputStream );
	}
	return toRet;
}

 inline bool	DeliveryNotificationMgr::ReadAndProcessState( InputBitStream& inInputStream )
 {
 	bool toRet = ProcessSequenceNumber( inInputStream );
 	if ( mShouldProcessAcks )
 	{
 		//ProcessAcks( inInputStream );
 	}
 	return toRet;
 }