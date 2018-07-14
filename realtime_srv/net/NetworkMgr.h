#pragma once

#ifdef IS_LINUX
//#include <muduo/base/Logging.h>
//#include <muduo/base/Mutex.h>
//#include <muduo/net/EventLoopThread.h>
//#include <muduo/net/EventLoopThreadPool.h>
//#include <muduo/base/ThreadLocalSingleton.h>
//#include <muduo/net/EventLoop.h>
//#include <muduo_udp_support/UdpServer.h>
//#include <muduo/base/ThreadPool.h>
//
//#include <muduo/net/Buffer.h>
//#include <muduo/net/Endian.h>
//#include <muduo_udp_support/UdpConnection.h>
//
//#include <concurrent_queue/concurrentqueue.h>
//#include <concurrent_queue/blockingconcurrentqueue.h>
//
//#include <set>
//#include <stdio.h>
//#include <unistd.h>

#include <realtime_srv/net/PktDispatcher.h>
#include <realtime_srv/net/PktHandler.h>

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

	//muduo::net::EventLoop* GetEventLoop() { return &serverBaseLoop_; }

protected:
	void OnConnOrDisconn( const muduo::net::UdpConnectionPtr& conn );
	void PrepareOutgoingPackets();
	void PrepareGamePacket( ClientProxyPtr inClientProxy, const uint32_t inConnFlag );

	virtual void ProcessPacket( InputBitStream& inInputStream,
		const muduo::net::UdpConnectionPtr& inUdpConnetction );

	void RemoveUdpConn( const muduo::net::UdpConnectionPtr& conn );

	void	HandlePacketFromNewClient( InputBitStream& inInputStream,
		const muduo::net::UdpConnectionPtr& inUdpConnetction );

	void PktHandleFunc( ReceivedPacket& recvedPacket );
private:

	ReceivedPacketBlockQueue recvedPacketBlockQ_;
	PendingSendPacketQueue pendingSndPacketQ_;

	PktDispatcher pktDispatcher_;
	PktHandler		pktHandler_;


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