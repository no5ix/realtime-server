#pragma once

#include <memory>

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>

#include <muduo_udp_support/UdpServer.h>
#include <muduo_udp_support/UdpConnection.h>

#include <realtime_srv/net/Packet.h>



namespace realtime_srv
{

class PktHandler;

// dispatch
class PktDispatcher
{
public:
	typedef std::function<
		void( const muduo::net::UdpConnectionPtr& )
	> UdpConnectionCallback;

	static const float	kSendPacketInterval;
public:

	PktDispatcher();

	bool Init( uint16_t inPort,
		ReceivedPacketBlockQueue* const inRecvPktBQ,
		PendingSendPacketQueue* const inSndPktQ,
		bool isLazy = false );

	void Start();

	void SetInterval( std::function<void()> func, double interval );

	void SetConnCallback( const UdpConnectionCallback& cb )
	{ connCb_ = cb; }


protected:
	void SendGamePacket();

	void IoThreadInit( muduo::net::EventLoop* loop );

	void onMessage( const muduo::net::UdpConnectionPtr& conn,
		muduo::net::Buffer* buf, muduo::Timestamp receiveTime );

	void onConnection( const muduo::net::UdpConnectionPtr& conn );

	//void onMessageLazy( const muduo::net::UdpConnectionPtr& conn,
	//	muduo::net::Buffer* buf, muduo::Timestamp receiveTime );


private:
	bool						isLazy_;
	float						mDropPacketChance;
	float						mSimulatedLatency;
	bool						mSimulateJitter;
	bool						isSimilateRealWorld_;
	float						mLastCheckDCTime;

	UdpConnectionCallback connCb_;
private:
	ReceivedPacketBlockQueue* recvedPktBQ_;
	PendingSendPacketQueue* pendingSndPktQ_;

	muduo::net::EventLoop serverBaseLoop_;
	std::unique_ptr<muduo::net::UdpServer> server_;
};

}