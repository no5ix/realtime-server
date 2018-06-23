#pragma once




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


#ifdef IS_LINUX

// thread shared var util begin

// thread shared var declaration & definition
#define THREAD_SHARED_VAR_DEF( AccessSpecifier, OriginalVarType, VarName, Mutex )   \
AccessSpecifier:																	\
	std::shared_ptr<OriginalVarType> VarName;										\
public:																				\
	std::shared_ptr<OriginalVarType> Get##VarName()									\
	{																				\
		MutexLockGuard lock( Mutex );												\
		return VarName;																\
	}																				\
	void VarName##COW()																\
	{																				\
		if ( !VarName.unique() )													\
		{																			\
			VarName.reset( new OriginalVarType( *VarName ) );						\
		}																			\
		assert( VarName.unique() );													\
	}																				\
AccessSpecifier:																	\


// thread shared var read
#define GET_THREAD_SHARED_VAR( VarName ) Get##VarName()

// thread shared var copy on write
#define THREAD_SHARED_VAR_COW( VarName ) VarName##COW()

// thread shared var write
#define SET_THREAD_SHARED_VAR( VarName, Mutex, WriteOperationLambda ) \
	{												\
		MutexLockGuard lock( Mutex );				\
		VarName##COW();								\
		WriteOperationLambda();								\
	}												\

// thread shared var util end


#endif //IS_LINUX
