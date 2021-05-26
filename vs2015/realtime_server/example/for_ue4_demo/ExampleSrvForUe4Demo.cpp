#include <realtime_srv/RealtimeServer.h>
#include "Character.h"


using namespace realtime_srv;


class ExampleSrvForUe4Demo : noncopyable
{
public:
	ExampleSrvForUe4Demo()
	{
		server_.GetNetworkManager()->SetNewPlayerCallback(
			std::bind(&ExampleSrvForUe4Demo::OnNewPlayer, this, _1));
	}

	void Run() { server_.Run(); }


	//	for spawning your own controlled GameObject to the World,
	//	just return a GameObj* , 
	//	realtime_srv will sync it to all the other clients.
	//	of course u can do anything else for return nullptr or
	//	u can regist ur GameObj to the World by urself. (see ExampleSrvForUe4DemoPlus.cpp)
	GameObj* OnNewPlayer(ClientProxyPtr& newClientProxy)
	{
		auto newCharacter(new Character);
		newCharacter->SetPlayerId(newClientProxy->GetNetId());
		return newCharacter;
	}

private:
	RealtimeServer server_;
};




int main(int argc, const char** argv)
{
	ExampleSrvForUe4Demo exmaple_server;
	exmaple_server.Run();
}

