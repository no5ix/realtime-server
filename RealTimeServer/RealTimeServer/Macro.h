
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


#ifdef _WIN32

typedef signed char							int8_t;
typedef unsigned char						uint8_t;
typedef signed short						int16_t;
typedef unsigned short						uint16_t;
typedef signed int							int32_t;
typedef unsigned int						uint32_t;
typedef signed long long					int64_t;
typedef unsigned long long					uint64_t;

typedef unsigned int						size_t;

#endif


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


#define MAX_PACKET_BYTE_LENGTH				(888)

#define ACK_BIT_FIELD_BYTE_LEN				(4)