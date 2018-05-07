#include "RealTimeSrvPCH.h"
#include <time.h>



std::unique_ptr< RealTimeSrv >	RealTimeSrv::sInstance;


//uncomment this when you begin working on the server

bool RealTimeSrv::StaticInit()
{
	sInstance.reset( new RealTimeSrv() );

	return true;
}

RealTimeSrv::~RealTimeSrv()
{
	UDPSocketInterface::CleanUp();
}

RealTimeSrv::RealTimeSrv()
{


	srand( static_cast< uint32_t >( time( nullptr ) ) );

	EntityFactory::StaticInit();


	World::StaticInit();

	EntityFactory::sInstance->RegisterCreationFunction( 'CHRT', CharacterSrv::StaticCreate );

	InitNetworkMgr();

	// Setup latency
	float latency = 0.0f;
	string latencyString = Utility::GetCommandLineArg( 2 );
	if (!latencyString.empty())
	{
		latency = stof( latencyString );
		NetworkMgrSrv::sInst->SetSimulatedLatency( latency );
	}

	// Setup DropPacketChance
	float dropPacketChance = 0.0f;
	string dropPacketChanceString = Utility::GetCommandLineArg( 3 );
	if (!dropPacketChanceString.empty())
	{
		dropPacketChance = stof( dropPacketChanceString );
		NetworkMgrSrv::sInst->SetDropPacketChance( dropPacketChance );
	}
	// Setup SimulateJitter
	int IsSimulatedJitter = 0;
	string IsSimulatedJitterString = Utility::GetCommandLineArg( 4 );
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
	string portString = Utility::GetCommandLineArg( 1 );
	uint16_t port = stoi( portString );

	return NetworkMgrSrv::StaticInit( port );
}

int RealTimeSrv::Run()
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


void RealTimeSrv::DoFrame()
{
	NetworkMgrSrv::sInst->ProcessIncomingPackets();

	//NetworkMgrSrv::sInst->CheckForDisconnects();

	World::sInst->Update();

	NetworkMgrSrv::sInst->SendOutgoingPackets();

}

void RealTimeSrv::HandleNewClient( ClientProxyPtr inClientProxy )
{
	int playerId = inClientProxy->GetPlayerId();

	//ScoreBoardManager::sInstance->AddEntry( playerId, inClientProxy->GetName() );
	SpawnCharacterForPlayer( playerId );
}

void RealTimeSrv::SpawnCharacterForPlayer( int inPlayerId )
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
