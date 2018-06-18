#include "RealTimeSrvPCH.h"


#ifdef NEW_EPOLL_INTERFACE
AtomicInt32 NetworkMgrSrv::kNewPlayerId;
AtomicInt32 NetworkMgrSrv::kNewNetworkId;
#else
int NetworkMgrSrv::kNewPlayerId = 1;
int NetworkMgrSrv::kNewNetworkId = 1;
#endif //NEW_EPOLL_INTERFACE

std::unique_ptr<NetworkMgrSrv> NetworkMgrSrv::sInst;

namespace
{
	const float kTimeBetweenStatePackets = 0.033f;
	const float kClientDisconnectTimeout = 6.f;
}

NetworkMgrSrv::NetworkMgrSrv() :
	mTimeBetweenStatePackets( 0.033f ),
	mLastCheckDCTime( 0.f ),
	mTimeOfLastStatePacket( 0.f )
{
#ifdef NEW_EPOLL_INTERFACE
	kNewPlayerId.getAndSet( 1 );
	kNewNetworkId.getAndSet( 1 );
#endif //NEW_EPOLL_INTERFACE
}

bool NetworkMgrSrv::StaticInit( uint16_t inPort )
{
	sInst.reset( new NetworkMgrSrv() );
	return sInst->Init( inPort );
}

#ifdef NEW_EPOLL_INTERFACE

