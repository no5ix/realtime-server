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

	//	假设 : 
	// 	LatencyCmdIndex = 2;
	//	dropPacketChanceCmdIndex = 3;
	//	JitterCmdIndex = 4;
	//
	//	那在Windows上,如果输入命令:   ./example_for_ue4_demo.exe 0 0.3 0.8 1
	//	则服务器将会模拟 : 
	//	-	模拟延迟为 0.3 : 当前延迟+300毫秒的延迟, 若当前延迟为30ms,
	//			则模拟之后的延迟约为30+300=330左右
	//	-	模拟丢包率为 0.8 : 百分之八十的丢包率
	//	-	模拟网络抖动为 : 1为随机抖动, 0为不抖动
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