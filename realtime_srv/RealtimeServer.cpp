#include "realtime_srv/RealtimeServer.h"


using namespace realtime_srv;


RealtimeServer::RealtimeServer( bool willDaemonizeOnLinux /*= false*/ ) 
	: world_( new World() )
{
	assert( world_ );

	if ( willDaemonizeOnLinux && !RealtimeSrvHelper::DaemonizeOnLinux() )
	{
		LOG( " Become Daemon Failed!! ");
	}

	networkManager_.reset( new NetworkMgr() );
	assert( networkManager_ );
}


void RealtimeServer::Run( const NewPlayerCallback& NewPlayerCB,
	uint16_t Port /*= DEFAULT_REALTIME_SRV_PORT*/ )
{
	if ( !networkManager_->Init( Port ) )
	{
		LOG( " Network Manager Init Failed!! " );
	}

	srand( static_cast< uint32_t >( time( nullptr ) ) );

	networkManager_->SetNewPlayerCallBack( NewPlayerCB );
	networkManager_->SetWorldUpdateCallback(
		std::bind( &World::Update, world_.get() ) );
	networkManager_->SetWorldRegistryCallback( std::bind(
		&World::Registry, world_.get(), _1, _2 ) );

	world_->SetNotifyAllClientCallBack( std::bind(
		&NetworkMgr::NotifyAllClient, networkManager_.get(), _1, _2 ) );

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

