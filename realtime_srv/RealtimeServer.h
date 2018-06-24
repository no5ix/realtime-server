#ifndef REALTIME_SERVER_H
#define REALTIME_SERVER_H

#include "realtime_srv/common/RealtimeSrvShared.h"





typedef std::function< GameObjPtr( ClientProxyPtr newClientProxy ) > NewPlayerCallback;


class RealtimeServer
{
public:
	RealtimeServer();

	//************************************
	// Parameter: const NewPlayerCallback & NewPlayerCB for spawning your own GameObject.
	// Parameter: bool BecomeDaemon is only for Linux, default is off.
	// Parameter: uint16_t Port default is DEFAULT_REALTIME_SRV_PORT, see RealtimeSrvShared.h
	//************************************
	bool Init( const NewPlayerCallback& NewPlayerCB,
		bool BecomeDaemon = false,
		uint16_t Port = DEFAULT_REALTIME_SRV_PORT );


	void Run();

	void SimulateRealWorldOnWindows(
		uint8_t LatencyCmdIndex,
		uint8_t dropPacketChanceCmdIndex = 0,
		uint8_t JitterCmdIndex = 0 );

private:
	std::unique_ptr<World>	world_;
	std::unique_ptr<NetworkMgr>	networkManager_;
};

#endif // REALTIME_SERVER_H