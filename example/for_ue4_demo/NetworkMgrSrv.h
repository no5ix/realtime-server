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

typedef unordered_map< UdpConnectionPtr, ClientProxyPtr >	UdpConnToClientMap;
typedef std::shared_ptr< UdpConnToClientMap > UdpConnToClientMapPtr;
typedef std::shared_ptr< unordered_map< int, ClientProxyPtr > > PlayerIdToClientMapPtr;
typedef std::shared_ptr< unordered_map< int, EntityPtr > > IntToGameObjectMapPtr;

#endif //IS_LINUX


typedef unordered_map< int, EntityPtr > IntToGameObjectMap;
typedef unordered_map< int, ClientProxyPtr >	IntToClientMap;

class ClientProxy;

class NetworkMgrSrv
{
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

	bool					Init( uint16_t inPort = DEFAULT_REALTIME_SRV_PORT );
	EntityPtr				GetGameObject( int inNetworkId );

	int						GetNewNetworkId();

public:
	static std::unique_ptr<NetworkMgrSrv>	sInst;

	static bool				StaticInit( uint16_t inPort );

	virtual void			SendOutgoingPackets();
	inline	EntityPtr		RegisterAndReturn( Entity* inGameObject );
	void					SetStateDirty( int inNetworkId, uint32_t inDirtyState );
	virtual void			CheckForDisconnects();
	ClientProxyPtr			GetClientProxy( int inPlayerId );

	uint32_t				HandleServerReset( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );
	void					SendGamePacket( ClientProxyPtr inClientProxy, const uint32_t inConnFlag );

	virtual int RegistEntityAndRetNetID( EntityPtr inGameObject ) override;
	virtual int UnregistEntityAndRetNetID( Entity* inGameObject ) override;
private:
	NetworkMgrSrv();

	void	DoProcessPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );

	void	WriteLastMoveTimestampIfDirty( OutputBitStream& inOutputStream, ClientProxyPtr inClientProxy );

	void	HandleInputPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );


	float			mTimeOfLastSatePacket;
	float			mTimeBetweenStatePackets;
	float			mTimeOfLastStatePacket;

	float			mLastCheckDCTime;


#ifdef IS_LINUX


public:

	IntToGameObjectMapPtr GetNetworkIdToGameObjectMap();
	void NetworkIdToGameObjectMapCOW();

	void	SendPacket( const OutputBitStream& inOutputStream,
		const UdpConnectionPtr& conn );

	//virtual void ProcessPacket( InputBitStream& inInputStream,
	//	const UdpConnectionPtr& inUdpConnetction ) = 0;

	void setWorldUpdateCallback( const std::function<void()>& cb )
	{ WorldUpdateCB_ = cb; }

	void Start();
	void threadInit( EventLoop* loop );

	//virtual void onConnection( const UdpConnectionPtr& conn );

	void onMessage(
		const muduo::net::UdpConnectionPtr& conn,
		muduo::net::Buffer* buf,
		muduo::Timestamp receiveTime
	);
private:
	EventLoop loop_;
	std::shared_ptr<UdpServer> server_;
	std::set<EventLoop*> loops_;

	std::function<void()> WorldUpdateCB_;

protected:
	static AtomicInt32		kNewNetworkId;
	IntToGameObjectMapPtr	mNetworkIdToGameObjectMap;
	MutexLock mutex_;
public:
	virtual void onConnection( const UdpConnectionPtr& conn ) override;

	UdpConnToClientMapPtr GetUdpConnToClientMap();
	void UdpConnToClientMapCOW();

	PlayerIdToClientMapPtr GetPlayerIdToClientMap();
	void PlayerIdToClientMapCOW();

	virtual void ProcessPacket( InputBitStream& inInputStream,
		const UdpConnectionPtr& inUdpConnetction ) override;
private:
	void	HandlePacketFromNewClient( InputBitStream& inInputStream,
		const UdpConnectionPtr& inUdpConnetction );
protected:
	UdpConnToClientMapPtr	mUdpConnToClientMap;
	PlayerIdToClientMapPtr	mPlayerIdToClientMap;
private:
	static AtomicInt32		kNewPlayerId;

#else //IS_LINUX

public:
	virtual ~NetworkMgrSrv() { UDPSocketInterface::CleanUp(); }


	void	SendPacket( const OutputBitStream& inOutputStream,
		const SocketAddrInterface& inSockAddr );
	void	SetDropPacketChance( float inChance ) { mDropPacketChance = inChance; }
	void	SetSimulatedLatency( float inLatency ) { mSimulatedLatency = inLatency; }

	void	SetIsSimulatedJitter( bool inIsSimulatedJitter ) { mWhetherToSimulateJitter = inIsSimulatedJitter; }
	bool	GetIsSimulatedJitter() const { return mWhetherToSimulateJitter; }

	void	HandleConnectionReset( const SocketAddrInterface& inFromAddress );
	void	ProcessIncomingPackets();
	//virtual void ProcessPacket( InputBitStream& inInputStream,
	//	const SocketAddrInterface& inFromAddress,
	//	const UDPSocketPtr& inUDPSocket ) = 0;
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
			const SocketAddrInterface& inFromAddress,
			UDPSocketPtr  inUDPSocket = nullptr
		) :
			mReceivedTime( inReceivedTime ),
			mFromAddress( inFromAddress ),
			mPacketBuffer( inInputMemoryBitStream ),
			mUDPSocket( inUDPSocket )
		{}

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
	queue< ReceivedPacket, list< ReceivedPacket > >	mPacketQueue;

protected:
	UDPSocketPtr				mSocket;
	static int					kNewNetworkId;
	IntToGameObjectMap			mNetworkIdToGameObjectMap;

public:
	virtual void ProcessPacket( InputBitStream& inInputStream,
		const SocketAddrInterface& inFromAddress,
		const UDPSocketPtr& inUDPSocket ) override;
private:
	void	HandlePacketFromNewClient(
		InputBitStream& inInputStream,
		const SocketAddrInterface& inFromAddress,
		const UDPSocketPtr& inUDPSocket
		// ,
		// const std::shared_ptr<UdpConnection>& inUdpConnetction
	);
private:
	static int				kNewPlayerId;

	typedef unordered_map< SocketAddrInterface, ClientProxyPtr >	AddressToClientMap;
	AddressToClientMap		mAddressToClientMap;

	IntToClientMap			mPlayerIdToClientMap;
#endif //IS_LINUX

};


inline EntityPtr NetworkMgrSrv::RegisterAndReturn( Entity* inGameObject )
{
	EntityPtr toRet( inGameObject );
	RegistEntityAndRetNetID( toRet );
	return toRet;
}
