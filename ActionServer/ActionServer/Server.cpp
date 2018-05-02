#include "ActionServerPCH.h"
#include <time.h>



std::unique_ptr< Server >	Server::sInstance;


//uncomment this when you begin working on the server

bool Server::StaticInit()
{
	sInstance.reset( new Server() );

	return true;
}

Server::~Server()
{
	UDPSocket::CleanUp();
}

Server::Server()
{


	srand( static_cast< uint32_t >( time( nullptr ) ) );

	GameObjectRegistry::StaticInit();


	World::StaticInit();

	GameObjectRegistry::sInstance->RegisterCreationFunction( 'CHRT', CharacterServer::StaticCreate );

	InitNetworkManager();

	//NetworkManagerServer::sInstance->SetDropPacketChance( 0.8f );
	//NetworkManagerServer::sInstance->SetSimulatedLatency( 0.25f );
	//NetworkManagerServer::sInstance->SetSimulatedLatency( 0.5f );
	//NetworkManagerServer::sInstance->SetSimulatedLatency( 0.1f );

	// Setup latency
	float latency = 0.0f;
	string latencyString = Utility::GetCommandLineArg( 2 );
	if (!latencyString.empty())
	{
		latency = stof( latencyString );
		NetworkManagerServer::sInstance->SetSimulatedLatency( latency );
	}

	// Setup DropPacketChance
	float dropPacketChance = 0.0f;
	string dropPacketChanceString = Utility::GetCommandLineArg( 3 );
	if (!dropPacketChanceString.empty())
	{
		dropPacketChance = stof( dropPacketChanceString );
		NetworkManagerServer::sInstance->SetDropPacketChance( dropPacketChance );
	}
	// Setup Whether Simulate Jitter
	int IsSimulatedJitter = 0;
	string IsSimulatedJitterString = Utility::GetCommandLineArg( 4 );
	if ( !IsSimulatedJitterString.empty() )
	{
		IsSimulatedJitter = stoi( IsSimulatedJitterString );
		if ( IsSimulatedJitter )
		{
			NetworkManagerServer::sInstance->SetIsSimulatedJitter( true );
		}
	}


}

bool Server::InitNetworkManager()
{
	string portString = Utility::GetCommandLineArg( 1 );
	uint16_t port = stoi( portString );

	return NetworkManagerServer::StaticInit( port );
}

int Server::Run()
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


void Server::DoFrame()
{
	NetworkManagerServer::sInstance->ProcessIncomingPackets();

	//NetworkManagerServer::sInstance->CheckForDisconnects();

	World::sInstance->Update();

	NetworkManagerServer::sInstance->SendOutgoingPackets();

}

void Server::HandleNewClient( ClientProxyPtr inClientProxy )
{
	int playerId = inClientProxy->GetPlayerId();

	//ScoreBoardManager::sInstance->AddEntry( playerId, inClientProxy->GetName() );
	SpawnCharacterForPlayer( playerId );
}

void Server::SpawnCharacterForPlayer( int inPlayerId )
{
	CharacterPtr character = std::static_pointer_cast< Character >( GameObjectRegistry::sInstance->CreateGameObject( 'CHRT' ) );

	character->SetPlayerId( inPlayerId );

	character->SetLocation( Vector3(
		-1000.f + ( static_cast< float >( inPlayerId ) * ActionServerMath::GetRandomFloat() * 400.f ),
		static_cast< float >( inPlayerId  * ActionServerMath::GetRandomFloat() * 100 ),
		0.f ) );

	character->SetRotation( Vector3(
		0.f,
		ActionServerMath::Clamp( static_cast< float >( inPlayerId ) * ActionServerMath::GetRandomFloat() * 200.f, 0.f, 180.f ),
		0.f ) );

	//character->SetRotation( Vector3( -1000.f + static_cast< float >( inPlayerId ), static_cast< float >( inPlayerId ), 40.f ) );

	//character->SetRotation( Vector3( 0.f, 180.f, 0.f ) );
}
