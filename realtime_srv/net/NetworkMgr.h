#pragma once

#ifdef IS_LINUX

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

#ifdef IS_LINUX

public:
	typedef std::function<void( GameObjPtr, ReplicationAction )> WorldRegistryCb;

	static const uint32_t	kNullCC = 0;
	static const uint32_t	kHelloCC = 'HELO';
	static const uint32_t	kWelcomeCC = 'WLCM';
	static const uint32_t	kResetCC = 'RSET';
	static const uint32_t	kResetedCC = 'RSTD';
	static const uint32_t	kStateCC = 'STAT';
	static const uint32_t	kInputCC = 'INPT';

public:
	void Start();

	void SetRepStateDirty( int inNetworkId, uint32_t inDirtyState );

	void NotifyAllClient( GameObjPtr inGameObject, ReplicationAction inAction );

	void SetNewPlayerCallback( const NewPlayerCallback& cb )
	{ newPlayerCb_ = cb; }

	void SetWorldUpdateCallback( const std::function<void()>& cb )
	{ worldUpdateCb_ = cb; }

	void SetWorldRegistryCallback( const WorldRegistryCb& cb )
	{ worldRegistryCb_ = cb; }

private:

	void CheckForDisconnects();

	uint32_t	 HandleServerReset( ClientProxyPtr inClientProxy,
		InputBitStream& inInputStream );

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
public:

	NetworkMgr( uint16_t inPort );

	bool Init();

public:
	typedef unordered_map< muduo::net::UdpConnectionPtr, ClientProxyPtr >	UdpConnToClientMap;
	typedef std::shared_ptr< UdpConnToClientMap > UdpConnToClientMapPtr;

	muduo::net::EventLoop* GetEventLoop() { return pktDispatcher_.GetBaseLoop(); }

protected:
	void OnConnOrDisconn( const muduo::net::UdpConnectionPtr& conn );
	void PrepareOutgoingPackets();
	void PrepareGamePacket( ClientProxyPtr inClientProxy, const uint32_t inConnFlag );

	virtual void ProcessPacket( InputBitStream& inInputStream,
		const muduo::net::UdpConnectionPtr& inUdpConnetction );

	void RemoveUdpConn( const muduo::net::UdpConnectionPtr& conn );

	void	HandlePacketFromNewClient( InputBitStream& inInputStream,
		const muduo::net::UdpConnectionPtr& inUdpConnetction );

	void PktProcessFunc( ReceivedPacketPtr& recvedPacket );
private:

	ReceivedPacketBlockQueue recvedPacketBlockQ_;
	PendingSendPacketQueue pendingSndPacketQ_;

	PktDispatcher pktDispatcher_;
	PktHandler		pktHandler_;

	UdpConnToClientMap udpConnToClientMap_;
	static muduo::AtomicInt32		kNewNetId;

#else //IS_LINUX
#include "realtime_srv/net/NetworkMgrWinH.h"
#endif //IS_LINUX

};
}