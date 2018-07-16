#include <realtime_srv/RealtimeServer.h>
#include "Character.h"


using namespace realtime_srv;


class ExampleSrvForUe4Demo : noncopyable
{
public:
	ExampleSrvForUe4Demo( bool willDaemonizeOnLinux = true )
		:
		server_( std::bind( &ExampleSrvForUe4Demo::SpawnNewCharacterForPlayer, this, _1 ),
			willDaemonizeOnLinux )
	{}

	void Run() { server_.Run(); }

	GameObjPtr SpawnNewCharacterForPlayer( ClientProxyPtr cliProxy )
	{
		CharacterPtr newCharacter = std::make_shared< Character >();

		newCharacter->SetPlayerId( cliProxy->GetNetId() );
		newCharacter->SetLocation(
			2500.f + RealtimeSrvMath::GetRandomFloat() * -5000.f,
			2500.f + RealtimeSrvMath::GetRandomFloat() * -5000.f,
			0.f );
		newCharacter->SetRotation(
			0.f,
			RealtimeSrvMath::GetRandomFloat() * 180.f,
			0.f );

		return  std::static_pointer_cast< GameObj >( newCharacter );
	}

private:
	RealtimeServer server_;
};




int main( int argc, const char** argv )
{
	ExampleSrvForUe4Demo exmaple_server;
	exmaple_server.Run();
}

