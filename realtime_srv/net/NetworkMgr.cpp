#include "realtime_srv/game_obj/GameObj.h"
#include "realtime_srv/net/ClientProxy.h"
#include "realtime_srv/rep/BitStream.h"

#include "realtime_srv/net/NetworkMgr.h"


using namespace realtime_srv;


//#ifdef IS_LINUX


using namespace muduo;
using namespace muduo::net;

AtomicInt32 NetworkMgr::kNewNetId;

NetworkMgr::NetworkMgr( const ServerConfig _serverConfig ) :
	bUnregistObjWhenCliDisconn_( _serverConfig.is_unregist_obj_when_cli_disconn ),
	actionCountPerTick_( _serverConfig.action_count_per_tick ),
	pktHandler_( _serverConfig,
		std::bind( &NetworkMgr::DoProcessPkt, this, _1 ),
		std::bind( &NetworkMgr::Tick, this ),
		std::bind( &NetworkMgr::CheckForDisconnects, this ) )
{
	assert( actionCountPerTick_ >= 1 );
	kNewNetId.getAndSet( 1 );
}

void realtime_srv::NetworkMgr::Start()
{

	//assert( newPlayerCb_ );
	//assert( customInputStatecb_ );

	assert( letCliProxyGetWorldState_ );
	assert( worldRegistryCb_ );
	assert( worldUpdateCb_ );

	pktHandler_.Start();
}

void NetworkMgr::DoPreparePacketToSend( ClientProxyPtr& _cliProxy, const uint32_t _connFlag )
{
	OutputBitStreamPtr outputPacket( new OutputBitStream() );
	outputPacket->Write( _connFlag );

	InflightPacket* ifp = _cliProxy->GetDeliveryNotifyMgr()
		.WriteState( *outputPacket, _cliProxy.get() );

	switch ( _connFlag )
	{
		case kResetCC:
			_cliProxy->SetRecvingServerResetFlag( true );
			LOG_INFO << "Send Reset Packet";
		case kWelcomeCC:
			outputPacket->Write( _cliProxy->GetNetId() );
			outputPacket->Write( pktHandler_.GetSendPacketInterval() );
			break;
		case kStateCC:
			WriteLastMoveTimestampIfDirty( *outputPacket, _cliProxy );
		default:
			break;
	}
	_cliProxy->GetReplicationMgr().Write( *outputPacket, ifp );

	pktHandler_.AddToPendingSndPktQ(
		PendingSendPacketPtr( new PendingSendPacket(
			outputPacket, _cliProxy->GetUdpConnection() ) ),
		_cliProxy->GetConnHoldedByThreadId() );
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
		CheckPacketType( ( *it ).second, *recvedPacket->GetPacketBuffer() );
}

void NetworkMgr::PreparePacketToSend()
{
	for ( auto& pair : udpConnToClientMap_ )
	{
		( pair.second )->GetDeliveryNotifyMgr().ProcessTimedOutPackets();

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
		RealtimeSrvTiming::sInst.GetCurrentGameTime() - pktHandler_.GetClientDisconnectTimeout();

	for ( auto it = udpConnToClientMap_.begin(); it != udpConnToClientMap_.end(); )
	{
		if ( it->second->GetLastPacketFromClientTime() < minAllowedTime )
		{
			if ( bUnregistObjWhenCliDisconn_ )
				it->second->SetAllOwnedGameObjsPendingToDie();
			else
				it->second->RealeaseAllOwnedGameObjs();

			it->second->GetUdpConnection()->forceClose();
			udpConnToClientMap_.erase( it++ );
		}
		else
			++it;
	}
}

ClientProxyPtr NetworkMgr::CreateNewClient(
	const UdpConnectionPtr& _udpConnetction, const pid_t _holdedByThreadId )
{
	ClientProxyPtr newClientProxy = std::make_shared<ClientProxy>(
		shared_from_this(),
		kNewNetId.getAndAdd( 1 ),
		_holdedByThreadId,
		_udpConnetction );

	udpConnToClientMap_[_udpConnetction] = newClientProxy;

	letCliProxyGetWorldState_( newClientProxy );

	if ( newPlayerCb_ )
	{
		GameObjPtr newControlledGameObj( newPlayerCb_( newClientProxy ) );
		if ( newControlledGameObj )
		{
			worldRegistryCb_( newControlledGameObj, RA_Create );
			newControlledGameObj->SetMaster( newClientProxy );
		}
	}
	return newClientProxy;
}

void NetworkMgr::WelcomeNewClient( InputBitStream& _inputStream,
	const UdpConnectionPtr& _udpConnetction, const pid_t _holdedByThreadId )
{
	uint32_t	packetType;
	_inputStream.Read( packetType );

	switch ( packetType )
	{
		case kHelloCC:
		case kInputCC:
		case kResetedCC:
		{
			ClientProxyPtr newClientProxy =
				CreateNewClient( _udpConnetction, _holdedByThreadId );
			if ( packetType == kHelloCC )
				DoPreparePacketToSend( newClientProxy, kWelcomeCC );
			else
				DoPreparePacketToSend( newClientProxy, kResetCC );
			LOG_INFO << "a new client as Net ID " << newClientProxy->GetNetId();
			break;
		}
		default:
			LOG_INFO << "Bad incoming packet from unknown client at socket "
				<< _udpConnetction->peerAddress().toIpPort() << " is "
				<< " - we're under attack!!";
			break;
	}
}

void NetworkMgr::OnObjCreateOrDestory( GameObjPtr& inGameObject, ReplicationAction inAction )
{
	if ( inAction == RA_Create )
		inGameObject->SetNetworkMgr( shared_from_this() );

	for ( const auto& pair : udpConnToClientMap_ )
	{
		switch ( inAction )
		{
			case RA_Create:
				pair.second->GetReplicationMgr().ReplicateCreate(
					inGameObject->GetObjId(), inGameObject->GetAllStateMask() );
				break;
			case RA_Destroy:
				pair.second->GetReplicationMgr().ReplicateDestroy(
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
		pair.second->GetReplicationMgr().SetReplicationStateDirty(
			_objId, inDirtyState );
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
			if ( inClientProxy->GetDeliveryNotifyMgr().ReadAndProcessState( inInputStream ) )
				HandleInputPacket( inClientProxy, inInputStream );
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
		return packetType;
}

void NetworkMgr::HandleInputPacket( ClientProxyPtr& inClientProxy, InputBitStream& inInputStream )
{
	uint32_t actionCount = 0;
	Action action( customInputStatecb_ ? customInputStatecb_() : ( new InputState ) );
	inInputStream.Read( actionCount, actionCountPerTick_ );

	for ( ; actionCount > 0; --actionCount )
		if ( action.Read( inInputStream ) )
			if ( inClientProxy->GetUnprocessedActionList().AddMoveIfNew( action ) )
				inClientProxy->SetIsLastMoveTimestampDirty( true );
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


//#else
//#include "realtime_srv/net/NetworkMgrWinS.h"
//#endif