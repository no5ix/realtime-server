
#ifndef UDP_MUDUO_NET_CLIENT_H
#define UDP_MUDUO_NET_CLIENT_H

#include <muduo/base/Mutex.h>
#include <realtinet/UdpConnection.h>

namespace muduo
{
	namespace net
	{
		class UdpConnector;
		typedef std::shared_ptr<UdpConnector> UdpConnectorPtr;

		class UdpClient : noncopyable
		{
		public:
			// UdpClient(EventLoop* loop);
			// UdpClient(EventLoop* loop, const string& host, uint16_t port);
			UdpClient( EventLoop* loop,
				const InetAddress& serverAddr,
				const string& nameArg );
			~UdpClient();  // force out-line dtor, for std::unique_ptr members.

			void connect();
			void disconnect();
			void stop();

			UdpConnectionPtr connection() const
			{
				MutexLockGuard lock( mutex_ );
				return connection_;
			}

			EventLoop* getLoop() const { return loop_; }
			bool retry() const { return retry_; }
			void enableRetry() { retry_ = true; }

			const string& name() const
			{ return name_; }

			/// Set connection callback.
			/// Not thread safe.
			void setConnectionCallback( UdpConnectionCallback cb )
			{ connectionCallback_ = std::move( cb ); }

			/// Set message callback.
			/// Not thread safe.
			void setMessageCallback( UdpMessageCallback cb )
			{ messageCallback_ = std::move( cb ); }

			/// Set write complete callback.
			/// Not thread safe.
			void setWriteCompleteCallback( UdpWriteCompleteCallback cb )
			{ writeCompleteCallback_ = std::move( cb ); }

		private:
			/// Not thread safe, but in loop
			void newConnection( Socket* connectedSocket );
			/// Not thread safe, but in loop
			void removeConnection( const UdpConnectionPtr& conn );

			EventLoop* loop_;
			UdpConnectorPtr connector_; // avoid revealing UdpConnector
			const string name_;
			UdpConnectionCallback connectionCallback_;
			UdpMessageCallback messageCallback_;
			UdpWriteCompleteCallback writeCompleteCallback_;
			bool retry_;   // atomic
			bool connect_; // atomic
						   // always in loop thread
			int nextConnId_;
			mutable MutexLock mutex_;
			UdpConnectionPtr connection_; // @GuardedBy mutex_
		};

	}
}

#endif  // UDP_MUDUO_NET_CLIENT_H
