#include "realtime_srv/common/RealtimeSrvShared.h"
#include <time.h>



using namespace realtime_srv;


namespace
{
const float kTimeBetweenStatePackets = 0.033f;
const float kClientDisconnectTimeout = 6.f;
}



#ifdef IS_LINUX

using namespace muduo;
using namespace muduo::net;

AtomicInt32 NetworkMgr::kNewNetId;

NetworkMgr::NetworkMgr() :
	udpConnToClientMap_( new UdpConnToClientMap )
{
	kNewNetId.getAndSet( 1 );
}

void NetworkMgr::SendPacket( const OutputBitStream& inOutputStream, const UdpConnectionPtr& conn )
{
	conn->send(
		inOutputStream.GetBufferPtr(), inOutputStream.GetByteLength() );
}

void NetworkMgr::Start()
{
	server_->start();
	loop_.loop();
}

void NetworkMgr::onMessage( const muduo::net::UdpConnectionPtr& conn,
	muduo::net::Buffer* buf,
	muduo::Timestamp receiveTime )
{
	if ( buf->readableBytes() > 0 ) // kHeaderLen == 0
	{
		InputBitStream inputStream( buf->peek(), buf->readableBytes() * 8 );
		buf->retrieveAll();

		ProcessPacket( inputStream, conn );
		//CheckForDisconnects();
		worldUpdateCB_();
		//SendOutgoingPackets();
	}
}

bool NetworkMgr::Init( uint16_t inPort )
{
	InetAddress serverAddr( inPort );
	server_.reset( new UdpServer( &loop_, serverAddr, "realtime_srv" ) );

	server_->setConnectionCallback(
		std::bind( &NetworkMgr::onConnection, this, _1 ) );
	server_->setMessageCallback(
		std::bind( &NetworkMgr::onMessage, this, _1, _2, _3 ) );
	server_->setThreadNum( THREAD_NUM );

	loop_.runEvery( static_cast< double >( kClientDisconnectTimeout ),
		std::bind( &NetworkMgr::CheckForDisconnects, this ) );
	loop_.runEvery( static_cast< double >( kTimeBetweenStatePackets ),
		std::bind( &NetworkMgr::SendOutgoingPackets, this ) );

	return true;
}

void NetworkMgr::onConnection( const UdpConnectionPtr& conn )
{
	//NetworkMgr::onConnection( conn );
	LOG_INFO << conn->localAddress().toIpPort() << " -> "
		<< conn->peerAddress().toIpPort() << " is "
		<< ( conn->connected() ? "UP" : "DOWN" );
	if ( !conn->connected() )
	{
		MutexLockGuard lock( mutex_ );
		THREAD_SHARED_VAR_COW( udpConnToClientMap_ );

		LOG( "Player %d disconnect", ( ( *udpConnToClientMap_ )[conn] )->GetPlayerId() );

		udpConnToClientMap_->erase( conn );
	}
}

void NetworkMgr::ProcessPacket( InputBitStream& inInputStream,
	const UdpConnectionPtr& inUdpConnetction )
{
	UdpConnToClientMapPtr tempUdpConnToClientMap = GET_THREAD_SHARED_VAR( udpConnToClientMap_ );
	auto it = tempUdpConnToClientMap->find( inUdpConnetction );
	if ( it == tempUdpConnToClientMap->end() )
	{
		HandlePacketFromNewClient( inInputStream, inUdpConnetction );
	}
	else
	{
		DoProcessPacket( ( *it ).second, inInputStream );
	}
}

