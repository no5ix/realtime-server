#include "UdpAcceptor.h"
#include "UdpSocketsOps.h"
#include "UdpConnector.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/SocketsOps.h>

#include <errno.h>
#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;


UdpAcceptor::UdpAcceptor( EventLoop* loop, const InetAddress& listenAddr, bool reuseport )
	: loop_( loop ),
	acceptSocket_( sockets::createUdpNonblockingOrDie( listenAddr.family() ) ),
	acceptChannel_( loop, acceptSocket_.fd() ),
	listenning_( false ),
	listenPort_( listenAddr.toPort() )
{
	acceptSocket_.setReuseAddr( true );
	acceptSocket_.setReusePort( reuseport );
	acceptSocket_.bindAddress( listenAddr );

	acceptChannel_.setReadCallback(
		std::bind( &UdpAcceptor::handleRead, this ) );
}

UdpAcceptor::~UdpAcceptor()
{
	acceptChannel_.disableAll();
	acceptChannel_.remove();

	peerAddrToUdpConnectors_.empty();
}

void UdpAcceptor::listen()
{
	loop_->assertInLoopThread();
	listenning_ = true;

	acceptChannel_.enableReading();
}

void UdpAcceptor::handleRead()
{
	loop_->assertInLoopThread();
	InetAddress peerAddr;

	struct sockaddr_in6 addr;
	bzero( &addr, sizeof addr );
	int readByteCount = sockets::recvfrom( acceptSocket_.fd(), &addr );

	if ( readByteCount >= 0 )
	{
		peerAddr.setSockAddrInet6( addr );

		if ( peerAddrToUdpConnectors_.find( peerAddr )
			== peerAddrToUdpConnectors_.end() ) // check whether is connecting
		{
			UdpConnectorPtr newUdpConnector( new UdpConnector( loop_, peerAddr, listenPort_ ) );
			peerAddrToUdpConnectors_[peerAddr] = newUdpConnector;

			newUdpConnector->setNewConnectionCallback(
				std::bind( &UdpAcceptor::newConnection, this, _1, peerAddr ) );
			newUdpConnector->start();
		}
	}
	else
	{
		LOG_SYSERR << "in UdpAcceptor::handleRead";
	}
}

void UdpAcceptor::newConnection( Socket* connectedSocket,
	const InetAddress& peerAddr )
{
	loop_->assertInLoopThread();
	if ( newConnectionCallback_ )
	{
		newConnectionCallback_( connectedSocket, peerAddr );
	}
	else
	{
		//sockets::close( connfd );
	}
}

void UdpAcceptor::RemoveConnector( const InetAddress& peerAddr )
{
	loop_->assertInLoopThread();
	assert( peerAddrToUdpConnectors_[peerAddr] );
	( peerAddrToUdpConnectors_[peerAddr] )->stop();
	loop_->runAfter( 1, std::bind( &UdpAcceptor::EraseConnector, this, peerAddr ) );
}

void UdpAcceptor::EraseConnector( const InetAddress& peerAddr )
{
	loop_->assertInLoopThread();
	peerAddrToUdpConnectors_.erase( peerAddr );
}