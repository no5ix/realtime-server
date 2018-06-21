#pragma once


#ifdef __linux__
#define IS_LINUX
#endif

#ifdef IS_LINUX

#define COMMAND_LINE_ARG_DAEMON_INDEX					(1)
#define COMMAND_LINE_ARG_PORT_INDEX						(2)
#define COMMAND_LINE_ARG_LATENCY_INDEX					(3)
#define COMMAND_LINE_ARG_DROP_PACKET_CHANCE_INDEX		(4)	    
#define COMMAND_LINE_ARG_IS_SIMULATED_JITTER_INDEX		(5)	    

#else

#define COMMAND_LINE_ARG_PORT_INDEX						(1)
#define COMMAND_LINE_ARG_LATENCY_INDEX					(2)
#define COMMAND_LINE_ARG_DROP_PACKET_CHANCE_INDEX		(3)	    
#define COMMAND_LINE_ARG_IS_SIMULATED_JITTER_INDEX		(4)	    

#endif //IS_LINUX

#define DEFAULT_REALTIME_SRV_PORT			(44444)


#ifdef PI
# undef PI
# define PI (3.1415926535897932f)
#else
# define PI (3.1415926535897932f)
#endif


#define SMALL_NUMBER						(1.e-8f)
#define KINDA_SMALL_NUMBER					(1.e-4f)
#define BIG_NUMBER							(3.4e+38f)
#define EULERS_NUMBER						(2.71828182845904523536f)

// PacketSequenceNumber
typedef unsigned short						PacketSN;
#define PACKET_SEQUENCE_NUMBER_BIT_WIDE		(16)
#define MAX_PACKET_SEQUENCE_NUMBER			(65535)
#define HALF_MAX_PACKET_SEQUENCE_NUMBER     (32768)

// ChunkPacketID
typedef unsigned int						ChunkPacketID;
#define CHUNK_PACKET_ID_BIT_WIDE			(32)
#define MAX_CHUNK_PACKET_ID					(4294967296)
#define HALF_MAX_CHUNK_PACKET_ID			(2147483648)


#define MAX_PACKET_BYTE_LENGTH				(512)

#define ACK_BIT_FIELD_BYTE_LEN				(4)

#define THREAD_NUM							(8)

#define MOVE_COUNT_NUM						(2)

