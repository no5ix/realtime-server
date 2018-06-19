#ifdef NEW_EPOLL_INTERFACE
#include <net/UdpConnection.h>

class UdpConnection;

using namespace muduo;
using namespace muduo::net;
#endif //NEW_EPOLL_INTERFACE

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
	ReplicationMgr&		GetReplicationManagerServer() { return mReplicationManagerServer; }

 	const	ActionList&				GetUnprocessedMoveList() const { return mUnprocessedMoveList; }
 	ActionList&				GetUnprocessedMoveList() { return mUnprocessedMoveList; }

	void	SetIsLastMoveTimestampDirty( bool inIsDirty ) { mIsLastMoveTimestampDirty = inIsDirty; }
	bool	IsLastMoveTimestampDirty()						const { return mIsLastMoveTimestampDirty; }

	bool	GetRecvingServerResetFlag() const { return mRecvingServerResetFlag; }
	void	SetRecvingServerResetFlag(bool inRecvingServerResetFlag)		 { mRecvingServerResetFlag = inRecvingServerResetFlag; }

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


#ifdef NEW_EPOLL_INTERFACE

public:
	ClientProxy(
		const std::string& inName,
		int inPlayerId,
		const UdpConnectionPtr& inUdpConnetction );

	UdpConnectionPtr GetUdpConnection() const { return UdpConnetction_; }
private:
	UdpConnectionPtr UdpConnetction_;

#else //NEW_EPOLL_INTERFACE

public:

	ClientProxy(
		const SocketAddrInterface& inSocketAddress,
		const std::string& inName,
		int inPlayerId,
		const UDPSocketPtr& inUDPSocket
		// ,
		// const std::shared_ptr<UdpConnection>& inUdpConnetction
	);

	const SocketAddrInterface& GetSocketAddress() const { return mSocketAddress; }
	UDPSocketPtr GetUDPSocket() const { return mUDPSocket; }

private:

	SocketAddrInterface	mSocketAddress;
	UDPSocketPtr	mUDPSocket;

#endif //NEW_EPOLL_INTERFACE

};

typedef shared_ptr< ClientProxy >	ClientProxyPtr;