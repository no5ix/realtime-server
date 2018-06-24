#include <realtime_srv/RealtimeServer.h>
#include "Character.h"


using namespace realtime_srv;


class ExampleSrvForUe4Demo : realtime_srv::noncopyable
{
public:
	ExampleSrvForUe4Demo( bool WhetherTobecomeDaemonOnLinux = false )
		: server_( WhetherTobecomeDaemonOnLinux )
	{}

	void Run()
	{
		//	在Windows上, 如果输入命令:   ./example_for_ue4_demo.exe 0 0.3 0.8 1
		//	则服务器将会模拟 : 
		//	-	模拟延迟为 : 当前延迟+300毫秒的延迟, 若当前延迟为30ms,
		//			则模拟之后的延迟约为30+300=330左右
		//	-	模拟丢包率为 : 百分之八十的丢包率
		//	-	模拟网络抖动为 : 1为随机抖动, 0为不抖动
		const uint8_t SimulateLatencyCmdIndexOnWindows = 2;
		const uint8_t SimulateDropPacketChanceCmdIndexOnWindows = 3;
		const uint8_t SimulateJitterCmdIndexOnWindows = 4;

		NewPlayerCallback newPlayerCb = std::bind(
			&ExampleSrvForUe4Demo::SpawnNewCharacterForPlayer,
			this, _1 );

		server_.Run( newPlayerCb );
		server_.SimulateRealWorldOnWindows(
			SimulateLatencyCmdIndexOnWindows,
			SimulateDropPacketChanceCmdIndexOnWindows,
			SimulateJitterCmdIndexOnWindows );
	}

	GameObjPtr SpawnNewCharacterForPlayer( ClientProxyPtr newClientProxy )
	{
		GameObjPtr newGameObj = Character::StaticCreate();
		CharacterPtr character = std::static_pointer_cast< Character >( newGameObj );

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

private:
	RealtimeServer server_;
};





int main( int argc, const char** argv )
{
	RealtimeSrvHelper::SaveCommandLineArg( argc, argv );

	// 在Linux上, 加上一个命令行参数 1 即可变为守护进程,   
	// 如:	   ./rs_example_for_ue4_demo  1
	const uint8_t becomeDaemonCmdIndexOnLinux = 1;
	bool whetherTobecomeDaemonOnLinux = RealtimeSrvHelper::GetCommandLineArg(
		becomeDaemonCmdIndexOnLinux ) == "1";

	ExampleSrvForUe4Demo exmaple_server( whetherTobecomeDaemonOnLinux );
	exmaple_server.Run();
}