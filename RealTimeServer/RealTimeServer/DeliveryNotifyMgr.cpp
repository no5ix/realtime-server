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

	PacketSequenceNumber sequenceNumber = mNextOutgoingSequenceNumber++;
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
	PacketSequenceNumber	sequenceNumber;

	inInputStream.Read( sequenceNumber );
	if ( RealTimeSrvHelper::SequenceGreaterThanOrEqual( sequenceNumber, mNextExpectedSequenceNumber ) )
	{
		PacketSequenceNumber lastSN = mNextExpectedSequenceNumber - 1;
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

	PacketSequenceNumber nextAckdSequenceNumber = 
		recvedAckBitField.GetLastAckSN() - ( ACK_BIT_FIELD_BYTE_LEN << 3 );

	uint32_t LastAckdSequenceNumber = recvedAckBitField.GetLastAckSN();
	while ( 
		RealTimeSrvHelper::SequenceGreaterThanOrEqual( LastAckdSequenceNumber, nextAckdSequenceNumber ) 
		&& !mInFlightPackets.empty() 
	)
	{
		const auto& nextInFlightPacket = mInFlightPackets.front();

		PacketSequenceNumber nextInFlightPacketSequenceNumber = nextInFlightPacket.GetSequenceNumber();
		if ( RealTimeSrvHelper::SequenceGreaterThan( nextAckdSequenceNumber, nextInFlightPacketSequenceNumber ) )
		{
			auto copyOfInFlightPacket = nextInFlightPacket;
			mInFlightPackets.pop_front();
			HandlePacketDeliveryFailure( copyOfInFlightPacket );
		}
		else if ( nextAckdSequenceNumber == nextInFlightPacketSequenceNumber )
		{
			HandlePacketDeliverySuccess( nextInFlightPacket );
			mInFlightPackets.pop_front();
			++nextAckdSequenceNumber;
		}
		else if ( RealTimeSrvHelper::SequenceGreaterThan( nextInFlightPacketSequenceNumber, nextAckdSequenceNumber ) )
		{
			nextAckdSequenceNumber = nextInFlightPacketSequenceNumber;
		}
	}
}