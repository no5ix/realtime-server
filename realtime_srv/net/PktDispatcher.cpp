#include "realtime_srv/common/RealtimeSrvShared.h"
#include <realtime_srv/net/PktDispatcher.h>


using namespace realtime_srv;


using namespace muduo;
using namespace muduo::net;


const float PktDispatcher::kSendPacketInterval = 0.033f;


PktDispatcher::PktDispatcher() :
	mDropPacketChance( 0.f ),
	mSimulatedLatency( 0.f ),
	mSimulateJitter( false ),
	mLastCheckDCTime( 0.f ),
	isSimilateRealWorld_( false ),
	isLazy_( false )
{}

bool PktDispatcher::Init( uint16_t inPort,
	ReceivedPacketBlockQueue* const inRecvPktBQ,
	PendingSendPacketQueue* const inSndPktQ,
	bool isLazy /*= false*/ )
{
	isLazy_ = isLazy;
	recvedPktBQ_ = inRecvPktBQ;
	pendingSndPktQ_ = inSndPktQ;
	InetAddress serverAddr( inPort );

	server_.reset( new UdpServer( &serverBaseLoop_, serverAddr, "rs_pkt_dispatcher" ) );
	server_->setConnectionCallback(
		std::bind( &PktDispatcher::onConnection, this, _1 ) );
	server_->setThreadNum( CONNECTION_THREAD_NUM );

	if ( !isLazy_ )
	{
		server_->setMessageCallback(
			std::bind( &PktDispatcher::onMessage, this, _1, _2, _3 ) );
		server_->setThreadInitCallback(
			std::bind( &PktDispatcher::IoThreadInit, this, _1 ) );
	}
	//else
	//{
	//	server_->setMessageCallback(
	//		std::bind( &PktDispatcher::onMessageLazy, this, _1, _2, _3 ) );
	//}
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
	loop->runEvery( static_cast< double >( kSendPacketInterval ),
		std::bind( &PktDispatcher::SendGamePacket, this ) );
}

void PktDispatcher::onMessage( const muduo::net::UdpConnectionPtr& conn,
	muduo::net::Buffer* buf, muduo::Timestamp receiveTime )
{
	if ( buf->readableBytes() > 0 )
	{
		shared_ptr<InputBitStream> inputStreamPtr(
			new InputBitStream( buf->peek(), buf->readableBytes() * 8 ) );
		buf->retrieveAll();

		recvedPktBQ_->enqueue( ReceivedPacket(
			RealtimeSrvTiming::sInst.GetCurrentGameTime(), inputStreamPtr, conn ) );
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
	PendingSendPacket pendingSndPkt;
	UdpConnectionPtr udpConn;
	while ( pendingSndPktQ_->try_dequeue( pendingSndPkt ) )
	{
		if ( udpConn = pendingSndPkt.GetUdpConnection() )
		{
			udpConn->send( pendingSndPkt.GetPacketBuffer()->GetBufferPtr(),
				pendingSndPkt.GetPacketBuffer()->GetByteLength() );
		}
	}
}