#include "RealTimeSrvPCH.h"



NetworkMgr::NetworkMgr() :
	mDropPacketChance( 0.f ),
	mSimulatedLatency( 0.f ),
	mIsSimulatedJitter( false ),
	mChunkPacketID( 1 )
{

}

NetworkMgr::~NetworkMgr()
{

}

bool NetworkMgr::Init(uint16_t inPort)
{
#if _WIN32
	UDPSocketInterface::StaticInit(); 
#endif

	mSocket = UDPSocketInterface::CreateUDPSocket();

	//did we bind okay?
	if ( mSocket == nullptr )
	{
		return false;
	}

#ifdef HAS_EPOLL
	if ( mSocket->SetReUse())
	{
		return false;
	}
#endif

	SocketAddrInterface ownAddress(INADDR_ANY, inPort);
	mSocket->Bind(ownAddress);

	LOG("Initializing NetworkManager at port %d", inPort);

	//mBytesReceivedPerSecond = WeightedTimedMovingAverage(1.f);
	//mBytesSentPerSecond = WeightedTimedMovingAverage(1.f);


	if (mSocket->SetNonBlockingMode(true) != NO_ERROR)
	{
		return false;
	}

#ifdef HAS_EPOLL
	EpollInterface::StaticInit();
	if ( !EpollInterface::sInst->Add( mSocket->GetSocket() ) )
	{
		return false;
	}
	EpollInterface::sInst->SetListener( mSocket, ownAddress );
#endif

	return true;

}


void NetworkMgr::ProcessIncomingPackets()
{
#ifdef HAS_EPOLL
	WaitForIncomingPackets();
#else
	ReadIncomingPacketsIntoQueue();
#endif

	ProcessQueuedPackets();

	//UpdateBytesSentLastFrame();

}

void NetworkMgr::ProcessQueuedPackets()
{
	//look at the front packet...
	while ( !mPacketQueue.empty() )
	{
		ReceivedPacket& nextPacket = mPacketQueue.front();
		if ( RealTimeSrvTiming::sInstance.GetCurrentGameTime() > nextPacket.GetReceivedTime() )
		{
			ProcessPacket( nextPacket.GetPacketBuffer(), nextPacket.GetFromAddress(), nextPacket.GetUDPSocket() );
			mPacketQueue.pop();
		}
		else
		{
			break;
		}

	}
}

// not complete, deprecated.
void NetworkMgr::RecombineSlicesToChunk( InputBitStream& refInputStream )
{
	//uint8_t slicedPacketIndex;
	//uint8_t slicedPacketCount;
	//refInputStream.Read( slicedPacketCount );
	//refInputStream.Read( slicedPacketIndex );

	//if ( slicedPacketIndex == mNextExpectedSlicedPacketIndex )
	//{
	//	++mNextExpectedSlicedPacketIndex;

	//	if ( !mIsReceivingSlicePacket )
	//	{
	//		mIsReceivingSlicePacket = true;

	//		mChunkInputStream.Reinit( slicedPacketCount * MAX_PACKET_BYTE_LENGTH * 8 );

	//		refInputStream.RecombineTo( mChunkInputStream );
	//	}
	//	else
	//	{
	//		refInputStream.RecombineTo( mChunkInputStream );
	//	}
	//}

	//if ( mNextExpectedSlicedPacketIndex == slicedPacketCount )
	//{
	//	mIsReceivingSlicePacket = false;
	//	mNextExpectedSlicedPacketIndex = 0;
	//	mChunkInputStream.ResetToCapacityFromBit( mChunkInputStream.GetRecombinePoint() );
	//	refInputStream = mChunkInputStream;
	//}
}

// not complete, deprecated. can not calc slicedPacketCount
void NetworkMgr::ProcessOutcomingPacket(
	OutputBitStream& inOutputStream,
	shared_ptr< ClientProxy > inClientProxy,
	TransmissionDataHandler* inTransmissionDataHandler)
{
	//bool isReachTheEnd = false;
	//uint8_t slicedPacketIndex = 0;

	//uint32_t ByteLenInOutputSteam =
	//	( ( ( inOutputStream.GetBitLength() + PACKET_SEQUENCE_NUMBER_BIT_WIDE + 1 ) + 7 ) >> 3 );

	//bool isSliced = false;
	//if ( ByteLenInOutputSteam > MAX_PACKET_BYTE_LENGTH )
	//{
	//	isSliced = true;
	//	++mChunkPacketID;
	//}

	//while ( !isReachTheEnd )
	//{
	//	OutputBitStream slicedPacket;

	//	InFlightPacket* ifp = 
	//		inClientProxy->GetDeliveryNotificationManager().WriteState( slicedPacket );

	//	slicedPacket.Write( isSliced );

	//	if ( isSliced )
	//	{
	//		slicedPacket.Write( mChunkPacketID );

	//		uint8_t slicedPacketCount = ( ByteLenInOutputSteam + 1023 ) >> 10;
	//		slicedPacket.Write( slicedPacketCount );
	//		slicedPacket.Write( slicedPacketIndex++ );
	//	}
	//	isReachTheEnd = inOutputStream.SliceTo( slicedPacket );
	//	//ifp->SetTransmissionData( 'RPLM', TransmissionDataPtr( inTransmissionDataHandler ) );
	//	ifp->SetTransmissionData( 'RPLM', inTransmissionDataHandler );

	//	SendPacket( slicedPacket, inClientProxy );
	//}
}

