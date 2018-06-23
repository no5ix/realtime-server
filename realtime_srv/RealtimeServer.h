#ifndef REALTIME_SERVER_H
#define REALTIME_SERVER_H

#include "realtime_srv/common/RealtimeSrvShared.h"





typedef std::function< GameObjPtr( int NewPlayerId ) > NewPlayerCallback;


namespace RealtimeServer
{

	//************************************
	// Parameter: const NewPlayerCallback & NewPlayerCB for spawning your own GameObject.
	// Parameter: bool BecomeDaemon is only for Linux, default is off.
	// Parameter: uint16_t Port default is DEFAULT_REALTIME_SRV_PORT, see RealtimeSrvShared.h
	//************************************
	bool Init( const NewPlayerCallback& NewPlayerCB,
		bool BecomeDaemonOnLinux = false,
		uint16_t Port = DEFAULT_REALTIME_SRV_PORT );

	void Run();

}

#endif // REALTIME_SERVER_H