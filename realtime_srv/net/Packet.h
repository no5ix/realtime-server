#pragma once

#include <memory>

#include <concurrent_queue/concurrentqueue.h>
#include <concurrent_queue/blockingconcurrentqueue.h>

#include <muduo_udp_support/UdpConnection.h>

#include "realtime_srv/rep/BitStream.h"


namespace realtime_srv
{

////// recv
class ReceivedPacket
{
public:
	ReceivedPacket() {}
	ReceivedPacket(
		const float inReceivedTime,
		const std::shared_ptr<InputBitStream>& inInputMemoryBitStreamPtr,
		const muduo::net::UdpConnectionPtr& inUdpConnetction )
		:
		mReceivedTime( inReceivedTime ),
		mPacketBuffer( inInputMemoryBitStreamPtr ),
		mUdpConn( inUdpConnetction )
	{}
	const	muduo::net::UdpConnectionPtr&	GetUdpConn() const { return mUdpConn; }
	float GetReceivedTime()	const { return mReceivedTime; }
	const std::shared_ptr<InputBitStream>& GetPacketBuffer() const { return mPacketBuffer; }

	bool operator<( const ReceivedPacket& other ) const
	{ return this->mReceivedTime < other.GetReceivedTime(); }

private:
	float																mReceivedTime;
	std::shared_ptr<InputBitStream>			mPacketBuffer;
	muduo::net::UdpConnectionPtr				mUdpConn;
};
typedef moodycamel::ConcurrentQueue<ReceivedPacket> ReceivedPacketQueue;
typedef moodycamel::BlockingConcurrentQueue<ReceivedPacket> ReceivedPacketBlockQueue;


////// snd
class PendingSendPacket
{
public:
	PendingSendPacket() {}
	PendingSendPacket( std::shared_ptr<OutputBitStream>& OutPutPacketBuffer,
		muduo::net::UdpConnectionPtr& UdpConnection )
		:
		outPacketBuf_( OutPutPacketBuffer ),
		udpConn_( UdpConnection )
	{}
	muduo::net::UdpConnectionPtr GetUdpConnection() { return udpConn_.lock(); }
	std::shared_ptr<OutputBitStream>& GetPacketBuffer() { return outPacketBuf_; }
private:
	std::shared_ptr<OutputBitStream>					outPacketBuf_;
	std::weak_ptr<muduo::net::UdpConnection>	udpConn_;
};
typedef moodycamel::ConcurrentQueue< PendingSendPacket > PendingSendPacketQueue;
typedef moodycamel::BlockingConcurrentQueue< PendingSendPacket > PendingSendPacketBlockQueue;

}