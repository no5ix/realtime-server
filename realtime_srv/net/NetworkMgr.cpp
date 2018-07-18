#include "realtime_srv/common/RealtimeSrvShared.h"


using namespace realtime_srv;


#ifdef IS_LINUX


using namespace muduo;
using namespace muduo::net;

AtomicInt32 NetworkMgr::kNewNetId;

NetworkMgr::NetworkMgr( uint16_t inPort /*= DEFAULT_REALTIME_SRV_PORT*/ ) :
	pktHandler_( inPort, PACKET_DISPATCHER_THREAD_NUM,
		std::bind( &NetworkMgr::DoProcessPkt, this, _1 ),
		std::bind( &NetworkMgr::Tick, this ),
		std::bind( &NetworkMgr::CheckForDisconnects, this ) ),
	bUnregistObjWhenCliDisconn_( false )
{
	kNewNetId.getAndSet( 1 );
}

void realtime_srv::NetworkMgr::Start()
{
	//assert( newPlayerCb_ );
	//assert( customInputStatecb_ );

	assert( worldRegistryCb_ );
	assert( worldUpdateCb_ );

	pktHandler_.Start();
}

void NetworkMgr::DoPreparePacketToSend( ClientProxyPtr cliProxy, const uint32_t inConnFlag )
{
	shared_ptr< OutputBitStream > outputPacket( new OutputBitStream() );
	outputPacket->Write( inConnFlag );

	InFlightPacket* ifp = cliProxy->GetDeliveryNotifyManager()
		.WriteState( *outputPacket, cliProxy.get() );

	switch ( inConnFlag )
	{
		case kResetCC:
		case kWelcomeCC:
			outputPacket->Write( cliProxy->GetNetId() );
			outputPacket->Write( PktHandler::kSendPacketInterval );
			break;
		case kStateCC:
			WriteLastMoveTimestampIfDirty( *outputPacket, cliProxy );
		default:
			break;
	}
	cliProxy->GetReplicationManager().Write( *outputPacket, ifp );

	pktHandler_.AppendToPendingSndPktQ(
		PendingSendPacketPtr( new PendingSendPacket(
			outputPacket, cliProxy->GetUdpConnection() ) ),
		cliProxy->GetConnHoldedByThreadId() );
}

void NetworkMgr::DoProcessPkt( ReceivedPacketPtr& recvedPacket )
{
	auto it = udpConnToClientMap_.find( recvedPacket->GetUdpConn() );
	if ( it == udpConnToClientMap_.end() )
	{
		WelcomeNewClient( *recvedPacket->GetPacketBuffer(),
			recvedPacket->GetUdpConn(), recvedPacket->GetHoldedByThreadId() );
	}
	else
	{
		CheckPacketType( ( *it ).second, *recvedPacket->GetPacketBuffer() );
	}
}

void NetworkMgr::PreparePacketToSend()
{
	for ( const auto& pair : udpConnToClientMap_ )
	{
		( pair.second )->GetDeliveryNotifyManager().ProcessTimedOutPackets();

		if ( ( pair.second )->IsLastMoveTimestampDirty() )
		{
			DoPreparePacketToSend( ( pair.second ), kStateCC );
		}
	}
}

void NetworkMgr::Tick()
{
	worldUpdateCb_();
	PreparePacketToSend();
}

void NetworkMgr::CheckForDisconnects()
{
	float minAllowedTime =
		RealtimeSrvTiming::sInst.GetCurrentGameTime() - PktHandler::kClientDisconnectTimeout;

	for ( auto it = udpConnToClientMap_.begin();
		it != udpConnToClientMap_.end(); )
	{
		if ( it->second->GetLastPacketFromClientTime() < minAllowedTime )
		{
			if ( bUnregistObjWhenCliDisconn_ )
				for ( auto& obj : it->second->GetAllOwnedGameObjs() )
					worldRegistryCb_( obj, RA_Destroy );
			else
				for ( auto& obj : it->second->GetAllOwnedGameObjs() )
					obj->LoseOwner();

			it->second->GetUdpConnection()->forceClose();
			udpConnToClientMap_.erase( it++ );
		}
		else
			++it;
	}
}

