#ifdef IS_LINUX
#include <muduo_udp_support/UdpConnection.h>

class UdpConnection;

using namespace muduo;
using namespace muduo::net;
#endif //IS_LINUX

class ClientProxy
{
public:

	int	GetPlayerId() const { return mPlayerId; }
	const std::string& GetName()			const { return mName; }

	void SetInputState( const InputState& inInputState ) { mInputState = inInputState; }
	const InputState& GetInputState()		const { return mInputState; }

	void UpdateLastPacketTime();
	float GetLastPacketFromClientTime()	const { return mLastPacketFromClientTime; }

	DeliveryNotifyMgr&	GetDeliveryNotifyManager() { return DeliveryNotifyManager_; }
	ReplicationMgr&	GetReplicationManager() { return ReplicationManager_; }

	World* GetWorld() const { return world_; }
	void SetWorld( World* const world ) { world_ = world; }

	NetworkMgr* GetNetworkManager() const { return networkManager_; }

	const ActionList& GetUnprocessedActionList() const { return mUnprocessedMoveList; }
	ActionList&	 GetUnprocessedActionList() { return mUnprocessedMoveList; }

	void SetIsLastMoveTimestampDirty( bool inIsDirty )
	{ mIsLastMoveTimestampDirty = inIsDirty; }
	bool IsLastMoveTimestampDirty()	const { return mIsLastMoveTimestampDirty; }

	bool GetRecvingServerResetFlag() const { return mRecvingServerResetFlag; }
	void SetRecvingServerResetFlag( bool inRecvingServerResetFlag )
	{ mRecvingServerResetFlag = inRecvingServerResetFlag; }

	void SetGameObjStateDirty( int inNetworkId, uint32_t inDirtyState );

private:

	std::string			mName;
	int				mPlayerId;

	InputState		mInputState;

	float			mLastPacketFromClientTime;
	float			mTimeToRespawn;

	ActionList		mUnprocessedMoveList;
	bool			mIsLastMoveTimestampDirty;

	bool			mRecvingServerResetFlag;

	DeliveryNotifyMgr		DeliveryNotifyManager_;
	ReplicationMgr			ReplicationManager_;
	World*					world_;
	NetworkMgr*				networkManager_;

#ifdef IS_LINUX

public:
	ClientProxy( NetworkMgr* inNetworkManager,
		const std::string& inName,
		int inPlayerId,
		const UdpConnectionPtr& inUdpConnection );

	UdpConnectionPtr GetUdpConnection() const { return UdpConnection_; }
private:
	UdpConnectionPtr UdpConnection_;

#else //IS_LINUX

public:

	ClientProxy( NetworkMgr* inNetworkManager,
		const SockAddrInterfc& inSocketAddress,
		const std::string& inName,
		int inPlayerId,
		const UDPSocketPtr& inUDPSocket );

	const SockAddrInterfc& GetSocketAddress() const { return mSocketAddress; }
	UDPSocketPtr GetUDPSocket() const { return mUDPSocket; }

private:

	SockAddrInterfc	mSocketAddress;
	UDPSocketPtr	mUDPSocket;

#endif //IS_LINUX

};

typedef shared_ptr< ClientProxy >	ClientProxyPtr;
typedef weak_ptr<ClientProxy>	ClientProxyWPtr;