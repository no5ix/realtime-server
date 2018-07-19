//#ifdef __linux__

#include <realtime_srv/net/PktHandler.h>
#include <realtime_srv/common/RealtimeSrvTiming.h>

using namespace realtime_srv;


using namespace muduo;
using namespace muduo::net;


const float PktHandler::kSendPacketInterval = 1.f / 30.f;
const float PktHandler::kClientDisconnectTimeout = 6.f;

namespace
{
__thread int t_handleCountThisRound_ = 0;
__thread int t_handleCountLastRound_ = 0;
__thread int t_noPktHandleRoundCount_ = 0;

__thread bool t_isDispatcherThreadSleeping_ = false;

const float kTickInterval = 1.f / 30.f;
const int sleepRoundCountThreshold = static_cast< int >( 1 / kTickInterval );
const size_t kMaxPacketsCountPerRound = 10;
}


PktHandler::PktHandler( uint16_t _port, uint32_t _threadCount,
	PktProcessCallback _pktProcessCallback,
	TickCallback _tickCb,
	CheckDisconnectCallback _checkDisconnCb /*= CheckDisconnectCallback()*/ )
	:
	isBaseThreadSleeping_( false ),
	baseThreadId_( CurrentThread::tid() ),
	pendingRecvedPktsCnt_( 0 ),
	pendingRecvedPkts_( std::vector<ReceivedPacketPtr>( kMaxPacketsCountPerRound ) ),
	pktProcessCb_( _pktProcessCallback ),
	tickCb_( _tickCb ),
	checkDisconnCb_( _checkDisconnCb )
{
	assert( _threadCount >= 1 );
	assert( tickCb_ );
	assert( pktProcessCb_ );

	InetAddress serverAddr( _port );

	server_.reset( new UdpServer( &baseLoop_, serverAddr, "rs_pkt_dispatcher" ) );

	server_->setThreadNum( _threadCount );

	server_->setConnectionCallback(
		std::bind( &PktHandler::OnConnection, this, _1 ) );
	server_->setMessageCallback(
		std::bind( &PktHandler::OnPktComing, this, _1, _2, _3 ) );
	server_->setThreadInitCallback(
		std::bind( &PktHandler::IoThreadInit, this, _1 ) );

	TimerId procPktTimerId = baseLoop_.runEvery( static_cast< double >( kTickInterval ),
		std::bind( &PktHandler::ProcessPkt, this ) );
	AddToAutoSleepSystem( &baseLoop_, procPktTimerId );

	if ( checkDisconnCb_ )
		baseLoop_.runEvery( static_cast< double >( kClientDisconnectTimeout ),
			checkDisconnCb_ );
}

void PktHandler::AddToAutoSleepSystem( EventLoop* _loop, TimerId _timerId )
{
	tidToLoopAndTimerIdMap_[CurrentThread::tid()] =
		LoopAndTimerId( _loop, _timerId );
}

void PktHandler::CheckForSleep()
{
	if ( t_handleCountThisRound_ == 0 && t_handleCountLastRound_ == 0 )
	{
		if ( ++t_noPktHandleRoundCount_ > sleepRoundCountThreshold )
		{
			t_noPktHandleRoundCount_ = 0;
			LoopAndTimerId& lat = tidToLoopAndTimerIdMap_.at( CurrentThread::tid() );
			lat.loop_->cancel( lat.timerId_ );

			if ( CurrentThread::tid() == baseThreadId_ )
			{
				isBaseThreadSleeping_ = true;
				LOG_INFO << "BaseThread stop ticking";
			}
			else
			{
				t_isDispatcherThreadSleeping_ = true;
				LOG_INFO << "DispatcherThread stop dispatching";
			}
		}
	}
	else
		t_noPktHandleRoundCount_ = 0;

	t_handleCountLastRound_ = t_handleCountThisRound_;
	t_handleCountThisRound_ = 0;
}

void PktHandler::CheckForWakingUp()
{
	if ( isBaseThreadSleeping_ )
	{
		muduo::net::TimerId curTimerId;
		LoopAndTimerId& lat = tidToLoopAndTimerIdMap_.at( baseThreadId_ );
		curTimerId = lat.loop_->runEvery(
			static_cast< double >( kTickInterval ),
			std::bind( &PktHandler::ProcessPkt, this ) );

		lat.timerId_ = curTimerId;
		isBaseThreadSleeping_ = false;
		LOG_INFO << "BaseThread begin ticking";
	}

	if ( t_isDispatcherThreadSleeping_ )
	{
		muduo::net::TimerId curTimerId;
		LoopAndTimerId& lat = tidToLoopAndTimerIdMap_.at( CurrentThread::tid() );
		curTimerId = lat.loop_->runEvery(
			static_cast< double >( kSendPacketInterval ),
			std::bind( &PktHandler::SendPkt, this ) );

		lat.timerId_ = curTimerId;
		t_isDispatcherThreadSleeping_ = false;
		LOG_INFO << "DispatcherThread begin dispatching";
	}
}

void PktHandler::ProcessPkt()
{
	while ( ( pendingRecvedPktsCnt_ = recvedPktBQ_.try_dequeue_bulk(
		pendingRecvedPkts_.begin(), kMaxPacketsCountPerRound ) ) != 0 )
	{
		for ( size_t i = 0; i != pendingRecvedPktsCnt_; ++i )
		{
			++t_handleCountThisRound_;
			pktProcessCb_( pendingRecvedPkts_[i] ); pendingRecvedPkts_[i].reset();
		}
	}
	tickCb_();
	CheckForSleep();
}

void PktHandler::IoThreadInit( EventLoop* loop )
{
	muduo::net::TimerId sndPktTimerId = loop->runEvery(
		static_cast< double >( kSendPacketInterval ),
		std::bind( &PktHandler::SendPkt, this ) );

	AddToAutoSleepSystem( loop, sndPktTimerId );
	tidToPendingSndPktQMap_[CurrentThread::tid()] = PendingSendPacketQueue();
}

void PktHandler::OnPktComing( const muduo::net::UdpConnectionPtr& conn,
	muduo::net::Buffer* buf, muduo::Timestamp receiveTime )
{
	if ( buf->readableBytes() > 0 )
	{
		std::shared_ptr<InputBitStream> inputStreamPtr(
			new InputBitStream( buf->peek(), buf->readableBytes() * 8 ) );
		buf->retrieveAll();

		recvedPktBQ_.enqueue( ReceivedPacketPtr( new ReceivedPacket(
			RealtimeSrvTiming::sInst.GetCurrentGameTime(),
			CurrentThread::tid(), inputStreamPtr, conn ) ) );

		CheckForWakingUp();
	}
}

void PktHandler::SendPkt()
{
	PendingSendPacketPtr pendingSndPkt;
	PendingSendPacketQueue& curPendingSndPktQ =
		tidToPendingSndPktQMap_.at( CurrentThread::tid() );

	while ( curPendingSndPktQ.try_dequeue( pendingSndPkt ) )
	{
		++t_handleCountThisRound_;
		pendingSndPkt->GetUdpConnection()->send(
			pendingSndPkt->GetPacketBuffer()->GetBufferPtr(),
			pendingSndPkt->GetPacketBuffer()->GetByteLength() );
	}
	CheckForSleep();
}

//#endif // __linux__