void NetworkMgrSrv::ProcessPacket( InputBitStream& inInputStream,
	const UdpConnectionPtr& inUdpConnetction )
{
	UdpConnToClientMapPtr tempUdpConnToClientMap = GetUdpConnToClientMap();
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

void NetworkMgrSrv::HandlePacketFromNewClient( InputBitStream& inInputStream,
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
				kNewPlayerId.getAndAdd(1),
				inUdpConnetction );

		{
			MutexLockGuard lock( mutex_ );
			UdpConnToClientMapCOW();
			PlayerIdToClientMapCOW();
			( *mUdpConnToClientMap )[inUdpConnetction] = newClientProxy;
			( *mPlayerIdToClientMap )[newClientProxy->GetPlayerId()] = newClientProxy;
		}

		RealTimeSrv::sInstance.get()->HandleNewClient( newClientProxy );

		IntToGameObjectMapPtr tempNetworkIdToGameObjectMap = GetNetworkIdToGameObjectMap();
		for ( const auto& pair : *tempNetworkIdToGameObjectMap )
		{
			newClientProxy->GetReplicationManagerServer().ReplicateCreate( pair.first, pair.second->GetAllStateMask() );
		}

		if ( packetType == kHelloCC )
		{
			SendWelcomePacket( newClientProxy );
		}
		else
		{
			// Server reset
			newClientProxy->SetRecvingServerResetFlag( true );
			SendResetPacket( newClientProxy );
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

int NetworkMgrSrv::GetNewNetworkId()
{
	int toRet = kNewNetworkId.getAndAdd(1);
	if ( kNewNetworkId.get() < toRet )
	{
		LOG( "Network ID Wrap Around!!! You've been playing way too long...", 0 );
	}

	return toRet;
}

void NetworkMgrSrv::RegisterGameObject( EntityPtr inGameObject )
{
	int newNetworkId = GetNewNetworkId();
	inGameObject->SetNetworkId( newNetworkId );
	{
		MutexLockGuard lock( mutex_ );
		NetworkIdToGameObjectMapCOW();
		( *mNetworkIdToGameObjectMap )[newNetworkId] = inGameObject;
	}

	UdpConnToClientMapPtr tempUdpConnToClientMap = GetUdpConnToClientMap();
	for ( const auto& pair : *tempUdpConnToClientMap )
	{
		pair.second->GetReplicationManagerServer().ReplicateCreate( newNetworkId, inGameObject->GetAllStateMask() );
	}
}

void NetworkMgrSrv::UnregisterGameObject( Entity* inGameObject )
{
	int networkId = inGameObject->GetNetworkId();
	{
		MutexLockGuard lock( mutex_ );
		NetworkIdToGameObjectMapCOW();
		mNetworkIdToGameObjectMap->erase( networkId );
	}

	UdpConnToClientMapPtr tempUdpConnToClientMap = GetUdpConnToClientMap();
	for ( const auto& pair : *tempUdpConnToClientMap )
	{
		pair.second->GetReplicationManagerServer().ReplicateDestroy( networkId );
	}
}

void NetworkMgrSrv::CheckForDisconnects()
{
	float curTime = RealTimeSrvTiming::sInstance.GetCurrentGameTime();

	if ( curTime - mLastCheckDCTime < kClientDisconnectTimeout )
	{
		return;
	}
	mLastCheckDCTime = curTime;

	float minAllowedTime =
		RealTimeSrvTiming::sInstance.GetCurrentGameTime() - kClientDisconnectTimeout;

	vector< ClientProxyPtr > clientsToDisconnect;

	auto tempUdpConnToClientMap = GetUdpConnToClientMap();
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

void NetworkMgrSrv::SendOutgoingPackets()
{
	float time = RealTimeSrvTiming::sInstance.GetCurrentGameTime();
	if ( time < mTimeOfLastStatePacket + kTimeBetweenStatePackets )
	{
		return;
	}
	mTimeOfLastStatePacket = time;

	UdpConnToClientMapPtr tempUdpConnToClientMap = GetUdpConnToClientMap();
	for ( const auto& pair : *tempUdpConnToClientMap )
	{
		( pair.second )->GetDeliveryNotificationManager().ProcessTimedOutPackets();

		if ( ( pair.second )->IsLastMoveTimestampDirty() )
		{
			SendStatePacketToClient( ( pair.second ) );
		}
	}
}

ClientProxyPtr NetworkMgrSrv::GetClientProxy( int inPlayerId )
{
	auto tempPlayerIdToClientMap = GetPlayerIdToClientMap();
	auto it = tempPlayerIdToClientMap->find( inPlayerId );
	if ( it != tempPlayerIdToClientMap->end() )
	{
		return it->second;
	}

	return nullptr;
}

void NetworkMgrSrv::SetStateDirty( int inNetworkId, uint32_t inDirtyState )
{
	UdpConnToClientMapPtr tempUdpConnToClientMap = GetUdpConnToClientMap();
	for ( const auto& pair : *tempUdpConnToClientMap )
	{
		pair.second->GetReplicationManagerServer().SetStateDirty( inNetworkId, inDirtyState );
	}
}

#else

void NetworkMgrSrv::ProcessPacket(
	InputBitStream& inInputStream,
	const SocketAddrInterface& inFromAddress,
	const UDPSocketPtr& inUDPSocket
	// ,
	// const std::shared_ptr<UdpConnection>& inUdpConnetction
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

void NetworkMgrSrv::HandlePacketFromNewClient(
	InputBitStream& inInputStream,
	const SocketAddrInterface& inFromAddress,
	const UDPSocketPtr& inUDPSocket
	// ,
	// const std::shared_ptr<UdpConnection>& inUdpConnetction
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
				// ,
				// inUdpConnetction
				);
		mAddressToClientMap[inFromAddress] = newClientProxy;
		mPlayerIdToClientMap[newClientProxy->GetPlayerId()] = newClientProxy;

		RealTimeSrv::sInstance.get()->HandleNewClient( newClientProxy );

		for ( const auto& pair : mNetworkIdToGameObjectMap )
		{
			newClientProxy->GetReplicationManagerServer().ReplicateCreate( pair.first, pair.second->GetAllStateMask() );
		}

		if ( packetType == kHelloCC )
		{
			SendWelcomePacket( newClientProxy );
		}
		else
		{
			// Server reset
			newClientProxy->SetRecvingServerResetFlag( true );
			SendResetPacket( newClientProxy );
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

int NetworkMgrSrv::GetNewNetworkId()
{
	int toRet = kNewNetworkId++;
	if ( kNewNetworkId < toRet )
	{
		LOG( "Network ID Wrap Around!!! You've been playing way too long...", 0 );
	}

	return toRet;

}

void NetworkMgrSrv::RegisterGameObject( EntityPtr inGameObject )
{
	int newNetworkId = GetNewNetworkId();
	inGameObject->SetNetworkId( newNetworkId );

	mNetworkIdToGameObjectMap[newNetworkId] = inGameObject;

	for ( const auto& pair : mAddressToClientMap )
	{
		pair.second->GetReplicationManagerServer().ReplicateCreate( newNetworkId, inGameObject->GetAllStateMask() );
	}
}

void NetworkMgrSrv::UnregisterGameObject( Entity* inGameObject )
{
	int networkId = inGameObject->GetNetworkId();
	mNetworkIdToGameObjectMap.erase( networkId );

	for ( const auto& pair : mAddressToClientMap )
	{
		pair.second->GetReplicationManagerServer().ReplicateDestroy( networkId );
	}
}

void NetworkMgrSrv::CheckForDisconnects()
{
	float curTime = RealTimeSrvTiming::sInstance.GetCurrentGameTime();

	if ( curTime - mLastCheckDCTime < kClientDisconnectTimeout )
	{
		return;
	}
	mLastCheckDCTime = curTime;

	float minAllowedTime =
		RealTimeSrvTiming::sInstance.GetCurrentGameTime() - kClientDisconnectTimeout;

	for (
		AddressToClientMap::iterator it = mAddressToClientMap.begin();
		it != mAddressToClientMap.end();
		//++it
		)
	{
		if ( it->second->GetLastPacketFromClientTime() < minAllowedTime )
		{
			LOG( "Player %d disconnect", it->second->GetPlayerId() );

			mPlayerIdToClientMap.erase( it->second->GetPlayerId() );

#ifdef DEPRECATED_EPOLL_INTERFACE
			EpollInterface::sInst->CloseSocket( it->second->GetUDPSocket()->GetSocket() );
#endif

			mAddressToClientMap.erase( it++ );
		}
		else
		{
			++it;
		}
	}
}

void NetworkMgrSrv::SendOutgoingPackets()
{
	float time = RealTimeSrvTiming::sInstance.GetCurrentGameTime();

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
			SendStatePacketToClient( clientProxy );
		}
	}
}

ClientProxyPtr NetworkMgrSrv::GetClientProxy( int inPlayerId )
{
	auto it = mPlayerIdToClientMap.find( inPlayerId );
	if ( it != mPlayerIdToClientMap.end() )
	{
		return it->second;
	}

	return nullptr;
}

void NetworkMgrSrv::SetStateDirty( int inNetworkId, uint32_t inDirtyState )
{

	for ( const auto& pair : mAddressToClientMap )
	{
		pair.second->GetReplicationManagerServer().SetStateDirty( inNetworkId, inDirtyState );
	}
}

#endif //NEW_EPOLL_INTERFACE


void NetworkMgrSrv::DoProcessPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream )
{
	inClientProxy->UpdateLastPacketTime();

	uint32_t packetType = HandleServerReset( inClientProxy, inInputStream );

	switch ( packetType )
	{
	case kHelloCC:
		SendWelcomePacket( inClientProxy );
		break;
	case kInputCC:
		if ( inClientProxy->GetDeliveryNotificationManager().ReadAndProcessState( inInputStream ) )
		{
			HandleInputPacket( inClientProxy, inInputStream );
		}
		break;
	case kNullCC:
		break;
	default:
#ifdef NEW_EPOLL_INTERFACE
		LOG_INFO << "Unknown packet type received from "
			<< inClientProxy->GetUdpConnection()->peerAddress().toIpPort();
#else
		LOG( "Unknown packet type received from %s", inClientProxy->GetSocketAddress().ToString().c_str() );
#endif //NEW_EPOLL_INTERFACE
		break;
	}
}

uint32_t NetworkMgrSrv::HandleServerReset( ClientProxyPtr inClientProxy, InputBitStream& inInputStream )
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
		SendResetPacket( inClientProxy );
		return kNullCC;
	}
	else
	{
		return packetType;
	}
}

