#include "realtime_srv/RealtimeServer.h"




RealtimeServer::RealtimeServer() : world_( new World() )
{}


bool RealtimeServer::Init( const NewPlayerCallback& NewPlayerCB,
	bool BecomeDaemon /*= false*/,
	uint16_t Port /*= DEFAULT_REALTIME_SRV_PORT*/ )
{
	assert( world_ );
	if ( BecomeDaemon && !RealtimeSrvHelper::BecomeDaemonOnLinux() )
	{
		LOG( " Become Daemon Failed!! ", 0 ); return false;
	}
	networkManager_.reset( new NetworkMgr() );
	assert( networkManager_ );
	if ( !networkManager_->Init( Port ) )
	{
		LOG( " Network Manager Init Failed!! ", 0 ); return false;
	}

	srand( static_cast< uint32_t >( time( nullptr ) ) );

	networkManager_->SetNewPlayerCallBack( NewPlayerCB );
	networkManager_->SetWorldUpdateCallback(
		std::bind( &World::Update, world_.get() ) );
	networkManager_->SetWorldRegistryCallback( std::bind(
		&World::Registry, world_.get(), _1, _2 ) );

	world_->SetNotifyAllClientCallBack( std::bind(
		&NetworkMgr::NotifyAllClient, networkManager_.get(), _1, _2 ) );

	return true;
}


void RealtimeServer::Run()
{
	assert( networkManager_ );
	assert( world_ );
	networkManager_->Start();
}


void RealtimeServer::SimulateRealWorldOnWindows(
	uint8_t LatencyCmdIndex,
	uint8_t DropPacketChanceCmdIndex /*= 0*/,
	uint8_t JitterCmdIndex /*= 0*/ )
{
	assert( networkManager_ );
	RealtimeSrvHelper::SimulateRealWorldOnWin(
		networkManager_.get(),
		LatencyCmdIndex,
		DropPacketChanceCmdIndex,
		JitterCmdIndex );
}

