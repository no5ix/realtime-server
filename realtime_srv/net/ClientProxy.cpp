#include "realtime_srv/common/RealtimeSrvShared.h"


using namespace realtime_srv;

#ifdef IS_LINUX

using namespace muduo::net;

ClientProxy::ClientProxy(
	const std::shared_ptr<NetworkMgr>& inNetworkManager,
	const std::string& inPlayerName,
	const int inNetId,
	const pid_t inHoldedByThreadId,
	const UdpConnectionPtr& inUdpConnection )
	:
	connHoldedByTid_( inHoldedByThreadId ),
	networkManager_( inNetworkManager ),
	replicationManager_( this ),
	mPlayerName( inPlayerName ),
	netId_( inNetId ),
	deliveryNotifyManager_( false, true ),
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
	const std::shared_ptr<NetworkMgr>& inNetworkManager,
	const SockAddrInterf& inSocketAddress,
	const std::string& inPlayerName,
	int inNetId,
	const UDPSocketPtr& inUDPSocket )
	:
	networkManager_( inNetworkManager ),
	replicationManager_( this ),
	mSocketAddress( inSocketAddress ),
	mPlayerName( inPlayerName ),
	netId_( inNetId ),
	mUDPSocket( inUDPSocket ),
	deliveryNotifyManager_( false, true ),
	mIsLastMoveTimestampDirty( false ),
	mTimeToRespawn( 0.f ),
	mRecvingServerResetFlag( false )
{
	UpdateLastPacketTime();
}
#endif //IS_LINUX