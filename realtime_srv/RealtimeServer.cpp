#include "realtime_srv/RealtimeServer.h"


using namespace realtime_srv;


RealtimeServer::RealtimeServer( 
	const NewPlayerCallback& NewPlayerCb,
	const CustomInputStateCallback& CustomInputStateCb,
	bool willDaemonizeOnLinux /*= false*/,
	uint16_t Port /*= DEFAULT_REALTIME_SRV_PORT*/ )
{
	world_.reset( new World() );
	assert( world_ );

	if ( willDaemonizeOnLinux && !RealtimeSrvHelper::DaemonizeOnLinux() )
	{
		LOG( " Become Daemon Failed!! " );
	}

	networkManager_.reset( new NetworkMgr( Port ) );
	assert( networkManager_ );

	srand( static_cast< uint32_t >( time( nullptr ) ) );

	networkManager_->SetNewPlayerCallback( NewPlayerCb );
	networkManager_->SetCustomInputStateCallback( CustomInputStateCb );

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

