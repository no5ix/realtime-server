#pragma once

#ifdef IS_LINUX
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/base/ThreadLocalSingleton.h>
#include <muduo/net/EventLoop.h>
#include <muduo_udp_support/UdpServer.h>
#include <muduo/base/ThreadPool.h>

#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo_udp_support/UdpConnection.h>

#include <concurrent_queue/concurrentqueue.h>
#include <concurrent_queue/blockingconcurrentqueue.h>

#include <set>
#include <stdio.h>
#include <unistd.h>

#endif //IS_LINUX

namespace realtime_srv
{

class GameObj;
class ClientProxy;
typedef std::function< GameObjPtr( std::shared_ptr<ClientProxy> ) > NewPlayerCallback;

class NetworkMgr : noncopyable
{
	typedef std::function<void( GameObjPtr, ReplicationAction )> WorldRegistryCb;
public:

	static const uint32_t	kNullCC = 0;
	static const uint32_t	kHelloCC = 'HELO';
	static const uint32_t	kWelcomeCC = 'WLCM';
	static const uint32_t	kResetCC = 'RSET';
	static const uint32_t	kResetedCC = 'RSTD';
	static const uint32_t	kStateCC = 'STAT';
	static const uint32_t	kInputCC = 'INPT';


public:

	NetworkMgr();
	bool Init( uint16_t inPort, bool isLazy = false );

	void	Start();

	void		 SetRepStateDirty( int inNetworkId, uint32_t inDirtyState );
	virtual void CheckForDisconnects();

	uint32_t	 HandleServerReset( ClientProxyPtr inClientProxy,
		InputBitStream& inInputStream );

	void NotifyAllClient( GameObjPtr inGameObject, ReplicationAction inAction );

	void SetNewPlayerCallback( const NewPlayerCallback& cb )
	{ newPlayerCb_ = cb; }

	void SetWorldUpdateCallback( const std::function<void()>& cb )
	{ worldUpdateCb_ = cb; }

	void SetWorldRegistryCallback( const WorldRegistryCb& cb )
	{ worldRegistryCb_ = cb; }

	void	SetDropPacketChance( float inChance )
	{ mDropPacketChance = inChance; isSimilateRealWorld_ = true; }
	void	SetSimulatedLatency( float inLatency )
	{ mSimulatedLatency = inLatency; isSimilateRealWorld_ = true; }
	void	SetIsSimulatedJitter( bool inIsSimulatedJitter )
	{ mSimulateJitter = inIsSimulatedJitter; isSimilateRealWorld_ = true; }
	void	SetIsSimilateRealWorld( bool inIsSimilateRealWorld )
	{ isSimilateRealWorld_ = inIsSimilateRealWorld; }

private:

	void	DoProcessPacket( ClientProxyPtr inClientProxy,
		InputBitStream& inInputStream );

	void	WriteLastMoveTimestampIfDirty( OutputBitStream& inOutputStream,
		ClientProxyPtr inClientProxy );

	void	HandleInputPacket( ClientProxyPtr inClientProxy,
		InputBitStream& inInputStream );
private:
	NewPlayerCallback newPlayerCb_;
	std::function<void()> worldUpdateCb_;
	WorldRegistryCb worldRegistryCb_;

	float						mDropPacketChance;
	float						mSimulatedLatency;
	bool						mSimulateJitter;
	bool						isSimilateRealWorld_;
	float						mLastCheckDCTime;

#ifdef IS_LINUX

public:
	typedef unordered_map< muduo::net::UdpConnectionPtr, ClientProxyPtr >	UdpConnToClientMap;
	typedef std::shared_ptr< UdpConnToClientMap > UdpConnToClientMapPtr;

	muduo::net::EventLoop* GetEventLoop() { return &serverBaseLoop_; }

protected:
	void PrepareOutgoingPackets();
	void PrepareGamePacket( ClientProxyPtr inClientProxy, const uint32_t inConnFlag );

	void SendGamePacket();
	void SendGamePacketLazy();

	void Tick();
	void TickLazy();

	void SendPacket( const OutputBitStream& inOutputStream,
		const muduo::net::UdpConnectionPtr& conn );

	void onMessage( const muduo::net::UdpConnectionPtr& conn,
		muduo::net::Buffer* buf, muduo::Timestamp receiveTime );

	void onMessageLazy( const muduo::net::UdpConnectionPtr& conn,
		muduo::net::Buffer* buf, muduo::Timestamp receiveTime );

	virtual void onConnection( const muduo::net::UdpConnectionPtr& conn );

