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

typedef std::shared_ptr< unordered_map< int, EntityPtr > > IntToGameObjectMapPtr;

#endif //IS_LINUX

typedef unordered_map< int, EntityPtr > IntToGameObjectMap;


class NetworkMgr
{
public:

	static const int		kMaxPacketsPerFrameCount = 10;

	NetworkMgr();

	bool					Init( uint16_t inPort );
	virtual void			CheckForDisconnects() {}
	virtual void			SendOutgoingPackets() {}

	virtual int RegistEntityAndRetNetID( EntityPtr inGameObject );
	virtual int UnregistEntityAndRetNetID( Entity* inGameObject );
	EntityPtr				GetGameObject( int inNetworkId );

	int						GetNewNetworkId();

#ifdef IS_LINUX

public:

	IntToGameObjectMapPtr GetNetworkIdToGameObjectMap();
	void NetworkIdToGameObjectMapCOW();

	void	SendPacket( const OutputBitStream& inOutputStream,
		const UdpConnectionPtr& conn );

	virtual void ProcessPacket( InputBitStream& inInputStream,
		const UdpConnectionPtr& inUdpConnetction ) = 0;

	void setWorldUpdateCallback( const std::function<void()>& cb )
	{ WorldUpdateCB_ = cb; }

	void Start();
	void threadInit( EventLoop* loop );

	virtual void onConnection( const UdpConnectionPtr& conn );

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

#else //IS_LINUX

public:
	virtual ~NetworkMgr() { UDPSocketInterface::CleanUp(); }


	void	SendPacket( const OutputBitStream& inOutputStream,
		const SocketAddrInterface& inSockAddr );
	void	SetDropPacketChance( float inChance ) { mDropPacketChance = inChance; }
	void	SetSimulatedLatency( float inLatency ) { mSimulatedLatency = inLatency; }

	void	SetIsSimulatedJitter( bool inIsSimulatedJitter ) { mWhetherToSimulateJitter = inIsSimulatedJitter; }
	bool	GetIsSimulatedJitter() const { return mWhetherToSimulateJitter; }

	void	HandleConnectionReset( const SocketAddrInterface& inFromAddress );
	void	ProcessIncomingPackets();
	virtual void ProcessPacket( InputBitStream& inInputStream,
		const SocketAddrInterface& inFromAddress,
		const UDPSocketPtr& inUDPSocket ) = 0;
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
			//,
			//std::shared_ptr<UdpConnection>& inUdpConnection = nullptr
		) :
			mReceivedTime( inReceivedTime ),
			mFromAddress( inFromAddress ),
			mPacketBuffer( inInputMemoryBitStream ),
			mUDPSocket( inUDPSocket )
			//,
			//mUdpConnection( inUdpConnection )
		{}

		const	SocketAddrInterface&			GetFromAddress()	const { return mFromAddress; }
		float					GetReceivedTime()	const { return mReceivedTime; }
		InputBitStream&	GetPacketBuffer() { return mPacketBuffer; }
		UDPSocketPtr	GetUDPSocket() const { return mUDPSocket; }
		//std::shared_ptr<UdpConnection>&	GetUdpConnection() const { return mUdpConnection; }

	private:

		float					mReceivedTime;
		InputBitStream			mPacketBuffer;
		SocketAddrInterface		mFromAddress;
		UDPSocketPtr			mUDPSocket;
		//std::shared_ptr<UdpConnection>			mUdpConnection;
	};
	queue< ReceivedPacket, list< ReceivedPacket > >	mPacketQueue;

protected:
	UDPSocketPtr				mSocket;
	static int					kNewNetworkId;
	IntToGameObjectMap			mNetworkIdToGameObjectMap;

#endif //IS_LINUX

};

