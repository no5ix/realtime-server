#ifdef __linux__

#include <realtime_srv/net/PktHandler.h>

using namespace realtime_srv;
using namespace muduo;


namespace
{
const size_t	kMaxPacketsCountPerRound = 10;
const int64_t kQueueWaitTimeoutUsec = 11000000;
}


PktHandler::PktHandler( ReceivedPacketBlockQueue* const inRecvPktBQ,
	PktProcessCallback pktProcessCallback )
	:
	threadId_( 0 ),
	isInvokingPendingFunc_( false ),
	recvedPktBQ_( inRecvPktBQ ),
	pktHandleThread_(
		std::bind( &PktHandler::ProcessPkt, this, pktProcessCallback ),
		"rs_pkt_handler" )
{}

void PktHandler::ProcessPkt( PktProcessCallback pktProcessCb )
{
	threadId_ = muduo::CurrentThread::tid();

	size_t cnt = 0;
	std::vector< ReceivedPacketPtr > tempRecvedPkts( kMaxPacketsCountPerRound );
	while ( true )
	{
		cnt = recvedPktBQ_->wait_dequeue_bulk_timed( tempRecvedPkts.begin(),
			kMaxPacketsCountPerRound, kQueueWaitTimeoutUsec );

		for ( size_t i = 0; i != cnt; ++i )
			if ( tempRecvedPkts[i] )
			{ pktProcessCb( tempRecvedPkts[i] ); tempRecvedPkts[i].reset(); }

		DoPendingFuncs();
	}
}

void PktHandler::DoPendingFuncs()
{
	isInvokingPendingFunc_ = true;

	while ( pendingFuncsQ_.try_dequeue( pendingFunc_ ) )
	{ pendingFunc_(); pendingFunc_ = PendingFunc(); } // do pendingFunc_ & release objs in the pendingFunc_

	isInvokingPendingFunc_ = false;
}

void PktHandler::AppendToPendingFuncs( PendingFunc func )
{
	pendingFuncsQ_.enqueue( std::move( func ) );
	if ( !IsInPktHandlerThread() || isInvokingPendingFunc_ ) Wakeup();
}


#endif // __linux__