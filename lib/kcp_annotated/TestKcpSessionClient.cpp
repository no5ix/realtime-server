//=====================================================================
//
// TestKcpSessionClient.cpp - KcpSession 测试用例
//
// 说明：
// g++ TestKcpSessionClient.cpp -o ClientTestKcpSession -std=c++11
//
//=====================================================================


#include <stdio.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <string.h>
#include <unistd.h>

#include "test.h" // for iclock
#include "ikcp.c"
#include "KcpSession.h"

#define SERVER_PORT 8888

#define SND_BUFF_LEN 666
#define RCV_BUFF_LEN 1500
#define SERVER_IP "172.96.239.56"
//#define SERVER_IP "127.0.0.1"


void udp_output(const void *buf, int len, int fd, struct sockaddr* dst)
{
	sendto(fd, buf, len, 0, dst, sizeof(*dst));
}

void udp_msg_sender(int fd, struct sockaddr* dst)
{
	KcpSession kcpClient(
		KcpSession::RoleTypeE::kCli,
		std::bind(udp_output, std::placeholders::_1, std::placeholders::_2, fd, dst),
		std::bind(iclock));

	socklen_t dstAddrLen = sizeof(*dst);
	int len = 0;
	struct sockaddr_in from;
	uint32_t index = 11;
	const uint32_t maxIndex = 222;
	while (1)
	{
		char sndBuf[SND_BUFF_LEN];
		char rcvBuf[RCV_BUFF_LEN];

		((uint32_t*)sndBuf)[0] = index++;

		printf("client:%d\n", ((uint32_t*)sndBuf)[0]);  //打印自己发送的信息

		kcpClient.Update();
		len = kcpClient.Send(sndBuf, SND_BUFF_LEN);
		//len = kcpClient.Send(sndBuf, SND_BUFF_LEN, KcpSession::DataTypeE::kUnreliable);
		printf("kcpClient.Send() = %d \n", len);
		//len = ::sendto(fd, sndBuf, SND_BUFF_LEN, 0, dst, sizeof(*dst));
		if (len < 0)
		{
			printf("kcpSession Send failed\n");
			return;
		}

		len = ::recvfrom(fd, rcvBuf, RCV_BUFF_LEN, 0, (struct sockaddr*)&from, &dstAddrLen);  //接收来自server的信息
		printf(" kcpClient.IsKcpConnected() = %d\n", (kcpClient.IsKcpConnected() ? 1 : 0));
		printf("recvfrom() = %d \n", len);
		//printf("server:%s\n", rcvBuf);

		if (len > 0)
		{
			int result = kcpClient.Recv(rcvBuf, len);
			printf("kcpClient.Recv() = %d \n", result);
			if (result < 0)
			{
				printf("kcpSession Recv failed, Recv() = %d \n", result);
				return;
			}
			else if (result > 0)
			{
				uint32_t srvRcvMaxIndex = *(uint32_t*)(rcvBuf + 0);
				printf("server: have recieved the max index = %d\n", (int)srvRcvMaxIndex);  //打印server发过来的信息
				if (srvRcvMaxIndex == maxIndex) break;
			}
		}
		else if (len < 0)
		{
			printf("recieve data fail!\n");
			//return;
		}
		usleep(16666); // 60fps
	}
}

/*
		client:
						socket-->sendto-->revcfrom-->close
*/

int main(int argc, char* argv[])
{
	int client_fd;
	struct sockaddr_in ser_addr;

	client_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (client_fd < 0)
	{
		printf("create socket fail!\n");
		return -1;
	}

	// set socket non-blocking
	{
		int flags = fcntl(client_fd, F_GETFL, 0);
		fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
	}

	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	//ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //注意网络序转换
	ser_addr.sin_port = htons(SERVER_PORT);  //注意网络序转换

	udp_msg_sender(client_fd, (struct sockaddr*)&ser_addr);

	close(client_fd);

	return 0;
}