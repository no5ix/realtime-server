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
		// Parameter: const NewPlayerCallback & NewPlayerCB for spawning your own GameObject.
		// Parameter: uint16_t Port default is DEFAULT_REALTIME_SRV_PORT, see RealtimeSrvShared.h
		//************************************
		void Run( const NewPlayerCallback& NewPlayerCb,
			uint16_t Port = DEFAULT_REALTIME_SRV_PORT );


		void SimulateRealWorldOnWindows(
			uint8_t LatencyCmdIndex,
			uint8_t dropPacketChanceCmdIndex = 0,
			uint8_t JitterCmdIndex = 0 );

	private:
		std::unique_ptr<World>	world_;
		std::unique_ptr<NetworkMgr>	networkManager_;
	};

}
#endif // REALTIME_SRV_REALTIME_SERVER_H