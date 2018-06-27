#pragma once

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

#endif //IS_LINUX

namespace realtime_srv {

class GameObj;
class ClientProxy;
typedef std::function< GameObjPtr() > NewPlayerCallback;

class NetworkMgr : noncopyable {
	typedef std::function<void( GameObjPtr, ReplicationAction )> WorldRegistryCB;
public:

	static const uint32_t	kNullCC = 0;
	static const uint32_t	kHelloCC = 'HELO';
	static const uint32_t	kWelcomeCC = 'WLCM';
	static const uint32_t	kResetCC = 'RSET';
	static const uint32_t	kResetedCC = 'RSTD';
	static const uint32_t	kStateCC = 'STAT';
	static const uint32_t	kInputCC = 'INPT';

	static const int		kMaxPacketsPerFrameCount = 10;

public:

	NetworkMgr();
	bool	Init( uint16_t inPort );

	void	Start();

	virtual void SendOutgoingPackets();
	void		 SetRepStateDirty( int inNetworkId, uint32_t inDirtyState );
	virtual void CheckForDisconnects();

	uint32_t	 HandleServerReset( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );
	void		 SendGamePacket( ClientProxyPtr inClientProxy, const uint32_t inConnFlag );

	void NotifyAllClient( GameObjPtr inGameObject, ReplicationAction inAction );

	void SetNewPlayerCallback( const NewPlayerCallback& cb ) { newPlayerCB_ = cb; }

	void SetWorldUpdateCallback( const std::function<void()>& cb ) { worldUpdateCB_ = cb; }

	void SetWorldRegistryCallback( const WorldRegistryCB& cb ) { worldRegistryCB_ = cb; }

private:
	void	DoProcessPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );

	void	WriteLastMoveTimestampIfDirty( OutputBitStream& inOutputStream, ClientProxyPtr inClientProxy );

	void	HandleInputPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );

	NewPlayerCallback newPlayerCB_;
	std::function<void()> worldUpdateCB_;
	WorldRegistryCB worldRegistryCB_;

	#ifdef IS_LINUX

public:

	typedef unordered_map< muduo::net::UdpConnectionPtr, ClientProxyPtr >	UdpConnToClientMap;
	typedef std::shared_ptr< UdpConnToClientMap > UdpConnToClientMapPtr;


	void SendPacket( const OutputBitStream& inOutputStream,
		const muduo::net::UdpConnectionPtr& conn );

	void onMessage( const muduo::net::UdpConnectionPtr& conn,
		muduo::net::Buffer* buf,
		muduo::Timestamp receiveTime );
	virtual void onConnection( const muduo::net::UdpConnectionPtr& conn );

	virtual void ProcessPacket( InputBitStream& inInputStream,
		const muduo::net::UdpConnectionPtr& inUdpConnetction );
private:
	void	HandlePacketFromNewClient( InputBitStream& inInputStream,
		const muduo::net::UdpConnectionPtr& inUdpConnetction );

private:
	THREAD_SHARED_VAR_DEF( protected, UdpConnToClientMap, udpConnToClientMap_, mutex_ );
	muduo::net::EventLoop loop_;
	std::shared_ptr<muduo::net::UdpServer> server_;
	muduo::MutexLock mutex_;
	static muduo::AtomicInt32		kNewPlayerId;
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
private:
	void	ProcessQueuedPackets();
	void	ReadIncomingPacketsIntoQueue();
	virtual void ProcessPacket( InputBitStream& inInputStream,
		const SockAddrInterfc& inFromAddress,
		const UDPSocketPtr& inUDPSocket );
	void	HandlePacketFromNewClient( InputBitStream& inInputStream,
		const SockAddrInterfc& inFromAddress,
		const UDPSocketPtr& inUDPSocket );
private:
	class ReceivedPacket {
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
			mUDPSocket( inUDPSocket ) {}

		const	SockAddrInterfc&			GetFromAddress()	const { return mFromAddress; }
		float					GetReceivedTime()	const { return mReceivedTime; }
		InputBitStream&	GetPacketBuffer() { return mPacketBuffer; }
		UDPSocketPtr	GetUDPSocket() const { return mUDPSocket; }

	private:

		float					mReceivedTime;
		InputBitStream			mPacketBuffer;
		SockAddrInterfc			mFromAddress;
		UDPSocketPtr			mUDPSocket;
	};
	queue< ReceivedPacket, list< ReceivedPacket > >	mPacketQueue;

private:
	UDPSocketPtr				mSocket;
	static int				kNewPlayerId;
	typedef unordered_map< SockAddrInterfc, ClientProxyPtr >	AddrToClientMap;
	AddrToClientMap		addrToClientMap_;

	float			mTimeOfLastStatePacket;
	float			mLastCheckDCTime;

	float						mDropPacketChance;
	float						mSimulatedLatency;
	bool						mWhetherToSimulateJitter;

	#endif //IS_LINUX

};
}