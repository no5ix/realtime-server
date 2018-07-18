#include <realtime_srv/RealtimeServer.h>

#ifdef IS_LINUX

#include "Character.h"
#include "ExampleRedisCli.h"
#include "CharacterLuaBind.h"
#include "ExampleInputState.h"
#include "Robot.h"

using namespace realtime_srv;


class ExampleSrvForUe4DemoPlus : noncopyable
{
public:
	ExampleSrvForUe4DemoPlus( bool willDaemonizeOnLinux = false )
		: server_( willDaemonizeOnLinux )
	{
		db_.Init( server_.GetNetworkManager()->GetEventLoop() );

		// for spawning your own controlled GameObject.
		server_.GetNetworkManager()->SetNewPlayerCallback(
			std::bind( &ExampleSrvForUe4DemoPlus::OnNewPlayer, this, _1 ) );

		// test -> ExampleInputState
		// for using your own InputState class.
		server_.GetNetworkManager()->SetCustomInputStateCallback( std::bind(
			&ExampleSrvForUe4DemoPlus::MyInputState, this ) );

		server_.GetNetworkManager()->SetUnregistObjWhenCliDisconn( true );
	}

	InputState* MyInputState() { return new ExampleInputState; }

	void Run() { server_.SimulateRealWorldOnWin();  AddRobot(); server_.Run(); }


	GameObj* OnNewPlayer( ClientProxyPtr& _newClientProxy )
	{
		// test -> hiredis
		db_.SaveNewPlayer( _newClientProxy->GetNetId(),
			"realtime_srv_test_player" );

		// test -> lua
		CharacterLuaBind clb;
		Character* newCharacter = clb.DoFile();
		newCharacter->SetPlayerId( _newClientProxy->GetNetId() );
		return newCharacter;
	}

	void AddRobot()
	{
		server_.GetWorld()->RegistGameObj( GameObjPtr( new Robot ) );
	}


private:

	RealtimeServer server_;
	ExampleRedisCli db_;
};



int main( int argc, const char** argv )
{
	RealtimeSrvHelper::SaveCommandLineArg( argc, argv );

	// 在Linux上, 加上一个命令行参数 1 即可变为守护进程,   
	// 如:	   ./rs_example_for_ue4_demo  1
	// 不加即为前台运行
	const uint8_t DaemonizeCmdIndexOnLinux = 1;
	bool willDaemonizeOnLinux = RealtimeSrvHelper::GetCommandLineArg(
		DaemonizeCmdIndexOnLinux ) == "1";

	ExampleSrvForUe4DemoPlus exmaple_server( willDaemonizeOnLinux );
	exmaple_server.Run();

}

#endif // IS_LINUX