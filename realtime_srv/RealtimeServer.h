#ifndef REALTIME_SRV_REALTIME_SERVER_H
#define REALTIME_SRV_REALTIME_SERVER_H

#include "realtime_srv/common/RealtimeSrvShared.h"



namespace realtime_srv
{

class RealtimeServer : noncopyable
{
public:

	//************************************
	// @Parameter uint16_t Port : default is DEFAULT_REALTIME_SRV_PORT, see RealtimeSrvMacro.h
	//************************************
	RealtimeServer(
		bool _willDaemonizeOnLinux = false,
		uint16_t _port = DEFAULT_REALTIME_SRV_PORT );


	void Run()
	{
		LOG( "Server running as '%s Unregist GameObj When Client Disconnect' mode.",
			( networkManager_->GetUnregistObjWhenCliDisconn() ? "" : "Not" ) );
		networkManager_->Start();
	}


	//	在Windows上 ( Linux为了减少代码分支, 不提供此功能 ), 
	//	如果输入命令:   ./example_for_ue4_demo.exe 0.3 0.8 1
	//	则服务器将会模拟 : 
	//	-	模拟延迟为 0.3 : 当前延迟+300毫秒的延迟, 若当前延迟为30ms, 则模拟之后的延迟约为30+300=330左右
	//	-	模拟丢包率为 0.8 : 百分之八十的丢包率
	//	-	模拟网络抖动为 1 : 1为随机抖动, 0为不抖动
	void SimulateRealWorldOnWin()
	{ RealtimeSrvHelper::SimulateRealWorldNetCondition( networkManager_ ); }


	std::shared_ptr<World> GetWorld()
	{ return world_; }

	std::shared_ptr<NetworkMgr> GetNetworkManager()
	{ return networkManager_; }


private:
	std::shared_ptr<World>	world_;
	std::shared_ptr<NetworkMgr>	networkManager_;
};

}
#endif // REALTIME_SRV_REALTIME_SERVER_H