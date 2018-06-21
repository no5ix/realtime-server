#include "RealTimeSrvPCH.h"
#include <time.h>



std::unique_ptr< RealTimeSrv >	RealTimeSrv::sInstance;

bool RealTimeSrv::StaticInit()
{
#ifdef NEW_EPOLL_INTERFACE
	::signal( SIGPIPE, SIG_IGN );

	int willBecomeDaemon = 0;
	std::string willBecomeDaemonString = RealTimeSrvHelper::GetCommandLineArg(
		COMMAND_LINE_ARG_DAEMON_INDEX );
	if ( !willBecomeDaemonString.empty() )
	{
		willBecomeDaemon = stoi( willBecomeDaemonString );
		if ( willBecomeDaemon )
		{
			if ( RealTimeSrv::BecomeDaemon() == -1 )
			{
				LOG( "BecomeDaemon failed", 0 );
				return false;
			}
		}
	}
#endif //NEW_EPOLL_INTERFACE

	sInstance.reset( new RealTimeSrv() );
	return true;
}

RealTimeSrv::~RealTimeSrv()
{
	UDPSocketInterface::CleanUp();
}

#ifdef NEW_EPOLL_INTERFACE
int RealTimeSrv::BecomeDaemon()
{
	int maxfd, fd;

	switch ( fork() )
	{                   /* Become background process */
	case -1:
		return -1;
	case 0:
		break;                     /* Child falls through... */
	default:
		_exit( EXIT_SUCCESS );       /* while parent terminates */
	}

	if ( setsid() == -1 )                 /* Become leader of new session */
		return -1;

	switch ( fork() )
	{                   /* Ensure we are not session leader */
	case -1:
		return -1;
	case 0:
		break;
	default:
		_exit( EXIT_SUCCESS );
	}

	umask( 0 );                       /* Clear file mode creation mask */

	chdir( "/" );                     /* Change to root directory */

	maxfd = sysconf( _SC_OPEN_MAX );
	if ( maxfd == -1 )                /* Limit is indeterminate... */
		maxfd = 8192;				  /* so take a guess */

	for ( fd = 0; fd < maxfd; fd++ )
		close( fd );

	close( STDIN_FILENO );            /* Reopen standard fd's to /dev/null */

	fd = open( "/dev/null", O_RDWR ); // open 返回的文件描述符一定是最小的未被使用的描述符。

	if ( fd != STDIN_FILENO )         /* 'fd' should be 0 */
		return -1;
	if ( dup2( STDIN_FILENO, STDOUT_FILENO ) != STDOUT_FILENO )
		return -1;
	if ( dup2( STDIN_FILENO, STDERR_FILENO ) != STDERR_FILENO )
		return -1;

	return 0;
}
#endif //NEW_EPOLL_INTERFACE

RealTimeSrv::RealTimeSrv()
{
	srand( static_cast< uint32_t >( time( nullptr ) ) );

	EntityFactory::StaticInit();

	World::StaticInit();

	EntityFactory::sInstance->RegisterCreationFunction( 'CHRT', CharacterSrv::StaticCreate );

	InitNetworkMgr();

	SimulateRealWorld();
}

void RealTimeSrv::SimulateRealWorld()
{
	float latency = 0.0f;
	std::string latencyString = RealTimeSrvHelper::GetCommandLineArg(
		COMMAND_LINE_ARG_LATENCY_INDEX );
	if ( !latencyString.empty() )
	{
		latency = stof( latencyString );
		NetworkMgrSrv::sInst->SetSimulatedLatency( latency );
	}

	float dropPacketChance = 0.0f;
	std::string dropPacketChanceString = RealTimeSrvHelper::GetCommandLineArg(
		COMMAND_LINE_ARG_DROP_PACKET_CHANCE_INDEX );
	if ( !dropPacketChanceString.empty() )
	{
		dropPacketChance = stof( dropPacketChanceString );
		NetworkMgrSrv::sInst->SetDropPacketChance( dropPacketChance );
	}

	int IsSimulatedJitter = 0;
	std::string IsSimulatedJitterString = RealTimeSrvHelper::GetCommandLineArg(
		COMMAND_LINE_ARG_IS_SIMULATED_JITTER_INDEX );
	if ( !IsSimulatedJitterString.empty() )
	{
		IsSimulatedJitter = stoi( IsSimulatedJitterString );
		if ( IsSimulatedJitter )
		{
			NetworkMgrSrv::sInst->SetIsSimulatedJitter( true );
		}
	}
}

bool RealTimeSrv::InitNetworkMgr()
{
	uint16_t port = 44444;
	std::string portString = RealTimeSrvHelper::GetCommandLineArg(
		COMMAND_LINE_ARG_PORT_INDEX );
	if ( portString != std::string() )
	{
		port = stoi( portString );
	}
	return NetworkMgrSrv::StaticInit( port );
}


void RealTimeSrv::HandleNewClient( ClientProxyPtr inClientProxy )
{
	int playerId = inClientProxy->GetPlayerId();

	SpawnCharacterForPlayer( playerId );
}

void RealTimeSrv::SpawnCharacterForPlayer( int inPlayerId )
{
	CharacterPtr character = std::static_pointer_cast< Character >(
		EntityFactory::sInstance->CreateGameObject( 'CHRT' ) );

	character->SetPlayerId( inPlayerId );

	character->SetLocation( Vector3(
		2500.f + RealTimeSrvMath::GetRandomFloat() * -5000.f,
		2500.f + RealTimeSrvMath::GetRandomFloat() * -5000.f,
		0.f ) );

	character->SetRotation( Vector3(
		0.f,
		RealTimeSrvMath::GetRandomFloat() * 180.f,
		0.f ) );


	//for ( int count = inPlayerId * 100; count < inPlayerId * 100 + 561; ++count )
	//{
	//	CharacterPtr character = std::static_pointer_cast< Character >( EntityFactory::sInstance->CreateGameObject( 'CHRT' ) );

	//	character->SetPlayerId( inPlayerId * count );


	//	character->SetLocation( Vector3(
	//		2500.f + RealTimeSrvMath::GetRandomFloat() * -5000.f,
	//		2500.f + RealTimeSrvMath::GetRandomFloat() * -5000.f,
	//		0.f ) );

	//	character->SetRotation( Vector3(
	//		0.f,
	//		RealTimeSrvMath::GetRandomFloat() * 180.f,
	//		0.f ) );

	//}
}


#ifdef NEW_EPOLL_INTERFACE

int RealTimeSrv::Run()
{
	NetworkMgrSrv::sInst->setWorldUpdateCallback(
		std::bind( &World::Update, World::sInst.get() )
	);
	NetworkMgrSrv::sInst->Start();
	return 0;
}

#else

int RealTimeSrv::Run()
{
	bool quit = false;
	while ( !quit )
	{
		//RealTimeSrvTiming::sInstance.Update();
		DoFrame();
	}
	return 0;
}

void RealTimeSrv::DoFrame()
{
	NetworkMgrSrv::sInst->ProcessIncomingPackets();

	NetworkMgrSrv::sInst->CheckForDisconnects();

	World::sInst->Update();

	NetworkMgrSrv::sInst->SendOutgoingPackets();
}
#endif //NEW_EPOLL_INTERFACE