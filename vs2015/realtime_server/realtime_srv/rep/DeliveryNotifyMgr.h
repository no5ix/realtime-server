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


	DeliveryNotifyMgr(bool willSendAcks, bool willProcessAcks);
	~DeliveryNotifyMgr();

	inline	InflightPacket*		WriteState(OutputBitStream& outputStream,
		ClientProxy* clientProxy);

	inline bool ReadAndProcessState(InputBitStream& inputStream);

	void ProcessTimedOutPackets();

	uint32_t GetDroppedPacketCount()		const { return droppedPacketCnt_; }
	uint32_t GetDeliveredPacketCount()	const { return deliveredPacketCnt_; }
	uint32_t GetDispatchedPacketCount()	const { return dispatchedPacketCnt_; }

	const std::deque<InflightPacket>&	GetInflightPackets() const
	{ return inflightPackets_; }

private:

	void					ProcessAckBitField(InputBitStream& inputStream);

	InflightPacket*		WriteSequenceNumber(OutputBitStream& outputStream,
		ClientProxy* clientProxy);

	bool				ProcessSequenceNumber(InputBitStream& inputStream);

	void				HandlePacketDeliveryFailure(const InflightPacket& inflightPkt);
	void				HandlePacketDeliverySuccess(const InflightPacket& inflightPkt);

protected:

	PacketSN	nextOutgoingSN_;
	PacketSN	nextExpectedSN_;

	std::deque<InflightPacket>	inflightPackets_;

	bool willSendAcks_;
	bool willProcessAcks_;

	uint32_t deliveredPacketCnt_;
	uint32_t droppedPacketCnt_;
	uint32_t dispatchedPacketCnt_;

	AckBitField*			ackBitField_;
};



inline InflightPacket* DeliveryNotifyMgr::WriteState(OutputBitStream& outputStream,
	ClientProxy* clientProxy)
{
	InflightPacket* toRet = WriteSequenceNumber(outputStream, clientProxy);
	if (willSendAcks_)
		ackBitField_->Write(outputStream);

	return toRet;
}

inline bool DeliveryNotifyMgr::ReadAndProcessState(InputBitStream& inputStream)
{
	bool toRet = ProcessSequenceNumber(inputStream);
	if (willProcessAcks_)
		ProcessAckBitField(inputStream);

	return toRet;
}

}