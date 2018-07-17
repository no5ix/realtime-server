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
		const NewPlayerCallback& _newPlayerCb,
		const CustomInputStateCallback& _customInputStateCb,
		bool _willDaemonizeOnLinux = false,
		uint16_t _port = DEFAULT_REALTIME_SRV_PORT );


	void Run()
	{
		LOG( "Server running as '%sUnregistObjWhenClientDisconnect' mode.",
			( networkManager_->GetUnregistObjWhenCliDisconn() ? "" : "Not" ) );
		networkManager_->Start();
	}


	void SetUnregistGameObjWhenClientDisconnect( bool _wheter )
	{
		networkManager_->SetUnregistObjWhenCliDisconn( _wheter );
	}


	void SimulateRealWorldOnWin()
	{
		RealtimeSrvHelper::SimulateRealWorldNetCondition( networkManager_.get() );
	}


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