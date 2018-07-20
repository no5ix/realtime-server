#pragma once

#include <stdint.h>
#include <deque>
#include "realtime_srv/common/RealtimeSrvMacro.h"
#include "realtime_srv/rep/AckBitField.h"

namespace realtime_srv
{


class OutputBitStream;
class InputBitStream;
class InflightPacket;
class ClientProxy;

class DeliveryNotifyMgr
{
public:


	DeliveryNotifyMgr( bool inShouldSendAcks, bool inShouldProcessAcks );
	~DeliveryNotifyMgr();

	inline	InflightPacket*		WriteState( OutputBitStream& inOutputStream,
		ClientProxy* inClientProxy );
	inline bool					ReadAndProcessState( InputBitStream& inInputStream );

	void						ProcessTimedOutPackets();

	uint32_t GetDroppedPacketCount()		const { return mDroppedPacketCount; }
	uint32_t GetDeliveredPacketCount()	const { return mDeliveredPacketCount; }
	uint32_t GetDispatchedPacketCount()	const { return mDispatchedPacketCount; }

	const std::deque<InflightPacket>&	GetInflightPackets() const
	{ return inflightPackets_; }

private:



	InflightPacket*		WriteSequenceNumber( OutputBitStream& inOutputStream,
		ClientProxy* inClientProxy );

	bool				ProcessSequenceNumber( InputBitStream& inInputStream );


	void				HandlePacketDeliveryFailure( const InflightPacket& inFlightPacket );
	void				HandlePacketDeliverySuccess( const InflightPacket& inFlightPacket );


	PacketSN	mNextOutgoingSequenceNumber;
	PacketSN	mNextExpectedSequenceNumber;

	std::deque<InflightPacket>	inflightPackets_;

	bool mShouldSendAcks;
	bool mShouldProcessAcks;

	uint32_t mDeliveredPacketCount;
	uint32_t mDroppedPacketCount;
	uint32_t mDispatchedPacketCount;

protected:
	AckBitField*			mAckBitField;
	void					ProcessAckBitField( InputBitStream& inInputStream );
};



inline InflightPacket* DeliveryNotifyMgr::WriteState( OutputBitStream& inOutputStream,
	ClientProxy* inClientProxy )
{
	InflightPacket* toRet = WriteSequenceNumber( inOutputStream, inClientProxy );
	if ( mShouldSendAcks )
	{
		mAckBitField->Write( inOutputStream );
	}
	return toRet;
}

inline bool DeliveryNotifyMgr::ReadAndProcessState( InputBitStream& inInputStream )
{
	bool toRet = ProcessSequenceNumber( inInputStream );
	if ( mShouldProcessAcks )
	{
		ProcessAckBitField( inInputStream );
	}
	return toRet;
}

}