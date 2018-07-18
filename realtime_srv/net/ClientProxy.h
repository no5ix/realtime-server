#pragma once

#ifdef IS_LINUX
#include <muduo/base/Logging.h>
#include <muduo_udp_support/UdpConnection.h>
#endif //IS_LINUX

namespace realtime_srv
{

class NetworkMgr;

class ClientProxy
{
public:

	int	GetNetId() const { return netId_; }

	void AddGameObj( const GameObjPtr& _gameObj )
	{ ownedGameObjs_.push_back( _gameObj ); }

	std::vector<GameObjPtr>& GetAllOwnedGameObjs() { return ownedGameObjs_; }
	const std::vector<GameObjPtr>& GetAllOwnedGameObjs() const { return ownedGameObjs_; }

	void UpdateLastPacketTime()
	{ mLastPacketFromClientTime = RealtimeSrvTiming::sInst.GetCurrentGameTime(); }

	float GetLastPacketFromClientTime()	const { return mLastPacketFromClientTime; }

	DeliveryNotifyMgr&	GetDeliveryNotifyManager() { return deliveryNotifyManager_; }
	ReplicationMgr&	GetReplicationManager() { return replicationManager_; }

	WorldPtr& GetWorld() { return world_; }
	void SetWorld( WorldPtr  world ) { world_ = world; }

	std::shared_ptr<NetworkMgr>& GetNetworkManager() { return networkManager_; }

	const ActionList& GetUnprocessedActionList() const { return mUnprocessedMoveList; }
	ActionList&	 GetUnprocessedActionList() { return mUnprocessedMoveList; }

	void SetIsLastMoveTimestampDirty( bool inIsDirty )
	{ mIsLastMoveTimestampDirty = inIsDirty; }
	bool IsLastMoveTimestampDirty()	const
	{ return mIsLastMoveTimestampDirty; }

	bool GetRecvingServerResetFlag() const
	{ return mRecvingServerResetFlag; }
	void SetRecvingServerResetFlag( bool inRecvingServerResetFlag )
	{ mRecvingServerResetFlag = inRecvingServerResetFlag; }


private:

	int							netId_;
	std::vector<GameObjPtr> ownedGameObjs_;

	float			mLastPacketFromClientTime;
	float			mTimeToRespawn;

	ActionList		mUnprocessedMoveList;
	bool			mIsLastMoveTimestampDirty;

	bool			mRecvingServerResetFlag;

	DeliveryNotifyMgr		deliveryNotifyManager_;
	ReplicationMgr			replicationManager_;
	WorldPtr							world_;
	std::shared_ptr<NetworkMgr>					networkManager_;

#ifdef IS_LINUX

public:
	ClientProxy( const std::shared_ptr<NetworkMgr>& inNetworkManager,
		const int inNetId,
		const pid_t inHoldedByThreadId,
		const muduo::net::UdpConnectionPtr& inUdpConnection );

	~ClientProxy() { LOG_INFO << "Netid " << netId_ << " disconnect"; }

	pid_t GetConnHoldedByThreadId() const { return connHoldedByTid_; }
	muduo::net::UdpConnectionPtr& GetUdpConnection() { return UdpConnection_; }

private:
	muduo::net::UdpConnectionPtr	UdpConnection_;
	pid_t													connHoldedByTid_;

#else //IS_LINUX

public:

	ClientProxy( const std::shared_ptr<NetworkMgr>& inNetworkManager,
		const SockAddrInterf& inSocketAddress,
		int inNetId,
		const UDPSocketPtr& inUDPSocket );

	const SockAddrInterf& GetSocketAddress() const { return mSocketAddress; }
	UDPSocketPtr GetUDPSocket() const { return mUDPSocket; }

private:

	SockAddrInterf	mSocketAddress;
	UDPSocketPtr	mUDPSocket;

#endif //IS_LINUX

};

typedef shared_ptr< ClientProxy >	ClientProxyPtr;
typedef weak_ptr<ClientProxy>	ClientProxyWPtr;

}