void NetworkMgrSrv::HandleInputPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream )
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

void NetworkMgrSrv::SendResetPacket( ClientProxyPtr inClientProxy )
{
	OutputBitStream resetPacket;
	resetPacket.Write( kResetCC );

	InFlightPacket* ifp =
		inClientProxy->GetDeliveryNotificationManager().WriteState( resetPacket );

	resetPacket.Write( inClientProxy->GetPlayerId() );
	resetPacket.Write( kTimeBetweenStatePackets );

	TransmissionDataPtr rmtd = std::make_shared<TransmissionDataHandler>(
		&inClientProxy->GetReplicationManagerServer() );

	inClientProxy->GetReplicationManagerServer().Write( resetPacket, rmtd );

	//ifp->SetTransmissionData( 'RPLM', TransmissionDataPtr( rmtd ) );
	ifp->SetTransmissionData( 'RPLM', rmtd );

	SendPacket( resetPacket, inClientProxy );

	//int sentByteCount = mSocket->SendTo( resetPacket.GetBufferPtr(), resetPacket.GetByteLength(), inFromAddress );
}

void NetworkMgrSrv::SendWelcomePacket( ClientProxyPtr inClientProxy )
{
	OutputBitStream welcomePacket;
	welcomePacket.Write( kWelcomeCC );

	InFlightPacket* ifp =
		inClientProxy->GetDeliveryNotificationManager().WriteState( welcomePacket );

	welcomePacket.Write( inClientProxy->GetPlayerId() );
	welcomePacket.Write( kTimeBetweenStatePackets );
	TransmissionDataPtr rmtd = std::make_shared<TransmissionDataHandler>(
		&inClientProxy->GetReplicationManagerServer() );

	inClientProxy->GetReplicationManagerServer().Write( welcomePacket, rmtd );

	//ifp->SetTransmissionData( 'RPLM', TransmissionDataPtr( rmtd ) );
	ifp->SetTransmissionData( 'RPLM', rmtd );

	SendPacket( welcomePacket, inClientProxy );
	//ProcessOutcomingPacket( welcomePacket, inClientProxy, rmtd );
}

