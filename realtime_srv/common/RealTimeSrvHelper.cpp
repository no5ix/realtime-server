#include "realtime_srv/common/RealtimeSrvShared.h"


#ifndef IS_WIN
const char** __argv;
int __argc;
void OutputDebugString( const char* inString )
{
	printf( "%s", inString );
}
#endif

void RealtimeSrvHelper::SaveCommandLineArg( const int argc, const char** argv )
{
#ifndef IS_WIN
	__argc = argc;
	__argv = argv;
#endif
}

std::string RealtimeSrvHelper::GetCommandLineArg( int inIndex )
{
	if ( inIndex < __argc )
	{
		return std::string( __argv[inIndex] );
	}

	return std::string();
}


std::string RealtimeSrvHelper::Sprintf( const char* inFormat, ... )
{
	//not thread safe...
	static char temp[4096];

	va_list args;
	va_start( args, inFormat );

#ifdef IS_WIN
	_vsnprintf_s( temp, 4096, 4096, inFormat, args );
#else
	vsnprintf( temp, 4096, inFormat, args );
#endif
	return std::string( temp );
}

// void RealTimeSrvHelper::Log( const char* inFormat )
// {
// 	OutputDebugString( inFormat );
// 	OutputDebugString( "\n" );
// }

void RealtimeSrvHelper::Log( const char* inFormat, ... )
{
	if ( !REAL_TIME_SRV_SHOW_DEBUG_MESSAGE )
		return;

	//not thread safe...
	static char temp[4096];

	va_list args;
	va_start( args, inFormat );

#ifdef IS_WIN
	_vsnprintf_s( temp, 4096, 4096, inFormat, args );
#else
	vsnprintf( temp, 4096, inFormat, args );
#endif
	OutputDebugString( temp );
	OutputDebugString( "\n" );
}

bool RealtimeSrvHelper::SequenceGreaterThanOrEqual( PacketSN s1, PacketSN s2 )
{
	return ( ( s1 >= s2 ) && ( s1 - s2 <= HALF_MAX_PACKET_SEQUENCE_NUMBER ) ) ||
		( ( s1 < s2 ) && ( s2 - s1 > HALF_MAX_PACKET_SEQUENCE_NUMBER ) );
	//return s1 >= s2;
}


bool RealtimeSrvHelper::SequenceGreaterThan( PacketSN s1, PacketSN s2 )
{
	return ( ( s1 > s2 ) && ( s1 - s2 <= HALF_MAX_PACKET_SEQUENCE_NUMBER ) ) ||
		( ( s1 < s2 ) && ( s2 - s1 > HALF_MAX_PACKET_SEQUENCE_NUMBER ) );
	//return s1 > s2;
}

bool RealtimeSrvHelper::ChunkPacketIDGreaterThanOrEqual( ChunkPacketID s1, ChunkPacketID s2 )
{
	return ( ( s1 >= s2 ) && ( s1 - s2 <= HALF_MAX_CHUNK_PACKET_ID ) ) ||
		( ( s1 < s2 ) && ( s2 - s1 > HALF_MAX_CHUNK_PACKET_ID ) );
}


bool RealtimeSrvHelper::ChunkPacketIDGreaterThan( ChunkPacketID s1, ChunkPacketID s2 )
{
	return ( ( s1 > s2 ) && ( s1 - s2 <= HALF_MAX_CHUNK_PACKET_ID ) ) ||
		( ( s1 < s2 ) && ( s2 - s1 > HALF_MAX_CHUNK_PACKET_ID ) );
}

bool RealtimeSrvHelper::BecomeDaemonOnLinux()
{
#ifndef IS_LINUX

	return true;

#else
	int maxfd, fd;
	switch ( fork() )
	{                   /* Become background process */
	case -1:
		return false;
	case 0:
		break;                     /* Child falls through... */
	default:
		_exit( EXIT_SUCCESS );       /* while parent terminates */
	}
	if ( setsid() == -1 )                 /* Become leader of new session */
		return false;
	switch ( fork() )
	{                   /* Ensure we are not session leader */
	case -1:
		return false;
	case 0:
		break;
	default:
		_exit( EXIT_SUCCESS );
	}
	umask( 0 );                       /* Clear file mode creation mask */
	chdir( "/" );                     /* Change to root directory */
	maxfd = sysconf( _SC_OPEN_MAX );
	if ( maxfd == -1 )                /* Limit is indeterminate... */
		maxfd = 8192;				  /* so take a guess */
	for ( fd = 0; fd < maxfd; fd++ )
		close( fd );
	close( STDIN_FILENO );            /* Reopen standard fd's to /dev/null */
	fd = open( "/dev/null", O_RDWR ); // open 返回的文件描述符一定是最小的未被使用的描述符。
	if ( fd != STDIN_FILENO )         /* 'fd' should be 0 */
		return false;
	if ( dup2( STDIN_FILENO, STDOUT_FILENO ) != STDOUT_FILENO )
		return false;
	if ( dup2( STDIN_FILENO, STDERR_FILENO ) != STDERR_FILENO )
		return false;

	return true;
#endif //IS_LINUX
}



void RealtimeSrvHelper::SimulateRealWorldOnWin(
	NetworkMgr* networkManager,
	uint8_t LatencyCmdIndex,
	uint8_t dropPacketChanceCmdIndex /*= 0*/,
	uint8_t JitterCmdIndex /*= 0*/ )
{
#ifdef IS_WIN
	assert( networkManager );

	std::string latencyString = RealtimeSrvHelper::GetCommandLineArg(
		LatencyCmdIndex );
	if ( !latencyString.empty() )
	{
		float latency = stof( latencyString );
		networkManager->SetSimulatedLatency( latency );
	}

	std::string dropPacketChanceString = RealtimeSrvHelper::GetCommandLineArg(
		dropPacketChanceCmdIndex );
	if ( !dropPacketChanceString.empty() )
	{
		float dropPacketChance = stof( dropPacketChanceString );
		networkManager->SetDropPacketChance( dropPacketChance );
	}

	std::string IsSimulatedJitterString = RealtimeSrvHelper::GetCommandLineArg(
		JitterCmdIndex );
	if ( !IsSimulatedJitterString.empty() )
	{
		int IsSimulatedJitter = stoi( IsSimulatedJitterString );
		if ( IsSimulatedJitter )
		{
			networkManager->SetIsSimulatedJitter( true );
		}
	}
#endif // IS_WIN
}
