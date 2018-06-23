#include <realtime_srv/RealtimeServer.h>
#include "Character.h"





class ExampleSrvForUe4Demo
{
public:
	ExampleSrvForUe4Demo()
	{
		const uint8_t becomeDaemonOnLinuxCmdIndex = 1;
		bool whetherTobecomeDaemonOnLinux = RealtimeSrvHelper::GetCommandLineArg(
			becomeDaemonOnLinuxCmdIndex ) == "1" ? true : false;

		NewPlayerCallback newPlayerCb = std::bind(
			&ExampleSrvForUe4Demo::SpawnNewCharacterForPlayer,
			this, _1 );

		RealtimeServer::Init( newPlayerCb, whetherTobecomeDaemonOnLinux );
	}

	void Run()
	{
		RealtimeServer::Run();
	}

	GameObjPtr SpawnNewCharacterForPlayer( int inPlayerId )
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
};





int main( int argc, const char** argv )
{
	RealtimeSrvHelper::SaveCommandLineArg( argc, argv );

	ExampleSrvForUe4Demo exmaple_server;
	exmaple_server.Run();
}