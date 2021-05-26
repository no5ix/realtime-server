#include <muduo/net/SocketsOps.h>
#include "UdpSocketsOps.h"



#include <muduo/base/Logging.h>
#include <muduo/base/Types.h>
#include <muduo/net/Endian.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  // snprintf
#include <strings.h>  // bzero
#include <sys/socket.h>
#include <sys/uio.h>  // readv
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

namespace
{

typedef struct sockaddr SA;


#if VALGRIND || defined (NO_ACCEPT4)
void setNonBlockAndCloseOnExec( int sockfd )
{
	// non-block
	int flags = ::fcntl( sockfd, F_GETFL, 0 );
	flags |= O_NONBLOCK;
	int ret = ::fcntl( sockfd, F_SETFL, flags );
	// FIXME check

	// close-on-exec
	flags = ::fcntl( sockfd, F_GETFD, 0 );
	flags |= FD_CLOEXEC;
	ret = ::fcntl( sockfd, F_SETFD, flags );
	// FIXME check

	( void )ret;
}
#endif

}

int sockets::createUdpNonblockingOrDie( sa_family_t family )
{
#if VALGRIND
	int sockfd = ::socket( family, SOCK_DGRAM, IPPROTO_UDP );

	if ( sockfd < 0 )
	{
		LOG_SYSFATAL << "sockets::createNonblockingOrDie";
	}

	setNonBlockAndCloseOnExec( sockfd );
#else
	int sockfd = ::socket( family, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP );
	if ( sockfd < 0 )
	{
		LOG_SYSFATAL << "sockets::createNonblockingOrDie";
	}
#endif
	return sockfd;
}



//////////
// for udp socket
/////////
int sockets::recvfrom( int sockfd, struct sockaddr_in6* addr )
{
	const uint16_t MAX_PACKET_BYTE_LENGTH = 512;

	char packetMem[MAX_PACKET_BYTE_LENGTH];
	int packetSize = sizeof( packetMem );
	socklen_t addrlen = static_cast< socklen_t >( sizeof *addr );

	int readByteCount = static_cast< int >( ::recvfrom( sockfd,
		packetMem,
		packetSize,
		0,
		sockaddr_cast( addr ),
		&addrlen ) );

	if ( readByteCount < 0 )
	{
		int savedErrno = errno;
		LOG_SYSERR << "Socket::accept";
		switch ( savedErrno )
		{
			case EAGAIN:
				// expected errors
				errno = savedErrno;
				break;
			default:
				LOG_FATAL << "unknown error of ::recvfrom " << savedErrno;
				break;
		}
	}
	return readByteCount;
}
