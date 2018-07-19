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
	typedef std::unordered_map<int, GameObjPtr> ObjIdToGameObjMap;

	int	GetNetId() const { return netId_; }

	void AddGameObj( const GameObjPtr& _gameObj )
	{ objIdToGameObjMap_[_gameObj->GetObjId()] = _gameObj; }

	ObjIdToGameObjMap& GetAllOwnedGameObjs() { return objIdToGameObjMap_; }
	const ObjIdToGameObjMap& GetAllOwnedGameObjs() const { return objIdToGameObjMap_; }

	void SetAllOwnedGameObjsPendingToDie();
	void RealeaseAllOwnedGameObjs();

	void RealeaseSpecificOwnedGameObj( int _objId )
	{ objIdToGameObjMap_.erase( _objId ); }

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

	ObjIdToGameObjMap objIdToGameObjMap_;

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