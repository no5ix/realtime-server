#include <realtime_srv/RealtimeServer.h>
#include "Character.h"
#include "ExampleSrvForUe4Demo.h"



std::unique_ptr< ExampleSrvForUe4Demo >	ExampleSrvForUe4Demo::sInst;

bool ExampleSrvForUe4Demo::StaticInit()
{
	sInst.reset( new ExampleSrvForUe4Demo() );
	return true;
}

void ExampleSrvForUe4Demo::Run()
{
	RealtimeServer::Run();
}

ExampleSrvForUe4Demo::ExampleSrvForUe4Demo()
{
	const uint8_t becomeDaemonCmdIndex = 1;
	bool whetherTobecomeDaemon =
		RealtimeSrvHelper::GetCommandLineArg( becomeDaemonCmdIndex ).empty() ? true : false;

	NewPlayerCallback newPlayerCb = std::bind( 
		&ExampleSrvForUe4Demo::SpawnNewCharacterForPlayer,
		ExampleSrvForUe4Demo::sInst.get(), _1 );

	RealtimeServer::Init( newPlayerCb, whetherTobecomeDaemon );
}

GameObjPtr ExampleSrvForUe4Demo::SpawnNewCharacterForPlayer( int inPlayerId )
{
	GameObjPtr newGameObj = Character::StaticCreate();
	CharacterPtr character = std::static_pointer_cast< Character >( newGameObj );

	character->SetPlayerId( inPlayerId );
	character->SetLocation( Vector3(
		2500.f + RealtimeSrvMath::GetRandomFloat() * -5000.f,
		2500.f + RealtimeSrvMath::GetRandomFloat() * -5000.f,
		0.f ) );
	character->SetRotation( Vector3(
		0.f,
		RealtimeSrvMath::GetRandomFloat() * 180.f,
		0.f ) );
	return newGameObj;
}
