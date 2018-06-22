#include "realtime_srv/common/RealtimeSrvShared.h"


#ifdef IS_LINUX
AtomicInt32 NetworkMgr::kNewPlayerId;
#else
int NetworkMgr::kNewPlayerId = 1;
#endif //IS_LINUX

std::unique_ptr<NetworkMgr> NetworkMgr::sInst;

namespace
{
	const float kTimeBetweenStatePackets = 0.033f;
	const float kClientDisconnectTimeout = 6.f;
}


bool NetworkMgr::StaticInit( uint16_t inPort )
{
	sInst.reset( new NetworkMgr() );
	return sInst->Init( inPort );
}

#ifdef IS_LINUX

AtomicInt32 NetworkMgr::kNewNetworkId;

NetworkMgr::NetworkMgr() :
	mTimeBetweenStatePackets( 0.033f ),
	mLastCheckDCTime( 0.f ),
	mTimeOfLastStatePacket( 0.f ),
	NetIdToEntityMap_( new NetIdToEntityMap ),
	UdpConnToClientMap_( new UdpConnToClientMap ),
	PlayerIdToClientMap_( new PlayerIdToClientMap )
{
	kNewNetworkId.getAndSet( 1 );
	kNewPlayerId.getAndSet( 1 );
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

void NetworkMgr::threadInit( EventLoop* loop )
{
	MutexLockGuard lock( mutex_ );
	loops_.insert( loop );
}

int NetworkMgr::GetNewNetworkId()
{
	int toRet = kNewNetworkId.getAndAdd( 1 );
	if ( kNewNetworkId.get() < toRet )
	{
		LOG( "Network ID Wrap Around!!! You've been playing way too long...", 0 );
	}

	return toRet;
}

EntityPtr NetworkMgr::GetGameObject( int inNetworkId )
{
	auto tempNetworkIdToGameObjectMap = GET_THREAD_VAR( NetIdToEntityMap_ );
	auto gameObjectIt = tempNetworkIdToGameObjectMap->find( inNetworkId );
	if ( gameObjectIt != tempNetworkIdToGameObjectMap->end() )
	{
		return gameObjectIt->second;
	}
	else
	{
		return EntityPtr();
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

		ProcessPacket( inputStream, conn );
		CheckForDisconnects();
		WorldUpdateCB_();
		SendOutgoingPackets();
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
	server_->setThreadInitCallback( std::bind( &NetworkMgr::threadInit, this, _1 ) );

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
		THREAD_VAR_COW( UdpConnToClientMap_ );
		THREAD_VAR_COW( PlayerIdToClientMap_ );

		LOG( "Player %d disconnect", ( ( *UdpConnToClientMap_ )[conn] )->GetPlayerId() );

		PlayerIdToClientMap_->erase( ( ( *UdpConnToClientMap_ )[conn] )->GetPlayerId() );
		UdpConnToClientMap_->erase( conn );
	}
}

void NetworkMgr::ProcessPacket( InputBitStream& inInputStream,
	const UdpConnectionPtr& inUdpConnetction )
{
	UdpConnToClientMapPtr tempUdpConnToClientMap = GET_THREAD_VAR( UdpConnToClientMap_ );
	auto it = tempUdpConnToClientMap->find( inUdpConnetction );
	if ( it == tempUdpConnToClientMap->end() )
	{
		HandlePacketFromNewClient(
			inInputStream,
			inUdpConnetction
		);
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
				playerName,
				kNewPlayerId.getAndAdd( 1 ),
				inUdpConnetction );

		{
			MutexLockGuard lock( mutex_ );
			THREAD_VAR_COW( UdpConnToClientMap_ );
			THREAD_VAR_COW( PlayerIdToClientMap_ );
			( *UdpConnToClientMap_ )[inUdpConnetction] = newClientProxy;
			( *PlayerIdToClientMap_ )[newClientProxy->GetPlayerId()] = newClientProxy;
		}

		// fixme
		//RealTimeSrv::sInstance.get()->HandleNewClient( newClientProxy );

		NetIdToEntityMapPtr tempNetworkIdToGameObjectMap = GET_THREAD_VAR( NetIdToEntityMap_ );
		for ( const auto& pair : *tempNetworkIdToGameObjectMap )
		{
			newClientProxy->GetReplicationManager().ReplicateCreate( pair.first, pair.second->GetAllStateMask() );
		}

		if ( packetType == kHelloCC )
		{
			//SendWelcomePacket( newClientProxy );
			SendGamePacket( newClientProxy, kWelcomeCC );
		}
		else
		{
			// Server reset
			newClientProxy->SetRecvingServerResetFlag( true );
			//SendResetPacket( newClientProxy );
			SendGamePacket( newClientProxy, kResetCC );
			LOG( "SendResetPacket", 0 );
		}

		//LOG_INFO << inUdpConnetction->localAddress().toIpPort() 
		//	<< " -> "
		//	<< inUdpConnetction->peerAddress().toIpPort() << " is "
		//	<< ( inUdpConnetction->connected() ? "UP" : "DOWN" );

		LOG( "a new client named '%s' as PlayerID %d ",
			newClientProxy->GetName().c_str(),
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

int NetworkMgr::RegistEntityAndRetNetID( EntityPtr inGameObject )
{
	int newNetworkId = GetNewNetworkId();
	inGameObject->SetNetworkId( newNetworkId );
	{
		MutexLockGuard lock( mutex_ );
		THREAD_VAR_COW( NetIdToEntityMap_ );
		( *NetIdToEntityMap_ )[newNetworkId] = inGameObject;
	}

	UdpConnToClientMapPtr tempUdpConnToClientMap = GET_THREAD_VAR( UdpConnToClientMap_ );
	for ( const auto& pair : *tempUdpConnToClientMap )
	{
		pair.second->GetReplicationManager().ReplicateCreate(
			newNetworkId, inGameObject->GetAllStateMask() );
	}
	return newNetworkId;
}

int NetworkMgr::UnregistEntityAndRetNetID( Entity* inGameObject )
{
	int networkId = inGameObject->GetNetworkId();
	{
		MutexLockGuard lock( mutex_ );
		THREAD_VAR_COW( NetIdToEntityMap_ );
		NetIdToEntityMap_->erase( networkId );
	}

	UdpConnToClientMapPtr tempUdpConnToClientMap = GET_THREAD_VAR( UdpConnToClientMap_ );
	for ( const auto& pair : *tempUdpConnToClientMap )
	{
		pair.second->GetReplicationManager().ReplicateDestroy( networkId );
	}
	return networkId;
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

	vector< ClientProxyPtr > clientsToDisconnect;

	auto tempUdpConnToClientMap = GET_THREAD_VAR( UdpConnToClientMap_ );
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
	float time = RealtimeSrvTiming::sInstance.GetCurrentGameTime();
	if ( time < mTimeOfLastStatePacket + kTimeBetweenStatePackets )
	{
		return;
	}
	mTimeOfLastStatePacket = time;

	UdpConnToClientMapPtr tempUdpConnToClientMap = GET_THREAD_VAR( UdpConnToClientMap_ );
	for ( const auto& pair : *tempUdpConnToClientMap )
	{
		( pair.second )->GetDeliveryNotificationManager().ProcessTimedOutPackets();

		if ( ( pair.second )->IsLastMoveTimestampDirty() )
		{
			//SendStatePacketToClient( ( pair.second ) );
			SendGamePacket( ( pair.second ), kStateCC );
		}
	}
}

ClientProxyPtr NetworkMgr::GetClientProxy( int inPlayerId )
{
	auto tempPlayerIdToClientMap = GET_THREAD_VAR( PlayerIdToClientMap_ );
	auto it = tempPlayerIdToClientMap->find( inPlayerId );
	if ( it != tempPlayerIdToClientMap->end() )
	{
		return it->second;
	}

	return nullptr;
}

void NetworkMgr::SetStateDirty( int inNetworkId, uint32_t inDirtyState )
{
	UdpConnToClientMapPtr tempUdpConnToClientMap = GET_THREAD_VAR( UdpConnToClientMap_ );
	for ( const auto& pair : *tempUdpConnToClientMap )
	{
		pair.second->GetReplicationManager().SetStateDirty( inNetworkId, inDirtyState );
	}
}

#else

int NetworkMgr::kNewNetworkId = 1;

NetworkMgr::NetworkMgr() :
	mTimeBetweenStatePackets( 0.033f ),
	mLastCheckDCTime( 0.f ),
	mDropPacketChance( 0.f ),
	mSimulatedLatency( 0.f ),
	mWhetherToSimulateJitter( false ),
	mTimeOfLastStatePacket( 0.f )
{}

void NetworkMgr::ProcessIncomingPackets()
{
	ReadIncomingPacketsIntoQueue();

	ProcessQueuedPackets();

	//UpdateBytesSentLastFrame();
}

void NetworkMgr::ReadIncomingPacketsIntoQueue()
{
	char packetMem[MAX_PACKET_BYTE_LENGTH];
	int packetSize = sizeof( packetMem );
	SockAddrInterfc fromAddress;

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

int NetworkMgr::GetNewNetworkId()
{
	int toRet = kNewNetworkId++;
	if ( kNewNetworkId < toRet )
	{
		LOG( "Network ID Wrap Around!!! You've been playing way too long...", 0 );
	}

	return toRet;
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
#if _WIN32
	UdpSockInterfc::StaticInit();
#endif //_WIN32

	mSocket = UdpSockInterfc::CreateUDPSocket();

	if ( mSocket == nullptr )
	{
		return false;
	}

	SockAddrInterfc ownAddress( INADDR_ANY, inPort );
	mSocket->Bind( ownAddress );

	LOG( "Initializing NetworkManager at port %d", inPort );

	if ( mSocket->SetNonBlockingMode( true ) != NO_ERROR )
	{
		return false;
	}

	return true;
}

void NetworkMgr::SendPacket( const OutputBitStream& inOutputStream,
	const SockAddrInterfc& inSockAddr )
{
	if ( RealtimeSrvMath::GetRandomFloat() < mDropPacketChance )
	{
		return;
	}

	int sentByteCount = mSocket->SendTo( inOutputStream.GetBufferPtr(),
		inOutputStream.GetByteLength(),
		inSockAddr );
}

void NetworkMgr::HandleConnectionReset( const SockAddrInterfc& inFromAddress )
{
	auto it = mAddressToClientMap.find( inFromAddress );
	if ( it != mAddressToClientMap.end() )
	{
		mPlayerIdToClientMap.erase( it->second->GetPlayerId() );
		mAddressToClientMap.erase( it );
	}
}

EntityPtr NetworkMgr::GetGameObject( int inNetworkId )
{
	auto gameObjectIt = NetIdToEntityMap_.find( inNetworkId );
	if ( gameObjectIt != NetIdToEntityMap_.end() )
	{
		return gameObjectIt->second;
	}
	else
	{
		return EntityPtr();
	}
}

void NetworkMgr::ProcessPacket(
	InputBitStream& inInputStream,
	const SockAddrInterfc& inFromAddress,
	const UDPSocketPtr& inUDPSocket
)
{
	auto it = mAddressToClientMap.find( inFromAddress );
	if ( it == mAddressToClientMap.end() )
	{
		HandlePacketFromNewClient(
			inInputStream,
			inFromAddress,
			inUDPSocket
			// , 
			// inUdpConnetction 
		);
	}
	else
	{
		DoProcessPacket( ( *it ).second, inInputStream );
	}
}

void NetworkMgr::HandlePacketFromNewClient(
	InputBitStream& inInputStream,
	const SockAddrInterfc& inFromAddress,
	const UDPSocketPtr& inUDPSocket
)
{
	uint32_t	packetType;
	inInputStream.Read( packetType );
	if ( packetType == kHelloCC
		|| packetType == kInputCC
		|| packetType == kResetedCC
		)
	{
		std::string playerName = "RealTimeSrvTestPlayerName";
		//inInputStream.Read( playerName );	

		ClientProxyPtr newClientProxy =
			std::make_shared< ClientProxy >(
				inFromAddress,
				playerName,
				kNewPlayerId++,
				inUDPSocket
				);
		mAddressToClientMap[inFromAddress] = newClientProxy;
		PlayerIdToClientMap_[newClientProxy->GetPlayerId()] = newClientProxy;

		//fixme
		//RealTimeSrv::sInstance.get()->HandleNewClient( newClientProxy );

		for ( const auto& pair : NetIdToEntityMap_ )
		{
			newClientProxy->GetReplicationManager().ReplicateCreate( pair.first, pair.second->GetAllStateMask() );
		}

		if ( packetType == kHelloCC )
		{
			//SendWelcomePacket( newClientProxy );
			SendGamePacket( newClientProxy, kWelcomeCC );
		}
		else
		{
			// Server reset
			newClientProxy->SetRecvingServerResetFlag( true );
			//SendResetPacket( newClientProxy );
			SendGamePacket( newClientProxy, kResetCC );
			LOG( "SendResetPacket", 0 );
		}

		LOG( "a new client at socket %s, named '%s' as PlayerID %d ",
			inFromAddress.ToString().c_str(),
			newClientProxy->GetName().c_str(),
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

	for (
		AddressToClientMap::iterator it = mAddressToClientMap.begin();
		it != mAddressToClientMap.end();
		//++it
		)
	{
		if ( it->second->GetLastPacketFromClientTime() < minAllowedTime )
		{
			LOG( "Player %d disconnect", it->second->GetPlayerId() );

			PlayerIdToClientMap_.erase( it->second->GetPlayerId() );

			mAddressToClientMap.erase( it++ );
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

	for ( auto it = mAddressToClientMap.begin(), end = mAddressToClientMap.end(); it != end; ++it )
	{
		ClientProxyPtr clientProxy = it->second;

		clientProxy->GetDeliveryNotificationManager().ProcessTimedOutPackets();

		if ( clientProxy->IsLastMoveTimestampDirty() )
		{
			//SendStatePacketToClient( clientProxy );
			SendGamePacket( clientProxy, kStateCC );
		}
	}
}

ClientProxyPtr NetworkMgr::GetClientProxy( int inPlayerId )
{
	auto it = PlayerIdToClientMap_.find( inPlayerId );
	if ( it != PlayerIdToClientMap_.end() )
	{
		return it->second;
	}

	return nullptr;
}

void NetworkMgr::SetStateDirty( int inNetworkId, uint32_t inDirtyState )
{

	for ( const auto& pair : mAddressToClientMap )
	{
		pair.second->GetReplicationManager().SetStateDirty( inNetworkId, inDirtyState );
	}
}
int NetworkMgr::RegistEntityAndRetNetID( EntityPtr inGameObject )
{
	int newNetworkId = GetNewNetworkId();
	inGameObject->SetNetworkId( newNetworkId );

	NetIdToEntityMap_[newNetworkId] = inGameObject;

	for ( const auto& pair : mAddressToClientMap )
	{
		pair.second->GetReplicationManager().ReplicateCreate( newNetworkId, inGameObject->GetAllStateMask() );
	}
	return newNetworkId;
}

int NetworkMgr::UnregistEntityAndRetNetID( Entity* inGameObject )
{
	int networkId = inGameObject->GetNetworkId();
	NetIdToEntityMap_.erase( networkId );

	for ( const auto& pair : mAddressToClientMap )
	{
		pair.second->GetReplicationManager().ReplicateDestroy( networkId );
	}
	return networkId;
}

#endif //IS_LINUX


void NetworkMgr::DoProcessPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream )
{
	inClientProxy->UpdateLastPacketTime();

	uint32_t packetType = HandleServerReset( inClientProxy, inInputStream );

	switch ( packetType )
	{
	case kHelloCC:
		//SendWelcomePacket( inClientProxy );
		SendGamePacket( inClientProxy, kWelcomeCC );
		break;
	case kInputCC:
		// fixme
		if ( inClientProxy->GetDeliveryNotificationManager().ReadAndProcessState( inInputStream ) )
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
		//SendResetPacket( inClientProxy );
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
			if ( inClientProxy->GetUnprocessedMoveList().AddMoveIfNew( move ) )
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

	InFlightPacket* ifp =
		inClientProxy->GetDeliveryNotificationManager().WriteState(
			outputPacket,
			&inClientProxy->GetReplicationManager(),
			this
		);

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
		inOutputStream.Write( inClientProxy->GetUnprocessedMoveList().GetLastMoveTimestamp() );
		inClientProxy->SetIsLastMoveTimestampDirty( false );
	}
}
