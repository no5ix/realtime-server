

#ifndef UDP_MUDUO_NET_SOCKETSOPS_H
#define UDP_MUDUO_NET_SOCKETSOPS_H

#include <arpa/inet.h>

namespace muduo
{
namespace net
{
namespace sockets
{
int createUdpNonblockingOrDie( sa_family_t family );
int recvfrom( int sockfd, struct sockaddr_in6* addr );
}
}
}

#endif  // UDP_MUDUO_NET_SOCKETSOPS_H
