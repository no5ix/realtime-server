#ifdef IS_LINUX
#include <muduo_udp_support/UdpConnection.h>

class UdpConnection;

using namespace muduo;
using namespace muduo::net;
#endif //IS_LINUX

class ClientProxy
{
public:

	int	GetPlayerId()		const { return mPlayerId; }
	const std::string&		GetName()			const { return mName; }
	void SetInputState( const InputState& inInputState ) { mInputState = inInputState; }
	const InputState&		GetInputState()		const { return mInputState; }

	void			UpdateLastPacketTime();
	float			GetLastPacketFromClientTime()	const { return mLastPacketFromClientTime; }

	DeliveryNotifyMgr&	GetDeliveryNotificationManager() { return mDeliveryNotificationManager; }
	ReplicationMgr&		GetReplicationManager() { return mReplicationManagerServer; }

	const	ActionList&				GetUnprocessedMoveList() const { return mUnprocessedMoveList; }
	ActionList&				GetUnprocessedMoveList() { return mUnprocessedMoveList; }

	void	SetIsLastMoveTimestampDirty( bool inIsDirty ) { mIsLastMoveTimestampDirty = inIsDirty; }
	bool	IsLastMoveTimestampDirty()						const { return mIsLastMoveTimestampDirty; }

	bool	GetRecvingServerResetFlag() const { return mRecvingServerResetFlag; }
	void	SetRecvingServerResetFlag( bool inRecvingServerResetFlag ) { mRecvingServerResetFlag = inRecvingServerResetFlag; }

private:

	std::string			mName;
	int				mPlayerId;

	DeliveryNotifyMgr	mDeliveryNotificationManager;
	ReplicationMgr	mReplicationManagerServer;

	InputState		mInputState;

	float			mLastPacketFromClientTime;
	float			mTimeToRespawn;

	ActionList		mUnprocessedMoveList;
	bool			mIsLastMoveTimestampDirty;

	bool			mRecvingServerResetFlag;


#ifdef IS_LINUX

public:
	ClientProxy(
		const std::string& inName,
		int inPlayerId,
		const UdpConnectionPtr& inUdpConnection );

	UdpConnectionPtr GetUdpConnection() const { return UdpConnection_; }
private:
	UdpConnectionPtr UdpConnection_;

#else //IS_LINUX

public:

	ClientProxy(
		const SockAddrInterfc& inSocketAddress,
		const std::string& inName,
		int inPlayerId,
		const UDPSocketPtr& inUDPSocket
		// ,
		// const std::shared_ptr<UdpConnection>& inUdpConnection	
	);

	const SockAddrInterfc& GetSocketAddress() const { return mSocketAddress; }
	UDPSocketPtr GetUDPSocket() const { return mUDPSocket; }

private:

	SockAddrInterfc	mSocketAddress;
	UDPSocketPtr	mUDPSocket;

#endif //IS_LINUX

};

typedef shared_ptr< ClientProxy >	ClientProxyPtr;