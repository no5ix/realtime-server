#include "for_ue4_demo_shared.h"
#include <time.h>



std::unique_ptr< ExampleSrvForUe4Demo >	ExampleSrvForUe4Demo::sInstance;

bool ExampleSrvForUe4Demo::StaticInit()
{
#ifdef IS_LINUX

	int willBecomeDaemon = 0;
	std::string willBecomeDaemonString = RealtimeSrvHelper::GetCommandLineArg(
		COMMAND_LINE_ARG_DAEMON_INDEX );
	if ( !willBecomeDaemonString.empty() )
	{
		willBecomeDaemon = stoi( willBecomeDaemonString );
		if ( willBecomeDaemon )
		{
			if ( RealtimeSrvHelper::BecomeDaemon() == -1 )
			{
				LOG( "BecomeDaemon failed", 0 );
				return false;
			}
		}
	}
#endif //IS_LINUX

	sInstance.reset( new ExampleSrvForUe4Demo() );
	return true;
}

ExampleSrvForUe4Demo::ExampleSrvForUe4Demo()
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
void ExampleSrvForUe4Demo::SimulateRealWorld()
{
	float latency = 0.0f;
	std::string latencyString = RealtimeSrvHelper::GetCommandLineArg(
		COMMAND_LINE_ARG_LATENCY_INDEX );
	if ( !latencyString.empty() )
	{
		latency = stof( latencyString );
		NetworkMgr::sInst->SetSimulatedLatency( latency );
	}

	float dropPacketChance = 0.0f;
	std::string dropPacketChanceString = RealtimeSrvHelper::GetCommandLineArg(
		COMMAND_LINE_ARG_DROP_PACKET_CHANCE_INDEX );
	if ( !dropPacketChanceString.empty() )
	{
		dropPacketChance = stof( dropPacketChanceString );
		NetworkMgr::sInst->SetDropPacketChance( dropPacketChance );
	}

	int IsSimulatedJitter = 0;
	std::string IsSimulatedJitterString = RealtimeSrvHelper::GetCommandLineArg(
		COMMAND_LINE_ARG_IS_SIMULATED_JITTER_INDEX );
	if ( !IsSimulatedJitterString.empty() )
	{
		IsSimulatedJitter = stoi( IsSimulatedJitterString );
		if ( IsSimulatedJitter )
		{
			NetworkMgr::sInst->SetIsSimulatedJitter( true );
		}
	}
}
#endif // !IS_LINUX

void ExampleSrvForUe4Demo::InitNetworkMgr()
{
	uint16_t port = DEFAULT_REALTIME_SRV_PORT;
	std::string portString = RealtimeSrvHelper::GetCommandLineArg(
		COMMAND_LINE_ARG_PORT_INDEX );
	if ( portString != std::string() )
	{
		port = stoi( portString );
	}
	NetworkMgr::StaticInit( port );
}


void ExampleSrvForUe4Demo::HandleNewClient( ClientProxyPtr inClientProxy )
{
	int playerId = inClientProxy->GetPlayerId();

	SpawnCharacterForPlayer( playerId );
}

void ExampleSrvForUe4Demo::SpawnCharacterForPlayer( int inPlayerId )
{
	CharacterPtr character = std::static_pointer_cast< Character >(
		EntityFactory::sInstance->CreateGameObject( 'CHRT' ) );

	character->SetPlayerId( inPlayerId );

	character->SetLocation( Vector3(
		2500.f + RealtimeSrvMath::GetRandomFloat() * -5000.f,
		2500.f + RealtimeSrvMath::GetRandomFloat() * -5000.f,
		0.f ) );

	character->SetRotation( Vector3(
		0.f,
		RealtimeSrvMath::GetRandomFloat() * 180.f,
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

int ExampleSrvForUe4Demo::Run()
{
	NetworkMgr::sInst->setWorldUpdateCallback(
		std::bind( &World::Update, World::sInst.get() )
	);
	NetworkMgr::sInst->Start();
	return 0;
}

#else

int ExampleSrvForUe4Demo::Run()
{
	bool quit = false;
	while ( !quit )
	{
		//RealTimeSrvTiming::sInstance.Update();
		NetworkMgr::sInst->ProcessIncomingPackets();

		NetworkMgr::sInst->CheckForDisconnects();

		World::sInst->Update();

		NetworkMgr::sInst->SendOutgoingPackets();
	}
	return 0;
}
#endif //IS_LINUX