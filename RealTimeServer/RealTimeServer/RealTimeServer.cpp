#include "RealTimeServerPCH.h"
#include <time.h>



std::unique_ptr< RealTimeServer >	RealTimeServer::sInstance;


//uncomment this when you begin working on the server

bool RealTimeServer::StaticInit()
{
	sInstance.reset( new RealTimeServer() );

	return true;
}

RealTimeServer::~RealTimeServer()
{
	UDPSocketInterface::CleanUp();
}

RealTimeServer::RealTimeServer()
{


	srand( static_cast< uint32_t >( time( nullptr ) ) );

	EntityFactory::StaticInit();


	World::StaticInit();

	EntityFactory::sInstance->RegisterCreationFunction( 'CHRT', CharacterServer::StaticCreate );

	InitNetworkMgr();

	// Setup latency
	float latency = 0.0f;
	string latencyString = Utility::GetCommandLineArg( 2 );
	if (!latencyString.empty())
	{
		latency = stof( latencyString );
		NetworkMgrSrv::sInstance->SetSimulatedLatency( latency );
	}

	// Setup DropPacketChance
	float dropPacketChance = 0.0f;
	string dropPacketChanceString = Utility::GetCommandLineArg( 3 );
	if (!dropPacketChanceString.empty())
	{
		dropPacketChance = stof( dropPacketChanceString );
		NetworkMgrSrv::sInstance->SetDropPacketChance( dropPacketChance );
	}
	// Setup SimulateJitter
	int IsSimulatedJitter = 0;
	string IsSimulatedJitterString = Utility::GetCommandLineArg( 4 );
	if ( !IsSimulatedJitterString.empty() )
	{
		IsSimulatedJitter = stoi( IsSimulatedJitterString );
		if ( IsSimulatedJitter )
		{
			NetworkMgrSrv::sInstance->SetIsSimulatedJitter( true );
		}
	}


}

bool RealTimeServer::InitNetworkMgr()
{
	string portString = Utility::GetCommandLineArg( 1 );
	uint16_t port = stoi( portString );

	return NetworkMgrSrv::StaticInit( port );
}

int RealTimeServer::Run()
{
	// Main message loop
	bool quit = false;

	while (!quit)
	{
		Timing::sInstance.Update();

		DoFrame();
	}
	return 0;
}


void RealTimeServer::DoFrame()
{
	NetworkMgrSrv::sInstance->ProcessIncomingPackets();

	//NetworkManagerServer::sInstance->CheckForDisconnects();

	World::sInstance->Update();

	NetworkMgrSrv::sInstance->SendOutgoingPackets();

}

void RealTimeServer::HandleNewClient( ClientProxyPtr inClientProxy )
{
	int playerId = inClientProxy->GetPlayerId();

	//ScoreBoardManager::sInstance->AddEntry( playerId, inClientProxy->GetName() );
	SpawnCharacterForPlayer( playerId );
}

void RealTimeServer::SpawnCharacterForPlayer( int inPlayerId )
{
	CharacterPtr character = std::static_pointer_cast< Character >( EntityFactory::sInstance->CreateGameObject( 'CHRT' ) );

	character->SetPlayerId( inPlayerId );

	character->SetLocation( Vector3(
		RealTimeSrvMath::Clamp( RealTimeSrvMath::GetRandomFloat() * -1500.f, -1500.f, -600.f ),
		-1500.f + ( RealTimeSrvMath::GetRandomFloat() * 3000.f ),
		0.f ) );

	character->SetRotation( Vector3(
		0.f,
		RealTimeSrvMath::GetRandomFloat() * 180.f,
		0.f ) );

	//character->SetRotation( Vector3( -1000.f + static_cast< float >( inPlayerId ), static_cast< float >( inPlayerId ), 40.f ) );

	//character->SetRotation( Vector3( 0.f, 180.f, 0.f ) );
}
