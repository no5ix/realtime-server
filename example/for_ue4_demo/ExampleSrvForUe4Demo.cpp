#include <realtime_srv/RealtimeServer.h>
#include "Character.h"


using namespace realtime_srv;


class ExampleSrvForUe4Demo : noncopyable
{
public:
	ExampleSrvForUe4Demo( bool _willDaemonizeOnLinux = false )
		: server_( _willDaemonizeOnLinux )
	{
		// for spawning your own GameObject class.
		server_.GetNetworkManager()->SetNewPlayerCallback(
			std::bind( &ExampleSrvForUe4Demo::OnNewPlayer, this, _1 ) );
	}

	void Run() { server_.Run(); }

	GameObj* OnNewPlayer( ClientProxyPtr& _newClientProxy )
	{
		auto newCharacter( new Character );
		newCharacter->SetPlayerId( _newClientProxy->GetNetId() );
		return newCharacter;
	}

private:
	RealtimeServer server_;
};




int main( int argc, const char** argv )
{
	ExampleSrvForUe4Demo exmaple_server;
	exmaple_server.Run();
}

