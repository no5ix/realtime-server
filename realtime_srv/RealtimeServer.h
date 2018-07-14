#ifndef REALTIME_SRV_REALTIME_SERVER_H
#define REALTIME_SRV_REALTIME_SERVER_H

#include "realtime_srv/common/RealtimeSrvShared.h"



namespace realtime_srv
{

class RealtimeServer : noncopyable
{
public:
	RealtimeServer( bool WhetherTobecomeDaemonOnLinux = false );

	//************************************
	// @Parameter const NewPlayerCallback & NewPlayerCB : for spawning your own GameObject.
	// @Parameter bool Port IsLazy : 
	//		default to false for high performance. 
	//		if true, the server would run as lazy mode ( block until a new packet arrives ).
	// @Parameter uint16_t Port : default is DEFAULT_REALTIME_SRV_PORT, see RealtimeSrvMacro.h
	//************************************
	void Run( const NewPlayerCallback& NewPlayerCb,
		bool IsLazy = false,
		uint16_t Port = DEFAULT_REALTIME_SRV_PORT );


	void SimulateRealWorld(
		uint8_t LatencyCmdIndex,
		uint8_t dropPacketChanceCmdIndex = 0,
		uint8_t JitterCmdIndex = 0 );


//#ifdef IS_LINUX
//	muduo::net::EventLoop* GetEventLoop()
//	{ return networkManager_->GetEventLoop(); }
//#endif //IS_LINUX

private:
	std::unique_ptr<World>	world_;
	std::unique_ptr<NetworkMgr>	networkManager_;
};

}
#endif // REALTIME_SRV_REALTIME_SERVER_H