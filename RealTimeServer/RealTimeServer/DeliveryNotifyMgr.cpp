#include "RealTimeSrvPCH.h"

namespace
{
	const float kDelayBeforeAckTimeout = 0.7f;
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
			//AddPendingAck( sequenceNumber );
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
			LOG( "DeliveryNotifyMgr::ProcessTimedOutPackets", 0 );
			LOG( "nextInFlightPacket.GetSequenceNumber() = %d", nextInFlightPacket.GetSequenceNumber() );

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

	PacketSN nextAckdSequenceNumber =
		recvedAckBitField.GetLatestAckSN() - ( ACK_BIT_FIELD_BYTE_LEN << 3 );

	uint32_t LastAckdSequenceNumber = recvedAckBitField.GetLatestAckSN();


	while (
		RealTimeSrvHelper::SequenceGreaterThanOrEqual( LastAckdSequenceNumber, nextAckdSequenceNumber )
		//LastAckdSequenceNumber >= nextAckdSequenceNumber
		&& !mInFlightPackets.empty()
		)
	{
		const auto& nextInFlightPacket = mInFlightPackets.front();
		PacketSN nextInFlightPacketSequenceNumber = nextInFlightPacket.GetSequenceNumber();

		if ( RealTimeSrvHelper::SequenceGreaterThan( nextAckdSequenceNumber, nextInFlightPacketSequenceNumber ) )
		{
			auto copyOfInFlightPacket = nextInFlightPacket;
			mInFlightPackets.pop_front();
			HandlePacketDeliveryFailure( copyOfInFlightPacket );
			LOG( "DeliveryNotifyMgr::RealTimeSrvHelper::SequenceGreaterThan( nextAckdSequenceNumber, nextInFlightPacketSequenceNumber )", 0 );
		}
		else if ( nextAckdSequenceNumber == nextInFlightPacketSequenceNumber )
		{
			if ( recvedAckBitField.IsSetCorrespondingAckBit( nextAckdSequenceNumber ) )
			{
				HandlePacketDeliverySuccess( nextInFlightPacket );
				mInFlightPackets.pop_front();
				++nextAckdSequenceNumber;
				LOG( "DeliveryNotifyMgr::IsSetCorrespondingAckBit", 0 );
			}
			else
			{
				auto copyOfInFlightPacket = nextInFlightPacket;
				mInFlightPackets.pop_front();
				HandlePacketDeliveryFailure( copyOfInFlightPacket );
				++nextAckdSequenceNumber;

				LOG( "DeliveryNotifyMgr::nnnIsSetCorrespondingAckBit", 0 );
			}
		}
		else if ( RealTimeSrvHelper::SequenceGreaterThan( nextInFlightPacketSequenceNumber, nextAckdSequenceNumber ) )
		{
			nextAckdSequenceNumber = nextInFlightPacketSequenceNumber;
			LOG( "GreaterThan( nextInFlightPacketSequenceNumber, nextAc", 0 );
		}


		LOG( "LastAckdSequenceNumber = %d", LastAckdSequenceNumber );
		LOG( "nextAckdSequenceNumber = %d", nextAckdSequenceNumber );

	}
}