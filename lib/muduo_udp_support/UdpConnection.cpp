// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo_udp_support/UdpConnection.h>

#include <muduo/base/Logging.h>
#include <muduo/base/WeakCallback.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/Socket.h>
#include <muduo/net/SocketsOps.h>
#include <muduo_udp_support/UdpSocketsOps.h>

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
	char*,
	size_t,
	Timestamp )
{}

UdpConnection::UdpConnection(EventLoop* loop,
	const string& nameArg,
	Socket* connectedSocket,
	int ConnectionId,
	const InetAddress& localAddr,
	const InetAddress& peerAddr)
	:
	loop_(CHECK_NOTNULL(loop)),
	name_(nameArg),
	connId_(ConnectionId),
	state_(kConnecting),
	reading_(true),
	socket_(connectedSocket),
	channel_(new Channel(loop, socket_->fd())),
	localAddr_(localAddr),
	peerAddr_(peerAddr),
	kcpSession_(new KcpSession(std::bind(
		(void(UdpConnection::*)(const void*, int))&UdpConnection::DoSend, this, _1, _2),
		[]() { return static_cast<IUINT32>((muduo::Timestamp::now().microSecondsSinceEpoch() / 1000) & 0xFFFFFFFFu); },
		connId_
		)),
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

	LOG_INFO << localAddress().toIpPort() << " -> "
		<< peerAddress().toIpPort() << " is "
		<< ( connected() ? "UP" : "DOWN" );

	assert( state_ == kDisconnected );
}

//void UdpConnection::send(const void* data, int len)
//{
//	if (IsKcpConnected())
//	{
//		int success = kcpSession_->Send(static_cast<const char*>(data), len);
//		if (success < 0)
//		{
//			LOG_ERROR << "ikcp_send data failed, len = " << len;
//		}
//		muduo::Timestamp now_ts = muduo::Timestamp::now();
//		uint32_t now_in_ms = (now_ts.microSecondsSinceEpoch() / 1000) & 0xFFFFFFFFu;
//		kcpSession_->Update(now_in_ms);
//	}
//	else
//	{
//		DoSend(data, len);
//	}
//}

//void UdpConnection::send(const void* data, int len,
//	IUINT8 dataType/* = KcpSession::DATA_TYPE_UNRELIABLE*/)
//{
//	len = kcpSession_->Send(data, len, dataType);
//	if (len < 0)
//	{
//		LOG_ERROR << "ikcp_send failed";
//	}
//	else if (len == 0)
//	{
//		muduo::Timestamp now_ts = muduo::Timestamp::now();
//		uint32_t now_in_ms = (now_ts.microSecondsSinceEpoch() / 1000) & 0xFFFFFFFFu;
//		//[]() { return static_cast<IUINT32>((muduo::Timestamp::now().microSecondsSinceEpoch() / 1000) & 0xFFFFFFFFu); }
//		kcpSession_->Update(now_in_ms);
//	}
//	else
//	{
//		DoSend(data, len);
//	}
//}

void UdpConnection::send(const void* data, int len,
	IUINT8 dataType/* = KcpSession::DATA_TYPE_UNRELIABLE*/)
{
	len = kcpSession_->Send(data, len, dataType);
	if (len < 0)
	{
		LOG_SYSERR << "ikcp_send failed";
	}
}

void UdpConnection::handleRead(Timestamp receiveTime)
{
	loop_->assertInLoopThread();
	ssize_t n = sockets::read(channel_->fd(), static_cast<void*>(packetBuf_), kPacketBufSize);
	if (n > 0)
	{
		n = kcpSession_->Feed(packetBuf_, n);
		if (n < 0 )
			LOG_ERROR << "kcpSession Feed() Error, Feed() = " << n;
		else if(n > 0)
			messageCallback_(shared_from_this(), packetBuf_, n, receiveTime);
	}
	else if (n == 0)
	{
		handleClose();
	}
	else
	{
		LOG_SYSERR << "UdpConnection::handleRead";
		handleError();
	}
}

void UdpConnection::DoSend( const void* data, int len )
{
	if (state_ == kConnected)
	{
		if (loop_->isInLoopThread())
		{
			//sendInLoop(message);
			sendInLoop(data, static_cast<size_t>(len));
		}
		else
		{
			void (UdpConnection::*fp)(const void* data, size_t len)
				= &UdpConnection::sendInLoop;
			loop_->runInLoop(
				std::bind(fp,
					this,     // FIXME
					data,
					static_cast<size_t>(len)));
		}
	}
}

void UdpConnection::DoSend( const StringPiece& message )
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
void UdpConnection::DoSend( Buffer* buf )
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
			if ( len - nwrote == 0 && writeCompleteCallback_ )
			{
				loop_->queueInLoop( std::bind( writeCompleteCallback_, shared_from_this() ) );
			}
		}
		else // nwrote < 0
		{
			if ( errno != EWOULDBLOCK )
				LOG_SYSERR << "UdpConnection::sendInLoop";
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

