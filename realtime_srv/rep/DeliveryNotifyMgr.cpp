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
	//LOG( "DNM destructor. Delivery rate %d%%, Drop rate %d%%",
	//	( 100 * mDeliveredPacketCount ) / mDispatchedPacketCount,
	//	( 100 * mDroppedPacketCount ) / mDispatchedPacketCount );

	if ( mShouldSendAcks && mAckBitField )
	{
		delete mAckBitField;
	}
}

InflightPacket* DeliveryNotifyMgr::WriteSequenceNumber( OutputBitStream& inOutputStream,
	ClientProxy* inClientProxy )
{

	PacketSN sequenceNumber = mNextOutgoingSequenceNumber++;
	inOutputStream.Write( sequenceNumber );

	++mDispatchedPacketCount;

	if ( mShouldProcessAcks )
	{
		inflightPackets_.emplace_back( sequenceNumber, inClientProxy );

		return &inflightPackets_.back();
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
	if ( RealtimeSrvHelper::SequenceGreaterThanOrEqual( sequenceNumber, mNextExpectedSequenceNumber ) )
		//if ( sequenceNumber >= mNextExpectedSequenceNumber )
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

void DeliveryNotifyMgr::ProcessTimedOutPackets()
{
	float timeoutTime = RealtimeSrvTiming::sInst.GetCurrentGameTime() - kDelayBeforeAckTimeout;

	while ( !inflightPackets_.empty() )
	{
		const auto& nextInFlightPacket = inflightPackets_.front();

		if ( nextInFlightPacket.GetTimeDispatched() < timeoutTime )
		{
			HandlePacketDeliveryFailure( nextInFlightPacket );
			inflightPackets_.pop_front();
		}
		else
		{
			break;
		}
	}
}

void DeliveryNotifyMgr::HandlePacketDeliveryFailure( const InflightPacket& inFlightPacket )
{
	++mDroppedPacketCount;
	inFlightPacket.HandleDeliveryFailure();
}

void DeliveryNotifyMgr::HandlePacketDeliverySuccess( const InflightPacket& inFlightPacket )
{
	++mDeliveredPacketCount;
	inFlightPacket.HandleDeliverySuccess();
}

void DeliveryNotifyMgr::ProcessAckBitField( InputBitStream& inInputStream )
{
	AckBitField recvedAckBitField;
	recvedAckBitField.Read( inInputStream );

	PacketSN LastAckedSN = recvedAckBitField.GetLatestAckSN();
	PacketSN nextAckedSN =
		LastAckedSN - ( ACK_BIT_FIELD_BYTE_LEN << 3 );


	while (
		RealtimeSrvHelper::SequenceGreaterThanOrEqual( LastAckedSN, nextAckedSN )
		//LastAckedSN >= nextAckdSequenceNumber
		&& !inflightPackets_.empty() )
	{
		const auto& nextInFlightPacket = inflightPackets_.front();
		PacketSN nextInFlightPacketSN = nextInFlightPacket.GetSequenceNumber();

		if ( RealtimeSrvHelper::SequenceGreaterThan( nextAckedSN, nextInFlightPacketSN ) )
			//if ( nextAckedSN > nextInFlightPacketSN )
		{
			auto copyOfInFlightPacket = nextInFlightPacket;
			inflightPackets_.pop_front();
			HandlePacketDeliveryFailure( copyOfInFlightPacket );
		}
		else if ( nextAckedSN == nextInFlightPacketSN )
		{
			if ( nextAckedSN == LastAckedSN
				|| recvedAckBitField.IsSetCorrespondingAckBit( nextAckedSN ) )
			{
				HandlePacketDeliverySuccess( nextInFlightPacket );
				inflightPackets_.pop_front();
				++nextAckedSN;
			}
			else
			{
				auto copyOfInFlightPacket = nextInFlightPacket;
				inflightPackets_.pop_front();
				HandlePacketDeliveryFailure( copyOfInFlightPacket );
				++nextAckedSN;
			}
		}
		else if ( RealtimeSrvHelper::SequenceGreaterThan( nextInFlightPacketSN, nextAckedSN ) )
	 //else if ( nextAckedSN < nextInFlightPacketSN )
		{
			nextAckedSN = nextInFlightPacketSN;
		}
	}
}