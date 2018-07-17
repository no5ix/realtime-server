#include <realtime_srv/RealtimeServer.h>
#include "Character.h"
#include "ExampleInputState.h"


using namespace realtime_srv;


class ExampleSrvForUe4Demo : noncopyable
{
public:
	ExampleSrvForUe4Demo( bool willDaemonizeOnLinux = true )
		:
		server_(
			std::bind( &ExampleSrvForUe4Demo::SpawnNewCharacter, this, _1 ),
			std::bind( &ExampleSrvForUe4Demo::MyInputState, this ),
			willDaemonizeOnLinux )
	{}

	InputState* MyInputState() { return new ExampleInputState(); }

	void Run() { server_.Run(); }

	GameObjPtr SpawnNewCharacter( ClientProxyPtr& cliProxy )
	{
		auto newCharacter = new Character;
		newCharacter->SetPlayerId( cliProxy->GetNetId() );
		return  GameObjPtr( newCharacter );
	}

private:
	RealtimeServer server_;
};




int main( int argc, const char** argv )
{
	ExampleSrvForUe4Demo exmaple_server;
	exmaple_server.Run();
}

