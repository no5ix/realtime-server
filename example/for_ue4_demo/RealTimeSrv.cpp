#include "for_ue4_demo_shared.h"
#include <time.h>



std::unique_ptr< RealTimeSrv >	RealTimeSrv::sInstance;

bool RealTimeSrv::StaticInit()
{
#ifdef IS_LINUX
	::signal( SIGPIPE, SIG_IGN );

	int willBecomeDaemon = 0;
	std::string willBecomeDaemonString = RealTimeSrvHelper::GetCommandLineArg(
		COMMAND_LINE_ARG_DAEMON_INDEX );
	if ( !willBecomeDaemonString.empty() )
	{
		willBecomeDaemon = stoi( willBecomeDaemonString );
		if ( willBecomeDaemon )
		{
			if ( RealTimeSrvHelper::BecomeDaemon() == -1 )
			{
				LOG( "BecomeDaemon failed", 0 );
				return false;
			}
		}
	}
#endif //IS_LINUX

	sInstance.reset( new RealTimeSrv() );
	return true;
}

RealTimeSrv::RealTimeSrv()
{
	srand( static_cast< uint32_t >( time( nullptr ) ) );

	EntityFactory::StaticInit();

	World::StaticInit();

	EntityFactory::sInstance->RegisterCreationFunction( 'CHRT', Character::StaticCreate );

	InitNetworkMgr();

#ifndef IS_LINUX
	SimulateRealWorld();
#endif
}

#ifndef IS_LINUX
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
#endif // !IS_LINUX

bool RealTimeSrv::InitNetworkMgr()
{
	uint16_t port = DEFAULT_REALTIME_SRV_PORT;
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


#ifdef IS_LINUX

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
		NetworkMgrSrv::sInst->ProcessIncomingPackets();

		NetworkMgrSrv::sInst->CheckForDisconnects();

		World::sInst->Update();

		NetworkMgrSrv::sInst->SendOutgoingPackets();
	}
	return 0;
}
#endif //IS_LINUX