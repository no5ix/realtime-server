#include "realtime_srv/RealtimeServer.h"


using namespace realtime_srv;


realtime_srv::RealtimeServer::RealtimeServer(
	bool _willDaemonizeOnLinux /*= false*/,
	uint16_t _port /*= DEFAULT_REALTIME_SRV_PORT */ )
{
	srand( static_cast< uint32_t >( time( nullptr ) ) );

	world_.reset( new World() );
	assert( world_ );

	if ( _willDaemonizeOnLinux && !RealtimeSrvHelper::DaemonizeOnLinux() )
	{
		LOG( " Become Daemon Failed!! " );
	}

	networkManager_.reset( new NetworkMgr( _port ) );
	assert( networkManager_ );
	networkManager_->SetWorldUpdateCallback( [&]() { world_->Update(); } );
	networkManager_->SetWorldRegistryCallback( std::bind(
		&World::Registry, world_.get(), _1, _2 ) );
                                   
	world_->SetNotifyAllClientCallback( std::bind(
		&NetworkMgr::OnObjCreateOrDestroy, networkManager_.get(), _1, _2 ) );
}