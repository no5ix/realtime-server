#pragma once

//#ifdef __linux__

#include <memory>

#include <concurrent_queue/concurrentqueue.h>
#include <concurrent_queue/blockingconcurrentqueue.h>

#include <muduo_udp_support/UdpConnection.h>

#include "realtime_srv/rep/BitStream.h"


namespace realtime_srv
{

class ReceivedPacket
{
public:
	ReceivedPacket() {}

	ReceivedPacket(
		const muduo::Timestamp& inReceivedTime,
		const pid_t inHoldByThreadId,
		const std::shared_ptr<InputBitStream>& inInputMemoryBitStreamPtr,
		const muduo::net::UdpConnectionPtr& inUdpConnetction )
		:
		recvedTime_( inReceivedTime ),
		holdedByThreadId_( inHoldByThreadId ),
		recvedPacketBuf_( inInputMemoryBitStreamPtr ),
		udpConn_( inUdpConnetction )
	{}

	ReceivedPacket( ReceivedPacket &&Other ) noexcept
		: recvedTime_( Other.recvedTime_ ),
		holdedByThreadId_( Other.holdedByThreadId_ ),
		recvedPacketBuf_( Other.recvedPacketBuf_ ),
		udpConn_( Other.udpConn_ )
	{
		Other.recvedPacketBuf_.reset();
		Other.udpConn_.reset();
	}

	ReceivedPacket& operator=( ReceivedPacket &&Other ) noexcept
	{
		if ( this != &Other )
		{
			recvedPacketBuf_.reset();
			udpConn_.reset();

			recvedTime_ = Other.recvedTime_;
			recvedPacketBuf_ = Other.recvedPacketBuf_;
			udpConn_ = Other.udpConn_;
			holdedByThreadId_ = Other.holdedByThreadId_;

			Other.recvedPacketBuf_.reset();
			Other.udpConn_.reset();
		}
		return *this;
	}

	muduo::net::UdpConnectionPtr&	GetUdpConn() { return udpConn_; }
	const muduo::Timestamp& GetReceivedTime() const { return recvedTime_; }
	muduo::Timestamp& GetMutableReceivedTime() { return recvedTime_; }
	std::shared_ptr<InputBitStream>& GetPacketBuffer() { return recvedPacketBuf_; }
	pid_t GetHoldedByThreadId() const { return holdedByThreadId_; }

private:
	muduo::Timestamp										recvedTime_;
	std::shared_ptr<InputBitStream>			recvedPacketBuf_;
	muduo::net::UdpConnectionPtr				udpConn_;
	pid_t																holdedByThreadId_;
};
typedef std::shared_ptr<ReceivedPacket> ReceivedPacketPtr;
typedef moodycamel::ConcurrentQueue<ReceivedPacketPtr> ReceivedPacketQueue;
typedef moodycamel::BlockingConcurrentQueue<ReceivedPacketPtr> ReceivedPacketBlockQueue;



class PendingSendPacket
{
public:
	PendingSendPacket() {}
	PendingSendPacket( 
		const std::shared_ptr<OutputBitStream>& OutPutPacketBuffer,
		const muduo::net::UdpConnectionPtr& UdpConnection )
		:
		sndPacketBuf_( OutPutPacketBuffer ),
		udpConn_( UdpConnection )
	{}
	muduo::net::UdpConnectionPtr& GetUdpConnection() { return udpConn_; }
	std::shared_ptr<OutputBitStream>& GetPacketBuffer() { return sndPacketBuf_; }
private:
	std::shared_ptr<OutputBitStream>					sndPacketBuf_;
	muduo::net::UdpConnectionPtr							udpConn_;
};
typedef std::shared_ptr<PendingSendPacket> PendingSendPacketPtr;
typedef moodycamel::ConcurrentQueue< PendingSendPacketPtr > PendingSendPacketQueue;
typedef moodycamel::BlockingConcurrentQueue< PendingSendPacketPtr > PendingSendPacketBlockQueue;


} // namespace realtime_srv

//#endif // __linux__