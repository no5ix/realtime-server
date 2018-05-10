#include "RealTimeSrvPCH.h"


NetworkMgrSrv*	NetworkMgrSrv::sInst;

namespace
{
	const float kTimeBetweenStatePackets = 0.033f;
	const float kClientDisconnectTimeout = 3.f;
}

NetworkMgrSrv::NetworkMgrSrv() :
	mNewPlayerId( 1 ),
	mNewNetworkId( 1 ),
	mTimeBetweenStatePackets( 0.033f ),
	mLastCheckDCTime( 0.f )
{
}

bool NetworkMgrSrv::StaticInit( uint16_t inPort )
{
	sInst = new NetworkMgrSrv();
	return sInst->Init( inPort );
}

void NetworkMgrSrv::ProcessPacket( InputBitStream& inInputStream, const SocketAddrInterface& inFromAddress, const UDPSocketPtr& inUDPSocket )
{
	auto it = mAddressToClientMap.find( inFromAddress );
	if ( it == mAddressToClientMap.end() )
	{
		HandlePacketFromNewClient( inInputStream, inFromAddress, inUDPSocket);
	}
	else
	{
		ProcessPacket( ( *it ).second, inInputStream );
	}
}


void NetworkMgrSrv::ProcessPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream )
{
	inClientProxy->UpdateLastPacketTime();

	uint32_t	packetType;
	inInputStream.Read( packetType );
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
	default:
		LOG( "Unknown packet type received from %s", inClientProxy->GetSocketAddress().ToString().c_str() );
		break;
	}
}

void NetworkMgrSrv::HandleInputPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream )
{
	uint32_t moveCount = 0;
	Action move;
	inInputStream.Read( moveCount, 2 );

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


void NetworkMgrSrv::HandlePacketFromNewClient( InputBitStream& inInputStream, const SocketAddrInterface& inFromAddress, const UDPSocketPtr& inUDPSocket )
{
	uint32_t	packetType;
	inInputStream.Read( packetType );
	if ( packetType == kHelloCC )
	{
		string name;
		inInputStream.Read( name );	
		
		ClientProxyPtr newClientProxy = std::make_shared< ClientProxy >( inFromAddress, name, mNewPlayerId++, inUDPSocket );
		mAddressToClientMap[inFromAddress] = newClientProxy;
		mPlayerIdToClientMap[newClientProxy->GetPlayerId()] = newClientProxy;

		RealTimeSrv::sInstance.get()->HandleNewClient( newClientProxy );


		for ( const auto& pair : mNetworkIdToGameObjectMap )
		{
			newClientProxy->GetReplicationManagerServer().ReplicateCreate( pair.first, pair.second->GetAllStateMask() );
		}

		SendWelcomePacket( newClientProxy );

		LOG( "a new client at socket %s, named '%s' as PlayerID %d ", 
			inFromAddress.ToString().c_str(), 
			newClientProxy->GetName().c_str(), 
			newClientProxy->GetPlayerId() 
		);
	}
	else if ( packetType == kInputCC )
	{
		// Server reset, Deprecated!
		//SendResetPacket( inFromAddress );
	}
	else
	{
		LOG( "Bad incoming packet from unknown client at socket %s - we're under attack!!", inFromAddress.ToString().c_str() );
	}
}

// Deprecated
void NetworkMgrSrv::SendResetPacket(const SocketAddrInterface& inFromAddress)
{
	OutputBitStream resetPacket;

	resetPacket.Write( kResetCC );

	//int sentByteCount = mSocket->SendTo( resetPacket.GetBufferPtr(), resetPacket.GetByteLength(), inFromAddress );
}

void NetworkMgrSrv::SendWelcomePacket( ClientProxyPtr inClientProxy )
{
	OutputBitStream welcomePacket;

	welcomePacket.Write( kWelcomeCC );
	welcomePacket.Write( inClientProxy->GetPlayerId() );
	welcomePacket.Write( kTimeBetweenStatePackets );

	TransmissionDataHandler* rmtd = new TransmissionDataHandler( &inClientProxy->GetReplicationManagerServer() );
	inClientProxy->GetReplicationManagerServer().Write( welcomePacket, rmtd );

	//SendPacket( welcomePacket, inClientProxy );
	ProcessOutcomingPacket( welcomePacket, inClientProxy, rmtd );


}

void NetworkMgrSrv::RegisterGameObject( GameObjectPtr inGameObject )
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

int NetworkMgrSrv::GetNewNetworkId()
{
	int toRet = mNewNetworkId++;
	if ( mNewNetworkId < toRet )
	{
		LOG( "Network ID Wrap Around!!! You've been playing way too long...", 0 );
	}

	return toRet;

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

void NetworkMgrSrv::SendStatePacketToClient( ClientProxyPtr inClientProxy )
{

	OutputBitStream	statePacket;

	statePacket.Write( kStateCC );

	//InFlightPacket* ifp = inClientProxy->GetDeliveryNotificationManager().WriteState( statePacket );

	WriteLastMoveTimestampIfDirty( statePacket, inClientProxy );

	TransmissionDataHandler* rmtd = new TransmissionDataHandler( &inClientProxy->GetReplicationManagerServer() );
	inClientProxy->GetReplicationManagerServer().Write( statePacket, rmtd );
	//ifp->SetTransmissionData( 'RPLM', TransmissionDataPtr( rmtd ) );

	//SendPacket( statePacket, inClientProxy );
	ProcessOutcomingPacket( statePacket, inClientProxy, rmtd );
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

ClientProxyPtr NetworkMgrSrv::GetClientProxy( int inPlayerId ) const
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



void NetworkMgrSrv::HandleConnectionReset( const SocketAddrInterface& inFromAddress )
{
	//just dc the client right away...
	auto it = mAddressToClientMap.find( inFromAddress );
	if ( it != mAddressToClientMap.end() )
	{
		mPlayerIdToClientMap.erase( it->second->GetPlayerId() );

#ifdef HAS_EPOLL
		EpollInterface::sInst->CloseSocket( it->second->GetUDPSocket()->GetSocket() );
#endif

		mAddressToClientMap.erase( it );
	}
}

void NetworkMgrSrv::CheckForDisconnects()
{

	float curTime = RealTimeSrvTiming::sInstance.GetCurrentGameTime();

	if (curTime - mLastCheckDCTime < kClientDisconnectTimeout)
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
			mPlayerIdToClientMap.erase( it->second->GetPlayerId() );

#ifdef HAS_EPOLL
			EpollInterface::sInst->CloseSocket( it->second->GetUDPSocket()->GetSocket() );
#endif

			mAddressToClientMap.erase( it++ );
			//it = mAddressToClientMap.erase( it );
		}
		else
		{
			++it;
		}
	}
}