#include "realtime_srv/RealtimeServer.h"


using namespace realtime_srv;


realtime_srv::RealtimeServer::RealtimeServer(
	const NewPlayerCallback& _newPlayerCb,
	const CustomInputStateCallback& _customInputStateCb,
	bool _willDaemonizeOnLinux /*= false*/,
	uint16_t _port /*= DEFAULT_REALTIME_SRV_PORT */ )
{
	world_.reset( new World() );
	assert( world_ );

	if ( _willDaemonizeOnLinux && !RealtimeSrvHelper::DaemonizeOnLinux() )
	{
		LOG( " Become Daemon Failed!! " );
	}

	networkManager_.reset( new NetworkMgr( _port ) );
	assert( networkManager_ );

	srand( static_cast< uint32_t >( time( nullptr ) ) );

	networkManager_->SetNewPlayerCallback( _newPlayerCb );
	networkManager_->SetCustomInputStateCallback( _customInputStateCb );

	networkManager_->SetWorldUpdateCallback( [&]() { world_->Update(); } );
	networkManager_->SetWorldRegistryCallback( std::bind(
		&World::Registry, world_.get(), _1, _2 ) );
                                   
	world_->SetNotifyAllClientCallback( std::bind(
		&NetworkMgr::NotifyAllClient, networkManager_.get(), _1, _2 ) );
}