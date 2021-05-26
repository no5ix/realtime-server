#include "realtime_srv/net/ClientProxy.h"


using namespace realtime_srv;
using namespace muduo::net;

ClientProxy::ClientProxy(
	const std::shared_ptr<NetworkMgr>& networkManager,
	const int netId,
	const pid_t holdedByThreadId,
	const UdpConnectionPtr& udpConnection)
	:
	connHoldedByTid_(holdedByThreadId),
	networkManager_(networkManager),
	replicationMgr_(this),
	netId_(netId),
	deliveryNotifyMgr_(false, true),
	isLastActionTimestampDirty_(false),
	recvingSrvResetFlag_(false),
	UdpConnection_(udpConnection)
{}


void ClientProxy::SetAllOwnedGameObjsPendingToDie()
{
	for (auto it = objIdToGameObjMap_.begin();
		it != objIdToGameObjMap_.end(); )
	{
		it++->second->SetPendingToDie();
	}
}


void ClientProxy::RealeaseAllOwnedGameObjs()
{
	for (auto it = objIdToGameObjMap_.begin();
		it != objIdToGameObjMap_.end(); )
	{
		it++->second->LoseMaster();
	}
}