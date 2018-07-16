#pragma once

#ifdef __linux__

#include <functional>
#include <vector>

#include <muduo/base/Thread.h>

#include <concurrent_queue/concurrentqueue.h>
#include <concurrent_queue/blockingconcurrentqueue.h>

#include "realtime_srv/net/Packet.h"


namespace realtime_srv
{

class PktHandler
{
public:
	typedef std::function<void( ReceivedPacketPtr& )> PktProcessCallback;
	typedef std::function< void() > PendingFunc;

public:

	PktHandler( ReceivedPacketBlockQueue* const inRecvPktBQ,
		PktProcessCallback pktProcessCallback );

	~PktHandler() { pktHandleThread_.join(); }

	void Start()
	{ assert( !pktHandleThread_.started() ); pktHandleThread_.start(); }

	void AppendToPendingFuncs( PendingFunc func );

private:
	void ProcessPkt( PktProcessCallback inPktHandleCallback );
	void DoPendingFuncs();

	void Wakeup() { recvedPktBQ_->enqueue( ReceivedPacketPtr() ); }

	bool IsInPktHandlerThread() const { return threadId_ == muduo::CurrentThread::tid(); }

private:
	typedef moodycamel::ConcurrentQueue<PendingFunc> PendingFuncsQueue;
	PendingFuncsQueue pendingFuncsQ_;
	PendingFunc pendingFunc_;
	bool isInvokingPendingFunc_;

	ReceivedPacketBlockQueue* recvedPktBQ_;

	muduo::Thread pktHandleThread_;
	pid_t					threadId_;
};

}

#endif // __linux__