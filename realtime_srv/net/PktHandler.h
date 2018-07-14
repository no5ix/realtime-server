#pragma once

#include <functional>
#include <vector>
#include <muduo/base/Thread.h>
#include <muduo/base/Mutex.h>

#include "realtime_srv/net/Packet.h"


namespace realtime_srv
{

class PktHandler
{
public:
	static const size_t	kMaxPacketsPerFrameCount = 10;
	typedef std::function<void( ReceivedPacket& )> PktHandleCallback;
	typedef std::function< void() > PendingFunc;

public:

	PktHandler( ReceivedPacketBlockQueue* const inRecvPktBQ,
		PktHandleCallback inPktHandleCallback );

	~PktHandler() { pktHandleThread_.join(); }

	void Start() { assert( !pktHandleThread_.started() ); pktHandleThread_.start(); }

	void Tick( PktHandleCallback inPktHandleCallback );

	void DoTickPendingFuncs();

	void AppendToPendingFuncs( PendingFunc cb );

private:
	std::vector< PendingFunc > pendingFuncs_;
	ReceivedPacketBlockQueue* recvedPktBQ_;
	muduo::Thread pktHandleThread_;
	muduo::MutexLock mutex_;
};

}