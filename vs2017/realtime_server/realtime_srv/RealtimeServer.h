#ifndef REALTIME_SRV_REALTIME_SERVER_H
#define REALTIME_SRV_REALTIME_SERVER_H

#include "realtime_srv/common/RealtimeSrvShared.h"



namespace realtime_srv
{


class RealtimeServer : noncopyable
{
public:

	RealtimeServer();

	void Run() { networkMgr_->Start(); }

	std::shared_ptr<World> GetWorld() { return world_; }

	std::shared_ptr<NetworkMgr> GetNetworkManager() { return networkMgr_; }

	ServerConfig GetServerConfig() const { return srvConf_; }

private:

	void ReadConfigFile();

private:

	std::shared_ptr<World>	world_;
	std::shared_ptr<NetworkMgr>	networkMgr_;
	ServerConfig srvConf_;
};

}
#endif // REALTIME_SRV_REALTIME_SERVER_H