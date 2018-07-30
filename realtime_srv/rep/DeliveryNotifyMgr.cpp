#include "realtime_srv/common/RealtimeSrvHelper.h"
#include "realtime_srv/rep/BitStream.h"
#include "realtime_srv/rep/InflightPacket.h"
#include "realtime_srv/rep/AckBitField.h"
#include "realtime_srv/net/ClientProxy.h"

#include "realtime_srv/rep/DeliveryNotifyMgr.h"


using namespace realtime_srv;

namespace
{
const float kDelayBeforeAckTimeout = 0.6f;
}

DeliveryNotifyMgr::DeliveryNotifyMgr(bool willSendAcks, bool willProcessAcks) :
	nextOutgoingSN_(0),
	nextExpectedSN_(0),
	willSendAcks_(willSendAcks),
	willProcessAcks_(willProcessAcks),
	deliveredPacketCnt_(0),
	droppedPacketCnt_(0),
	dispatchedPacketCnt_(0)
{
	if (willSendAcks_)
	{
		ackBitField_ = new AckBitField();
	}
}

DeliveryNotifyMgr::~DeliveryNotifyMgr()
{
	//LOG( "DNM destructor. Delivery rate %d%%, Drop rate %d%%",
	//	( 100 * mDeliveredPacketCount ) / mDispatchedPacketCount,
	//	( 100 * mDroppedPacketCount ) / mDispatchedPacketCount );

	if (willSendAcks_ && ackBitField_)
	{
		delete ackBitField_;
	}
}

InflightPacket* DeliveryNotifyMgr::WriteSequenceNumber(OutputBitStream& outputStream,
	ClientProxy* clientProxy)
{

	PacketSN sequenceNumber = nextOutgoingSN_++;
	outputStream.Write(sequenceNumber);

	++dispatchedPacketCnt_;

	if (willProcessAcks_)
	{
		inflightPackets_.emplace_back(sequenceNumber, clientProxy);

		return &inflightPackets_.back();
	}
	else
	{
		return nullptr;
	}
}

bool DeliveryNotifyMgr::ProcessSequenceNumber(InputBitStream& inputStream)
{
	PacketSN	sequenceNumber;

	inputStream.Read(sequenceNumber);
	if (RealtimeSrvHelper::SNGreaterThanOrEqual(sequenceNumber, nextExpectedSN_))
	{
		PacketSN lastSN = nextExpectedSN_ - 1;
		nextExpectedSN_ = sequenceNumber + 1;

		if (willSendAcks_)
		{
			ackBitField_->AddToAckBitField(sequenceNumber, lastSN);
		}

		return true;
	}
	else
	{
		return false;
	}

	return false;
}

void DeliveryNotifyMgr::ProcessTimedOutPackets()
{
	float timeoutTime = RealtimeSrvTiming::sInst.GetCurrentGameTime() - kDelayBeforeAckTimeout;

	while (!inflightPackets_.empty())
	{
		const auto& nextInFlightPacket = inflightPackets_.front();

		if (nextInFlightPacket.GetTimeDispatched() < timeoutTime)
		{
			HandlePacketDeliveryFailure(nextInFlightPacket);
			inflightPackets_.pop_front();
		}
		else
		{
			break;
		}
	}
}

void DeliveryNotifyMgr::HandlePacketDeliveryFailure(const InflightPacket& inflightPkt)
{
	++droppedPacketCnt_;
	inflightPkt.HandleDeliveryFailure();
}

void DeliveryNotifyMgr::HandlePacketDeliverySuccess(const InflightPacket& inflightPkt)
{
	++deliveredPacketCnt_;
	inflightPkt.HandleDeliverySuccess();
}

void DeliveryNotifyMgr::ProcessAckBitField(InputBitStream& inputStream)
{
	AckBitField recvedAckBitField;
	recvedAckBitField.Read(inputStream);

	PacketSN latestAckedSN = recvedAckBitField.GetLatestAckSN();
	PacketSN nextAckedSN =
		latestAckedSN - (ACK_BIT_FIELD_BYTE_LEN << 3);


	while (
		RealtimeSrvHelper::SNGreaterThanOrEqual(latestAckedSN, nextAckedSN)
		&& !inflightPackets_.empty())
	{
		const auto& nextInflightPacket = inflightPackets_.front();
		PacketSN nextInFlightPacketSN = nextInflightPacket.GetSequenceNumber();

		if (RealtimeSrvHelper::SNGreaterThan(nextAckedSN, nextInFlightPacketSN))
		{
			auto copyOfInFlightPacket = nextInflightPacket;
			inflightPackets_.pop_front();
			HandlePacketDeliveryFailure(copyOfInFlightPacket);
		}
		else if (nextAckedSN == nextInFlightPacketSN)
		{
			if (nextAckedSN == latestAckedSN
				|| recvedAckBitField.IsSetCorrespondingAckBit(nextAckedSN))
			{
				HandlePacketDeliverySuccess(nextInflightPacket);
				inflightPackets_.pop_front();
				++nextAckedSN;
			}
			else
			{
				auto copyOfInFlightPacket = nextInflightPacket;
				inflightPackets_.pop_front();
				HandlePacketDeliveryFailure(copyOfInFlightPacket);
				++nextAckedSN;
			}
		}
		else if (RealtimeSrvHelper::SNGreaterThan(nextInFlightPacketSN, nextAckedSN))
			nextAckedSN = nextInFlightPacketSN;
	}
}