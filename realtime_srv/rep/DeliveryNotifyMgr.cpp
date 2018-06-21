#include "realtime_srv/common/RealTimeSrvShared.h"

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

InFlightPacket* DeliveryNotifyMgr::WriteSequenceNumber( OutputBitStream& inOutputStream, 
	ReplicationMgr* inRepMgr, NetworkMgr* inNetworkMgr )
{

	PacketSN sequenceNumber = mNextOutgoingSequenceNumber++;
	inOutputStream.Write( sequenceNumber );

	++mDispatchedPacketCount;

	if ( mShouldProcessAcks )
	{
		mInFlightPackets.emplace_back( sequenceNumber, inRepMgr, inNetworkMgr );

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
	float timeoutTime = RealTimeSrvTiming::sInstance.GetCurrentGameTime() - kDelayBeforeAckTimeout;

	while ( !mInFlightPackets.empty() )
	{
		const auto& nextInFlightPacket = mInFlightPackets.front();

		if ( nextInFlightPacket.GetTimeDispatched() < timeoutTime )
		{
			HandlePacketDeliveryFailure( nextInFlightPacket );
			mInFlightPackets.pop_front();
		}
		else
		{
			break;
		}
	}
}

void DeliveryNotifyMgr::HandlePacketDeliveryFailure( const InFlightPacket& inFlightPacket )
{
	++mDroppedPacketCount;
	inFlightPacket.HandleDeliveryFailure( this );
}

void DeliveryNotifyMgr::HandlePacketDeliverySuccess( const InFlightPacket& inFlightPacket )
{
	++mDeliveredPacketCount;
	inFlightPacket.HandleDeliverySuccess( this );
}

void DeliveryNotifyMgr::ProcessAckBitField( InputBitStream& inInputStream )
{
	AckBitField recvedAckBitField;
	recvedAckBitField.Read( inInputStream );

	PacketSN nextAckedSN =
		recvedAckBitField.GetLatestAckSN() - ( ACK_BIT_FIELD_BYTE_LEN << 3 );

	PacketSN LastAckedSN = recvedAckBitField.GetLatestAckSN();


	while (
		RealTimeSrvHelper::SequenceGreaterThanOrEqual( LastAckedSN, nextAckedSN )
		//LastAckedSN >= nextAckdSequenceNumber
		&& !mInFlightPackets.empty()
		)
	{
		const auto& nextInFlightPacket = mInFlightPackets.front();
		PacketSN nextInFlightPacketSN = nextInFlightPacket.GetSequenceNumber();

		if ( RealTimeSrvHelper::SequenceGreaterThan( nextAckedSN, nextInFlightPacketSN ) )
		//if ( nextAckedSN > nextInFlightPacketSN )
		{
			auto copyOfInFlightPacket = nextInFlightPacket;
			mInFlightPackets.pop_front();
			HandlePacketDeliveryFailure( copyOfInFlightPacket );
		}
		else if ( nextAckedSN == nextInFlightPacketSN )
		{
			if ( nextAckedSN == LastAckedSN
				|| recvedAckBitField.IsSetCorrespondingAckBit( nextAckedSN ) )
			{
				HandlePacketDeliverySuccess( nextInFlightPacket );
				mInFlightPackets.pop_front();
				++nextAckedSN;
			}
			else
			{
				auto copyOfInFlightPacket = nextInFlightPacket;
				mInFlightPackets.pop_front();
				HandlePacketDeliveryFailure( copyOfInFlightPacket );
				++nextAckedSN;
			}
		}
		else if ( RealTimeSrvHelper::SequenceGreaterThan( nextInFlightPacketSN, nextAckedSN ) )
		//else if ( nextAckedSN < nextInFlightPacketSN )
		{
			nextAckedSN = nextInFlightPacketSN;
		}
	}
}