void NetworkMgr::WelcomeNewClient( InputBitStream& inInputStream,
	const UdpConnectionPtr& inUdpConnetction, const pid_t inHoldedByThreadId )
{
	uint32_t	packetType;
	inInputStream.Read( packetType );
	if ( packetType == kHelloCC
		|| packetType == kInputCC
		|| packetType == kResetedCC )
	{
		std::string playerName = "realtime_srv_test_player_name";
		//inInputStream.Read( playerName );	

		ClientProxyPtr newClientProxy = std::make_shared< ClientProxy >(
			shared_from_this(),
			playerName,
			kNewNetId.getAndAdd( 1 ),
			inHoldedByThreadId,
			inUdpConnetction );

		udpConnToClientMap_[inUdpConnetction] = newClientProxy;

		if ( newPlayerCb_ )
		{
			GameObjPtr newGameObj( newPlayerCb_( newClientProxy ) );
			assert( newGameObj );

			newGameObj->SetOwner( newClientProxy );
			worldRegistryCb_( newGameObj, RA_Create );
			newClientProxy->AddGameObj( newGameObj );
		}

		if ( packetType == kHelloCC )
		{ DoPreparePacketToSend( newClientProxy, kWelcomeCC ); }
		else
		{
			newClientProxy->SetRecvingServerResetFlag( true );
			DoPreparePacketToSend( newClientProxy, kResetCC );
			LOG_INFO << "Send Reset Packet";
		}

		LOG_INFO << "a new client named '"
			<< newClientProxy->GetPlayerName().c_str()
			<< "' as Net ID " << newClientProxy->GetNetId();
	}
	else
	{
		LOG_INFO << "Bad incoming packet from unknown client at socket "
			<< inUdpConnetction->peerAddress().toIpPort() << " is "
			<< " - we're under attack!!";
	}
}

void NetworkMgr::OnObjCreateOrDestory( GameObjPtr& inGameObject, ReplicationAction inAction )
{
	if (inAction == RA_Create)
		inGameObject->SetNetworkMgr( shared_from_this() );

	for ( const auto& pair : udpConnToClientMap_ )
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

void NetworkMgr::SetRepStateDirty( int _objId, uint32_t inDirtyState )
{
	for ( const auto& pair : udpConnToClientMap_ )
	{
		pair.second->GetReplicationManager().SetReplicationStateDirty(
			_objId, inDirtyState );
	}
}



void NetworkMgr::CheckPacketType( ClientProxyPtr& inClientProxy, InputBitStream& inInputStream )
{
	inClientProxy->UpdateLastPacketTime();

	uint32_t packetType = HandleServerReset( inClientProxy, inInputStream );
	switch ( packetType )
	{
		case kHelloCC:
			DoPreparePacketToSend( inClientProxy, kWelcomeCC );
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
			LOG_INFO << "Unknown packet type received from "
				<< inClientProxy->GetUdpConnection()->peerAddress().toIpPort();
			break;
	}
}

uint32_t NetworkMgr::HandleServerReset( ClientProxyPtr& inClientProxy, InputBitStream& inInputStream )
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
		DoPreparePacketToSend( inClientProxy, kWelcomeCC );
		return kNullCC;
	}
	else
	{
		return packetType;
	}
}

void NetworkMgr::HandleInputPacket( ClientProxyPtr& inClientProxy, InputBitStream& inInputStream )
{
	uint32_t actionCount = 0;
	Action action( customInputStatecb_ ? customInputStatecb_() : ( new InputState ) );
	inInputStream.Read( actionCount, ACTION_COUNT_NUM );

	for ( ; actionCount > 0; --actionCount )
	{
		if ( action.Read( inInputStream ) )
		{
			if ( inClientProxy->GetUnprocessedActionList().AddMoveIfNew( action ) )
			{
				inClientProxy->SetIsLastMoveTimestampDirty( true );
			}
		}
	}
}

void NetworkMgr::WriteLastMoveTimestampIfDirty( OutputBitStream& inOutputStream,
	ClientProxyPtr& inClientProxy )
{
	bool isTimestampDirty = inClientProxy->IsLastMoveTimestampDirty();
	inOutputStream.Write( isTimestampDirty );
	if ( isTimestampDirty )
	{
		inOutputStream.Write( inClientProxy->GetUnprocessedActionList()
			.GetLastMoveTimestamp() );
		inClientProxy->SetIsLastMoveTimestampDirty( false );
	}
}


#else
#include "realtime_srv/net/NetworkMgrWinS.h"
#endif