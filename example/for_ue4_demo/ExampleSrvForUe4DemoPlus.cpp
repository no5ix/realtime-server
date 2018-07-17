#include <realtime_srv/RealtimeServer.h>

#ifdef IS_LINUX

#include "Character.h"
#include "ExampleRedisCli.h"
#include "CharacterLuaBind.h"
#include "ExampleInputState.h"


using namespace realtime_srv;


class ExampleSrvForUe4Demo : noncopyable
{
public:
	ExampleSrvForUe4Demo( bool willDaemonizeOnLinux = false )
		:
		server_(
			std::bind( &ExampleSrvForUe4Demo::SpawnNewCharacterForPlayer, this, _1 ),
			std::bind( &ExampleSrvForUe4Demo::MyInputState, this ),
			willDaemonizeOnLinux )
	{ db_.Init( server_.GetEventLoop() ); }

	InputState* MyInputState() { return new ExampleInputState(); }

	void Run() { server_.SimulateRealWorldOnWin( 2, 3, 4 ); server_.Run(); }

	GameObjPtr SpawnNewCharacterForPlayer( ClientProxyPtr& cliProxy )
	{
		db_.SaveNewPlayer( cliProxy->GetNetId(),
			cliProxy->GetPlayerName() );

		CharacterLuaBind clb;
		CharacterPtr newCharacter = clb.DoFile();
		newCharacter->SetPlayerId( cliProxy->GetNetId() );
		return  std::static_pointer_cast< GameObj >( newCharacter );
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

	ExampleSrvForUe4Demo exmaple_server( willDaemonizeOnLinux );
	exmaple_server.Run();

}

#endif // IS_LINUX