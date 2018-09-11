#pragma once

//#ifdef __linux__

#include <memory>
#include <unordered_map>
#include <vector>

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>

#include <realtinet/UdpServer.h>
#include <realtinet/UdpConnection.h>

#include <realtime_srv/net/Packet.h>
#include <realtime_srv/common/RealtimeSrvHelper.h>


namespace realtime_srv
{


class PktHandler
{
public:
	typedef std::function<
		void( const muduo::net::UdpConnectionPtr& )
	> UdpConnectionCallback;

	typedef std::function<void( ReceivedPacketPtr& )> PktProcessCallback;
	typedef std::function<void()> TickCallback;

public:

	PktHandler( const ServerConfig serverConfig,
		PktProcessCallback pktProcessCallback,
		TickCallback tickCb );

	void Start() { assert( server_ ); server_->start(); baseLoop_.loop(); }

	void SetInterval( std::function<void()> func, double interval )
	{ baseLoop_.runEvery( interval, std::move( func ) ); }

	void SetConnCallback(const UdpConnectionCallback& cb);

	muduo::net::EventLoop* GetBaseLoop() { return &baseLoop_; }

	void AddToPendingSndPktQ( const PendingSendPacketPtr& psp, const pid_t threadId )
	{ tidToPendingSndPktQMap_.at( threadId ).enqueue( psp ); }

	ReceivedPacketBlockQueue* GetReceivedPacketBlockQueue()
	{ return &recvedPktBQ_; }

	double GetSendPacketInterval() const
	{ return sendPacketInterval_; }

protected:
	void SendPkt();
	void ProcessPkt();

	// long time no pkt to snd, then sleep
	void CheckForSleep();
	void CheckForWakingUp();
	void AddToAutoSleepSystem( muduo::net::EventLoop* loop,
		muduo::net::TimerId timerId );

	void IoThreadInit( muduo::net::EventLoop* loop );

	void OnPktComing(const muduo::net::UdpConnectionPtr& conn,
		char* buf, size_t bufBytes, muduo::Timestamp receiveTime);

	void OnConnection( const muduo::net::UdpConnectionPtr& conn )
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
	std::unordered_map<int, LoopAndTimerId> tidToLoopAndTimerIdMap_;

	std::unordered_map<int, PendingSendPacketQueue> tidToPendingSndPktQMap_;

	PktProcessCallback pktProcessCb_;
	TickCallback	tickCb_;

	UdpConnectionCallback connCb_;

	ReceivedPacketBlockQueue recvedPktBQ_;
	std::vector<ReceivedPacketPtr> pendingRecvedPkts_;
	size_t pendingRecvedPktsCnt_;

	muduo::net::EventLoop baseLoop_;
	std::unique_ptr<muduo::net::UdpServer> server_;
	pid_t baseThreadId_;
	bool isBaseThreadSleeping_;

	const int sleepRoundCountThreshold_;

	// conf
	uint16_t port_;
	double tickInterval_;
	const size_t maxPacketsCountPerFetch_;
	const double sendPacketInterval_;
	const uint8_t pktDispatcherThreadCnt_;

};

}

//#endif // __linux__