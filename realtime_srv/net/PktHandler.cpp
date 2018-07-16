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
	recvedPktBQ_( inRecvPktBQ ),
	pktHandleThread_(
		std::bind( &PktHandler::ProcessPkt, this, pktProcessCallback ),
		"rs_pkt_handler" )
{}

void PktHandler::ProcessPkt( PktProcessCallback pktProcessCb )
{
	size_t cnt = 0;
	ReceivedPacketPtr rp;
	// - correct way below
	std::vector< ReceivedPacketPtr > tempRecvedPkts( kMaxPacketsCountPerRound );
	// - wrong way below : concurrentQueue will not release the last group
	//std::vector< ReceivedPacketPtr > tempRecvedPkts;
	//tempRecvedPkts.reserve( kMaxPacketsCountPerRound );
	while ( true )
	{
		cnt = recvedPktBQ_->wait_dequeue_bulk_timed( tempRecvedPkts.begin(),
			kMaxPacketsCountPerRound, kQueueWaitTimeoutUsec );

		for ( size_t i = 0; i != cnt; ++i )
			if ( rp = tempRecvedPkts[i] ) { pktProcessCb( rp ); rp.reset(); }

		DoPendingFuncs();
	}
}

void PktHandler::DoPendingFuncs()
{
	while ( pendingFuncsQ_.try_dequeue( pendingFunc_ ) )
	{ pendingFunc_(); pendingFunc_ = PendingFunc(); } // invoke pendingFunc_ & release objs in the pendingFunc_
}

void PktHandler::AppendToPendingFuncs( PendingFunc func )
{
	pendingFuncsQ_.enqueue( std::move( func ) );
	Wakeup();
}

void PktHandler::Wakeup()
{
	recvedPktBQ_->enqueue( ReceivedPacketPtr() );
}

#endif // __linux__