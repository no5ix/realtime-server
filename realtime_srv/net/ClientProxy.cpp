#include "realtime_srv/net/ClientProxy.h"


using namespace realtime_srv;
using namespace muduo::net;

ClientProxy::ClientProxy(
	const std::shared_ptr<NetworkMgr>& inNetworkManager,
	const int inNetId,
	const pid_t inHoldedByThreadId,
	const UdpConnectionPtr& inUdpConnection )
	:
	connHoldedByTid_( inHoldedByThreadId ),
	networkManager_( inNetworkManager ),
	replicationMgr_( this ),
	netId_( inNetId ),
	deliveryNotifyMgr_( false, true ),
	mIsLastMoveTimestampDirty( false ),
	mLastPacketFromClientTime( 0.f ),
	mTimeToRespawn( 0.f ),
	mRecvingServerResetFlag( false ),
	UdpConnection_( inUdpConnection )
{
	UpdateLastPacketTime();
}


void ClientProxy::SetAllOwnedGameObjsPendingToDie()
{
	for ( auto it = objIdToGameObjMap_.begin();
		it != objIdToGameObjMap_.end(); )
	{
		it++->second->SetPendingToDie();
	}
}


void ClientProxy::RealeaseAllOwnedGameObjs()
{
	for ( auto it = objIdToGameObjMap_.begin();
		it != objIdToGameObjMap_.end(); )
	{
		it++->second->LoseMaster();
	}
}