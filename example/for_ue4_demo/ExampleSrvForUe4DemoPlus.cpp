#include <realtime_srv/RealtimeServer.h>

#ifdef IS_LINUX

#include "Character.h"
#include "ExampleRedisCli.h"
#include "CharacterLuaBind.h"
#include "ExampleInputState.h"


using namespace realtime_srv;


class ExampleSrvForUe4DemoPlus : noncopyable
{
public:
	ExampleSrvForUe4DemoPlus( bool _willDaemonizeOnLinux = false )
		:
		server_(
			std::bind( &ExampleSrvForUe4DemoPlus::SpawnNewCharacter, this, _1 ),
			std::bind( &ExampleSrvForUe4DemoPlus::MyInputState, this ),
			_willDaemonizeOnLinux )
	{
		db_.Init( server_.GetEventLoop() );
		server_.SetUnregistGameObjWhenClientDisconnect( true );
	}

	InputState* MyInputState() { return new ExampleInputState(); }

	GameObjPtr SpawnNewCharacter( ClientProxyPtr& cliProxy )
	{
		// test hiredis
		db_.SaveNewPlayer( cliProxy->GetNetId(),
			cliProxy->GetPlayerName() );

		// test lua
		CharacterLuaBind clb;
		CharacterPtr newCharacter = clb.DoFile();
		newCharacter->SetPlayerId( cliProxy->GetNetId() );
		return  std::static_pointer_cast< GameObj >( newCharacter );
	}

	void Run() { server_.SimulateRealWorldOnWin(); server_.Run(); }

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