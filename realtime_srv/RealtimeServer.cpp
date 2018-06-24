#include "realtime_srv/RealtimeServer.h"




RealtimeServer::RealtimeServer()
	: world_( new World() ), networkManager_( new NetworkMgr() )
{}


void RealtimeServer::Init( const NewPlayerCallback& NewPlayerCB,
	uint16_t Port /*= DEFAULT_REALTIME_SRV_PORT*/ )
{
	if ( !networkManager_->Init( Port ) )
	{
		LOG( " Network Manager Init Failed!! ", 0 );
	}
	srand( static_cast< uint32_t >( time( nullptr ) ) );


	networkManager_->SetNewPlayerCallBack( NewPlayerCB );
	networkManager_->SetWorldUpdateCallback(
		std::bind( &World::Update, world_.get() ) );
	networkManager_->SetWorldRegistryCallback( std::bind(
		&World::Registry, world_.get(), _1, _2 ) );

	world_->SetNotifyAllClientCallBack( std::bind(
		&NetworkMgr::NotifyAllClient, networkManager_.get(), _1, _2 ) );
}


void RealtimeServer::Run()
{
	assert( networkManager_ );
	networkManager_->Start();
}