#ifdef IS_LINUX
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/ThreadLocalSingleton.h>
#include <muduo/net/EventLoop.h>
#include <muduo_udp_support/UdpServer.h>

#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo_udp_support/UdpConnection.h>

#include <set>
#include <stdio.h>
#include <unistd.h>

class UdpConnection;

using namespace muduo;
using namespace muduo::net;

#endif //IS_LINUX

class ClientProxy;

class NetworkMgr
{
	typedef unordered_map< int, ClientProxyPtr > PlayerIdToClientMap;
	typedef unordered_map< int, GameObjPtr > NetIdToGameObjMap;
	typedef std::function<void( ClientProxyPtr inClientProxy )> NewPlayerCB;
public:

	static const uint32_t	kNullCC = 0;
	static const uint32_t	kHelloCC = 'HELO';
	static const uint32_t	kWelcomeCC = 'WLCM';
	static const uint32_t	kResetCC = 'RSET';
	static const uint32_t	kResetedCC = 'RSTD';
	static const uint32_t	kStateCC = 'STAT';
	static const uint32_t	kInputCC = 'INPT';
public:

	static const int		kMaxPacketsPerFrameCount = 10;

	GameObjPtr				GetGameObject( int inNetworkId );

	int						GetNewNetworkId();

public:
	static std::unique_ptr<NetworkMgr>	sInst;

	static bool				StaticInit( uint16_t inPort = DEFAULT_REALTIME_SRV_PORT );

	virtual void			SendOutgoingPackets();
	inline	GameObjPtr		RegisterAndReturn( GameObj* inGameObject );
	void					SetStateDirty( int inNetworkId, uint32_t inDirtyState );
	virtual void			CheckForDisconnects();
	ClientProxyPtr			GetClientProxy( int inPlayerId );

	uint32_t				HandleServerReset( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );
	void					SendGamePacket( ClientProxyPtr inClientProxy, const uint32_t inConnFlag );

	virtual int RegistGameObjAndRetNetID( GameObjPtr inGameObject );
	virtual int UnregistGameObjAndRetNetID( GameObj* inGameObject );

	void SetNewPlayerCallBack( const NewPlayerCB& cb ) { newPlayerCB_ = cb; }
private:
	NetworkMgr();
	bool					Init( uint16_t inPort );

	void	DoProcessPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );

	void	WriteLastMoveTimestampIfDirty( OutputBitStream& inOutputStream, ClientProxyPtr inClientProxy );

	void	HandleInputPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );

	float			mTimeOfLastSatePacket;
	float			mTimeBetweenStatePackets;
	float			mTimeOfLastStatePacket;
	float			mLastCheckDCTime;

	NewPlayerCB newPlayerCB_;


#ifdef IS_LINUX


public:

	typedef unordered_map< UdpConnectionPtr, ClientProxyPtr >	UdpConnToClientMap;
	typedef std::shared_ptr< UdpConnToClientMap > UdpConnToClientMapPtr;

	typedef std::shared_ptr< PlayerIdToClientMap > PlayerIdToClientMapPtr;

	typedef std::shared_ptr< unordered_map< int, GameObjPtr > > NetIdToGameObjMapPtr;

	void	SendPacket( const OutputBitStream& inOutputStream,
		const UdpConnectionPtr& conn );

	void setWorldUpdateCallback( const std::function<void()>& cb )
	{ worldUpdateCB_ = cb; }

	void Start();

	void onMessage(
		const muduo::net::UdpConnectionPtr& conn,
		muduo::net::Buffer* buf,
		muduo::Timestamp receiveTime
	);
private:
	EventLoop loop_;
	std::shared_ptr<UdpServer> server_;

	std::function<void()> worldUpdateCB_;

protected:
	static AtomicInt32		kNewNetworkId;
	MutexLock mutex_;