	virtual void ProcessPacket( InputBitStream& inInputStream,
		const muduo::net::UdpConnectionPtr& inUdpConnetction );

	void RemoveUdpConn( const muduo::net::UdpConnectionPtr& conn );

	void	HandlePacketFromNewClient( InputBitStream& inInputStream,
		const muduo::net::UdpConnectionPtr& inUdpConnetction );

private:
	////// recv
	class ReceivedPacket
	{
	public:
		ReceivedPacket() {}
		ReceivedPacket(
			const float inReceivedTime,
			const shared_ptr<InputBitStream>& inInputMemoryBitStreamPtr,
			const muduo::net::UdpConnectionPtr& inUdpConnetction )
			:
			mReceivedTime( inReceivedTime ),
			mPacketBuffer( inInputMemoryBitStreamPtr ),
			mUdpConn( inUdpConnetction )
		{}
		const	muduo::net::UdpConnectionPtr&	GetUdpConn() const { return mUdpConn; }
		float GetReceivedTime()	const { return mReceivedTime; }
		const shared_ptr<InputBitStream>& GetPacketBuffer() const { return mPacketBuffer; }

		bool operator<( const ReceivedPacket& other ) const
		{ return this->mReceivedTime < other.GetReceivedTime(); }
	private:
		float					mReceivedTime;
		shared_ptr<InputBitStream>			mPacketBuffer;
		muduo::net::UdpConnectionPtr			mUdpConn;
	};
	typedef moodycamel::ConcurrentQueue<ReceivedPacket> ReceivedPacketQueue;
	typedef moodycamel::BlockingConcurrentQueue<ReceivedPacket> ReceivedPacketBlockQueue;
	ReceivedPacketQueue recvedPacketQ_;
	ReceivedPacketBlockQueue recvedPacketBlockQ_;

	////// snd
	class PendingSendPacket
	{
	public:
		PendingSendPacket() {}
		PendingSendPacket( std::shared_ptr<OutputBitStream>& OutPutPacketBuffer,
			muduo::net::UdpConnectionPtr& UdpConnection )
			:
			outPacketBuf_( OutPutPacketBuffer ),
			udpConn_( UdpConnection )
		{}
		muduo::net::UdpConnectionPtr GetUdpConnection() { return udpConn_.lock(); }
		std::shared_ptr<OutputBitStream>& GetPacketBuffer() { return outPacketBuf_; }
	private:
		std::shared_ptr<OutputBitStream> outPacketBuf_;
		std::weak_ptr<muduo::net::UdpConnection> udpConn_;
	};
	typedef moodycamel::ConcurrentQueue< PendingSendPacket > PendingSendPacketQueue;
	PendingSendPacketQueue pendingSndPacketQ_;
	void DoSendPacketTask( muduo::net::EventLoop* p );
	std::shared_ptr<muduo::net::EventLoopThreadPool> sndPktThreadLoopPool_;
	muduo::net::EventLoopThread sndPktBaseLoopThread_;
	muduo::net::EventLoop* sndPktBaseLoop;

	typedef moodycamel::BlockingConcurrentQueue< PendingSendPacket > PendingSendPacketBlockQueue;
	PendingSendPacketBlockQueue pendingSndPacketBlockQ_;
	muduo::ThreadPool sndPktThreadPoolLazy_;

	////// tick
	void DoTickPendingFuncs();
	muduo::Thread tickThread_;
	muduo::Thread tickThreadLazy_;
	typedef std::function<void()> TickPendingFunctor;
	std::vector<TickPendingFunctor> tickPendingFuncs_;

	////// conn
	muduo::net::EventLoop serverBaseLoop_;
	std::unique_ptr<muduo::net::UdpServer> server_;
	muduo::MutexLock mutex_;

private:
	std::unique_ptr< UdpConnToClientMap > udpConnToClientMap_;
	static muduo::AtomicInt32		kNewNetId;
	bool isLazy_;

#else //IS_LINUX

public:
	virtual ~NetworkMgr() { UdpSockInterf::CleanUp(); }
	void SendOutgoingPackets();

	void	SendPacket( const OutputBitStream& inOutputStream,
		const SockAddrInterf& inSockAddr );

	void	HandleConnectionReset( const SockAddrInterf& inFromAddress );
private:
	void	ProcessQueuedPackets();
	void		 SendGamePacket( ClientProxyPtr inClientProxy,
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

	float			mTimeOfLastStatePacket;

#endif //IS_LINUX

};
}