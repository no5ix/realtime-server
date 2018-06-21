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


#endif //IS_LINUX

typedef unordered_map< int, ClientProxyPtr >	IntToClientMap;

class ClientProxy;

class NetworkMgrSrv : public NetworkMgr
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
