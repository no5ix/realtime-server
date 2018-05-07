class ClientProxy
{
public:

	ClientProxy( const SocketAddressInterface& inSocketAddress, const string& inName, int inPlayerId );

	const	SocketAddressInterface&	GetSocketAddress()	const { return mSocketAddress; }
	int				GetPlayerId()		const { return mPlayerId; }
	const	string&		GetName()			const { return mName; }

	void			SetInputState( const InputState& inInputState ) { mInputState = inInputState; }
	const	InputState&		GetInputState()		const { return mInputState; }

	void			UpdateLastPacketTime();
	float			GetLastPacketFromClientTime()	const { return mLastPacketFromClientTime; }

	DeliveryNotificationMgr&	GetDeliveryNotificationManager() { return mDeliveryNotificationManager; }
	ReplicationMgr&		GetReplicationManagerServer() { return mReplicationManagerServer; }

 	const	ActionList&				GetUnprocessedMoveList() const { return mUnprocessedMoveList; }
 	ActionList&				GetUnprocessedMoveList() { return mUnprocessedMoveList; }

	void	SetIsLastMoveTimestampDirty( bool inIsDirty ) { mIsLastMoveTimestampDirty = inIsDirty; }
	bool	IsLastMoveTimestampDirty()						const { return mIsLastMoveTimestampDirty; }


private:

	DeliveryNotificationMgr	mDeliveryNotificationManager;
	ReplicationMgr	mReplicationManagerServer;

	SocketAddressInterface	mSocketAddress;
	string			mName;
	int				mPlayerId;

	//going away!
	InputState		mInputState;

	float			mLastPacketFromClientTime;
	float			mTimeToRespawn;

	ActionList		mUnprocessedMoveList;
	bool			mIsLastMoveTimestampDirty;



};

typedef shared_ptr< ClientProxy >	ClientProxyPtr;