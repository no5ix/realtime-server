#ifdef __linux__

#include <realtime_srv/net/PktHandler.h>

using namespace realtime_srv;
using namespace muduo;


namespace
{
const size_t	kMaxPacketsCountPerRound = 10;
}


PktHandler::PktHandler( ReceivedPacketBlockQueue* const inRecvPktBQ,
	PktProcessCallback pktProcessCallback )
	:
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
	ReceivedPacketPtr rp;
	// - correct way below
	std::vector< ReceivedPacketPtr > tempRecvedPkts( kMaxPacketsCountPerRound );
	// - wrong way below : concurrentQueue will not release the last group
	//std::vector< ReceivedPacketPtr > tempRecvedPkts;
	//tempRecvedPkts.reserve( kMaxPacketsCountPerRound );
	while ( true )
	{
		cnt = recvedPktBQ_->wait_dequeue_bulk(
			tempRecvedPkts.begin(), kMaxPacketsCountPerRound );

		for ( size_t i = 0; i != cnt; ++i )
			if ( rp = tempRecvedPkts[i] ) { pktProcessCb( rp ); rp.reset(); }

		DoPendingFuncs();
	}
}

void PktHandler::DoPendingFuncs()
{
	isInvokingPendingFunc_ = true;

	while ( pendingFuncsQ_.try_dequeue( pendingFunc_ ) )
	{ pendingFunc_(); pendingFunc_ = PendingFunc(); } // invoke pendingFunc_ & release objs in the pendingFunc_

	isInvokingPendingFunc_ = false;
}

void PktHandler::AppendToPendingFuncs( PendingFunc func )
{
	pendingFuncsQ_.enqueue( std::move( func ) );
	if ( !isInPktHandlerThread() || isInvokingPendingFunc_ ) Wakeup();
}

void PktHandler::Wakeup()
{
	recvedPktBQ_->enqueue( ReceivedPacketPtr() );
}

#endif // __linux__