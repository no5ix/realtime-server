// class UdpConnection;

class ClientProxy
{
public:

	ClientProxy( 
		const SocketAddrInterface& inSocketAddress, 
		const std::string& inName, 
		int inPlayerId, 
		const UDPSocketPtr& inUDPSocket 
		// ,
		// const std::shared_ptr<UdpConnection>& inUdpConnetction
	);

	const	SocketAddrInterface&	GetSocketAddress()	const { return mSocketAddress; }
	int				GetPlayerId()		const { return mPlayerId; }
	const	std::string&		GetName()			const { return mName; }
	UDPSocketPtr		GetUDPSocket()			const { return mUDPSocket; }

	void			SetInputState( const InputState& inInputState ) { mInputState = inInputState; }
	const	InputState&		GetInputState()		const { return mInputState; }

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

	// std::shared_ptr<UdpConnection> GetUdpConnection() const { return UdpConnetction_; }

private:

	DeliveryNotifyMgr	mDeliveryNotificationManager;
	ReplicationMgr	mReplicationManagerServer;

	SocketAddrInterface	mSocketAddress;
	std::string			mName;
	int				mPlayerId;
	UDPSocketPtr	mUDPSocket;

	//going away!
	InputState		mInputState;

	float			mLastPacketFromClientTime;
	float			mTimeToRespawn;

	ActionList		mUnprocessedMoveList;
	bool			mIsLastMoveTimestampDirty;

	bool			mRecvingServerResetFlag;

	// std::shared_ptr<UdpConnection> UdpConnetction_;
};

typedef shared_ptr< ClientProxy >	ClientProxyPtr;