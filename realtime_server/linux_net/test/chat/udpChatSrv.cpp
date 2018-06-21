#include "udpChatCodec.h"

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <net/UdpServer.h>

#include <set>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer : noncopyable
{
public:
	ChatServer( EventLoop* loop,
		const InetAddress& listenAddr )
		: server_( loop, listenAddr, "ChatServer" ),
		codec_( std::bind( &ChatServer::onStringMessage, this, _1, _2, _3 ) )
	{
		server_.setConnectionCallback(
			std::bind( &ChatServer::onConnection, this, _1 ) );
		server_.setMessageCallback(
			std::bind( &UdpLengthHeaderCodec::onMessage, &codec_, _1, _2, _3 ) );
	}

	void start()
	{
		server_.start();
	}

private:
	void onConnection( const UdpConnectionPtr& conn )
	{
		LOG_INFO << conn->localAddress().toIpPort() << " -> "
			<< conn->peerAddress().toIpPort() << " is "
			<< ( conn->connected() ? "UP" : "DOWN" );

		if ( conn->connected() )
		{
			connections_.insert( conn );
		}
		else
		{
			connections_.erase( conn );
		}
	}

	void onStringMessage( const UdpConnectionPtr&,
		const string& message,
		Timestamp )
	{
		for ( ConnectionList::iterator it = connections_.begin();
			it != connections_.end();
			++it )
		{
			codec_.send( get_pointer( *it ), message );
		}
	}

	typedef std::set<UdpConnectionPtr> ConnectionList;
	UdpServer server_;
	UdpLengthHeaderCodec codec_;
	ConnectionList connections_;
};

int main( int argc, char* argv[] )
{
	LOG_INFO << "pid = " << getpid();
	if ( argc > 1 )
	{
		EventLoop loop;
		uint16_t port = static_cast< uint16_t >( atoi( argv[1] ) );
		InetAddress serverAddr( port );
		ChatServer server( &loop, serverAddr );
		server.start();
		loop.loop();
	}
	else
	{
		printf( "Usage: %s port\n", argv[0] );
	}
}

