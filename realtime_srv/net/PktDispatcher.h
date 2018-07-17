#pragma once

#ifdef __linux__

#include <memory>
#include <map>

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>

#include <muduo_udp_support/UdpServer.h>
#include <muduo_udp_support/UdpConnection.h>

#include <realtime_srv/net/Packet.h>



namespace realtime_srv
{


class PktDispatcher
{
public:
	typedef std::function<
		void( const muduo::net::UdpConnectionPtr& )
	> UdpConnectionCallback;

	static const float	kSendPacketInterval;

public:

	PktDispatcher( uint16_t inPort, uint32_t inThreadCount );

	void Start() { assert( server_ ); server_->start(); baseLoop_.loop(); }

	void SetInterval( std::function<void()> func, double interval )
	{ baseLoop_.runEvery( interval, func ); }

	void SetConnCallback( const UdpConnectionCallback& cb )
	{ connCb_ = cb; }

	muduo::net::EventLoop* GetBaseLoop() { return &baseLoop_; }

	void AppendToPendingSndPktQ( const PendingSendPacketPtr& psp,
		const pid_t threadId )
	{ tidToPendingSndPktQMap_.at( threadId ).enqueue( psp ); }

	ReceivedPacketBlockQueue* GetReceivedPacketBlockQueue()
	{ return &recvedPktBQ_; }

protected:
	void SendGamePacket();

	void IoThreadInit( muduo::net::EventLoop* loop );

	void onMessage( const muduo::net::UdpConnectionPtr& conn,
		muduo::net::Buffer* buf, muduo::Timestamp receiveTime );

	void onConnection( const muduo::net::UdpConnectionPtr& conn )
	{ if ( connCb_ ) connCb_( conn ); }

private:
	struct LoopAndTimerId
	{
		LoopAndTimerId() {}
		LoopAndTimerId(
			muduo::net::EventLoop* inLoop,
			muduo::net::TimerId		inTimerId )
			:
			loop_( inLoop ),
			timerId_( inTimerId )
		{}
		muduo::net::EventLoop* loop_;
		muduo::net::TimerId		timerId_;
	};
	std::map<int, LoopAndTimerId> tidToLoopAndTimerIdMap_;

	std::map<int, PendingSendPacketQueue> tidToPendingSndPktQMap_;

	UdpConnectionCallback connCb_;
	ReceivedPacketBlockQueue recvedPktBQ_;

	muduo::net::EventLoop baseLoop_;
	std::unique_ptr<muduo::net::UdpServer> server_;

};

}

#endif // __linux__