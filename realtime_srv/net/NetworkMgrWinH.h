

namespace realtime_srv
{

class GameObj;
class ClientProxy;
typedef std::function< GameObjPtr( ClientProxyPtr& ) > NewPlayerCallback;
typedef std::function<InputState*()> CustomInputStateCallback;

class NetworkMgr : noncopyable
{

public:
	typedef std::function<void( GameObjPtr&, ReplicationAction )> WorldRegistryCb;

	static const uint32_t	kNullCC = 0;
	static const uint32_t	kHelloCC = 'HELO';
	static const uint32_t	kWelcomeCC = 'WLCM';
	static const uint32_t	kResetCC = 'RSET';
	static const uint32_t	kResetedCC = 'RSTD';
	static const uint32_t	kStateCC = 'STAT';
	static const uint32_t	kInputCC = 'INPT';

public:
	void Start();

	void SetRepStateDirty( int inNetworkId, uint32_t inDirtyState );

	void NotifyAllClient( GameObjPtr& inGameObject, ReplicationAction inAction );

	void SetNewPlayerCallback( const NewPlayerCallback& cb )
	{ newPlayerCb_ = cb; }

	void SetWorldUpdateCallback( const std::function<void()>& cb )
	{ worldUpdateCb_ = cb; }

	void SetWorldRegistryCallback( const WorldRegistryCb& cb )
	{ worldRegistryCb_ = cb; }

	void SetCustomInputStateCallback( const CustomInputStateCallback& cb )
	{ customInputStatecb_ = cb; }

private:

	void CheckForDisconnects();

	uint32_t	 HandleServerReset( ClientProxyPtr& inClientProxy,
		InputBitStream& inInputStream );

	void	CheckPacketType( ClientProxyPtr& inClientProxy,
		InputBitStream& inInputStream );

	void	WriteLastMoveTimestampIfDirty( OutputBitStream& inOutputStream,
		ClientProxyPtr& inClientProxy );

	void	HandleInputPacket( ClientProxyPtr& inClientProxy,
		InputBitStream& inInputStream );

private:
	NewPlayerCallback newPlayerCb_;
	std::function<void()> worldUpdateCb_;
	WorldRegistryCb worldRegistryCb_;
	CustomInputStateCallback customInputStatecb_;

public:

	NetworkMgr( uint16_t inPort );
	virtual ~NetworkMgr() { UdpSockInterf::CleanUp(); }

	void	SetDropPacketChance( float inChance )
	{ mDropPacketChance = inChance; isSimilateRealWorld_ = true; }
	void	SetSimulatedLatency( float inLatency )
	{ mSimulatedLatency = inLatency; isSimilateRealWorld_ = true; }
	void	SetIsSimulatedJitter( bool inIsSimulatedJitter )
	{ mSimulateJitter = inIsSimulatedJitter; isSimilateRealWorld_ = true; }
	void	SetIsSimilateRealWorld( bool inIsSimilateRealWorld )
	{ isSimilateRealWorld_ = inIsSimilateRealWorld; }

	void SendOutgoingPackets();

	void	SendPacket( const OutputBitStream& inOutputStream,
		const SockAddrInterf& inSockAddr );

	void	HandleConnectionReset( const SockAddrInterf& inFromAddress );
private:
	void	ProcessQueuedPackets();

	void	SendGamePacket( ClientProxyPtr inClientProxy,
		const uint32_t inConnFlag );

	void	ReadIncomingPacketsIntoQueue();

	virtual void ProcessPacket( InputBitStream& inInputStream,
		const SockAddrInterf& inFromAddress,
		const UDPSocketPtr& inUDPSocket );

	void	HandlePacketFromNewClient( InputBitStream& inInputStream,
		const SockAddrInterf& inFromAddress,
		const UDPSocketPtr& inUDPSocket );

private:
	class ReceivedPacket
	{
	public:
		ReceivedPacket(
			float inReceivedTime,
			InputBitStream& inInputMemoryBitStream,
			const SockAddrInterf& inFromAddress,
			UDPSocketPtr  inUDPSocket = nullptr )
			:
			mReceivedTime( inReceivedTime ),
			mFromAddress( inFromAddress ),
			mPacketBuffer( inInputMemoryBitStream ),
			mUDPSocket( inUDPSocket )
		{}

		const	SockAddrInterf&			GetFromAddress()	const { return mFromAddress; }
		float					GetReceivedTime()	const { return mReceivedTime; }
		InputBitStream&	GetPacketBuffer() { return mPacketBuffer; }
		UDPSocketPtr	GetUDPSocket() const { return mUDPSocket; }

	private:

		float					mReceivedTime;
		InputBitStream			mPacketBuffer;
		SockAddrInterf			mFromAddress;
		UDPSocketPtr			mUDPSocket;
	};
	queue< ReceivedPacket, list< ReceivedPacket > >	mPacketQueue;

private:
	UDPSocketPtr				mSocket;
	static int				kNewNetId;
	typedef unordered_map< SockAddrInterf, ClientProxyPtr >	AddrToClientMap;
	AddrToClientMap		addrToClientMap_;

	float						mTimeOfLastStatePacket;
	float						mDropPacketChance;
	float						mSimulatedLatency;
	bool						mSimulateJitter;
	bool						isSimilateRealWorld_;
	float						mLastCheckDCTime;
};
}