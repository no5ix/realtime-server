// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <net/UdpConnection.h>

#include <muduo/base/Logging.h>
#include <muduo/base/WeakCallback.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/Socket.h>
#include <muduo/net/SocketsOps.h>
#include <net/UdpSocketsOps.h>

#include <errno.h>

using namespace muduo;
using namespace muduo::net;

void muduo::net::UdpDefaultConnectionCallback( const UdpConnectionPtr& conn )
{
	LOG_TRACE << conn->localAddress().toIpPort() << " -> "
		<< conn->peerAddress().toIpPort() << " is "
		<< ( conn->connected() ? "UP" : "DOWN" );
	// do not call conn->forceClose(), because some users want to register message callback only.
}

void muduo::net::UdpDefaultMessageCallback( const UdpConnectionPtr&,
	Buffer* buf,
	Timestamp )
{
	buf->retrieveAll();
}

UdpConnection::UdpConnection( EventLoop* loop,
	const string& nameArg,
	const std::shared_ptr< Socket >& connectedSocket,
	const InetAddress& localAddr,
	const InetAddress& peerAddr )
	: 
	loop_( CHECK_NOTNULL( loop ) ),
	name_( nameArg ),
	state_( kConnecting ),
	reading_( true ),
	socket_( connectedSocket ),
	channel_( new Channel( loop, socket_->fd() ) ),
	localAddr_( localAddr ),
	peerAddr_( peerAddr ),
	highWaterMark_( 64 * 1024 * 1024 )
{
	channel_->setReadCallback(
		std::bind( &UdpConnection::handleRead, this, _1 ) );
	channel_->setWriteCallback(
		std::bind( &UdpConnection::handleWrite, this ) );
	channel_->setCloseCallback(
		std::bind( &UdpConnection::handleClose, this ) );
	channel_->setErrorCallback(
		std::bind( &UdpConnection::handleError, this ) );
	LOG_DEBUG << "UdpConnection::ctor[" << name_ << "] at " << this
		<< " fd=" << socket_->fd();
	//socket_->setKeepAlive( true );
}

UdpConnection::~UdpConnection()
{
	LOG_DEBUG << "UdpConnection::dtor[" << name_ << "] at " << this
		<< " fd=" << channel_->fd()
		<< " state=" << stateToString();
	assert( state_ == kDisconnected );
}

void UdpConnection::send( const void* data, int len )
{
	send( StringPiece( static_cast< const char* >( data ), len ) );
}

void UdpConnection::send( const StringPiece& message )
{
	if ( state_ == kConnected )
	{
		if ( loop_->isInLoopThread() )
		{
			sendInLoop( message );
		}
		else
		{
			void ( UdpConnection::*fp )( const StringPiece& message ) = &UdpConnection::sendInLoop;
			loop_->runInLoop(
				std::bind( fp,
					this,     // FIXME
					message.as_string() ) );
			//std::forward<string>(message)));
		}
	}
}

// FIXME efficiency!!!
void UdpConnection::send( Buffer* buf )
{
	if ( state_ == kConnected )
	{
		if ( loop_->isInLoopThread() )
		{
			sendInLoop( buf->peek(), buf->readableBytes() );
			buf->retrieveAll();
		}
		else
		{
			void ( UdpConnection::*fp )( const StringPiece& message ) = &UdpConnection::sendInLoop;
			loop_->runInLoop(
				std::bind( fp,
					this,     // FIXME
					buf->retrieveAllAsString() ) );
			//std::forward<string>(message)));
		}
	}
}

void UdpConnection::sendInLoop( const StringPiece& message )
{
	sendInLoop( message.data(), message.size() );
}

void UdpConnection::sendInLoop( const void* data, size_t len )
{
	loop_->assertInLoopThread();
	ssize_t nwrote = 0;
	size_t remaining = len;
	bool faultError = false;
	if ( state_ == kDisconnected )
	{
		LOG_WARN << "disconnected, give up writing";
		return;
	}
	// if no thing in output queue, try writing directly
	if ( !channel_->isWriting() && outputBuffer_.readableBytes() == 0 )
	{
		nwrote = sockets::write( channel_->fd(), data, len );
		if ( nwrote >= 0 )
		{
			remaining = len - nwrote;
			if ( remaining == 0 && writeCompleteCallback_ )
			{
				loop_->queueInLoop( std::bind( writeCompleteCallback_, shared_from_this() ) );
			}
		}
		else // nwrote < 0
		{
			nwrote = 0;
			if ( errno != EWOULDBLOCK )
			{
				LOG_SYSERR << "UdpConnection::sendInLoop";
				if ( errno == EPIPE || errno == ECONNRESET ) // FIXME: any others?
				{
					faultError = true;
				}
			}
		}
	}

	assert( remaining <= len );
	if ( !faultError && remaining > 0 )
	{
		size_t oldLen = outputBuffer_.readableBytes();
		if ( oldLen + remaining >= highWaterMark_
			&& oldLen < highWaterMark_
			&& highWaterMarkCallback_ )
		{
			loop_->queueInLoop( std::bind( highWaterMarkCallback_, shared_from_this(), oldLen + remaining ) );
		}
		outputBuffer_.append( static_cast< const char* >( data ) + nwrote, remaining );
		if ( !channel_->isWriting() )
		{
			channel_->enableWriting();
		}
	}
}

