#include "RealTimeSrvPCH.h"


#if !_WIN32
extern const char** __argv;
extern int __argc;
void OutputDebugString( const char* inString )
{
	printf( "%s", inString );
}
#endif

string RealTimeSrvHelper::GetCommandLineArg( int inIndex )
{
	if (inIndex < __argc)
	{
		return string( __argv[inIndex] );
	}

	return string();
}


string RealTimeSrvHelper::Sprintf( const char* inFormat, ... )
{
	//not thread safe...
	static char temp[4096];

	va_list args;
	va_start( args, inFormat );

#if _WIN32
	_vsnprintf_s( temp, 4096, 4096, inFormat, args );
#else
	vsnprintf( temp, 4096, inFormat, args );
#endif
	return string( temp );
}

// void RealTimeSrvHelper::Log( const char* inFormat )
// {
// 	OutputDebugString( inFormat );
// 	OutputDebugString( "\n" );
// }

void RealTimeSrvHelper::Log( const char* inFormat, ... )
{
	if ( !REAL_TIME_SRV_SHOW_DEBUG_MESSAGE )
		return;

	//not thread safe...
	static char temp[4096];

	va_list args;
	va_start( args, inFormat );

#if _WIN32
	_vsnprintf_s( temp, 4096, 4096, inFormat, args );
#else
	vsnprintf( temp, 4096, inFormat, args );
#endif
	OutputDebugString( temp );
	OutputDebugString( "\n" );
}




bool RealTimeSrvHelper::SequenceGreaterThanOrEqual( PacketSN s1, PacketSN s2 )
{
	return ( ( s1 >= s2 ) && ( s1 - s2 <= HALF_MAX_PACKET_SEQUENCE_NUMBER ) ) ||
		( ( s1 < s2 ) && ( s2 - s1 > HALF_MAX_PACKET_SEQUENCE_NUMBER ) );
}


bool RealTimeSrvHelper::SequenceGreaterThan( PacketSN s1, PacketSN s2 )
{
	return ( ( s1 > s2 ) && ( s1 - s2 <= HALF_MAX_PACKET_SEQUENCE_NUMBER ) ) ||
		( ( s1 < s2 ) && ( s2 - s1 > HALF_MAX_PACKET_SEQUENCE_NUMBER ) );
}

bool RealTimeSrvHelper::ChunkPacketIDGreaterThanOrEqual( ChunkPacketID s1, ChunkPacketID s2 )
{
	return ( ( s1 >= s2 ) && ( s1 - s2 <= HALF_MAX_CHUNK_PACKET_ID ) ) ||
		( ( s1 < s2 ) && ( s2 - s1 > HALF_MAX_CHUNK_PACKET_ID ) );
}


bool RealTimeSrvHelper::ChunkPacketIDGreaterThan( ChunkPacketID s1, ChunkPacketID s2 )
{
	return ( ( s1 > s2 ) && ( s1 - s2 <= HALF_MAX_CHUNK_PACKET_ID ) ) ||
		( ( s1 < s2 ) && ( s2 - s1 > HALF_MAX_CHUNK_PACKET_ID ) );
}