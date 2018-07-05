

// 是否要显示调试打印信息
#define REAL_TIME_SRV_SHOW_DEBUG_MESSAGE	true

namespace realtime_srv
{
	class NetworkMgr;

	namespace RealtimeSrvHelper
	{

		void SaveCommandLineArg( const int argc, const char** argv );
		string GetCommandLineArg( int inIndex );

		string Sprintf( const char* inFormat, ... );

		void	Log( const char* inFormat, ... );

		bool	SequenceGreaterThanOrEqual( PacketSN s1, PacketSN s2 );
		bool	SequenceGreaterThan( PacketSN s1, PacketSN s2 );

		bool ChunkPacketIDGreaterThanOrEqual( ChunkPacketID s1, ChunkPacketID s2 );
		bool ChunkPacketIDGreaterThan( ChunkPacketID s1, ChunkPacketID s2 );

		bool DaemonizeOnLinux();

		void SimulateRealWorldNetCondition( NetworkMgr* networkManager,
			uint8_t LatencyCmdIndex,
			uint8_t dropPacketChanceCmdIndex = 0,
			uint8_t JitterCmdIndex = 0 );
	}

#define LOG( ... ) RealtimeSrvHelper::Log( __VA_ARGS__ );

}