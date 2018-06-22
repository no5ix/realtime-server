#include "realtime_srv/common/RealtimeSrvShared.h"



#ifdef IS_LINUX
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

#else //IS_LINUX

ClientProxy::ClientProxy(
	const SockAddrInterfc& inSocketAddress,
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
#endif //IS_LINUX

void ClientProxy::UpdateLastPacketTime()
{
	mLastPacketFromClientTime = RealtimeSrvTiming::sInstance.GetCurrentGameTime();
}