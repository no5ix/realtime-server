// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef UDP_MUDUO_NET_CALLBACKS_H
#define UDP_MUDUO_NET_CALLBACKS_H

#include <muduo/base/Timestamp.h>

#include <functional>
#include <memory>

namespace muduo
{
	namespace net
	{

		// All client visible callbacks go here.

		class Buffer;
		class UdpConnection;
		typedef std::shared_ptr<UdpConnection> UdpConnectionPtr;
		typedef std::function<void()> UdpTimerCallback;
		typedef std::function<void( const UdpConnectionPtr& )> UdpConnectionCallback;
		typedef std::function<void( const UdpConnectionPtr& )> UdpCloseCallback;
		typedef std::function<void( const UdpConnectionPtr& )> UdpWriteCompleteCallback;
		typedef std::function<void( const UdpConnectionPtr&, size_t )> UdpHighWaterMarkCallback;

		// the data has been read to (buf, len)
		typedef std::function<void( const UdpConnectionPtr&,
			char*,
			size_t,
			Timestamp )> UdpMessageCallback;

		void UdpDefaultConnectionCallback( const UdpConnectionPtr& conn );
		void UdpDefaultMessageCallback(const UdpConnectionPtr& conn,
			char* buf,
			size_t bufBytes,
			Timestamp recvTime);
	}
}

#endif  // MUDUO_NET_CALLBACKS_H
