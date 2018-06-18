
#ifndef UDP_MUDUO_NET_ACCEPTOR_H
#define UDP_MUDUO_NET_ACCEPTOR_H

#include <functional>
#include <unordered_set>

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
			typedef std::function<void( int sockfd, const InetAddress&, const UdpConnectorPtr& )> NewConnectionCallback;

			UdpAcceptor( EventLoop* loop, const InetAddress& listenAddr, bool reuseport );
			~UdpAcceptor();

			void setNewConnectionCallback( const NewConnectionCallback& cb )
			{ newConnectionCallback_ = cb; }

			bool listenning() const { return listenning_; }
			void listen();

			uint16_t GetListenPort() const { return listenPort_; }

			void RemoveConnector( UdpConnectorPtr udpConn );

		private:
			void handleRead();
			void newConnection( int sockfd, const InetAddress& peerAddr, const UdpConnectorPtr& UdpConnector );

			EventLoop* loop_;
			Socket acceptSocket_;
			Channel acceptChannel_;
			NewConnectionCallback newConnectionCallback_;
			bool listenning_;
			uint16_t listenPort_;

			typedef std::unordered_set< UdpConnectorPtr > UdpConnectorPtrSet;
			UdpConnectorPtrSet udpConnectors_;
		};

	}
}

#endif  // MUDUO_NET_ACCEPTOR_H