public:
	virtual void onConnection( const UdpConnectionPtr& conn );

	virtual void ProcessPacket( InputBitStream& inInputStream,
		const UdpConnectionPtr& inUdpConnetction );
private:
	void	HandlePacketFromNewClient( InputBitStream& inInputStream,
		const UdpConnectionPtr& inUdpConnetction );
protected:
	THREAD_SHARED_VAR_DEF( protected, UdpConnToClientMap, udpConnToClientMap_, mutex_ );
	THREAD_SHARED_VAR_DEF( protected, PlayerIdToClientMap, playerIdToClientMap_, mutex_ );
	THREAD_SHARED_VAR_DEF( protected, NetIdToGameObjMap, netIdToGameObjMap_, mutex_ );

private:
	static AtomicInt32		kNewPlayerId;

#else //IS_LINUX

public:
	virtual ~NetworkMgr() { UdpSockInterfc::CleanUp(); }


	void	SendPacket( const OutputBitStream& inOutputStream,
		const SockAddrInterfc& inSockAddr );
	void	SetDropPacketChance( float inChance ) { mDropPacketChance = inChance; }
	void	SetSimulatedLatency( float inLatency ) { mSimulatedLatency = inLatency; }

	void	SetIsSimulatedJitter( bool inIsSimulatedJitter ) { mWhetherToSimulateJitter = inIsSimulatedJitter; }
	bool	GetIsSimulatedJitter() const { return mWhetherToSimulateJitter; }

	void	HandleConnectionReset( const SockAddrInterfc& inFromAddress );
	void	ProcessIncomingPackets();
private:
	void	ProcessQueuedPackets();
	void	ReadIncomingPacketsIntoQueue();
private:

	float						mDropPacketChance;
	float						mSimulatedLatency;
	bool						mWhetherToSimulateJitter;
private:
	class ReceivedPacket
	{
	public:
		ReceivedPacket(
			float inReceivedTime,
			InputBitStream& inInputMemoryBitStream,
			const SockAddrInterfc& inFromAddress,
			UDPSocketPtr  inUDPSocket = nullptr
		) :
			mReceivedTime( inReceivedTime ),
			mFromAddress( inFromAddress ),
			mPacketBuffer( inInputMemoryBitStream ),
			mUDPSocket( inUDPSocket )
		{}

		const	SockAddrInterfc&			GetFromAddress()	const { return mFromAddress; }
		float					GetReceivedTime()	const { return mReceivedTime; }
		InputBitStream&	GetPacketBuffer() { return mPacketBuffer; }
		UDPSocketPtr	GetUDPSocket() const { return mUDPSocket; }

	private:

		float					mReceivedTime;
		InputBitStream			mPacketBuffer;
		SockAddrInterfc		mFromAddress;
		UDPSocketPtr			mUDPSocket;
	};
	queue< ReceivedPacket, list< ReceivedPacket > >	mPacketQueue;

protected:
	UDPSocketPtr				mSocket;
	static int					kNewNetworkId;
	NetIdToGameObjMap			netIdToGameObjMap_;

public:
	virtual void ProcessPacket( InputBitStream& inInputStream,
		const SockAddrInterfc& inFromAddress,
		const UDPSocketPtr& inUDPSocket );
private:
	void	HandlePacketFromNewClient(
		InputBitStream& inInputStream,
		const SockAddrInterfc& inFromAddress,
		const UDPSocketPtr& inUDPSocket
	);
private:
	static int				kNewPlayerId;

	typedef unordered_map< SockAddrInterfc, ClientProxyPtr >	AddrToClientMap;
	AddrToClientMap		addrToClientMap_;

	PlayerIdToClientMap			playerIdToClientMap_;
#endif //IS_LINUX

};


inline GameObjPtr NetworkMgr::RegisterAndReturn( GameObj* inGameObject )
{
	GameObjPtr toRet( inGameObject );
	RegistGameObjAndRetNetID( toRet );
	return toRet;
}
