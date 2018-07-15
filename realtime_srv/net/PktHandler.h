#pragma once

#ifdef __linux__

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
	static const size_t	kMaxPacketsCountPerRound = 10;
	typedef std::function<void( ReceivedPacket& )> PktProcessCallback;
	typedef std::function< void() > PendingFunc;

public:

	PktHandler( ReceivedPacketBlockQueue* const inRecvPktBQ,
		PktProcessCallback pktProcessCallback );

	~PktHandler() { pktHandleThread_.join(); }

	void Start() { assert( !pktHandleThread_.started() ); pktHandleThread_.start(); }

	void AppendToPendingFuncs( PendingFunc func );

private:
	void ProcessPkt( PktProcessCallback inPktHandleCallback );
	void DoPendingFuncs();

private:
	std::vector< PendingFunc > pendingFuncs_;
	ReceivedPacketBlockQueue* recvedPktBQ_;
	muduo::Thread pktHandleThread_;
	muduo::MutexLock mutex_;
};

}

#endif // __linux__