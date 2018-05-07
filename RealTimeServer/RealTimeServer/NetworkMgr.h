typedef unordered_map< int, GameObjectPtr > IntToGameObjectMap;

class NetworkMgr
{
public:
	static const uint32_t	kHelloCC = 'HELO';
	static const uint32_t	kWelcomeCC = 'WLCM';
	static const uint32_t	kStateCC = 'STAT';
	static const uint32_t	kInputCC = 'INPT';
	static const int		kMaxPacketsPerFrameCount = 10;

	NetworkMgr();
	virtual ~NetworkMgr();

	bool	Init( uint16_t inPort );
	void	WaitForIncomingPackets();
	void	msg_process( UDPSocketPtr inUDPSocketPtr, SocketAddressInterface infromAddress );
	void	ProcessIncomingPackets();

	virtual void	ProcessPacket( InputBitStream& inInputStream, const SocketAddressInterface& inFromAddress ) = 0;
	virtual void HandleConnectionReset( const SocketAddressInterface& inFromAddress ) {}

	void	SendPacket( const OutputBitStream& inOutputStream, const SocketAddressInterface& inFromAddress );

	void	SetDropPacketChance( float inChance ) { mDropPacketChance = inChance; }
	void	SetSimulatedLatency( float inLatency ) { mSimulatedLatency = inLatency; }

	void	SetIsSimulatedJitter( bool inIsSimulatedJitter ) { mIsSimulatedJitter = inIsSimulatedJitter; }
	bool	GetIsSimulatedJitter() const { return mIsSimulatedJitter; }

	inline	GameObjectPtr	GetGameObject( int inNetworkId ) const;

protected:

	IntToGameObjectMap		mNetworkIdToGameObjectMap;

private:

	class ReceivedPacket
	{
	public:
		ReceivedPacket( float inReceivedTime, InputBitStream& ioInputMemoryBitStream, const SocketAddressInterface& inFromAddress ) :
		mReceivedTime( inReceivedTime ),
		mFromAddress( inFromAddress ),
		mPacketBuffer( ioInputMemoryBitStream )
		{
		}

		const	SocketAddressInterface&			GetFromAddress()	const { return mFromAddress; }
		float					GetReceivedTime()	const { return mReceivedTime; }
		InputBitStream&	GetPacketBuffer() { return mPacketBuffer; }

	private:

		float					mReceivedTime;
		InputBitStream	mPacketBuffer;
		SocketAddressInterface			mFromAddress;

	};

	//void	UpdateBytesSentLastFrame();
	void	ReadIncomingPacketsIntoQueue();
	void	ProcessQueuedPackets();

	queue< ReceivedPacket, list< ReceivedPacket > >	mPacketQueue;

	UDPSocketPtr	mSocket;

	WeightedTimedMovingAverage	mBytesReceivedPerSecond;
	WeightedTimedMovingAverage	mBytesSentPerSecond;

	int							mBytesSentThisFrame;

	float						mDropPacketChance;
	float						mSimulatedLatency;
	bool						mIsSimulatedJitter;
};

inline	GameObjectPtr NetworkMgr::GetGameObject( int inNetworkId ) const
{
	auto gameObjectIt = mNetworkIdToGameObjectMap.find( inNetworkId );
	if (gameObjectIt != mNetworkIdToGameObjectMap.end())
	{
		return gameObjectIt->second;
	}
	else
	{
		return GameObjectPtr();
	}
}