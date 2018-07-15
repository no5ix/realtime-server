#include <realtime_srv/net/PktHandler.h>


using namespace realtime_srv;

using namespace muduo;

PktHandler::PktHandler( ReceivedPacketBlockQueue* const inRecvPktBQ,
	PktProcessCallback pktProcessCallback )
	:
	recvedPktBQ_( inRecvPktBQ ),
	pktHandleThread_(
		std::bind( &PktHandler::ProcessPkt, this, pktProcessCallback ),
		"rs_pkt_handler" )
{}


void PktHandler::ProcessPkt( PktProcessCallback pktProcessCallback )
{
	const int wakeupIntervalSec = 6;
	const int MicroSecsPerSec = 1000 * 1000;
	const int64_t waitTimeOut = static_cast< int64_t >(
		wakeupIntervalSec * MicroSecsPerSec );

	size_t count = 0;
	std::vector< ReceivedPacket > recvedPackets;
	recvedPackets.reserve( kMaxPacketsCountPerRound );

	while ( true )
	{
		count = recvedPktBQ_->wait_dequeue_bulk_timed(
			recvedPackets.begin(), kMaxPacketsCountPerRound, waitTimeOut );

		DoPendingFuncs();
		for ( size_t i = 0; i != count; ++i )
		{
			pktProcessCallback( recvedPackets[i] );
		}
		recvedPackets.clear();
	}
}

void PktHandler::DoPendingFuncs()
{
	std::vector<PendingFunc> tempPendingFuncs;
	{
		MutexLockGuard lock( mutex_ );
		tempPendingFuncs.swap( pendingFuncs_ );
	}
	for ( size_t i = 0; i < tempPendingFuncs.size(); ++i )
	{
		tempPendingFuncs[i]();
	}
}

void realtime_srv::PktHandler::AppendToPendingFuncs( PendingFunc func )
{
	MutexLockGuard lock( mutex_ );
	pendingFuncs_.push_back( std::move( func ) );
}
