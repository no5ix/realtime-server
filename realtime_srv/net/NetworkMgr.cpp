#include "realtime_srv/common/RealtimeSrvShared.h"
#include <time.h>



std::unique_ptr<NetworkMgr> NetworkMgr::sInstance;

namespace
{
	const float kTimeBetweenStatePackets = 0.033f;
	const float kClientDisconnectTimeout = 6.f;
}


bool NetworkMgr::StaticInit( uint16_t Port )
{
	sInstance.reset( new NetworkMgr() );
	return sInstance->Init( Port );
}

#ifdef IS_LINUX

AtomicInt32 NetworkMgr::kNewNetId;
AtomicInt32 NetworkMgr::kNewPlayerId;

NetworkMgr::NetworkMgr() :
	mTimeBetweenStatePackets( 0.033f ),
	mLastCheckDCTime( 0.f ),
	mTimeOfLastStatePacket( 0.f ),
	netIdToGameObjMap_( new NetIdToGameObjMap ),
	udpConnToClientMap_( new UdpConnToClientMap )
{
	kNewNetId.getAndSet( 1 );
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

int NetworkMgr::GetNewNetworkId()
{
	int toRet = kNewNetId.getAndAdd( 1 );
	if ( kNewNetId.get() < toRet )
	{
		LOG( "Network ID Wrap Around!!! You've been playing way too long...", 0 );
	}

	return toRet;
}

GameObjPtr NetworkMgr::GetGameObject( int inNetworkId )
{
	auto tempNetworkIdToGameObjectMap = GET_THREAD_SHARED_VAR( netIdToGameObjMap_ );
	auto gameObjectIt = tempNetworkIdToGameObjectMap->find( inNetworkId );
	if ( gameObjectIt != tempNetworkIdToGameObjectMap->end() )
	{
		return gameObjectIt->second;
	}
	else
	{
		return GameObjPtr();
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
		worldUpdateCB_();
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

		int newPlayerId = newClientProxy->GetPlayerId();
		{
			MutexLockGuard lock( mutex_ );
			THREAD_SHARED_VAR_COW( udpConnToClientMap_ );
			( *udpConnToClientMap_ )[inUdpConnetction] = newClientProxy;
		}

		World::sInstance->AddGameObject( newPlayerCB_( newPlayerId ) );

		NetIdToGameObjMapPtr tempNetworkIdToGameObjectMap =
			GET_THREAD_SHARED_VAR( netIdToGameObjMap_ );
		for ( const auto& pair : *tempNetworkIdToGameObjectMap )
		{
			newClientProxy->GetReplicationManager().ReplicateCreate(
				pair.first, pair.second->GetAllStateMask() );
		}

		if ( packetType == kHelloCC )
		{
			SendGamePacket( newClientProxy, kWelcomeCC );
		}
		else
		{
			// Server reset
			newClientProxy->SetRecvingServerResetFlag( true );
			SendGamePacket( newClientProxy, kResetCC );
			LOG( "SendResetPacket", 0 );
		}

		LOG( "a new client named '%s' as PlayerID %d ",
			newClientProxy->GetName().c_str(),
			newPlayerId );
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

int NetworkMgr::RegistGameObjAndRetNetID( GameObjPtr inGameObject )
{
	int newNetworkId = GetNewNetworkId();
	inGameObject->SetNetworkId( newNetworkId );
	//{
	//	MutexLockGuard lock( mutex_ );
	//	THREAD_VAR_COW( NetIdToEntityMap_ );
	//	( *NetIdToEntityMap_ )[newNetworkId] = inGameObject;
	//}
	auto regFunc = [newNetworkId, &inGameObject, this]()
	{ ( *netIdToGameObjMap_ )[newNetworkId] = inGameObject; };
	SET_THREAD_SHARED_VAR( netIdToGameObjMap_, mutex_, regFunc );

	UdpConnToClientMapPtr tempUdpConnToClientMap = GET_THREAD_SHARED_VAR( udpConnToClientMap_ );
	for ( const auto& pair : *tempUdpConnToClientMap )
	{
		pair.second->GetReplicationManager().ReplicateCreate(
			newNetworkId, inGameObject->GetAllStateMask() );
	}
	return newNetworkId;
}

int NetworkMgr::UnregistGameObjAndRetNetID( GameObj* inGameObject )
{
	int networkId = inGameObject->GetNetworkId();
	//{
	//	MutexLockGuard lock( mutex_ );
	//	THREAD_VAR_COW( NetIdToEntityMap_ );
	//	NetIdToEntityMap_->erase( networkId );
	//}
	auto tempFunc = [&, this]()
	{ netIdToGameObjMap_->erase( networkId ); };
	SET_THREAD_SHARED_VAR( netIdToGameObjMap_, mutex_, tempFunc );

	UdpConnToClientMapPtr tempUdpConnToClientMap = GET_THREAD_SHARED_VAR( udpConnToClientMap_ );
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
	float time = RealtimeSrvTiming::sInstance.GetCurrentGameTime();
	if ( time < mTimeOfLastStatePacket + kTimeBetweenStatePackets )
	{
		return;
	}
	mTimeOfLastStatePacket = time;

	UdpConnToClientMapPtr tempUdpConnToClientMap = GET_THREAD_SHARED_VAR( udpConnToClientMap_ );
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

void NetworkMgr::SetStateDirty( int inNetworkId, uint32_t inDirtyState )
{
	UdpConnToClientMapPtr tempUdpConnToClientMap = 
		GET_THREAD_SHARED_VAR( udpConnToClientMap_ );
	for ( const auto& pair : *tempUdpConnToClientMap )
	{
		pair.second->GetReplicationManager().SetStateDirty( inNetworkId, inDirtyState );
	}
}

#else

int NetworkMgr::kNewNetId = 1;
int NetworkMgr::kNewPlayerId = 1;

NetworkMgr::NetworkMgr() :
	mTimeBetweenStatePackets( 0.033f ),
	mLastCheckDCTime( 0.f ),
	mDropPacketChance( 0.f ),
	mSimulatedLatency( 0.f ),
	mWhetherToSimulateJitter( false ),
	mTimeOfLastStatePacket( 0.f )
{}

void NetworkMgr::Start()
{
	while ( true )
	{
		sInstance->ProcessIncomingPackets();

		sInstance->CheckForDisconnects();

		worldUpdateCB_();

		sInstance->SendOutgoingPackets();
	}
}

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
	int toRet = kNewNetId++;
	if ( kNewNetId < toRet )
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
	UdpSockInterfc::StaticInit();

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
	auto it = addrToClientMap_.find( inFromAddress );
	if ( it != addrToClientMap_.end() )
	{
		addrToClientMap_.erase( it );
	}
}

GameObjPtr NetworkMgr::GetGameObject( int inNetworkId )
{
	auto gameObjectIt = netIdToGameObjMap_.find( inNetworkId );
	if ( gameObjectIt != netIdToGameObjMap_.end() )
	{
		return gameObjectIt->second;
	}
	else
	{
		return GameObjPtr();
	}
}

void NetworkMgr::ProcessPacket(
	InputBitStream& inInputStream,
	const SockAddrInterfc& inFromAddress,
	const UDPSocketPtr& inUDPSocket
)
{
	auto it = addrToClientMap_.find( inFromAddress );
	if ( it == addrToClientMap_.end() )
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

		int newPlayerId = newClientProxy->GetPlayerId();
		addrToClientMap_[inFromAddress] = newClientProxy;

		World::sInstance->AddGameObject( newPlayerCB_( newPlayerId ) );

		for ( const auto& pair : netIdToGameObjMap_ )
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
			newPlayerId
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
		AddrToClientMap::iterator it = addrToClientMap_.begin();
		it != addrToClientMap_.end();
		//++it
		)
	{
		if ( it->second->GetLastPacketFromClientTime() < minAllowedTime )
		{
			LOG( "Player %d disconnect", it->second->GetPlayerId() );

			addrToClientMap_.erase( it++ );
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

		clientProxy->GetDeliveryNotificationManager().ProcessTimedOutPackets();

		if ( clientProxy->IsLastMoveTimestampDirty() )
		{
			//SendStatePacketToClient( clientProxy );
			SendGamePacket( clientProxy, kStateCC );
		}
	}
}

void NetworkMgr::SetStateDirty( int inNetworkId, uint32_t inDirtyState )
{

	for ( const auto& pair : addrToClientMap_ )
	{
		pair.second->GetReplicationManager().SetStateDirty( inNetworkId, inDirtyState );
	}
}
int NetworkMgr::RegistGameObjAndRetNetID( GameObjPtr inGameObject )
{
	int newNetworkId = GetNewNetworkId();
	inGameObject->SetNetworkId( newNetworkId );

	netIdToGameObjMap_[newNetworkId] = inGameObject;

	for ( const auto& pair : addrToClientMap_ )
	{
		pair.second->GetReplicationManager().ReplicateCreate( newNetworkId, inGameObject->GetAllStateMask() );
	}
	return newNetworkId;
}

int NetworkMgr::UnregistGameObjAndRetNetID( GameObj* inGameObject )
{
	int networkId = inGameObject->GetNetworkId();
	netIdToGameObjMap_.erase( networkId );

	for ( const auto& pair : addrToClientMap_ )
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
