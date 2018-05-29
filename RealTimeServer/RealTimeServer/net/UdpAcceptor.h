
#ifndef UDP_MUDUO_NET_ACCEPTOR_H
#define UDP_MUDUO_NET_ACCEPTOR_H

#include <functional>
#include <map>

#include <muduo/net/Channel.h>
#include <muduo/net/Socket.h>

namespace muduo
{
	namespace net
	{

		class EventLoop;
		class InetAddress;

		class UdpConnector;
		typedef std::shared_ptr<UdpConnector> UdpConnectorPtr;

		class UdpAcceptor : noncopyable
		{
		public:
			typedef std::function<void( int sockfd, const InetAddress& )> NewConnectionCallback;

			UdpAcceptor( EventLoop* loop, const InetAddress& listenAddr, bool reuseport );
			~UdpAcceptor();

			void setNewConnectionCallback( const NewConnectionCallback& cb )
			{ newConnectionCallback_ = cb; }

			bool listenning() const { return listenning_; }
			void listen();

		private:
			void handleRead();
			void newConnection( int sockfd, const InetAddress& peerAddr );

			EventLoop* loop_;
			Socket acceptSocket_;
			Channel acceptChannel_;
			NewConnectionCallback newConnectionCallback_;
			bool listenning_;
			uint16_t listenPort_;

			typedef std::map<int, UdpConnectorPtr> UdpConnectorMap;
			UdpConnectorMap udpConnectors_;
		};

	}
}

#endif  // MUDUO_NET_ACCEPTOR_H
