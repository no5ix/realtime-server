#pragma once

//#ifdef __linux__

#include <memory>
#include <map>
#include <vector>

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
	typedef std::function<void()> CheckDisconnectCallback;

public:

	PktHandler( const ServerConfig _serverConfig,
		PktProcessCallback _pktProcessCallback,
		TickCallback _tickCb,
		CheckDisconnectCallback _checkDisconnCb = CheckDisconnectCallback() );

	void Start() { assert( server_ ); server_->start(); baseLoop_.loop(); }

	void SetInterval( std::function<void()> func, double interval )
	{ baseLoop_.runEvery( interval, std::move( func ) ); }

	void SetConnCallback( const UdpConnectionCallback& cb )
	{ connCb_ = cb; }

	muduo::net::EventLoop* GetBaseLoop() { return &baseLoop_; }

	void AddToPendingSndPktQ( const PendingSendPacketPtr& psp, const pid_t threadId )
	{ tidToPendingSndPktQMap_.at( threadId ).enqueue( psp ); }

	ReceivedPacketBlockQueue* GetReceivedPacketBlockQueue()
	{ return &recvedPktBQ_; }

	double GetSendPacketInterval() const
	{ return sendPacketInterval_; }

	double GetClientDisconnectTimeout() const
	{ return clientDisconnectTimeout_; }

protected:
	void SendPkt();
	void ProcessPkt();

	// long time no pkt to snd, then sleep
	void CheckForSleep();
	void CheckForWakingUp();
	void AddToAutoSleepSystem( muduo::net::EventLoop* _loop,
		muduo::net::TimerId _timerId );

	void IoThreadInit( muduo::net::EventLoop* loop );

	void OnPktComing( const muduo::net::UdpConnectionPtr& conn,
		muduo::net::Buffer* buf, muduo::Timestamp receiveTime );

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
	std::map<int, LoopAndTimerId> tidToLoopAndTimerIdMap_;

	std::map<int, PendingSendPacketQueue> tidToPendingSndPktQMap_;

	CheckDisconnectCallback checkDisconnCb_;
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
	const double clientDisconnectTimeout_;
	const uint8_t pktDispatcherThreadCnt_;

};

}

//#endif // __linux__