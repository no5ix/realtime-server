// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <deque>
#include "BitStream.h"
#include "AckRange.h"
#include "InFlightPacket.h"


class DeliveryNotifyMgr
{
public:


	DeliveryNotifyMgr( bool inShouldSendAcks, bool inShouldProcessAcks );
	~DeliveryNotifyMgr();

	inline	InFlightPacket*		WriteState( OutputBitStream& inOutputStream );
 	inline bool ReadAndProcessState( InputBitStream& inInputStream, bool inIsSliced );

	//void				ProcessTimedOutPackets();

	uint32_t			GetDroppedPacketCount()		const { return mDroppedPacketCount; }
	uint32_t			GetDeliveredPacketCount()	const { return mDeliveredPacketCount; }
	uint32_t			GetDispatchedPacketCount()	const { return mDispatchedPacketCount; }

	const std::deque< InFlightPacket >&	GetInFlightPackets()	const { return mInFlightPackets; }

private:



	InFlightPacket*		WriteSequenceNumber( OutputBitStream& inOutputStream );
	void				WriteAckData( OutputBitStream& inOutputStream );

 	//returns wether to drop the packet- if sequence number is too low!
 	bool ProcessSequenceNumber( InputBitStream& inInputStream, bool inIsSliced );
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


inline InFlightPacket* DeliveryNotifyMgr::WriteState( OutputBitStream& inOutputStream )
{
	InFlightPacket* toRet = WriteSequenceNumber( inOutputStream );
	if ( mShouldSendAcks )
	{
		WriteAckData( inOutputStream );
	}
	return toRet;
}

 inline bool DeliveryNotifyMgr::ReadAndProcessState( InputBitStream& inInputStream, bool inIsSliced )
 {
 	bool toRet = ProcessSequenceNumber( inInputStream, inIsSliced );
 	if ( mShouldProcessAcks )
 	{
 		//ProcessAcks( inInputStream );
 	}
 	return toRet;
 }