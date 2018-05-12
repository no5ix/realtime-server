#include "RealTimeSrvPCH.h"
#include <time.h>



std::unique_ptr< RealTimeSrv >	RealTimeSrv::sInstance;



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

	float latency = 0.0f;
	string latencyString = RealTimeSrvHelper::GetCommandLineArg( 2 );
	if (!latencyString.empty())
	{
		latency = stof( latencyString );
		NetworkMgrSrv::sInst->SetSimulatedLatency( latency );
	}

	float dropPacketChance = 0.0f;
	string dropPacketChanceString = RealTimeSrvHelper::GetCommandLineArg( 3 );
	if (!dropPacketChanceString.empty())
	{
		dropPacketChance = stof( dropPacketChanceString );
		NetworkMgrSrv::sInst->SetDropPacketChance( dropPacketChance );
	}

	int IsSimulatedJitter = 0;
	string IsSimulatedJitterString = RealTimeSrvHelper::GetCommandLineArg( 4 );
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
	string portString = RealTimeSrvHelper::GetCommandLineArg( 1 );
	if (portString != string())
	{
		port = stoi( portString );
	}

	return NetworkMgrSrv::StaticInit( port );
}

int RealTimeSrv::Run()
{
	bool quit = false;

	while (!quit)
	{
		RealTimeSrvTiming::sInstance.Update();

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

void RealTimeSrv::HandleNewClient( ClientProxyPtr inClientProxy )
{
	int playerId = inClientProxy->GetPlayerId();

	SpawnCharacterForPlayer( playerId );
}

void RealTimeSrv::SpawnCharacterForPlayer( int inPlayerId )
{
	CharacterPtr character = std::static_pointer_cast< Character >( EntityFactory::sInstance->CreateGameObject( 'CHRT' ) );

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