
// 是否要显示调试打印信息
#define REAL_TIME_SRV_SHOW_DEBUG_MESSAGE					true

namespace RealTimeSrvHelper
{
	string GetCommandLineArg( int inIndex );

	string Sprintf( const char* inFormat, ... );

	void	Log( const char* inFormat );
	void	Log( const char* inFormat, ... );

	bool	SequenceGreaterThanOrEqual( PacketSN s1, PacketSN s2 );
	bool	SequenceGreaterThan( PacketSN s1, PacketSN s2 );

	bool ChunkPacketIDGreaterThanOrEqual( ChunkPacketID s1, ChunkPacketID s2 );
	bool ChunkPacketIDGreaterThan( ChunkPacketID s1, ChunkPacketID s2 );
}

#define LOG( ... ) RealTimeSrvHelper::Log( __VA_ARGS__ );