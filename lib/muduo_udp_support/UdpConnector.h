// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef UDP_MUDUO_NET_CONNECTOR_H
#define UDP_MUDUO_NET_CONNECTOR_H

#include <muduo/net/InetAddress.h>
#include <muduo/net/Socket.h>

#include <functional>
#include <memory>

namespace muduo
{
	namespace net
	{
		class Channel;
		class EventLoop;

		class UdpConnector : noncopyable,
			public std::enable_shared_from_this<UdpConnector>
		{
		public:
			typedef std::function<void( Socket* )> NewConnectionCallback;

			UdpConnector( EventLoop* loop, const InetAddress& serverAddr, const uint16_t localPort );
			~UdpConnector();

			void setNewConnectionCallback( const NewConnectionCallback& cb )
			{ newConnectionCallback_ = cb; }

			void start();  // can be called in any thread
			void restart();  // must be called in loop thread
			void stop();  // can be called in any thread

			const InetAddress& serverAddress() const { return peerAddr_; }

			/////////// new : for UDP
			//const Socket& GetConnectSocket() const { return connectSocket_; }

		private:
			enum States { kDisconnected, kConnecting, kConnected };
			static const int kMaxRetryDelayMs = 30 * 1000;
			static const int kInitRetryDelayMs = 500;

			void setState( States s ) { state_ = s; }
			void startInLoop();
			void stopInLoop();
			void connect();
			void connecting( int sockfd );
			void handleWrite();
			void handleError();
			void retry( int sockfd );
			int removeAndResetChannel();
			void resetChannel();

			/////////// new : for UDP
			void connected( Socket* connectedSocket );
			//Socket connectSocket_;


			EventLoop* loop_;
			InetAddress peerAddr_;
			uint16_t localPort_;
			bool connect_; // atomic
			States state_;  // FIXME: use atomic variable
			std::unique_ptr<Channel> channel_;
			NewConnectionCallback newConnectionCallback_;
			int retryDelayMs_;

		};

	}
}

#endif  // MUDUO_NET_CONNECTOR_H