void UdpConnection::shutdown()
{
	// FIXME: use compare and swap
	if ( state_ == kConnected )
	{
		setState( kDisconnecting );
		// FIXME: shared_from_this()?
		loop_->runInLoop( std::bind( &UdpConnection::shutdownInLoop, this ) );
	}
}

void UdpConnection::shutdownInLoop()
{
	loop_->assertInLoopThread();
	if ( !channel_->isWriting() )
	{
		// we are not writing
		socket_->shutdownWrite();
	}
}

void UdpConnection::forceClose()
{
	// FIXME: use compare and swap
	if ( state_ == kConnected || state_ == kDisconnecting )
	{
		setState( kDisconnecting );
		loop_->queueInLoop( std::bind( &UdpConnection::forceCloseInLoop, shared_from_this() ) );
	}
}

void UdpConnection::forceCloseWithDelay( double seconds )
{
	if ( state_ == kConnected || state_ == kDisconnecting )
	{
		setState( kDisconnecting );
		loop_->runAfter(
			seconds,
			makeWeakCallback( shared_from_this(),
				&UdpConnection::forceClose ) );  // not forceCloseInLoop to avoid race condition
	}
}

void UdpConnection::forceCloseInLoop()
{
	loop_->assertInLoopThread();
	if ( state_ == kConnected || state_ == kDisconnecting )
	{
		// as if we received 0 byte in handleRead();
		handleClose();
	}
}

const char* UdpConnection::stateToString() const
{
	switch ( state_ )
	{
	case kDisconnected:
		return "kDisconnected";
	case kConnecting:
		return "kConnecting";
	case kConnected:
		return "kConnected";
	case kDisconnecting:
		return "kDisconnecting";
	default:
		return "unknown state";
	}
}

void UdpConnection::startRead()
{
	loop_->runInLoop( std::bind( &UdpConnection::startReadInLoop, this ) );
}

void UdpConnection::startReadInLoop()
{
	loop_->assertInLoopThread();
	if ( !reading_ || !channel_->isReading() )
	{
		channel_->enableReading();
		reading_ = true;
	}
}

void UdpConnection::stopRead()
{
	loop_->runInLoop( std::bind( &UdpConnection::stopReadInLoop, this ) );
}

void UdpConnection::stopReadInLoop()
{
	loop_->assertInLoopThread();
	if ( reading_ || channel_->isReading() )
	{
		channel_->disableReading();
		reading_ = false;
	}
}

void UdpConnection::connectEstablished()
{
	loop_->assertInLoopThread();
	assert( state_ == kConnecting );
	setState( kConnected );
	channel_->tie( shared_from_this() );
	channel_->enableReading();

	connectionCallback_( shared_from_this() );
}

void UdpConnection::connectDestroyed()
{
	loop_->assertInLoopThread();
	if ( state_ == kConnected )
	{
		setState( kDisconnected );
		channel_->disableAll();

		connectionCallback_( shared_from_this() );
	}
	channel_->remove();
}

void UdpConnection::handleRead( Timestamp receiveTime )
{
	loop_->assertInLoopThread();
	int savedErrno = 0;
	ssize_t n = inputBuffer_.readFd( channel_->fd(), &savedErrno );
	if ( n > 0 )
	{
		messageCallback_( shared_from_this(), &inputBuffer_, receiveTime );
	}
	else if ( n == 0 )
	{
		handleClose();
	}
	else
	{
		errno = savedErrno;
		LOG_SYSERR << "UdpConnection::handleRead";
		handleError();
	}
}

void UdpConnection::handleWrite()
{
	loop_->assertInLoopThread();
	if ( channel_->isWriting() )
	{
		ssize_t n = sockets::write( channel_->fd(),
			outputBuffer_.peek(),
			outputBuffer_.readableBytes() );
		if ( n > 0 )
		{
			outputBuffer_.retrieve( n );
			if ( outputBuffer_.readableBytes() == 0 )
			{
				channel_->disableWriting();
				if ( writeCompleteCallback_ )
				{
					loop_->queueInLoop( std::bind( writeCompleteCallback_, shared_from_this() ) );
				}
				if ( state_ == kDisconnecting )
				{
					shutdownInLoop();
				}
			}
		}
		else
		{
			LOG_SYSERR << "UdpConnection::handleWrite";
			// if (state_ == kDisconnecting)
			// {
			//   shutdownInLoop();
			// }
		}
	}
	else
	{
		LOG_TRACE << "Connection fd = " << channel_->fd()
			<< " is down, no more writing";
	}
}

void UdpConnection::handleClose()
{
	loop_->assertInLoopThread();
	LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
	assert( state_ == kConnected || state_ == kDisconnecting );
	// we don't close fd, leave it to dtor, so we can find leaks easily.
	setState( kDisconnected );
	channel_->disableAll();

	UdpConnectionPtr guardThis( shared_from_this() );
	connectionCallback_( guardThis );
	// must be the last line
	closeCallback_( guardThis );
}

void UdpConnection::handleError()
{
	int err = sockets::getSocketError( channel_->fd() );
	if ( err == ECONNREFUSED )
	{
		LOG_INFO << peerAddr_.toIpPort() << " is disconnected";
	}
	else
	{
		LOG_ERROR << "UdpConnection::handleError [" << name_
			<< "] - SO_ERROR = " << err << " " << strerror_tl( err );
	}
	handleClose();
}