#ifdef HAS_EPOLL
void NetworkMgr::WaitForIncomingPackets()
{
	EpollInterface::sInst->Wait( -1.f );
}

void NetworkMgr::RecvIncomingPacketsIntoQueue( UDPSocketPtr inUDPSocketPtr, SocketAddrInterface infromAddress )
{
	char packetMem[MAX_PACKET_BYTE_LENGTH];
	int packetSize = sizeof( packetMem );
	InputBitStream inputStream( packetMem, packetSize * 8 );

	int receivedPackedCount = 0;
	int totalReadByteCount = 0;

	while ( receivedPackedCount < kMaxPacketsPerFrameCount )
	{
		int readByteCount = inUDPSocketPtr->Recv( packetMem, packetSize );
		if ( readByteCount == 0 )
		{
			break;
		}
		else if ( readByteCount == -WSAECONNRESET )
		{
			HandleConnectionReset( infromAddress );
		}
		else if ( readByteCount > 0 )
		{
			inputStream.ResetToCapacity( readByteCount );
			++receivedPackedCount;
			totalReadByteCount += readByteCount;

			if ( RealTimeSrvMath::GetRandomFloat() >= mDropPacketChance )
			{
				float simulatedReceivedTime =
					RealTimeSrvTiming::sInstance.GetCurrentGameTime() +
					mSimulatedLatency +
					( GetIsSimulatedJitter() ?
						RealTimeSrvMath::Clamp( RealTimeSrvMath::GetRandomFloat(), 0.f, 0.06f ) : 0.f );
				mPacketQueue.emplace( simulatedReceivedTime, inputStream, infromAddress, inUDPSocketPtr );
			}
			else
			{
				//LOG( "Dropped packet!", 0 );
			}
		}
		else
		{
		}
	}
}

void NetworkMgr::SendPacket( const OutputBitStream& inOutputStream, ClientProxyPtr inClientProxy )
{
	if ( RealTimeSrvMath::GetRandomFloat() < mDropPacketChance )
	{
		return;
	}

	int sentByteCount = inClientProxy->GetUDPSocket()->Send( inOutputStream.GetBufferPtr(), inOutputStream.GetByteLength() );
	if ( sentByteCount > 0 )
	{
		mBytesSentThisFrame += sentByteCount;
	}
}


#else

void NetworkMgr::ReadIncomingPacketsIntoQueue()
{
	char packetMem[MAX_PACKET_BYTE_LENGTH];
	int packetSize = sizeof( packetMem );
	InputBitStream inputStream( packetMem, packetSize * 8 );
	SocketAddrInterface fromAddress;

	int receivedPackedCount = 0;
	int totalReadByteCount = 0;

	while (receivedPackedCount < kMaxPacketsPerFrameCount)
	{
		int readByteCount = mSocket->ReceiveFrom( packetMem, packetSize, fromAddress );
		if (readByteCount == 0)
		{
			//LOG( "ReadIncomingPacketsIntoQueue readByteCount = %d ", 0 );
			break;
		}
		else if (readByteCount == -WSAECONNRESET)
		{
			HandleConnectionReset( fromAddress );
		}
		else if (readByteCount > 0)
		{
			inputStream.ResetToCapacity( readByteCount );
			++receivedPackedCount;
			totalReadByteCount += readByteCount;

			if (RealTimeSrvMath::GetRandomFloat() >= mDropPacketChance)
			{
				float simulatedReceivedTime =
					RealTimeSrvTiming::sInstance.GetCurrentGameTime() +
					mSimulatedLatency +  
					( GetIsSimulatedJitter() ? 
						RealTimeSrvMath::Clamp( RealTimeSrvMath::GetRandomFloat(), 0.f, 0.06f ) : 0.f );
				mPacketQueue.emplace( simulatedReceivedTime, inputStream, fromAddress );
			}
			else
			{
				//LOG( "Dropped packet!", 0 );
			}
		}
		else
		{
		}
	}

	if (totalReadByteCount > 0)
	{
		//mBytesReceivedPerSecond.UpdatePerSecond( static_cast< float >( totalReadByteCount ) );
	}
}


void NetworkMgr::SendPacket( const OutputBitStream& inOutputStream, ClientProxyPtr inClientProxy )
{
	if ( RealTimeSrvMath::GetRandomFloat() < mDropPacketChance )
	{
		return;
	}

	int sentByteCount = mSocket->SendTo( inOutputStream.GetBufferPtr(), inOutputStream.GetByteLength(), inClientProxy->GetSocketAddress() );
	if (sentByteCount > 0)
	{
		mBytesSentThisFrame += sentByteCount;
	}
}

#endif