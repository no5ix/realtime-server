#include "UdpServer.h"

#include <muduo/base/Logging.h>
#include "UdpAcceptor.h"
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/SocketsOps.h>

#include "UdpConnector.h"

#include <stdio.h>  // snprintf


using namespace muduo;
using namespace muduo::net;

UdpServer::UdpServer( EventLoop* loop,
	const InetAddress& listenAddr,
	const string& nameArg,
	Option option )
	: loop_( CHECK_NOTNULL( loop ) ),
	ipPort_( listenAddr.toIpPort() ),
	name_( nameArg ),
	acceptor_( new UdpAcceptor( loop, listenAddr, option == kReusePort ) ),
	threadPool_( new EventLoopThreadPool( loop, name_ ) ),
	connectionCallback_( UdpDefaultConnectionCallback ),
	messageCallback_( UdpDefaultMessageCallback ),
	nextConnId_( 1 )
{
	acceptor_->setNewConnectionCallback(
		std::bind( &UdpServer::newConnection, this, _1, _2 ) );
}

UdpServer::~UdpServer()
{
	loop_->assertInLoopThread();
	LOG_TRACE << "UdpServer::~UdpServer [" << name_ << "] destructing";

	for ( auto& item : connections_ )
	{
		UdpConnectionPtr conn( item.second );
		item.second.reset();
		conn->getLoop()->runInLoop(
			std::bind( &UdpConnection::connectDestroyed, conn ) );
	}
}

void UdpServer::setThreadNum( int numThreads )
{
	assert( 0 <= numThreads );
	threadPool_->setThreadNum( numThreads );
}

void UdpServer::start()
{
	if ( started_.getAndSet( 1 ) == 0 )
	{
		threadPool_->start( threadInitCallback_ );

		assert( !acceptor_->listenning() );
		loop_->runInLoop(
			std::bind( &UdpAcceptor::listen, get_pointer( acceptor_ ) ) );
	}
}

void UdpServer::newConnection( Socket* connectedSocket,
	const InetAddress& peerAddr )
{
	loop_->assertInLoopThread();
	EventLoop* ioLoop = threadPool_->getNextLoop();
	char buf[64];
	snprintf( buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_ );
	string connName = name_ + buf;

	//LOG_INFO << "UdpServer::newConnection [" << name_
		//<< "] - new connection [" << connName
		//<< "] - new connection sockfd [" << connectedSocket->fd()
		//<< "] from " << peerAddr.toIpPort();
	//InetAddress localAddr( sockets::getLocalAddr( connectedSocket->fd() ) );
	InetAddress localAddr( acceptor_->GetListenPort() );

	// FIXME poll with zero timeout to double confirm the new connection
	// FIXME use make_shared if necessary
	UdpConnectionPtr conn( new UdpConnection( ioLoop,
		connName,
		connectedSocket,
		nextConnId_,
		localAddr,
		peerAddr ) );

	++nextConnId_;

	connections_[connName] = conn;
	conn->setConnectionCallback( connectionCallback_ );
	conn->setMessageCallback( messageCallback_ );
	conn->setWriteCompleteCallback( writeCompleteCallback_ );
	conn->setCloseCallback(
		std::bind( &UdpServer::removeConnection, this, _1 ) ); // FIXME: unsafe
	ioLoop->runInLoop( std::bind( &UdpConnection::connectEstablished, conn ) );
}

void UdpServer::removeConnection( const UdpConnectionPtr& conn )
{
	// FIXME: unsafe
	loop_->runInLoop( std::bind( &UdpServer::removeConnectionInLoop, this, conn ) );
}

void UdpServer::removeConnectionInLoop( const UdpConnectionPtr& conn )
{
	loop_->assertInLoopThread();
	//LOG_INFO << "UdpServer::removeConnectionInLoop [" << name_
		//<< "] - connection " << conn->name();

	size_t n = connections_.erase( conn->name() );
	( void )n;
	assert( n == 1 );

	acceptor_->getLoop()->queueInLoop(
		std::bind( &UdpAcceptor::RemoveConnector, get_pointer( acceptor_ ), conn->peerAddress() ) );
	conn->getLoop()->queueInLoop(
		std::bind( &UdpConnection::connectDestroyed, conn ) );
}