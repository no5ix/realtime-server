#pragma once

#ifdef IS_LINUX
#include <muduo_udp_support/UdpConnection.h>
#endif //IS_LINUX

namespace realtime_srv
{

class NetworkMgr;

class ClientProxy
{
public:

	int	GetNetId() const { return netId_; }
	const std::string& GetPlayerName()			const { return mPlayerName; }

	void SetInputState( const InputState& inInputState ) { mInputState = inInputState; }
	const InputState& GetInputState()		const { return mInputState; }

	void UpdateLastPacketTime();
	float GetLastPacketFromClientTime()	const { return mLastPacketFromClientTime; }

	DeliveryNotifyMgr&	GetDeliveryNotifyManager() { return deliveryNotifyManager_; }
	ReplicationMgr&	GetReplicationManager() { return replicationManager_; }

	World* GetWorld() const { return world_; }
	void SetWorld( World* const world ) { world_ = world; }

	NetworkMgr* GetNetworkManager() const { return networkManager_; }

	const ActionList& GetUnprocessedActionList() const { return mUnprocessedMoveList; }
	ActionList&	 GetUnprocessedActionList() { return mUnprocessedMoveList; }

	void SetIsLastMoveTimestampDirty( bool inIsDirty ) { mIsLastMoveTimestampDirty = inIsDirty; }
	bool IsLastMoveTimestampDirty()	const { return mIsLastMoveTimestampDirty; }

	bool GetRecvingServerResetFlag() const { return mRecvingServerResetFlag; }
	void SetRecvingServerResetFlag( bool inRecvingServerResetFlag )
	{ mRecvingServerResetFlag = inRecvingServerResetFlag; }

	void SetGameObjStateDirty( int inNetworkId, uint32_t inDirtyState );


private:

	std::string			mPlayerName;
	int							netId_;

	InputState		mInputState;

	float			mLastPacketFromClientTime;
	float			mTimeToRespawn;

	ActionList		mUnprocessedMoveList;
	bool			mIsLastMoveTimestampDirty;

	bool			mRecvingServerResetFlag;

	DeliveryNotifyMgr		deliveryNotifyManager_;
	ReplicationMgr			replicationManager_;
	World*							world_;
	NetworkMgr*					networkManager_;

#ifdef IS_LINUX

public:
	ClientProxy( NetworkMgr* inNetworkManager,
		const std::string& inName,
		const int inNetId,
		const pid_t inHoldedByThreadId,
		const muduo::net::UdpConnectionPtr& inUdpConnection );

	pid_t GetConnHoldedByThreadId() const { return connHoldedByTid_; }
	muduo::net::UdpConnectionPtr& GetUdpConnection() { return UdpConnection_; }

private:
	muduo::net::UdpConnectionPtr	UdpConnection_;
	pid_t													connHoldedByTid_;

#else //IS_LINUX

public:

	ClientProxy( NetworkMgr* inNetworkManager,
		const SockAddrInterf& inSocketAddress,
		const std::string& inName,
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