#ifdef NEW_EPOLL_INTERFACE
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/ThreadLocalSingleton.h>
#include <muduo/net/EventLoop.h>
#include <net/UdpServer.h>

#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <net/UdpConnection.h>

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

#endif //NEW_EPOLL_INTERFACE



typedef unordered_map< int, ClientProxyPtr >	IntToClientMap;
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
	virtual ~NetworkMgr() {}

	bool					Init( uint16_t inPort );
	virtual void			CheckForDisconnects() {}
	virtual void			SendOutgoingPackets() {}

	void	SendPacket( const OutputBitStream& inOutputStream, shared_ptr< ClientProxy > inClientProxy );

	void	SetDropPacketChance( float inChance ) { mDropPacketChance = inChance; }
	void	SetSimulatedLatency( float inLatency ) { mSimulatedLatency = inLatency; }

	void	SetIsSimulatedJitter( bool inIsSimulatedJitter ) { mWhetherToSimulateJitter = inIsSimulatedJitter; }
	bool	GetIsSimulatedJitter() const { return mWhetherToSimulateJitter; }

	inline	EntityPtr	GetGameObject( int inNetworkId );


protected:

	// not complete, deprecated. can not calc slicedPacketCount.
	void ProcessOutcomingPacket( OutputBitStream& inOutputStream, shared_ptr< ClientProxy > inClientProxy, TransmissionDataHandler* inTransmissionDataHandler );
	// not complete, deprecated.
	void RecombineSlicesToChunk( InputBitStream& refInputStream );

private:
	//void	UpdateBytesSentLastFrame();
	void	ProcessQueuedPackets();

	WeightedTimedMovingAverage	mBytesReceivedPerSecond;
	WeightedTimedMovingAverage	mBytesSentPerSecond;

	int							mBytesSentThisFrame;

	float						mDropPacketChance;
	float						mSimulatedLatency;
	bool						mWhetherToSimulateJitter;

protected:
	bool						mIsReceivingSlicePacket;
	uint8_t						mNextExpectedSlicedPacketIndex;
	InputBitStream				mChunkInputStream;
	uint32_t					mChunkPacketID;


#ifdef DEPRECATED_EPOLL_INTERFACE
	void	WaitForIncomingPackets();
	void	RecvIncomingPacketsIntoQueue( UDPSocketPtr inUDPSocketPtr, SocketAddrInterface infromAddress );
#endif //DEPRECATED_EPOLL_INTERFACE


#ifdef NEW_EPOLL_INTERFACE
public:
	virtual void ProcessPacket( InputBitStream& inInputStream,
		const UdpConnectionPtr& inUdpConnetction ) = 0;
private:
	class ReceivedPacket
	{
	public:
		ReceivedPacket(
			float inReceivedTime,
			InputBitStream& inInputMemoryBitStream,
			UdpConnectionPtr inUdpConnection = nullptr )
			:
			mReceivedTime( inReceivedTime ),
			mPacketBuffer( inInputMemoryBitStream ),
			mUdpConnection( inUdpConnection )
		{}

		float GetReceivedTime()	const { return mReceivedTime; }
		InputBitStream&	GetPacketBuffer() { return mPacketBuffer; }
		UdpConnectionPtr GetUdpConnection() const { return mUdpConnection; }

	private:
		float							mReceivedTime;
		InputBitStream					mPacketBuffer;
		UdpConnectionPtr	mUdpConnection;
	};
	typedef ThreadLocalSingleton<
		queue< ReceivedPacket, list< ReceivedPacket > >
	>	mPacketQueue;
public:
	void setWorldUpdateCallback( const std::function<void()>& cb )
	{ WorldUpdateCB_ = cb; }

	void Start();
	void threadInit( EventLoop* loop );

	UdpConnToClientMapPtr GetUdpConnToClientMap();
	void UdpConnToClientMapCOW();

	IntToGameObjectMapPtr GetNetworkIdToGameObjectMap();
	void NetworkIdToGameObjectMapCOW();

	PlayerIdToClientMapPtr GetPlayerIdToClientMap();
	void PlayerIdToClientMapCOW();

	void onConnection( const UdpConnectionPtr& conn );
	void onMessage(
		const muduo::net::UdpConnectionPtr& conn,
		muduo::net::Buffer* buf,
		muduo::Timestamp receiveTime
	);
protected:
	UdpConnToClientMapPtr	mUdpConnToClientMap;
	IntToGameObjectMapPtr	mNetworkIdToGameObjectMap;
	PlayerIdToClientMapPtr	mPlayerIdToClientMap;
private:
	EventLoop loop_;
	std::shared_ptr<UdpServer> server_;

	std::set<EventLoop*> loops_;

	std::function<void()> WorldUpdateCB_;
protected:
	MutexLock mutex_;

#else //NEW_EPOLL_INTERFACE

public:
	void	HandleConnectionReset( const SocketAddrInterface& inFromAddress );
	void	ProcessIncomingPackets();
	virtual void ProcessPacket( InputBitStream& inInputStream,
		const SocketAddrInterface& inFromAddress,
		const UDPSocketPtr& inUDPSocket ) = 0;
private:
	void	ReadIncomingPacketsIntoQueue();
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
			//std::shared_ptr<UdpConnection> inUdpConnection = nullptr
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
		//std::shared_ptr<UdpConnection>	GetUdpConnection() const { return mUdpConnection; }

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
	IntToGameObjectMap			mNetworkIdToGameObjectMap;

	typedef unordered_map< SocketAddrInterface, ClientProxyPtr >	AddressToClientMap;
	AddressToClientMap		mAddressToClientMap;

	IntToClientMap			mPlayerIdToClientMap;

#endif //NEW_EPOLL_INTERFACE

};


#ifdef NEW_EPOLL_INTERFACE

inline	EntityPtr NetworkMgr::GetGameObject( int inNetworkId )
{
	auto tempNetworkIdToGameObjectMap = GetNetworkIdToGameObjectMap();
	auto gameObjectIt = tempNetworkIdToGameObjectMap->find( inNetworkId );
	if ( gameObjectIt != tempNetworkIdToGameObjectMap->end() )
	{
		return gameObjectIt->second;
	}
	else
	{
		return EntityPtr();
	}
}

#else //NEW_EPOLL_INTERFACE

inline	EntityPtr NetworkMgr::GetGameObject( int inNetworkId )
{
	auto gameObjectIt = mNetworkIdToGameObjectMap.find( inNetworkId );
	if ( gameObjectIt != mNetworkIdToGameObjectMap.end() )
	{
		return gameObjectIt->second;
	}
	else
	{
		return EntityPtr();
	}
}
#endif //NEW_EPOLL_INTERFACE