#include "RealTimeSrvPCH.h"




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
	TransmissionDataHandler* inTransmissionDataHandler )
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

#ifdef DEPRECATED_EPOLL_INTERFACE
void NetworkMgr::WaitForIncomingPackets()
{
	EpollInterface::sInst->Wait( -1.f );
}

void NetworkMgr::RecvIncomingPacketsIntoQueue( UDPSocketPtr inUDPSocketPtr, SocketAddrInterface infromAddress )
{
	char packetMem[MAX_PACKET_BYTE_LENGTH];
	int packetSize = sizeof( packetMem );

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
			InputBitStream inputStream( packetMem, readByteCount * 8 );
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


#endif //DEPRECATED_EPOLL_INTERFACE


#ifdef NEW_EPOLL_INTERFACE

NetworkMgr::NetworkMgr() :
	mDropPacketChance( 0.f ),
	mSimulatedLatency( 0.f ),
	mWhetherToSimulateJitter( false ),
	mChunkPacketID( 1 ),
	mUdpConnToClientMap( new UdpConnToClientMap ),
	mNetworkIdToGameObjectMap( new IntToGameObjectMap ),
	mPlayerIdToClientMap( new IntToClientMap )
{}

void NetworkMgr::SendPacket( const OutputBitStream& inOutputStream, ClientProxyPtr inClientProxy )
{
	if ( RealTimeSrvMath::GetRandomFloat() < mDropPacketChance )
	{
		return;
	}

	inClientProxy->GetUdpConnection()->send( 
		inOutputStream.GetBufferPtr(), inOutputStream.GetByteLength() );
}

void NetworkMgr::Start()
{
	server_->start();
	loop_.loop();
}

void NetworkMgr::threadInit( EventLoop* loop )
{
	assert( mPacketQueue::pointer() == NULL );
	mPacketQueue::instance();
	assert( mPacketQueue::pointer() != NULL );

	MutexLockGuard lock( mutex_ );
	loops_.insert( loop );
}

UdpConnToClientMapPtr NetworkMgr::GetUdpConnToClientMap()
{
	MutexLockGuard lock( mutex_ );
	return mUdpConnToClientMap;
}

void NetworkMgr::UdpConnToClientMapCOW()
{
	if ( !mUdpConnToClientMap.unique() )
	{
		mUdpConnToClientMap.reset( new UdpConnToClientMap( *mUdpConnToClientMap ) );
	}
	assert( mUdpConnToClientMap.unique() );
}

IntToGameObjectMapPtr NetworkMgr::GetNetworkIdToGameObjectMap()
{
	MutexLockGuard lock( mutex_ );
	return mNetworkIdToGameObjectMap;
}

void NetworkMgr::NetworkIdToGameObjectMapCOW()
{
	if ( !mNetworkIdToGameObjectMap.unique() )
	{
		mNetworkIdToGameObjectMap.reset( new IntToGameObjectMap( *mNetworkIdToGameObjectMap ) );
	}
	assert( mNetworkIdToGameObjectMap.unique() );
}

PlayerIdToClientMapPtr NetworkMgr::GetPlayerIdToClientMap()
{
	MutexLockGuard lock( mutex_ );
	return mPlayerIdToClientMap;
}

void NetworkMgr::PlayerIdToClientMapCOW()
{
	if ( !mPlayerIdToClientMap.unique() )
	{
		mPlayerIdToClientMap.reset( new IntToClientMap( *mPlayerIdToClientMap ) );
	}
	assert( mPlayerIdToClientMap.unique() );
}

void NetworkMgr::onConnection( const UdpConnectionPtr& conn )
{
	LOG_INFO << conn->localAddress().toIpPort() << " -> "
		<< conn->peerAddress().toIpPort() << " is "
		<< ( conn->connected() ? "UP" : "DOWN" );

	if ( !conn->connected() )
	{
		MutexLockGuard lock( mutex_ );
		UdpConnToClientMapCOW();
		PlayerIdToClientMapCOW();

		LOG( "Player %d disconnect", ( ( *mUdpConnToClientMap )[conn] )->GetPlayerId() );

		mPlayerIdToClientMap->erase( ( ( *mUdpConnToClientMap )[conn] )->GetPlayerId() );
		mUdpConnToClientMap->erase( conn );
	}
}

void NetworkMgr::onMessage( const muduo::net::UdpConnectionPtr& conn,
	muduo::net::Buffer* buf,
	muduo::Timestamp receiveTime )
{
	if ( buf->readableBytes() > 0 ) // kHeaderLen == 0
	{
		InputBitStream inputStream( buf->peek(), buf->readableBytes() * 8 );
		buf->retrieveAll();

		if ( RealTimeSrvMath::GetRandomFloat() >= mDropPacketChance )
		{
			float simulatedReceivedTime =
				RealTimeSrvTiming::sInstance.GetCurrentGameTime() +
				mSimulatedLatency +
				( GetIsSimulatedJitter() ?
					RealTimeSrvMath::Clamp( RealTimeSrvMath::GetRandomFloat(), 0.f, 0.06f ) : 0.f );

			mPacketQueue::instance().emplace(
				simulatedReceivedTime,
				inputStream,
				conn
			);
		}
		else
		{
			//LOG( "Dropped packet!", 0 );
		}
	}

	ProcessQueuedPackets();
	CheckForDisconnects();
	WorldUpdateCB_();
	SendOutgoingPackets();
}

void NetworkMgr::ProcessQueuedPackets()
{
	while ( !mPacketQueue::instance().empty() )
	{
		ReceivedPacket& nextPacket = mPacketQueue::instance().front();
		if ( RealTimeSrvTiming::sInstance.GetCurrentGameTime() > nextPacket.GetReceivedTime() )
		{
			ProcessPacket(
				nextPacket.GetPacketBuffer(),
				nextPacket.GetUdpConnection()
			);
			mPacketQueue::instance().pop();
		}
		else
		{
			break;
		}
	}
}

bool NetworkMgr::Init( uint16_t inPort )
{
	InetAddress serverAddr( inPort );
	server_.reset( new UdpServer( &loop_, serverAddr, "RealTimeServer" ) );

	server_->setConnectionCallback(
		std::bind( &NetworkMgr::onConnection, this, _1 ) );
	server_->setMessageCallback(
		std::bind( &NetworkMgr::onMessage, this, _1, _2, _3 ) );

	server_->setThreadNum( THREAD_NUM );
	server_->setThreadInitCallback( std::bind( &NetworkMgr::threadInit, this, _1 ) );
}

#else //NEW_EPOLL_INTERFACE

NetworkMgr::NetworkMgr() :
	mDropPacketChance( 0.f ),
	mSimulatedLatency( 0.f ),
	mWhetherToSimulateJitter( false ),
	mChunkPacketID( 1 )
{}

void NetworkMgr::ProcessIncomingPackets()
{
#ifdef DEPRECATED_EPOLL_INTERFACE
	WaitForIncomingPackets();
#else
	ReadIncomingPacketsIntoQueue();
#endif

	ProcessQueuedPackets();

	//UpdateBytesSentLastFrame();
}

void NetworkMgr::ReadIncomingPacketsIntoQueue()
{
	char packetMem[MAX_PACKET_BYTE_LENGTH];
	int packetSize = sizeof( packetMem );
	SocketAddrInterface fromAddress;

	int receivedPackedCount = 0;
	int totalReadByteCount = 0;

	while ( receivedPackedCount < kMaxPacketsPerFrameCount )
	{
		int readByteCount = mSocket->ReceiveFrom( packetMem, packetSize, fromAddress );
		if ( readByteCount == 0 )
		{
			//LOG( "ReadIncomingPacketsIntoQueue readByteCount = %d ", 0 );
			break;
		}
		else if ( readByteCount == -WSAECONNRESET )
		{
			HandleConnectionReset( fromAddress );
		}
		else if ( readByteCount > 0 )
		{
			InputBitStream inputStream( packetMem, readByteCount * 8 );
			++receivedPackedCount;
			totalReadByteCount += readByteCount;

			if ( RealTimeSrvMath::GetRandomFloat() >= mDropPacketChance )
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

	if ( totalReadByteCount > 0 )
	{
		//mBytesReceivedPerSecond.UpdatePerSecond( static_cast< float >( totalReadByteCount ) );
	}
}

void NetworkMgr::ProcessQueuedPackets()
{
	while ( !mPacketQueue.empty() )
	{
		ReceivedPacket& nextPacket = mPacketQueue.front();
		if ( RealTimeSrvTiming::sInstance.GetCurrentGameTime() > nextPacket.GetReceivedTime() )
		{
			ProcessPacket(
				nextPacket.GetPacketBuffer(),
				nextPacket.GetFromAddress(),
				nextPacket.GetUDPSocket()
				// , 
				// nextPacket.GetUdpConnection() 
			);
			mPacketQueue.pop();
		}
		else
		{
			break;
		}
	}
}

bool NetworkMgr::Init( uint16_t inPort )
{
#if _WIN32
	UDPSocketInterface::StaticInit();
#endif //_WIN32

	mSocket = UDPSocketInterface::CreateUDPSocket();

	if ( mSocket == nullptr )
	{
		return false;
	}

#ifdef DEPRECATED_EPOLL_INTERFACE
	if ( mSocket->SetReUse() )
	{
		return false;
	}
#endif //DEPRECATED_EPOLL_INTERFACE

	SocketAddrInterface ownAddress( INADDR_ANY, inPort );
	mSocket->Bind( ownAddress );

	LOG( "Initializing NetworkManager at port %d", inPort );

	//mBytesReceivedPerSecond = WeightedTimedMovingAverage(1.f);
	//mBytesSentPerSecond = WeightedTimedMovingAverage(1.f);

	if ( mSocket->SetNonBlockingMode( true ) != NO_ERROR )
	{
		return false;
	}

#ifdef DEPRECATED_EPOLL_INTERFACE
	EpollInterface::StaticInit();
	if ( !EpollInterface::sInst->Add( mSocket->GetSocket() ) )
	{
		return false;
	}
	EpollInterface::sInst->SetListener( mSocket, ownAddress );
#endif //DEPRECATED_EPOLL_INTERFACE

	return true;
}

void NetworkMgr::SendPacket( const OutputBitStream& inOutputStream, ClientProxyPtr inClientProxy )
{
	if ( RealTimeSrvMath::GetRandomFloat() < mDropPacketChance )
	{
		return;
	}

	int sentByteCount = mSocket->SendTo( inOutputStream.GetBufferPtr(), inOutputStream.GetByteLength(), inClientProxy->GetSocketAddress() );
	if ( sentByteCount > 0 )
	{
		mBytesSentThisFrame += sentByteCount;
	}
}

void NetworkMgr::HandleConnectionReset( const SocketAddrInterface& inFromAddress )
{
	auto it = mAddressToClientMap.find( inFromAddress );
	if ( it != mAddressToClientMap.end() )
	{
		mPlayerIdToClientMap.erase( it->second->GetPlayerId() );

#ifdef DEPRECATED_EPOLL_INTERFACE
		EpollInterface::sInst->CloseSocket( it->second->GetUDPSocket()->GetSocket() );
#endif

		mAddressToClientMap.erase( it );
	}
}
#endif //NEW_EPOLL_INTERFACE