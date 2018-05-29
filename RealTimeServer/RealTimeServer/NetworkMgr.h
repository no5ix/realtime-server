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
#endif //NEW_EPOLL_INTERFACE


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

	bool	Init( uint16_t inPort );
	void	ProcessIncomingPackets();
	virtual void			CheckForDisconnects() {}
	virtual void			SendOutgoingPackets() {}

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
			//,
			//std::shared_ptr<UdpConnection> inUdpConnection = nullptr
			) :
			mReceivedTime( inReceivedTime ),
			mFromAddress( inFromAddress ),
			mPacketBuffer( inInputMemoryBitStream ),
			mUDPSocket( inUDPSocket )
			//,
			//mUdpConnection( inUdpConnection )
		{
		}

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


#ifdef DEPRECATED_EPOLL_INTERFACE
	void	WaitForIncomingPackets();
	void	RecvIncomingPacketsIntoQueue( UDPSocketPtr inUDPSocketPtr, SocketAddrInterface infromAddress );
#endif

#ifdef NEW_EPOLL_INTERFACE
public:
	void setWorldUpdateCallback( const std::function<void()>& cb )
	{ WorldUpdateCB_ = cb; }

	void Start();
	void threadInit( EventLoop* loop );
	void onConnection( const UdpConnectionPtr& conn );
	void onMessage( 
		const muduo::net::UdpConnectionPtr& conn, 
		muduo::net::Buffer* buf, 
		muduo::Timestamp receiveTime 
	);

private:
	typedef std::set<UdpConnectionPtr> ConnectionList;
	EventLoop loop_;
	std::shared_ptr<UdpServer> server_;
	typedef ThreadLocalSingleton<ConnectionList> LocalConnections;

	MutexLock mutex_;
	std::set<EventLoop*> loops_;

	std::function<void()> WorldUpdateCB_;
#endif
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