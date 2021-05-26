#include <realtime_srv/RealtimeServer.h>

// #ifdef IS_LINUX


#include "Character.h"
#include "ExampleRedisCli.h"
#include "CharacterLuaBind.h"
#include "ExampleInputState.h"
#include "Robot.h"

using namespace realtime_srv;


class ExampleSrvForUe4DemoPlus : noncopyable
{
public:
	ExampleSrvForUe4DemoPlus()
	{
		// for spawning your own controlled GameObject.
		server_.GetNetworkManager()->SetNewPlayerCallback(
			std::bind(&ExampleSrvForUe4DemoPlus::OnNewPlayer, this, _1));

		// test -> ExampleInputState
		// for using your own InputState class.
		server_.GetNetworkManager()->SetCustomInputStateCallback(std::bind(
			&ExampleSrvForUe4DemoPlus::MyInputState, this));

		// init hiredis
		db_.Init(server_.GetNetworkManager()->GetEventLoop());
	}

	InputState* MyInputState() { return new ExampleInputState; }

	void Run() { AddRobot(); server_.Run(); }

	void AddRobot() { server_.GetWorld()->RegistGameObj(GameObjPtr(new Robot)); }


	//	for spawning your own controlled GameObject to the World,
	//	just return a GameObj* , 
	//	realtime_srv will sync it to all the other clients.
	//	of course u can do anything else for return nullptr or
	//	u can regist ur GameObj to the World by urself.
	GameObj* OnNewPlayer(ClientProxyPtr& newClientProxy)
	{
		// test -> hiredis
		db_.SaveNewPlayer(newClientProxy->GetNetId(),
			"realtime_srv_test_player");


		// test -> lua
		CharacterLuaBind clb;
		Character* newCharacter = clb.DoFile();
		newCharacter->SetPlayerId(newClientProxy->GetNetId());


		//  after 3 sec, your character die.
		server_.GetNetworkManager()->GetEventLoop()->runAfter(3.0, [=]() {
			newCharacter->SetPendingToDie();
		});


		//  after 6 sec, create a new character to play.
		server_.GetNetworkManager()->GetEventLoop()->runAfter(5.0, [=]() {
			CharacterPtr anotherCharacter(new Character);
			// one NetId, One Client.
			anotherCharacter->SetPlayerId(newClientProxy->GetNetId());
			// let the client controll the new character.
			anotherCharacter->SetMaster(newClientProxy);
			// regist this character ( derived from GameObj class ) to World.
			server_.GetWorld()->RegistGameObj(GameObjPtr(anotherCharacter));
		});

		return newCharacter;
	}


private:

	RealtimeServer server_;
	ExampleRedisCli db_;
};



int main(int argc, const char** argv)
{
	ExampleSrvForUe4DemoPlus exmaple_server;
	exmaple_server.Run();
}


// #endif // IS_LINUX
