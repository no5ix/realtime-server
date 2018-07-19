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
		LOG( "Server running as '%s Destroy GameObj When Client Disconnect' mode.",
			( networkManager_->GetUnregistObjWhenCliDisconn() ? "" : "Not" ) );
		networkManager_->Start();
	}


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