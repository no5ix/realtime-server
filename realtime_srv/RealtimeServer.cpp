#include "realtime_srv/RealtimeServer.h"




bool RealtimeServer::Init( const NewPlayerCallback& NewPlayerCB,
	bool BecomeDaemonOnLinux /*= false*/, uint16_t Port /*= DEFAULT_REALTIME_SRV_PORT */ )
{
#ifdef IS_LINUX
	if ( BecomeDaemonOnLinux && RealtimeSrvHelper::BecomeDaemon() == -1 )
	{
		LOG( " Become daemon failed!! ", 0 );
		return false;
	}
#endif //IS_LINUX

	srand( static_cast< uint32_t >( time( nullptr ) ) );

	World::StaticInit();

	NetworkMgr::StaticInit( Port );
	NetworkMgr::sInstance->SetNewPlayerCallBack( NewPlayerCB );
	NetworkMgr::sInstance->SetWorldUpdateCallback(
		std::bind( &World::Update, World::sInstance.get() ) );

	return true;
}


void RealtimeServer::Run()
{
	assert( NetworkMgr::sInstance );
	NetworkMgr::sInstance->Start();
}