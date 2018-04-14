#include "ActionServerPCH.h"



std::unique_ptr< Server >	Server::sInstance;


//uncomment this when you begin working on the server

bool Server::StaticInit()
{
	sInstance.reset( new Server() );

	return true;
}


void Server::DoFrame()
{
	NetworkManagerServer::sInstance->ProcessIncomingPackets();

	//NetworkManagerServer::sInstance->CheckForDisconnects();

	//NetworkManagerServer::sInstance->RespawnCats();

	//Engine::DoFrame();

	//NetworkManagerServer::sInstance->SendOutgoingPackets();

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

Server::Server()
{
	InitNetworkManager();

	//NetworkManagerServer::sInstance->SetDropPacketChance( 0.8f );
	//NetworkManagerServer::sInstance->SetSimulatedLatency( 0.25f );
	//NetworkManagerServer::sInstance->SetSimulatedLatency( 0.5f );
	//NetworkManagerServer::sInstance->SetSimulatedLatency( 0.1f );

	// Setup latency & Setup DropPacketChance
	float latency = 0.0f;
	string latencyString = Utility::GetCommandLineArg( 2 ); // 以秒为单位, 建议填入 0.5 来测试, 即模拟延迟500毫秒的情况; 不填, 则不模拟延迟
	if (!latencyString.empty())
	{
		latency = stof( latencyString );
	}
	NetworkManagerServer::sInstance->SetSimulatedLatency( latency );
	NetworkManagerServer::sInstance->SetDropPacketChance( latency );
}

bool Server::InitNetworkManager()
{
	string portString = Utility::GetCommandLineArg( 1 );
	uint16_t port = stoi( portString );

	return NetworkManagerServer::StaticInit( port );
}

void Server::HandleNewClient( ClientProxyPtr inClientProxy )
{

	//int playerId = inClientProxy->GetPlayerId();

	//ScoreBoardManager::sInstance->AddEntry( playerId, inClientProxy->GetName() );
	//SpawnCatForPlayer( playerId );
}