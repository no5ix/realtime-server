#pragma once

#include <string>
#include "realtime_srv/common/RealtimeSrvMacro.h"


namespace realtime_srv
{

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
	uint8_t fps;
	size_t max_packets_count_per_fetch;
};

namespace RealtimeSrvHelper
{

void SaveCommandLineArg(const int argc, const char** argv);
std::string GetCommandLineArg(int index);

std::string Sprintf(const char* format, ...);

void	Log(const char* format, ...);

bool	SNGreaterThanOrEqual(PacketSN s1, PacketSN s2);
bool	SNGreaterThan(PacketSN s1, PacketSN s2);

bool ChunkPacketIDGreaterThanOrEqual(ChunkPacketID s1, ChunkPacketID s2);
bool ChunkPacketIDGreaterThan(ChunkPacketID s1, ChunkPacketID s2);

bool Daemonize();

}
#define LOG( ... ) RealtimeSrvHelper::Log( __VA_ARGS__ );

}