void NetworkMgr::HandlePacketFromNewClient( InputBitStream& inInputStream,
	const UdpConnectionPtr& inUdpConnetction )
{
	uint32_t	packetType;
	inInputStream.Read( packetType );
	if ( packetType == kHelloCC
		|| packetType == kInputCC
		|| packetType == kResetedCC )
	{
		std::string playerName = "realtime_srv_test_player_name";
		//inInputStream.Read( playerName );	

		ClientProxyPtr newClientProxy =
			std::make_shared< ClientProxy >(
				this,
				playerName,
				kNewNetId.getAndAdd( 1 ),
				inUdpConnetction );

		{
			MutexLockGuard lock( mutex_ );
			THREAD_SHARED_VAR_COW( udpConnToClientMap_ );
			( *udpConnToClientMap_ )[inUdpConnetction] = newClientProxy;
		}

		GameObjPtr newGameObj = newPlayerCB_( newClientProxy );
		newGameObj->SetClientProxy( newClientProxy );
		worldRegistryCB_( newGameObj, RA_Create );

		if ( packetType == kHelloCC )
		{
			SendGamePacket( newClientProxy, kWelcomeCC );
		}
		else
		{
	 // Server reset
			newClientProxy->SetRecvingServerResetFlag( true );
			SendGamePacket( newClientProxy, kResetCC );
			LOG( "SendResetPacket" );
		}

		LOG( "a new client named '%s' as PlayerID %d ",
			newClientProxy->GetPlayerName().c_str(),
			newClientProxy->GetPlayerId() );
	}
	//else if ( packetType == kInputCC )
	//{
	//	// Server reset
	//	SendResetPacket( inFromAddress );
	//}
	else
	{
		LOG_INFO << "Bad incoming packet from unknown client at socket "
			<< inUdpConnetction->peerAddress().toIpPort() << " is "
			<< " - we're under attack!!";
	}
}

void NetworkMgr::NotifyAllClient( GameObjPtr inGameObject, ReplicationAction inAction )
{
	UdpConnToClientMapPtr tempUdpConnToClientMap =
		GET_THREAD_SHARED_VAR( udpConnToClientMap_ );
	for ( const auto& pair : *tempUdpConnToClientMap )
	{
		switch ( inAction )
		{
			case RA_Create:
				pair.second->GetReplicationManager().ReplicateCreate(
					inGameObject->GetObjId(), inGameObject->GetAllStateMask() );
				break;
			case RA_Destroy:
				pair.second->GetReplicationManager().ReplicateDestroy(
					inGameObject->GetObjId() );
				break;
			default:
				break;
		}
	}
}

void NetworkMgr::CheckForDisconnects()
{
	float minAllowedTime =
		RealtimeSrvTiming::sInstance.GetCurrentGameTime() - kClientDisconnectTimeout;

	vector< ClientProxyPtr > clientsToDisconnect;

	auto tempUdpConnToClientMap = GET_THREAD_SHARED_VAR( udpConnToClientMap_ );
	for ( const auto& pair : *tempUdpConnToClientMap )
	{
		if ( pair.second->GetLastPacketFromClientTime() < minAllowedTime )
		{
//LOG( "Player %d disconnect", pair.second->GetPlayerId() );
			clientsToDisconnect.push_back( pair.second );
		}
	}
	if ( clientsToDisconnect.size() > 0 )
	{
		for ( auto cliToDC : clientsToDisconnect )
		{
			cliToDC->GetUdpConnection()->forceClose(); // -> NetworkMgr::onConnection
		}
	}
}

void NetworkMgr::SendOutgoingPackets()
{
	UdpConnToClientMapPtr tempUdpConnToClientMap =
		GET_THREAD_SHARED_VAR( udpConnToClientMap_ );
	for ( const auto& pair : *tempUdpConnToClientMap )
	{
		( pair.second )->GetDeliveryNotifyManager().ProcessTimedOutPackets();

		if ( ( pair.second )->IsLastMoveTimestampDirty() )
		{
			SendGamePacket( ( pair.second ), kStateCC );
		}
	}
}

void NetworkMgr::SetRepStateDirty( int inNetworkId, uint32_t inDirtyState )
{
	UdpConnToClientMapPtr tempUdpConnToClientMap =
		GET_THREAD_SHARED_VAR( udpConnToClientMap_ );
	for ( const auto& pair : *tempUdpConnToClientMap )
	{
		pair.second->GetReplicationManager().SetReplicationStateDirty(
			inNetworkId, inDirtyState );
	}
}

