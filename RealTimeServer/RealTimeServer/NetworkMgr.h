typedef unordered_map< int, EntityPtr > IntToGameObjectMap;

class ClientProxy;
class TransmissionDataHandler;

class NetworkMgr
{
public:
	static const uint32_t	kNullCC = 0;
	static const uint32_t	kHelloCC = 'HELO';
	static const uint32_t	kWelcomeCC = 'WLCM';
	static const uint32_t	kResetCC = 'RSET';
	static const uint32_t	kResetedCC = 'RSTD';
	static const uint32_t	kStateCC = 'STAT';
	static const uint32_t	kInputCC = 'INPT';

	static const int		kMaxPacketsPerFrameCount = 10;

	NetworkMgr();
	virtual ~NetworkMgr();

	bool	Init( uint16_t inPort );

#ifdef HAS_EPOLL
	void	WaitForIncomingPackets();
	void	RecvIncomingPacketsIntoQueue( UDPSocketPtr inUDPSocketPtr, SocketAddrInterface infromAddress );
#endif

	void	ProcessIncomingPackets();

	virtual void	ProcessPacket( InputBitStream& inInputStream, const SocketAddrInterface& inFromAddress, const UDPSocketPtr& inUDPSocket ) = 0;
	virtual void HandleConnectionReset( const SocketAddrInterface& inFromAddress ) {}

	void	SendPacket( const OutputBitStream& inOutputStream, shared_ptr< ClientProxy > inClientProxy );

	void	SetDropPacketChance( float inChance ) { mDropPacketChance = inChance; }
	void	SetSimulatedLatency( float inLatency ) { mSimulatedLatency = inLatency; }

	void	SetIsSimulatedJitter( bool inIsSimulatedJitter ) { mWhetherToSimulateJitter = inIsSimulatedJitter; }
	bool	GetIsSimulatedJitter() const { return mWhetherToSimulateJitter; }

	inline	EntityPtr	GetGameObject( int inNetworkId ) const;


protected:

	IntToGameObjectMap		mNetworkIdToGameObjectMap;

	// not complete, deprecated. can not calc slicedPacketCount.
	void ProcessOutcomingPacket( OutputBitStream& inOutputStream, shared_ptr< ClientProxy > inClientProxy, TransmissionDataHandler* inTransmissionDataHandler );
	// not complete, deprecated.
	void RecombineSlicesToChunk( InputBitStream& refInputStream );

private:

	class ReceivedPacket
	{
	public:
		ReceivedPacket(
			float inReceivedTime,
			InputBitStream& inInputMemoryBitStream,
			const SocketAddrInterface& inFromAddress,
			UDPSocketPtr  inUDPSocket = nullptr
		) :
			mReceivedTime( inReceivedTime ),
			mFromAddress( inFromAddress ),
			mPacketBuffer( inInputMemoryBitStream ),
			mUDPSocket( inUDPSocket )
		{
		}

		const	SocketAddrInterface&			GetFromAddress()	const { return mFromAddress; }
		float					GetReceivedTime()	const { return mReceivedTime; }
		InputBitStream&	GetPacketBuffer() { return mPacketBuffer; }
		UDPSocketPtr	GetUDPSocket() const { return mUDPSocket; }

	private:

		float					mReceivedTime;
		InputBitStream			mPacketBuffer;
		SocketAddrInterface		mFromAddress;
		UDPSocketPtr			mUDPSocket;
	};

	//void	UpdateBytesSentLastFrame();
	void ReadIncomingPacketsIntoQueue();
	void	ProcessQueuedPackets();

	queue< ReceivedPacket, list< ReceivedPacket > >	mPacketQueue;


	WeightedTimedMovingAverage	mBytesReceivedPerSecond;
	WeightedTimedMovingAverage	mBytesSentPerSecond;

	int							mBytesSentThisFrame;

	float						mDropPacketChance;
	float						mSimulatedLatency;
	bool						mWhetherToSimulateJitter;

protected:
	UDPSocketPtr	mSocket;

	bool						mIsReceivingSlicePacket;
	uint8_t						mNextExpectedSlicedPacketIndex;
	InputBitStream				mChunkInputStream;
	uint32_t					mChunkPacketID;

};

inline	EntityPtr NetworkMgr::GetGameObject( int inNetworkId ) const
{
	auto gameObjectIt = mNetworkIdToGameObjectMap.find( inNetworkId );
	if (gameObjectIt != mNetworkIdToGameObjectMap.end())
	{
		return gameObjectIt->second;
	}
	else
	{
		return EntityPtr();
	}
}