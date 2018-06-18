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

class NetworkMgrSrv : public NetworkMgr
{
public:
	static std::unique_ptr<NetworkMgrSrv>	sInst;

	static bool				StaticInit( uint16_t inPort );

	virtual void			SendOutgoingPackets();
	void					RegisterGameObject( EntityPtr inGameObject );
	inline	EntityPtr		RegisterAndReturn( Entity* inGameObject );
	void					UnregisterGameObject( Entity* inGameObject );
	void					SetStateDirty( int inNetworkId, uint32_t inDirtyState );
	virtual void			CheckForDisconnects();
	ClientProxyPtr			GetClientProxy( int inPlayerId );

	void					SendResetPacket ( ClientProxyPtr inClientProxy );
	uint32_t				HandleServerReset( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );

private:
	NetworkMgrSrv();

	void	DoProcessPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );

	void	SendWelcomePacket( ClientProxyPtr inClientProxy );


	void	SendStatePacketToClient( ClientProxyPtr inClientProxy );
	void	WriteLastMoveTimestampIfDirty( OutputBitStream& inOutputStream, ClientProxyPtr inClientProxy );

	void	HandleInputPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );

	int		GetNewNetworkId();

	float			mTimeOfLastSatePacket;
	float			mTimeBetweenStatePackets;
	float			mTimeOfLastStatePacket;

	float			mLastCheckDCTime;


#ifdef NEW_EPOLL_INTERFACE

public:
	virtual void ProcessPacket( InputBitStream& inInputStream,
		const UdpConnectionPtr& inUdpConnetction ) override;
private:
	void	HandlePacketFromNewClient( InputBitStream& inInputStream,
		const UdpConnectionPtr& inUdpConnetction );
private:
	static AtomicInt32		kNewPlayerId;
	static AtomicInt32		kNewNetworkId;

#else //NEW_EPOLL_INTERFACE

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
	static int				kNewNetworkId;
#endif //NEW_EPOLL_INTERFACE

};


inline EntityPtr NetworkMgrSrv::RegisterAndReturn( Entity* inGameObject )
{
	EntityPtr toRet( inGameObject );
	RegisterGameObject( toRet );
	return toRet;
}
