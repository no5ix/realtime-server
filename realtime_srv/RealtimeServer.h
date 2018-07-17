#ifndef REALTIME_SRV_REALTIME_SERVER_H
#define REALTIME_SRV_REALTIME_SERVER_H

#include "realtime_srv/common/RealtimeSrvShared.h"



namespace realtime_srv
{

class RealtimeServer : noncopyable
{
public:

	//************************************
	// @Parameter const NewPlayerCallback : for spawning your own GameObject class.
	// @Parameter const CustomInputStateCallback : for using your own InputState class.
	// @Parameter uint16_t Port : default is DEFAULT_REALTIME_SRV_PORT, see RealtimeSrvMacro.h
	//************************************
	RealtimeServer( 
		const NewPlayerCallback& NewPlayerCb,
		const CustomInputStateCallback& CustomInputStateCb,
		bool willDaemonizeOnLinux = false,
		uint16_t Port = DEFAULT_REALTIME_SRV_PORT );


	void Run() { networkManager_->Start(); }


	void SimulateRealWorldOnWin(
		uint8_t LatencyCmdIndex,
		uint8_t dropPacketChanceCmdIndex = 0,
		uint8_t JitterCmdIndex = 0 );


#ifdef IS_LINUX
	muduo::net::EventLoop* GetEventLoop()
	{ return networkManager_->GetEventLoop(); }
#endif //IS_LINUX

private:
	std::unique_ptr<World>	world_;
	std::unique_ptr<NetworkMgr>	networkManager_;
};

}
#endif // REALTIME_SRV_REALTIME_SERVER_H