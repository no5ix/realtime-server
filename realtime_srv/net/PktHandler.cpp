#include <realtime_srv/net/PktHandler.h>


using namespace realtime_srv;

using namespace muduo;

PktHandler::PktHandler( ReceivedPacketBlockQueue* const inRecvPktBQ,
	PktHandleCallback inPktHandleCallback )
	:
	recvedPktBQ_( inRecvPktBQ ),
	pktHandleThread_(
		std::bind( &PktHandler::Tick, this, inPktHandleCallback ),
		"rs_pkt_handler" )
{}


void PktHandler::Tick( PktHandleCallback inPktHandleCallback )
{
	//const int kMicroSecondsPerSecond = 1000 * 1000;
	//const int64_t checkDisConnTimeOut = static_cast< int64_t >(
	//	kClientDisconnectTimeout * kMicroSecondsPerSecond );

	size_t count = 0;
	std::vector<	ReceivedPacket > recvedPackets;
	recvedPackets.reserve( kMaxPacketsPerFrameCount );

	while ( true )
	{
		//count = recvedPacketBlockQ_.wait_dequeue_bulk_timed(
		//	recvedPacket.begin(), kMaxPacketsPerFrameCount, checkDisConnTimeOut );
		//while ( recvedPacketBlockQ_.wait_dequeue_timed( recvedPacket, checkDisConnTimeOut ) )

		count = recvedPktBQ_->wait_dequeue_bulk(
			recvedPackets.begin(), kMaxPacketsPerFrameCount );
		for ( size_t i = 0; i != count; ++i )
		{
			//ProcessPacket( *( recvedPackets[i].GetPacketBuffer() ), recvedPackets[i].GetUdpConn() );
			//worldUpdateCb_();
			//PrepareOutgoingPackets();

			PktHandleCallback( recvedPackets[i] );
		}
		DoTickPendingFuncs();
	}
}

void PktHandler::DoTickPendingFuncs()
{
	std::vector<PendingFunc> tempTickPendingFuncs;

	{
		MutexLockGuard lock( mutex_ );
		tempTickPendingFuncs.swap( pendingFuncs_ );
	}

	for ( size_t i = 0; i < tempTickPendingFuncs.size(); ++i )
	{
		tempTickPendingFuncs[i]();
	}
}

void realtime_srv::PktHandler::AppendToPendingFuncs( PendingFunc func )
{
	MutexLockGuard lock( mutex_ );
	pendingFuncs_.push_back( std::move( func ) );
}
