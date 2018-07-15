//#include "realtime_srv/common/RealtimeSrvShared.h"
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



bool PktDispatcher::Init( uint16_t inPort,
	ReceivedPacketBlockQueue* const inRecvPktBQ,
	PendingSendPacketQueue* const inSndPktQ )
{
	recvedPktBQ_ = inRecvPktBQ;
	pendingSndPktQ_ = inSndPktQ;
	InetAddress serverAddr( inPort );

	server_.reset( new UdpServer( &serverBaseLoop_, serverAddr, "rs_pkt_dispatcher" ) );
	server_->setConnectionCallback(
		std::bind( &PktDispatcher::onConnection, this, _1 ) );
	server_->setThreadNum( CONNECTION_THREAD_NUM );
	server_->setMessageCallback(
		std::bind( &PktDispatcher::onMessage, this, _1, _2, _3 ) );
	server_->setThreadInitCallback(
		std::bind( &PktDispatcher::IoThreadInit, this, _1 ) );

	return true;
}

void PktDispatcher::SetInterval( std::function<void()> func, double interval )
{
	serverBaseLoop_.runEvery( interval, func );
}

void PktDispatcher::Start()
{
	assert( server_ );

	server_->start();
	serverBaseLoop_.loop();
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

		// test
		recvedPktBQ_->enqueue( ReceivedPacket(
			RealtimeSrvTiming::sInst.GetCurrentGameTime(), inputStreamPtr, conn ) );

		//wake up
		if ( t_isSleep_ )
		{
			LoopAndTimerId& lat = tidToLoopAndTimerIdMap_[muduo::CurrentThread::tid()];
			muduo::net::TimerId curTimerId = lat.loop_->runEvery(
				static_cast< double >( kSendPacketInterval ),
				std::bind( &PktDispatcher::SendGamePacket, this ) );
			lat.timerId_ = curTimerId;
			t_isSleep_ = false;
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
	// test
	t_sndCountThisRound_ = 0;
	PendingSendPacket pendingSndPkt;
	while ( pendingSndPktQ_->try_dequeue( pendingSndPkt ) )
	{
		++t_sndCountThisRound_;
		pendingSndPkt.GetUdpConnection()->send(
			pendingSndPkt.GetPacketBuffer()->GetBufferPtr(),
			pendingSndPkt.GetPacketBuffer()->GetByteLength() );
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
		}
	}
	else
	{
		t_noPktSndRoundCount_ = 0;
	}
	t_sndCountLastRound_ = t_sndCountThisRound_;
}