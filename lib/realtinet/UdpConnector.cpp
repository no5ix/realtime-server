

#include <realtinet/UdpConnector.h>

#include <muduo/base/Logging.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/SocketsOps.h>
#include <realtinet/UdpSocketsOps.h>

#include <errno.h>

using namespace muduo;
using namespace muduo::net;

const int UdpConnector::kMaxRetryDelayMs;

UdpConnector::UdpConnector( EventLoop* loop, const InetAddress& _peerAddr,
	const uint16_t localPort )
	: 
	loop_( loop ),
	peerAddr_( _peerAddr ),
	localPort_( localPort ),
	//connectSocket_( sockets::createUdpNonblockingOrDie( serverAddr.family() ) ),
	connect_( false ),
	state_( kDisconnected ),
	retryDelayMs_( kInitRetryDelayMs )
{
	//connectSocket_.setReuseAddr( true );
	//connectSocket_.setReusePort( true );
	//if ( localPort_ != 0 ) // not udp client call
	//	connectSocket_.bindAddress( InetAddress( localPort_ ) );

	LOG_DEBUG << "ctor[" << this << "]";
}

UdpConnector::~UdpConnector()
{
	LOG_DEBUG << "dtor[" << this << "]";
	assert( !channel_ );
}

void UdpConnector::start()
{
	connect_ = true;
	loop_->runInLoop( std::bind( &UdpConnector::startInLoop, this ) ); // FIXME: unsafe
}

void UdpConnector::startInLoop()
{
	loop_->assertInLoopThread();
	assert( state_ == kDisconnected );
	if ( connect_ )
	{
		connect();
	}
	else
	{
		LOG_DEBUG << "do not connect";
	}
}

void UdpConnector::stop()
{
	connect_ = false;
	loop_->queueInLoop( std::bind( &UdpConnector::stopInLoop, this ) ); // FIXME: unsafe
																 // FIXME: cancel timer
}

void UdpConnector::stopInLoop()
{
	loop_->assertInLoopThread();
	if ( state_ == kConnecting )
	{
		setState( kDisconnected );
		int sockfd = removeAndResetChannel();
		retry( sockfd );
	}
}

void UdpConnector::connect()
{
	//int sockfd = connectSocket_.fd();
	int sockfd = sockets::createUdpNonblockingOrDie( peerAddr_.family() );

	Socket* connectSocket( new Socket( sockfd ) );
	connectSocket->setReuseAddr( true );
	connectSocket->setReusePort( true );
	if ( localPort_ != 0 ) // not udp client call
		connectSocket->bindAddress( InetAddress( localPort_ ) );

	int ret = sockets::connect( sockfd, peerAddr_.getSockAddr() );
	int savedErrno = ( ret == 0 ) ? 0 : errno;
	switch ( savedErrno )
	{
	case 0:
	case EINPROGRESS:
	case EINTR:
	case EISCONN:
		//connecting( sockfd );
		connected( connectSocket );
		break;

	case EAGAIN:
	case EADDRINUSE:
	case EADDRNOTAVAIL:
	case ECONNREFUSED:
	case ENETUNREACH:
		retry( sockfd );
		break;

	case EACCES:
	case EPERM:
	case EAFNOSUPPORT:
	case EALREADY:
	case EBADF:
	case EFAULT:
	case ENOTSOCK:
		LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
		//sockets::close( sockfd );
		break;

	default:
		LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
		//sockets::close( sockfd );
		// connectErrorCallback_();
		break;
	}
}

void UdpConnector::restart()
{
	loop_->assertInLoopThread();
	setState( kDisconnected );
	retryDelayMs_ = kInitRetryDelayMs;
	connect_ = true;
	startInLoop();
}

void UdpConnector::connected( Socket* connectedSocket )
{
	/////////// new : for UDP
	setState( kConnecting );
	setState( kConnected );
	if ( connect_ )
	{
		newConnectionCallback_( connectedSocket );
	}
	else
	{
		//sockets::close( sockfd );
	}
}

void UdpConnector::connecting( int sockfd )
{
	setState( kConnecting );

	assert( !channel_ );
	channel_.reset( new Channel( loop_, sockfd ) );
	channel_->setWriteCallback(
		std::bind( &UdpConnector::handleWrite, this ) ); // FIXME: unsafe
	channel_->setErrorCallback(
		std::bind( &UdpConnector::handleError, this ) ); // FIXME: unsafe

												   // channel_->tie(shared_from_this()); is not working,
												   // as channel_ is not managed by shared_ptr
	channel_->enableWriting();
}

int UdpConnector::removeAndResetChannel()
{
	channel_->disableAll();
	channel_->remove();
	int sockfd = channel_->fd();
	// Can't reset channel_ here, because we are inside Channel::handleEvent
	loop_->queueInLoop( std::bind( &UdpConnector::resetChannel, this ) ); // FIXME: unsafe
	return sockfd;
}

void UdpConnector::resetChannel()
{
	channel_.reset();
}

void UdpConnector::handleWrite()
{
	LOG_TRACE << "Connector::handleWrite " << state_;

	if ( state_ == kConnecting )
	{
		int sockfd = removeAndResetChannel();
		int err = sockets::getSocketError( sockfd );
		if ( err )
		{
			LOG_WARN << "Connector::handleWrite - SO_ERROR = "
				<< err << " " << strerror_tl( err );
			retry( sockfd );
		}
		else if ( sockets::isSelfConnect( sockfd ) )
		{
			LOG_WARN << "Connector::handleWrite - Self connect";
			retry( sockfd );
		}
		else
		{
			setState( kConnected );
			if ( connect_ )
			{
				//newConnectionCallback_( sockfd );
			}
			else
			{
				//sockets::close( sockfd );
			}
		}
	}
	else
	{
		// what happened?
		assert( state_ == kDisconnected );
	}
}

void UdpConnector::handleError()
{
	LOG_ERROR << "Connector::handleError state=" << state_;
	if ( state_ == kConnecting )
	{
		int sockfd = removeAndResetChannel();
		int err = sockets::getSocketError( sockfd );
		LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl( err );
		retry( sockfd );
	}
}

void UdpConnector::retry( int sockfd )
{
	//sockets::close( sockfd );
	setState( kDisconnected );
	if ( connect_ )
	{
		LOG_INFO << "Connector::retry - Retry connecting to " << peerAddr_.toIpPort()
			<< " in " << retryDelayMs_ << " milliseconds. ";
		loop_->runAfter( retryDelayMs_ / 1000.0,
			std::bind( &UdpConnector::startInLoop, shared_from_this() ) );
		retryDelayMs_ = std::min( retryDelayMs_ * 2, kMaxRetryDelayMs );
	}
	else
	{
		LOG_DEBUG << "do not connect";
	}
}