void NetworkMgrSrv::SendStatePacketToClient( ClientProxyPtr inClientProxy )
{
	OutputBitStream	statePacket;
	statePacket.Write( kStateCC );

	InFlightPacket* ifp =
		inClientProxy->GetDeliveryNotificationManager().WriteState( statePacket );

	WriteLastMoveTimestampIfDirty( statePacket, inClientProxy );

	TransmissionDataPtr rmtd =
		std::make_shared<TransmissionDataHandler>( &inClientProxy->GetReplicationManagerServer() );

	inClientProxy->GetReplicationManagerServer().Write( statePacket, rmtd );

	//ifp->SetTransmissionData( 'RPLM', TransmissionDataPtr( rmtd ) );
	ifp->SetTransmissionData( 'RPLM', rmtd );

	SendPacket( statePacket, inClientProxy );
	//ProcessOutcomingPacket( statePacket, inClientProxy, rmtd );
}

void NetworkMgrSrv::WriteLastMoveTimestampIfDirty( OutputBitStream& inOutputStream, ClientProxyPtr inClientProxy )
{
	bool isTimestampDirty = inClientProxy->IsLastMoveTimestampDirty();
	inOutputStream.Write( isTimestampDirty );
	if ( isTimestampDirty )
	{
		inOutputStream.Write( inClientProxy->GetUnprocessedMoveList().GetLastMoveTimestamp() );
		inClientProxy->SetIsLastMoveTimestampDirty( false );
	}
}
