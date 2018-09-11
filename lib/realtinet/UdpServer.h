
#ifndef UDP_MUDUO_NET_TCPSERVER_H
#define UDP_MUDUO_NET_TCPSERVER_H

#include <muduo/base/Atomic.h>
#include <muduo/base/Types.h>
#include <realtinet/UdpConnection.h>

#include <map>

namespace muduo
{
	namespace net
	{

		class UdpAcceptor;
		class EventLoop;
		class EventLoopThreadPool;

		class UdpConnector;
		typedef std::shared_ptr<UdpConnector> UdpConnectorPtr;

		class UdpServer : noncopyable
		{
		public:
			typedef std::function<void( EventLoop* )> ThreadInitCallback;
			enum Option
			{
				kNoReusePort,
				kReusePort,
			};

			//UdpServer(EventLoop* loop, const InetAddress& listenAddr);
			UdpServer( EventLoop* loop,
				const InetAddress& listenAddr,
				const string& nameArg,
				Option option = kNoReusePort );
			~UdpServer();  // force out-line dtor, for std::unique_ptr members.

			const string& ipPort() const { return ipPort_; }
			const string& name() const { return name_; }
			EventLoop* getLoop() const { return loop_; }

			/// Set the number of threads for handling input.
			///
			/// Always accepts new connection in loop's thread.
			/// Must be called before @c start
			/// @param numThreads
			/// - 0 means all I/O in loop's thread, no thread will created.
			///   this is the default value.
			/// - 1 means all I/O in another thread.
			/// - N means a thread pool with N threads, new connections
			///   are assigned on a round-robin basis.
			void setThreadNum( int numThreads );
			void setThreadInitCallback( const ThreadInitCallback& cb )
			{ threadInitCallback_ = cb; }
			/// valid after calling start()
			std::shared_ptr<EventLoopThreadPool> threadPool()
			{ return threadPool_; }

			/// Starts the server if it's not listenning.
			///
			/// It's harmless to call it multiple times.
			/// Thread safe.
			void start();

			/// Set connection callback.
			/// Not thread safe.
			void setConnectionCallback( const UdpConnectionCallback& cb )
			{ connectionCallback_ = cb; }

			/// Set message callback.
			/// Not thread safe.
			void setMessageCallback( const UdpMessageCallback& cb )
			{ messageCallback_ = cb; }

			/// Set write complete callback.
			/// Not thread safe.
			void setWriteCompleteCallback( const UdpWriteCompleteCallback& cb )
			{ writeCompleteCallback_ = cb; }

		private:
			/// Not thread safe, but in loop
			void newConnection( Socket* connectedSocket,
				const InetAddress& peerAddr );
			/// Thread safe.
			void removeConnection( const UdpConnectionPtr& conn );
			/// Not thread safe, but in loop
			void removeConnectionInLoop( const UdpConnectionPtr& conn );

			typedef std::map<string, UdpConnectionPtr> ConnectionMap;

			EventLoop* loop_;  // the acceptor loop
			const string ipPort_;
			const string name_;
			std::unique_ptr<UdpAcceptor> acceptor_; // avoid revealing Acceptor
			std::shared_ptr<EventLoopThreadPool> threadPool_;
			UdpConnectionCallback connectionCallback_;
			UdpMessageCallback messageCallback_;
			UdpWriteCompleteCallback writeCompleteCallback_;
			ThreadInitCallback threadInitCallback_;
			AtomicInt32 started_;
			// always in loop thread
			int nextConnId_;
			ConnectionMap connections_;
		};

	}
}

#endif  // MUDUO_NET_TCPSERVER_H
