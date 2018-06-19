#pragma once


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

#define BECOME_DAEMON						(true)

#define MOVE_COUNT_NUM						(2)



#ifdef __linux__
//#define DEPRECATED_EPOLL_INTERFACE
#define NEW_EPOLL_INTERFACE
#endif