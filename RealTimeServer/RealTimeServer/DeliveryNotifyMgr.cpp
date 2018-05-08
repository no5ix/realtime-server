#include "RealTimeSrvPCH.h"

namespace
{
	const float kDelayBeforeAckTimeout = 2.5f;
}

DeliveryNotifyMgr::DeliveryNotifyMgr( bool inShouldSendAcks, bool inShouldProcessAcks ) :
mNextOutgoingSequenceNumber( 0 ),
mNextExpectedSequenceNumber( 0 ),
//everybody starts at 0...
mShouldSendAcks( inShouldSendAcks ),
mShouldProcessAcks( inShouldProcessAcks ),
mDeliveredPacketCount( 0 ),
mDroppedPacketCount( 0 ),
mDispatchedPacketCount( 0 )
{
}



DeliveryNotifyMgr::~DeliveryNotifyMgr()
{
	//LOG( "DNM destructor. Delivery rate %d%%, Drop rate %d%%",
	//	( 100 * mDeliveredPacketCount ) / mDispatchedPacketCount,
	//	( 100 * mDroppedPacketCount ) / mDispatchedPacketCount );
}



InFlightPacket* DeliveryNotifyMgr::WriteSequenceNumber( OutputBitStream& inOutputStream )
{
	
	PacketSequenceNumber sequenceNumber = mNextOutgoingSequenceNumber++;
	inOutputStream.Write( sequenceNumber );

	++mDispatchedPacketCount;

	if( mShouldProcessAcks )
	{
		mInFlightPackets.emplace_back( sequenceNumber );

		return &mInFlightPackets.back();
	}
	else
	{
		return nullptr;
	}
}

void DeliveryNotifyMgr::WriteAckData( OutputBitStream& inOutputStream )
{
	bool hasAcks = ( mPendingAcks.size() > 0 );

	inOutputStream.Write( hasAcks );
	if( hasAcks )
	{
		
		mPendingAcks.front().Write( inOutputStream );
		mPendingAcks.pop_front();
	}
}




bool DeliveryNotifyMgr::ProcessSequenceNumber( InputBitStream& inInputStream )
{
	PacketSequenceNumber	sequenceNumber;

	inInputStream.Read( sequenceNumber );
	if( sequenceNumber == mNextExpectedSequenceNumber )
	{
		mNextExpectedSequenceNumber = sequenceNumber + 1;
		
		if( mShouldSendAcks )
		{
			AddPendingAck( sequenceNumber );
		}
		
		return true;
	}
	
	
	
	else if( sequenceNumber < mNextExpectedSequenceNumber )
	{
		return false;
	}
	else if( sequenceNumber > mNextExpectedSequenceNumber )
	{
		
		
		mNextExpectedSequenceNumber = sequenceNumber + 1;
		
		
		
		if( mShouldSendAcks )
		{
			AddPendingAck( sequenceNumber );
		}
		return true;
	}

	
	return false;
}




void DeliveryNotifyMgr::ProcessAcks( InputBitStream& inInputStream )
{

	bool hasAcks;
	inInputStream.Read( hasAcks );
	if( hasAcks )
	{
		AckRange ackRange;
		ackRange.Read( inInputStream );

		
		PacketSequenceNumber nextAckdSequenceNumber = ackRange.GetStart();
		uint32_t onePastAckdSequenceNumber = nextAckdSequenceNumber + ackRange.GetCount();
		while( nextAckdSequenceNumber < onePastAckdSequenceNumber && !mInFlightPackets.empty() )
		{
			const auto& nextInFlightPacket = mInFlightPackets.front();
			
			PacketSequenceNumber nextInFlightPacketSequenceNumber = nextInFlightPacket.GetSequenceNumber();
			if( nextInFlightPacketSequenceNumber < nextAckdSequenceNumber )
			{
				
				auto copyOfInFlightPacket = nextInFlightPacket;
				mInFlightPackets.pop_front();
				HandlePacketDeliveryFailure( copyOfInFlightPacket );
			}
			else if( nextInFlightPacketSequenceNumber == nextAckdSequenceNumber )
			{
				HandlePacketDeliverySuccess( nextInFlightPacket );
				
				mInFlightPackets.pop_front();
				
				++nextAckdSequenceNumber;
			}
			else if( nextInFlightPacketSequenceNumber > nextAckdSequenceNumber )
			{
				
				
				++nextAckdSequenceNumber;
			}
		}
	}
}

void DeliveryNotifyMgr::ProcessTimedOutPackets()
{
	float timeoutTime = RealTimeSrvTiming::sInstance.GetCurrentGameTime() - kDelayBeforeAckTimeout;

	while( !mInFlightPackets.empty() )
	{
		const auto& nextInFlightPacket = mInFlightPackets.front();

		
		if( nextInFlightPacket.GetTimeDispatched() < timeoutTime )
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

void DeliveryNotifyMgr::AddPendingAck( PacketSequenceNumber inSequenceNumber )
{
	
	
	if( mPendingAcks.size() == 0 || !mPendingAcks.back().ExtendIfShould( inSequenceNumber ) )
	{
		mPendingAcks.emplace_back( inSequenceNumber );
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
