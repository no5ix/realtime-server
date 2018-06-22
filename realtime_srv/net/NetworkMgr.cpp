//#include "realtime_srv/common/RealTimeSrvShared.h"
//
//
//
//
//#ifdef IS_LINUX
//
//AtomicInt32 NetworkMgr::kNewNetworkId;
//
//NetworkMgr::NetworkMgr() :
//	mNetworkIdToGameObjectMap( new IntToGameObjectMap )
//{
//	kNewNetworkId.getAndSet( 1 );
//}
//
//void NetworkMgr::SendPacket( const OutputBitStream& inOutputStream, const UdpConnectionPtr& conn )
//{
//	conn->send(
//		inOutputStream.GetBufferPtr(), inOutputStream.GetByteLength() );
//}
//
//void NetworkMgr::Start()
//{
//	server_->start();
//	loop_.loop();
//}
//
//void NetworkMgr::threadInit( EventLoop* loop )
//{
//	MutexLockGuard lock( mutex_ );
//	loops_.insert( loop );
//}
//
//IntToGameObjectMapPtr NetworkMgr::GetNetworkIdToGameObjectMap()
//{
//	MutexLockGuard lock( mutex_ );
//	return mNetworkIdToGameObjectMap;
//}
//
//void NetworkMgr::NetworkIdToGameObjectMapCOW()
//{
//	if ( !mNetworkIdToGameObjectMap.unique() )
//	{
//		mNetworkIdToGameObjectMap.reset( new IntToGameObjectMap( *mNetworkIdToGameObjectMap ) );
//	}
//	assert( mNetworkIdToGameObjectMap.unique() );
//}
//
//int NetworkMgr::GetNewNetworkId()
//{
//	int toRet = kNewNetworkId.getAndAdd( 1 );
//	if ( kNewNetworkId.get() < toRet )
//	{
//		LOG( "Network ID Wrap Around!!! You've been playing way too long...", 0 );
//	}
//
//	return toRet;
//}
//
//EntityPtr NetworkMgr::GetGameObject( int inNetworkId )
//{
//	auto tempNetworkIdToGameObjectMap = GetNetworkIdToGameObjectMap();
//	auto gameObjectIt = tempNetworkIdToGameObjectMap->find( inNetworkId );
//	if ( gameObjectIt != tempNetworkIdToGameObjectMap->end() )
//	{
//		return gameObjectIt->second;
//	}
//	else
//	{
//		return EntityPtr();
//	}
//}
//
////int NetworkMgr::RegistEntityAndRetNetID( EntityPtr inGameObject )
////{
////	int newNetworkId = GetNewNetworkId();
////	inGameObject->SetNetworkId( newNetworkId );
////	{
////		MutexLockGuard lock( mutex_ );
////		NetworkIdToGameObjectMapCOW();
////		( *mNetworkIdToGameObjectMap )[newNetworkId] = inGameObject;
////	}
////	return newNetworkId;
////	//UdpConnToClientMapPtr tempUdpConnToClientMap = GetUdpConnToClientMap();
////	//for ( const auto& pair : *tempUdpConnToClientMap )
////	//{
////	//	pair.second->GetReplicationManager().ReplicateCreate( newNetworkId, inGameObject->GetAllStateMask() );
////	//}
////}
//
//int NetworkMgr::UnregistEntityAndRetNetID( Entity* inGameObject )
//{
//	int networkId = inGameObject->GetNetworkId();
//	{
//		MutexLockGuard lock( mutex_ );
//		NetworkIdToGameObjectMapCOW();
//		mNetworkIdToGameObjectMap->erase( networkId );
//	}
//	return networkId;
//
//	//UdpConnToClientMapPtr tempUdpConnToClientMap = GetUdpConnToClientMap();
//	//for ( const auto& pair : *tempUdpConnToClientMap )
//	//{
//	//	pair.second->GetReplicationManager().ReplicateDestroy( networkId );
//	//}
//}
//
////void NetworkMgr::onConnection( const UdpConnectionPtr& conn )
////{
////	LOG_INFO << conn->localAddress().toIpPort() << " -> "
////		<< conn->peerAddress().toIpPort() << " is "
////		<< ( conn->connected() ? "UP" : "DOWN" );
////}
//
//void NetworkMgr::onMessage( const muduo::net::UdpConnectionPtr& conn,
//	muduo::net::Buffer* buf,
//	muduo::Timestamp receiveTime )
//{
//	if ( buf->readableBytes() > 0 ) // kHeaderLen == 0
//	{
//		InputBitStream inputStream( buf->peek(), buf->readableBytes() * 8 );
//		buf->retrieveAll();
//
//		ProcessPacket( inputStream, conn );
//		CheckForDisconnects();
//		WorldUpdateCB_();
//		SendOutgoingPackets();
//	}
//}
//
//bool NetworkMgr::Init( uint16_t inPort )
//{
//	InetAddress serverAddr( inPort );
//	server_.reset( new UdpServer( &loop_, serverAddr, "realtime_srv" ) );
//
//	server_->setConnectionCallback(
//		std::bind( &NetworkMgr::onConnection, this, _1 ) );
//	server_->setMessageCallback(
//		std::bind( &NetworkMgr::onMessage, this, _1, _2, _3 ) );
//
//	server_->setThreadNum( THREAD_NUM );
//	server_->setThreadInitCallback( std::bind( &NetworkMgr::threadInit, this, _1 ) );
//
//	return true;
//}
//
//#else //IS_LINUX
//
//int NetworkMgr::kNewNetworkId = 1;
//
//NetworkMgr::NetworkMgr() :
//	mDropPacketChance( 0.f ),
//	mSimulatedLatency( 0.f ),
//	mWhetherToSimulateJitter( false )
//{}
//
//void NetworkMgr::ProcessIncomingPackets()
//{
//	ReadIncomingPacketsIntoQueue();
//
//	ProcessQueuedPackets();
//
//	//UpdateBytesSentLastFrame();
//}
//
//void NetworkMgr::ReadIncomingPacketsIntoQueue()
//{
//	char packetMem[MAX_PACKET_BYTE_LENGTH];
//	int packetSize = sizeof( packetMem );
//	SocketAddrInterface fromAddress;
//
//	int receivedPackedCount = 0;
//	int totalReadByteCount = 0;
//
//	while ( receivedPackedCount < kMaxPacketsPerFrameCount )
//	{
//		int readByteCount = mSocket->ReceiveFrom( packetMem, packetSize, fromAddress );
//		if ( readByteCount == 0 )
//		{
//			//LOG( "ReadIncomingPacketsIntoQueue readByteCount = %d ", 0 );
//			break;
//		}
//		else if ( readByteCount == -WSAECONNRESET )
//		{
//			HandleConnectionReset( fromAddress );
//		}
//		else if ( readByteCount > 0 )
//		{
//			InputBitStream inputStream( packetMem, readByteCount * 8 );
//			++receivedPackedCount;
//			totalReadByteCount += readByteCount;
//
//			if ( RealTimeSrvMath::GetRandomFloat() >= mDropPacketChance )
//			{
//				float simulatedReceivedTime =
//					RealTimeSrvTiming::sInstance.GetCurrentGameTime() +
//					mSimulatedLatency +
//					( GetIsSimulatedJitter() ?
//						RealTimeSrvMath::Clamp( RealTimeSrvMath::GetRandomFloat(), 0.f, 0.06f ) : 0.f );
//				mPacketQueue.emplace( simulatedReceivedTime, inputStream, fromAddress );
//			}
//			else
//			{
//				//LOG( "Dropped packet!", 0 );
//			}
//		}
//		else
//		{
//		}
//	}
//
//	if ( totalReadByteCount > 0 )
//	{
//		//mBytesReceivedPerSecond.UpdatePerSecond( static_cast< float >( totalReadByteCount ) );
//	}
//}
//
//int NetworkMgr::GetNewNetworkId()
//{
//	int toRet = kNewNetworkId++;
//	if ( kNewNetworkId < toRet )
//	{
//		LOG( "Network ID Wrap Around!!! You've been playing way too long...", 0 );
//	}
//
//	return toRet;
//}
//
//void NetworkMgr::ProcessQueuedPackets()
//{
//	while ( !mPacketQueue.empty() )
//	{
//		ReceivedPacket& nextPacket = mPacketQueue.front();
//		if ( RealTimeSrvTiming::sInstance.GetCurrentGameTime() > nextPacket.GetReceivedTime() )
//		{
//			ProcessPacket(
//				nextPacket.GetPacketBuffer(),
//				nextPacket.GetFromAddress(),
//				nextPacket.GetUDPSocket()
//				// , 
//				// nextPacket.GetUdpConnection() 
//			);
//			mPacketQueue.pop();
//		}
//		else
//		{
//			break;
//		}
//	}
//}
//
//bool NetworkMgr::Init( uint16_t inPort )
//{
//#if _WIN32
//	UDPSocketInterface::StaticInit();
//#endif //_WIN32
//
//	mSocket = UDPSocketInterface::CreateUDPSocket();
//
//	if ( mSocket == nullptr )
//	{
//		return false;
//	}
//
//	SocketAddrInterface ownAddress( INADDR_ANY, inPort );
//	mSocket->Bind( ownAddress );
//
//	LOG( "Initializing NetworkManager at port %d", inPort );
//
//	if ( mSocket->SetNonBlockingMode( true ) != NO_ERROR )
//	{
//		return false;
//	}
//
//	return true;
//}
//
//void NetworkMgr::SendPacket( const OutputBitStream& inOutputStream,
//	const SocketAddrInterface& inSockAddr )
//{
//	if ( RealTimeSrvMath::GetRandomFloat() < mDropPacketChance )
//	{
//		return;
//	}
//
//	int sentByteCount = mSocket->SendTo( inOutputStream.GetBufferPtr(), 
//		inOutputStream.GetByteLength(), 
//		inSockAddr );
//}
//
//void NetworkMgr::HandleConnectionReset( const SocketAddrInterface& inFromAddress )
//{
//	//auto it = mAddressToClientMap.find( inFromAddress );
//	//if ( it != mAddressToClientMap.end() )
//	//{
//	//	mPlayerIdToClientMap.erase( it->second->GetPlayerId() );
//	//	mAddressToClientMap.erase( it );
//	//}
//}
//
////int NetworkMgr::RegistEntityAndRetNetID( EntityPtr inGameObject )
////{
////	int newNetworkId = GetNewNetworkId();
////	inGameObject->SetNetworkId( newNetworkId );
////
////	mNetworkIdToGameObjectMap[newNetworkId] = inGameObject;
////	return newNetworkId;
////
////	//for ( const auto& pair : mAddressToClientMap )
////	//{
////	//	pair.second->GetReplicationManager().ReplicateCreate( newNetworkId, inGameObject->GetAllStateMask() );
////	//}
////}
////
////int NetworkMgr::UnregistEntityAndRetNetID( Entity* inGameObject )
////{
////	int networkId = inGameObject->GetNetworkId();
////	mNetworkIdToGameObjectMap.erase( networkId );
////	return networkId;
////
////	//for ( const auto& pair : mAddressToClientMap )
////	//{
////	//	pair.second->GetReplicationManager().ReplicateDestroy( networkId );
////	//}
////}
//
//EntityPtr NetworkMgr::GetGameObject( int inNetworkId )
//{
//	auto gameObjectIt = mNetworkIdToGameObjectMap.find( inNetworkId );
//	if ( gameObjectIt != mNetworkIdToGameObjectMap.end() )
//	{
//		return gameObjectIt->second;
//	}
//	else
//	{
//		return EntityPtr();
//	}
//}
//#endif //IS_LINUX