// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "UdpConnection.h"

#include <muduo/base/Logging.h>
#include <muduo/base/WeakCallback.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/Socket.h>
#include <muduo/net/SocketsOps.h>
#include "UdpSocketsOps.h"

#include <errno.h>

using namespace muduo;
using namespace muduo::net;
using kcpsess::KcpSession;


void muduo::net::UdpDefaultConnectionCallback(const UdpConnectionPtr& conn)
{
	LOG_TRACE << conn->localAddress().toIpPort() << " -> "
		<< conn->peerAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");
	// do not call conn->forceClose(), because some users want to register message callback only.
}

void muduo::net::UdpDefaultMessageCallback(const UdpConnectionPtr&,
	char*,
	size_t,
	Timestamp)
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
	kcpSession_(new KcpSession(
		KcpSession::RoleTypeE::kSrv,
		std::bind(&UdpConnection::DoSend, this, _1, _2),
		std::bind(&UdpConnection::DoRecv, this),
		[]() { return static_cast<IUINT32>(
		(Timestamp::now().microSecondsSinceEpoch() / 1000)); }))
{
	channel_->setReadCallback(
		std::bind(&UdpConnection::handleRead, this, _1));
	channel_->setCloseCallback(
		std::bind(&UdpConnection::handleClose, this));
	channel_->setErrorCallback(
		std::bind(&UdpConnection::handleError, this));
	LOG_DEBUG << "UdpConnection::ctor[" << name_ << "] at " << this
		<< " fd=" << socket_->fd();
}

UdpConnection::~UdpConnection()
{
	LOG_DEBUG << "UdpConnection::dtor[" << name_ << "] at " << this
		<< " fd=" << channel_->fd()
		<< " state=" << stateToString();

	LOG_INFO << localAddress().toIpPort() << " -> "
		<< peerAddress().toIpPort() << " is "
		<< (connected() ? "UP" : "DOWN");

	assert(state_ == kDisconnected);
}

void UdpConnection::send(const void* data, int len,
	//KcpSession::DataTypeE transmitMode /*= KcpSession::DataTypeE::kUnreliable*/)
	KcpSession::TransmitModeE transmitMode /*= KcpSession::DataTypeE::kReliable*/)
{
	len = kcpSession_->Send(data, len, transmitMode);
	if (len < 0)
		LOG_ERROR << "kcpSession send failed";
}

void UdpConnection::handleRead(Timestamp receiveTime)
{
	loop_->assertInLoopThread();
	int n = 0;
	while (kcpSession_->Recv(packetBuf_, n))
	{
		if (n < 0)
			LOG_ERROR << "kcpSession Recv() Error, Recv() = " << n;
		else if (n > 0)
			messageCallback_(shared_from_this(), packetBuf_, n, receiveTime);
	}
}

void UdpConnection::KcpSessionUpdate()
{
	if (loop_->isInLoopThread())
		kcpSession_->Update();
	else
		loop_->runInLoop([&]() { kcpSession_->Update(); });
}

void UdpConnection::DoSend(const void* data, int len)
{
	if (state_ == kConnected)
	{
		if (loop_->isInLoopThread())
		{
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

KcpSession::InputData UdpConnection::DoRecv()
{
	int n = sockets::read(channel_->fd(), static_cast<void*>(packetBuf_), kPacketBufSize);
	if (n == 0)
	{
		handleClose();
	}
	else if (n < 0)
	{
		LOG_SYSERR << "UdpConnection::handleRead";
		handleError();
	}
	return kcpsessInputData_.SetAndReturnSelf(packetBuf_, n);
}

void UdpConnection::sendInLoop(const void* data, size_t len)
{
	loop_->assertInLoopThread();
	ssize_t nwrote = 0;
	if (state_ == kDisconnected)
	{
		LOG_WARN << "disconnected, give up writing";
		return;
	}
	nwrote = sockets::write(channel_->fd(), data, len);
	if (nwrote >= 0)
	{
		if (len - nwrote == 0 && writeCompleteCallback_)
			loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
	}
	else // nwrote < 0
	{
		if (errno != EWOULDBLOCK)
			LOG_SYSERR << "UdpConnection::sendInLoop";
	}
}

void UdpConnection::shutdown()
{
	// FIXME: use compare and swap
	if (state_ == kConnected)
	{
		setState(kDisconnecting);
	}
}

void UdpConnection::forceClose()
{
	// FIXME: use compare and swap
	if (state_ == kConnected || state_ == kDisconnecting)
	{
		setState(kDisconnecting);
		loop_->queueInLoop(std::bind(&UdpConnection::forceCloseInLoop, shared_from_this()));
	}
}

void UdpConnection::forceCloseWithDelay(double seconds)
{
	if (state_ == kConnected || state_ == kDisconnecting)
	{
		setState(kDisconnecting);
		loop_->runAfter(
			seconds,
			makeWeakCallback(shared_from_this(),
				&UdpConnection::forceClose));  // not forceCloseInLoop to avoid race condition
	}
}

void UdpConnection::forceCloseInLoop()
{
	loop_->assertInLoopThread();
	if (state_ == kConnected || state_ == kDisconnecting)
	{
		// as if we received 0 byte in handleRead();
		handleClose();
	}
}

const char* UdpConnection::stateToString() const
{
	switch (state_)
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
	loop_->runInLoop(std::bind(&UdpConnection::startReadInLoop, this));
}

void UdpConnection::startReadInLoop()
{
	loop_->assertInLoopThread();
	if (!reading_ || !channel_->isReading())
	{
		channel_->enableReading();
		reading_ = true;
	}
}

void UdpConnection::stopRead()
{
	loop_->runInLoop(std::bind(&UdpConnection::stopReadInLoop, this));
}

void UdpConnection::stopReadInLoop()
{
	loop_->assertInLoopThread();
	if (reading_ || channel_->isReading())
	{
		channel_->disableReading();
		reading_ = false;
	}
}

void UdpConnection::connectEstablished()
{
	loop_->assertInLoopThread();
	assert(state_ == kConnecting);
	setState(kConnected);
	channel_->tie(shared_from_this());
	channel_->enableReading();

	connectionCallback_(shared_from_this());
}

void UdpConnection::connectDestroyed()
{
	loop_->assertInLoopThread();
	if (state_ == kConnected)
	{
		setState(kDisconnected);
		channel_->disableAll();

		connectionCallback_(shared_from_this());
	}
	channel_->remove();
}

void UdpConnection::handleClose()
{
	loop_->assertInLoopThread();
	LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
	assert(state_ == kConnected || state_ == kDisconnecting);
	// we don't close fd, leave it to dtor, so we can find leaks easily.
	setState(kDisconnected);
	channel_->disableAll();

	UdpConnectionPtr guardThis(shared_from_this());
	connectionCallback_(guardThis);
	// must be the last line
	closeCallback_(guardThis);
}

void UdpConnection::handleError()
{
	int err = sockets::getSocketError(channel_->fd());
	if (err == ECONNREFUSED)
	{
		LOG_INFO << peerAddr_.toIpPort() << " is disconnected";
	}
	else
	{
		LOG_ERROR << "UdpConnection::handleError [" << name_
			<< "] - SO_ERROR = " << err << " " << strerror_tl(err);
	}
	handleClose();
}

