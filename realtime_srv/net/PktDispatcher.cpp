#ifdef __linux__

#include <realtime_srv/net/PktDispatcher.h>
#include <realtime_srv/common/RealtimeSrvTiming.h>

using namespace realtime_srv;


using namespace muduo;
using namespace muduo::net;


const float PktDispatcher::kSendPacketInterval = 0.033f;

namespace
{
__thread int t_sndCountThisRound_ = 0;
__thread int t_sndCountLastRound_ = 0;
__thread int t_noPktSndRoundCount_ = 0;
__thread bool t_isSleep_ = false;

const int sleepRoundCountThreshold = static_cast< int >( 1 / PktDispatcher::kSendPacketInterval );
}



PktDispatcher::PktDispatcher( uint16_t inPort, uint32_t inThreadCount,
	ReceivedPacketBlockQueue* const inRecvPktBQ,
	PendingSendPacketQueue* const inSndPktQ )
	:
	recvedPktBQ_( inRecvPktBQ ),
	pendingSndPktQ_( inSndPktQ )
{
	InetAddress serverAddr( inPort );

	server_.reset( new UdpServer( &baseLoop_, serverAddr, "rs_pkt_dispatcher" ) );
	server_->setConnectionCallback(
		std::bind( &PktDispatcher::onConnection, this, _1 ) );
	server_->setThreadNum( inThreadCount );
	server_->setMessageCallback(
		std::bind( &PktDispatcher::onMessage, this, _1, _2, _3 ) );
	server_->setThreadInitCallback(
		std::bind( &PktDispatcher::IoThreadInit, this, _1 ) );
}

void PktDispatcher::Start()
{
	assert( server_ );

	server_->start();
	baseLoop_.loop();
}

void PktDispatcher::IoThreadInit( EventLoop* loop )
{
	muduo::net::TimerId curTimerId = loop->runEvery(
		static_cast< double >( kSendPacketInterval ),
		std::bind( &PktDispatcher::SendGamePacket, this ) );

		tidToLoopAndTimerIdMap_[muduo::CurrentThread::tid()] =
			LoopAndTimerId( loop, curTimerId );
}

void PktDispatcher::onMessage( const muduo::net::UdpConnectionPtr& conn,
	muduo::net::Buffer* buf, muduo::Timestamp receiveTime )
{
	if ( buf->readableBytes() > 0 )
	{
		std::shared_ptr<InputBitStream> inputStreamPtr(
			new InputBitStream( buf->peek(), buf->readableBytes() * 8 ) );
		buf->retrieveAll();

		recvedPktBQ_->enqueue( ReceivedPacketPtr( new ReceivedPacket(
			RealtimeSrvTiming::sInst.GetCurrentGameTime(), inputStreamPtr, conn ) ) );

		//wake up
		if ( t_isSleep_ )
		{
			LoopAndTimerId& lat = tidToLoopAndTimerIdMap_[muduo::CurrentThread::tid()];
			muduo::net::TimerId curTimerId = lat.loop_->runEvery(
				static_cast< double >( kSendPacketInterval ),
				std::bind( &PktDispatcher::SendGamePacket, this ) );
			lat.timerId_ = curTimerId;
			t_isSleep_ = false;
			LOG_INFO << "wake uuuuuup";
		}
	}
}

void PktDispatcher::onConnection( const UdpConnectionPtr& conn )
{
	LOG_INFO << conn->localAddress().toIpPort() << " -> "
		<< conn->peerAddress().toIpPort() << " is "
		<< ( conn->connected() ? "UP" : "DOWN" );

	connCb_( conn );
}

void PktDispatcher::SendGamePacket()
{
	t_sndCountThisRound_ = 0;
	PendingSendPacketPtr pendingSndPkt;
	while ( pendingSndPktQ_->try_dequeue( pendingSndPkt ) )
	{
		++t_sndCountThisRound_;
		pendingSndPkt->GetUdpConnection()->send(
			pendingSndPkt->GetPacketBuffer()->GetBufferPtr(),
			pendingSndPkt->GetPacketBuffer()->GetByteLength() );
	}
	// 1 sec no pkt to snd, then sleep
	if ( t_sndCountThisRound_ == 0 && t_sndCountLastRound_ == 0 )
	{
		if ( ++t_noPktSndRoundCount_ > sleepRoundCountThreshold )
		{
			t_noPktSndRoundCount_ = 0;
			LoopAndTimerId& lat = tidToLoopAndTimerIdMap_[muduo::CurrentThread::tid()];
			lat.loop_->cancel( lat.timerId_ );
			t_isSleep_ = true;
			LOG_INFO << "go to sleeeeeeeep";
		}
	}
	else
	{
		t_noPktSndRoundCount_ = 0;
	}
	t_sndCountLastRound_ = t_sndCountThisRound_;
}

#endif // __linux__
