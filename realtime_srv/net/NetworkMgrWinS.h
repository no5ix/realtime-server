


namespace
{
const float kSendPacketInterval = 0.033f;
const float kClientDisconnectTimeout = 6.f;
const int		kMaxPacketsPerFrameCount = 10;
}

int NetworkMgr::kNewNetId = 1;

NetworkMgr::NetworkMgr( uint16_t inPort /*= DEFAULT_REALTIME_SRV_PORT*/ ) :
	mTimeOfLastStatePacket( 0.f ),
	mLastCheckDCTime( 0.f ),
	mDropPacketChance( 0.f ),
	mSimulatedLatency( 0.f ),
	mSimulateJitter( false ),
	bUnregistObjWhenCliDisconn_( false ),
	isSimilateRealWorld_( false )
{
	UdpSockInterf::StaticInit();

	mSocket = UdpSockInterf::CreateUDPSocket();
	assert( mSocket );

	SockAddrInterf ownAddress( INADDR_ANY, inPort );
	mSocket->Bind( ownAddress );
	mSocket->SetNonBlockingMode( true );

	LOG( "Initializing NetworkManager at port %d", inPort );
}

void NetworkMgr::Start()
{
	while ( true )
	{
		ReadIncomingPacketsIntoQueue();
		ProcessQueuedPackets();
		CheckForDisconnects();
		worldUpdateCb_();
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
					RealtimeSrvTiming::sInst.GetCurrentGameTime() +
					mSimulatedLatency +
					( mSimulateJitter ?
						RealtimeSrvMath::Clamp( RealtimeSrvMath::GetRandomFloat(),
							0.f, mSimulatedLatency ) : 0.f );
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
		if ( RealtimeSrvTiming::sInst.GetCurrentGameTime()
			> nextPacket.GetReceivedTime() )
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
		CheckPacketType( ( *it ).second, inInputStream );
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
			shared_from_this(),
			inFromAddress,
			playerName,
			kNewNetId++,
			inUDPSocket );

		addrToClientMap_[inFromAddress] = newClientProxy;


		if ( newPlayerCb_ )
		{
			GameObjPtr newGameObj( newPlayerCb_( newClientProxy ) );
			assert( newGameObj );

			newGameObj->SetOwner( newClientProxy );
			newGameObj->SetNetworkMgr( shared_from_this() );
			worldRegistryCb_( newGameObj, RA_Create );
			newClientProxy->AddGameObj( newGameObj );
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
			LOG( "SendResetPacket" );
		}
		LOG( "a new client at socket %s, named '%s' as PlayerID %d ",
			inFromAddress.ToString().c_str(),
			newClientProxy->GetPlayerName().c_str(),
			newClientProxy->GetNetId()
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
	float curTime = RealtimeSrvTiming::sInst.GetCurrentGameTime();

	if ( curTime - mLastCheckDCTime < kClientDisconnectTimeout )
	{
		return;
	}
	mLastCheckDCTime = curTime;

	float minAllowedTime = RealtimeSrvTiming::sInst.GetCurrentGameTime()
		- kClientDisconnectTimeout;
	for ( AddrToClientMap::iterator it = addrToClientMap_.begin();
		it != addrToClientMap_.end(); )
	{
		if ( it->second->GetLastPacketFromClientTime() < minAllowedTime )
		{
			LOG( "Player %d disconnect", it->second->GetNetId() );

			if ( bUnregistObjWhenCliDisconn_ )
				for ( auto& obj : it->second->GetAllOwnedGameObjs() )
					worldRegistryCb_( obj, RA_Destroy );
			else
				for ( auto& obj : it->second->GetAllOwnedGameObjs() )
					obj->LoseOwner();

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
	float time = RealtimeSrvTiming::sInst.GetCurrentGameTime();

	if ( time < mTimeOfLastStatePacket + kSendPacketInterval )
	{
		return;
	}
	mTimeOfLastStatePacket = time;

	for ( auto it = addrToClientMap_.begin(), end = addrToClientMap_.end();
		it != end; ++it )
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

void NetworkMgr::SetRepStateDirty( int _objId, uint32_t inDirtyState )
{
	for ( const auto& pair : addrToClientMap_ )
		pair.second->GetReplicationManager().SetReplicationStateDirty(
			_objId, inDirtyState );
}

void NetworkMgr::OnObjCreateOrDestroy( GameObjPtr& inGameObject,
	ReplicationAction inAction )
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
			outputPacket.Write( inClientProxy->GetNetId() );
			outputPacket.Write( kSendPacketInterval );
			break;
		case kStateCC:
			WriteLastMoveTimestampIfDirty( outputPacket, inClientProxy );
		default:
			break;
	}
	inClientProxy->GetReplicationManager().Write( outputPacket, ifp );

	SendPacket( outputPacket, inClientProxy->GetSocketAddress() );
}



void NetworkMgr::CheckPacketType( ClientProxyPtr& inClientProxy, InputBitStream& inInputStream )
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
			LOG( "Unknown packet type received from %s", inClientProxy->GetSocketAddress().ToString().c_str() );
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
		SendGamePacket( inClientProxy, kWelcomeCC );
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