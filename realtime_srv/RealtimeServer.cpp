#include "realtime_srv/RealtimeServer.h"


using namespace realtime_srv;


RealtimeServer::RealtimeServer( const NewPlayerCallback& NewPlayerCb,
	bool willDaemonizeOnLinux /*= false*/,
	uint16_t Port /*= DEFAULT_REALTIME_SRV_PORT*/ )
	: world_( new World() )
{
	assert( world_ );
	if ( willDaemonizeOnLinux && !RealtimeSrvHelper::DaemonizeOnLinux() )
	{
		LOG( " Become Daemon Failed!! " );
	}

#ifdef IS_LINUX
	networkManager_.reset( new NetworkMgr( Port ) );
	assert( networkManager_ );
	if ( !networkManager_->Init() )
#else
	networkManager_.reset( new NetworkMgr() );
	assert( networkManager_ );
	if ( !networkManager_->Init( Port ) )
#endif //IS_LINUX
	{
		LOG( " Network Manager Init Failed!! " );
	}

	srand( static_cast< uint32_t >( time( nullptr ) ) );

	networkManager_->SetNewPlayerCallback( NewPlayerCb );
	networkManager_->SetWorldUpdateCallback( [&]() { world_->Update(); } );
	networkManager_->SetWorldRegistryCallback( std::bind(
		&World::Registry, world_.get(), _1, _2 ) );

	world_->SetNotifyAllClientCallback( std::bind(
		&NetworkMgr::NotifyAllClient, networkManager_.get(), _1, _2 ) );
}



void RealtimeServer::SimulateRealWorldOnWin(
	uint8_t LatencyCmdIndex,
	uint8_t DropPacketChanceCmdIndex /*= 0*/,
	uint8_t JitterCmdIndex /*= 0*/ )
{
	assert( networkManager_ );
	RealtimeSrvHelper::SimulateRealWorldNetCondition(
		networkManager_.get(),
		LatencyCmdIndex,
		DropPacketChanceCmdIndex,
		JitterCmdIndex );
}

