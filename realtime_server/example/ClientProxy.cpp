#include "RealTimeSrvPCH.h"



#ifdef NEW_EPOLL_INTERFACE
ClientProxy::ClientProxy( const std::string& inName, 
	int inPlayerId, 
	const UdpConnectionPtr& inUdpConnection )
	:
	mName( inName ),
	mPlayerId( inPlayerId ),
	mDeliveryNotificationManager( false, true ),
	mIsLastMoveTimestampDirty( false ),
	mLastPacketFromClientTime( 0.f ),
	mTimeToRespawn( 0.f ),
	mRecvingServerResetFlag( false ),
	UdpConnection_( inUdpConnection )
{
	UpdateLastPacketTime();
}

#else //NEW_EPOLL_INTERFACE

ClientProxy::ClientProxy(
	const SocketAddrInterface& inSocketAddress,
	const std::string& inName,
	int inPlayerId,
	const UDPSocketPtr& inUDPSocket
	// ,
	// const std::shared_ptr<UdpConnection>& inUdpConnetction
) :
	mSocketAddress( inSocketAddress ),
	mName( inName ),
	mPlayerId( inPlayerId ),
	mUDPSocket( inUDPSocket ),
	mDeliveryNotificationManager( false, true ),
	mIsLastMoveTimestampDirty( false ),
	mTimeToRespawn( 0.f ),
	mRecvingServerResetFlag( false )
	// ,
	// UdpConnetction_( inUdpConnetction )
{
	UpdateLastPacketTime();
}
#endif //NEW_EPOLL_INTERFACE

void ClientProxy::UpdateLastPacketTime()
{
	mLastPacketFromClientTime = RealTimeSrvTiming::sInstance.GetCurrentGameTime();
}