#else

int NetworkMgr::kNewNetId = 1;

NetworkMgr::NetworkMgr() :
	mTimeOfLastStatePacket( 0.f ),
	mLastCheckDCTime( 0.f ),
	mDropPacketChance( 0.f ),
	mSimulatedLatency( 0.f ),
	mWhetherToSimulateJitter( false )
{}

void NetworkMgr::Start()
{
	while ( true )
	{
		ReadIncomingPacketsIntoQueue();
		ProcessQueuedPackets();
		CheckForDisconnects();
		worldUpdateCB_();
		SendOutgoingPackets();
	}
}

void NetworkMgr::ReadIncomingPacketsIntoQueue()
{
	char packetMem[MAX_PACKET_BYTE_LENGTH];
	int packetSize = sizeof( packetMem );
	SockAddrInterf fromAddress;

	int receivedPackedCount = 0;
	int totalReadByteCount = 0;

	while ( receivedPackedCount < kMaxPacketsPerFrameCount )
	{
		int readByteCount = mSocket->ReceiveFrom( packetMem, packetSize, fromAddress );
		if ( readByteCount == 0 )
		{
//LOG( "ReadIncomingPacketsIntoQueue readByteCount = %d " );
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

			if ( RealtimeSrvMath::GetRandomFloat() >= mDropPacketChance )
			{
				float simulatedReceivedTime =
					RealtimeSrvTiming::sInstance.GetCurrentGameTime() +
					mSimulatedLatency +
					( GetIsSimulatedJitter() ?
						RealtimeSrvMath::Clamp( RealtimeSrvMath::GetRandomFloat(), 0.f, 0.06f ) : 0.f );
				mPacketQueue.emplace( simulatedReceivedTime, inputStream, fromAddress );
			}
			else
			{
			 //LOG( "Dropped packet!" );
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
		if ( RealtimeSrvTiming::sInstance.GetCurrentGameTime() > nextPacket.GetReceivedTime() )
		{
			ProcessPacket(
				nextPacket.GetPacketBuffer(),
				nextPacket.GetFromAddress(),
				nextPacket.GetUDPSocket()
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
	UdpSockInterf::StaticInit();

	mSocket = UdpSockInterf::CreateUDPSocket();
	if ( mSocket == nullptr )
	{
		return false;
	}
	SockAddrInterf ownAddress( INADDR_ANY, inPort );
	mSocket->Bind( ownAddress );
	if ( mSocket->SetNonBlockingMode( true ) != NO_ERROR )
	{
		return false;
	}

	LOG( "Initializing NetworkManager at port %d", inPort );
	return true;
}

void NetworkMgr::SendPacket( const OutputBitStream& inOutputStream,
	const SockAddrInterf& inSockAddr )
{
	if ( RealtimeSrvMath::GetRandomFloat() < mDropPacketChance )
	{
		return;
	}
	int sentByteCount = mSocket->SendTo( inOutputStream.GetBufferPtr(),
		inOutputStream.GetByteLength(),
		inSockAddr );
}

void NetworkMgr::HandleConnectionReset( const SockAddrInterf& inFromAddress )
{
	auto it = addrToClientMap_.find( inFromAddress );
	if ( it != addrToClientMap_.end() )
	{
		addrToClientMap_.erase( it );
	}
}

void NetworkMgr::ProcessPacket(
	InputBitStream& inInputStream,
	const SockAddrInterf& inFromAddress,
	const UDPSocketPtr& inUDPSocket
)
{
	auto it = addrToClientMap_.find( inFromAddress );
	if ( it == addrToClientMap_.end() )
	{
		HandlePacketFromNewClient( inInputStream,
			inFromAddress,
			inUDPSocket
		);
	}
	else
	{
		DoProcessPacket( ( *it ).second, inInputStream );
	}
}

void NetworkMgr::HandlePacketFromNewClient( InputBitStream& inInputStream,
	const SockAddrInterf& inFromAddress,
	const UDPSocketPtr& inUDPSocket )
{
	uint32_t	packetType;
	inInputStream.Read( packetType );
	if ( packetType == kHelloCC
		|| packetType == kInputCC
		|| packetType == kResetedCC
		)
	{
		std::string playerName = "rs_test_player_name";
		//inInputStream.Read( playerName );	

		ClientProxyPtr newClientProxy = std::make_shared< ClientProxy >(
			this,
			inFromAddress,
			playerName,
			kNewNetId++,
			inUDPSocket );

		addrToClientMap_[inFromAddress] = newClientProxy;

		GameObjPtr newGameObj = newPlayerCB_( newClientProxy );
		newGameObj->SetClientProxy( newClientProxy );
		worldRegistryCB_( newGameObj, RA_Create );

		if ( packetType == kHelloCC )
		{
			SendGamePacket( newClientProxy, kWelcomeCC );
		}
		else
		{
	 // Server reset
			newClientProxy->SetRecvingServerResetFlag( true );
			SendGamePacket( newClientProxy, kResetCC );
			LOG( "SendResetPacket" );
		}
		LOG( "a new client at socket %s, named '%s' as PlayerID %d ",
			inFromAddress.ToString().c_str(),
			newClientProxy->GetPlayerName().c_str(),
			newClientProxy->GetPlayerId()
		);
	}
	//else if ( packetType == kInputCC )
	//{
	//	// Server reset
	//	SendResetPacket( inFromAddress );
	//}
	else
	{
		LOG( "Bad incoming packet from unknown client at socket %s - we're under attack!!", inFromAddress.ToString().c_str() );
	}
}


void NetworkMgr::CheckForDisconnects()
{
	float curTime = RealtimeSrvTiming::sInstance.GetCurrentGameTime();

	if ( curTime - mLastCheckDCTime < kClientDisconnectTimeout )
	{
		return;
	}
	mLastCheckDCTime = curTime;

	float minAllowedTime =
		RealtimeSrvTiming::sInstance.GetCurrentGameTime() - kClientDisconnectTimeout;
	for ( AddrToClientMap::iterator it = addrToClientMap_.begin();
		it != addrToClientMap_.end(); )
	{
		if ( it->second->GetLastPacketFromClientTime() < minAllowedTime )
		{
			LOG( "Player %d disconnect", it->second->GetPlayerId() );

			it = addrToClientMap_.erase( it );
		}
		else
		{
			++it;
		}
	}
}

void NetworkMgr::SendOutgoingPackets()
{
	float time = RealtimeSrvTiming::sInstance.GetCurrentGameTime();

	if ( time < mTimeOfLastStatePacket + kTimeBetweenStatePackets )
	{
		return;
	}
	mTimeOfLastStatePacket = time;

	for ( auto it = addrToClientMap_.begin(), end = addrToClientMap_.end(); it != end; ++it )
	{
		ClientProxyPtr clientProxy = it->second;
		clientProxy->GetDeliveryNotifyManager().ProcessTimedOutPackets();
		if ( clientProxy->IsLastMoveTimestampDirty() )
		{
//SendStatePacketToClient( clientProxy );
			SendGamePacket( clientProxy, kStateCC );
		}
	}
}

void NetworkMgr::SetRepStateDirty( int inNetworkId, uint32_t inDirtyState )
{
	for ( const auto& pair : addrToClientMap_ )
		pair.second->GetReplicationManager().SetReplicationStateDirty(
			inNetworkId, inDirtyState );
}

void NetworkMgr::NotifyAllClient( GameObjPtr inGameObject, ReplicationAction inAction )
{
	for ( const auto& pair : addrToClientMap_ )
	{
		switch ( inAction )
		{
			case RA_Create:
				pair.second->GetReplicationManager().ReplicateCreate(
					inGameObject->GetObjId(), inGameObject->GetAllStateMask() );
				break;
			case RA_Destroy:
				pair.second->GetReplicationManager().ReplicateDestroy(
					inGameObject->GetObjId() );
				break;
			default:
				break;
		}
	}
}

#endif //IS_LINUX


void NetworkMgr::DoProcessPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream )
{
	inClientProxy->UpdateLastPacketTime();

	uint32_t packetType = HandleServerReset( inClientProxy, inInputStream );
	switch ( packetType )
	{
		case kHelloCC:
			SendGamePacket( inClientProxy, kWelcomeCC );
			break;
		case kInputCC:
			if ( inClientProxy->GetDeliveryNotifyManager().ReadAndProcessState( inInputStream ) )
			{
				HandleInputPacket( inClientProxy, inInputStream );
			}
			break;
		case kNullCC:
			break;
		default:
		#ifdef IS_LINUX
			LOG_INFO << "Unknown packet type received from "
				<< inClientProxy->GetUdpConnection()->peerAddress().toIpPort();
		#else
			LOG( "Unknown packet type received from %s", inClientProxy->GetSocketAddress().ToString().c_str() );
		#endif //IS_LINUX
			break;
	}
}

uint32_t NetworkMgr::HandleServerReset( ClientProxyPtr inClientProxy, InputBitStream& inInputStream )
{
	uint32_t packetType;
	inInputStream.Read( packetType );

	if ( packetType == kResetedCC )
	{
		inClientProxy->SetRecvingServerResetFlag( false );
		inInputStream.Read( packetType );
	}
	if ( inClientProxy->GetRecvingServerResetFlag() == true )
	{
		SendGamePacket( inClientProxy, kResetCC );
		return kNullCC;
	}
	else
	{
		return packetType;
	}
}

void NetworkMgr::HandleInputPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream )
{
	uint32_t moveCount = 0;
	Action move;
	inInputStream.Read( moveCount, MOVE_COUNT_NUM );

	for ( ; moveCount > 0; --moveCount )
	{
		if ( move.Read( inInputStream ) )
		{
			if ( inClientProxy->GetUnprocessedActionList().AddMoveIfNew( move ) )
			{
				inClientProxy->SetIsLastMoveTimestampDirty( true );
			}
		}
	}
}

void NetworkMgr::SendGamePacket( ClientProxyPtr inClientProxy, const uint32_t inConnFlag )
{
	OutputBitStream outputPacket;
	outputPacket.Write( inConnFlag );

	InFlightPacket* ifp = inClientProxy->GetDeliveryNotifyManager()
		.WriteState( outputPacket, inClientProxy.get() );

	switch ( inConnFlag )
	{
		case kResetCC:
		case kWelcomeCC:
			outputPacket.Write( inClientProxy->GetPlayerId() );
			outputPacket.Write( kTimeBetweenStatePackets );
			break;
		case kStateCC:
			WriteLastMoveTimestampIfDirty( outputPacket, inClientProxy );
		default:
			break;
	}
	inClientProxy->GetReplicationManager().Write( outputPacket, ifp );

#ifdef IS_LINUX
	SendPacket( outputPacket, inClientProxy->GetUdpConnection() );
#else
	SendPacket( outputPacket, inClientProxy->GetSocketAddress() );
#endif //IS_LINUX
}

void NetworkMgr::WriteLastMoveTimestampIfDirty( OutputBitStream& inOutputStream, ClientProxyPtr inClientProxy )
{
	bool isTimestampDirty = inClientProxy->IsLastMoveTimestampDirty();
	inOutputStream.Write( isTimestampDirty );
	if ( isTimestampDirty )
	{
		inOutputStream.Write( inClientProxy->GetUnprocessedActionList().GetLastMoveTimestamp() );
		inClientProxy->SetIsLastMoveTimestampDirty( false );
	}
}
