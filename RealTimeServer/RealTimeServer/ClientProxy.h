class ClientProxy
{
public:

	ClientProxy( const SocketAddrInterface& inSocketAddress, const string& inName, int inPlayerId, const UDPSocketPtr& inUDPSocket );

	const	SocketAddrInterface&	GetSocketAddress()	const { return mSocketAddress; }
	int				GetPlayerId()		const { return mPlayerId; }
	const	string&		GetName()			const { return mName; }
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

private:

	DeliveryNotifyMgr	mDeliveryNotificationManager;
	ReplicationMgr	mReplicationManagerServer;

	SocketAddrInterface	mSocketAddress;
	string			mName;
	int				mPlayerId;
	UDPSocketPtr	mUDPSocket;

	//going away!
	InputState		mInputState;

	float			mLastPacketFromClientTime;
	float			mTimeToRespawn;

	ActionList		mUnprocessedMoveList;
	bool			mIsLastMoveTimestampDirty;

	bool			mRecvingServerResetFlag;


};

typedef shared_ptr< ClientProxy >	ClientProxyPtr;