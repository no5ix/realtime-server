#pragma once

#include <string>

// 是否要显示调试打印信息
#define REAL_TIME_SRV_SHOW_DEBUG_MESSAGE	true

namespace realtime_srv
{
class NetworkMgr;



struct ServerConfig
{
	// [RealtimeServer]
	bool	daemonize;

	// [NetworkMgr]
	bool is_unregist_obj_when_cli_disconn;
	uint8_t action_count_per_tick;

	// [PktHandler]
	uint16_t port;
	double send_packet_interval;
	double client_disconnect_timeout;
	uint8_t packet_dispatcher_thread_count;
	double tick_interval;
	double max_packets_count_per_fetch;
};

namespace RealtimeSrvHelper
{

void SaveCommandLineArg( const int argc, const char** argv );
std::string GetCommandLineArg( int inIndex );

std::string Sprintf( const char* inFormat, ... );

void	Log( const char* inFormat, ... );

bool	SequenceGreaterThanOrEqual( PacketSN s1, PacketSN s2 );
bool	SequenceGreaterThan( PacketSN s1, PacketSN s2 );

bool ChunkPacketIDGreaterThanOrEqual( ChunkPacketID s1, ChunkPacketID s2 );
bool ChunkPacketIDGreaterThan( ChunkPacketID s1, ChunkPacketID s2 );

bool Daemonize();

void SimulateRealWorldNetCondition(
	std::shared_ptr<NetworkMgr>& networkManager,
	uint8_t LatencyCmdIndex = 1,
	uint8_t dropPacketChanceCmdIndex = 2,
	uint8_t JitterCmdIndex = 3 );
}
#define LOG( ... ) RealtimeSrvHelper::Log( __VA_ARGS__ );

}