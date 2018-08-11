//=====================================================================
//
// TestKcpSessionClient.cpp - KcpSession 测试用例
//
// 说明：
// g++ TestKcpSessionClient.cpp -o TestKcpSessionClient -std=c++11
//
//=====================================================================


#include <stdio.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <unistd.h>

#include "test.h" // for iclock
#include "ikcp.c"
#include "KcpSession.h"

#define SERVER_PORT 8888
#define BUFF_LEN 512
#define SERVER_IP "172.96.239.56"
//#define SERVER_IP "127.0.0.1"


void udp_output(const void *buf, int len, int fd, struct sockaddr* dst)
{
	sendto(fd, buf, len, 0, dst, sizeof(*dst));
}

void udp_msg_sender(int fd, struct sockaddr* dst)
{
	KcpSession kcpClient(
		std::bind(udp_output, std::placeholders::_1, std::placeholders::_2, fd, dst),
		std::bind(iclock),
		0x11223344);

	socklen_t dstAddrLen = sizeof(*dst);
	int len = 0;
	struct sockaddr_in src;
	IUINT32 index = 0;

	while (1)
	{
		char buf[BUFF_LEN];
		((IUINT32*)buf)[0] = index++;

		IUINT32 sn = *(IUINT32*)(buf + 0);
		printf("client:%d\n", (int)sn);  //打印自己发送的信息

		// sendto(fd, buf, BUFF_LEN, 0, dst, dstAddrLen);
		//len = kcpClient.Send(buf, BUFF_LEN, KcpSession::DATA_TYPE_UNRELIABLE);
		len = kcpClient.Send(buf, BUFF_LEN);
		if (len < 0)
		{
			printf("kcpSession Send failed\n");
			return;
		}
		memset(buf, 0, BUFF_LEN);

		len = recvfrom(fd, buf, BUFF_LEN, 0, (struct sockaddr*)&src, &dstAddrLen);  //接收来自server的信息
		if (len > 0)
		{
			len = kcpClient.Recv(buf, len);
			if (len < 0)
			{
				printf("kcpSession Recv failed\n");
				return;
			}
		}
		else
		{
			printf("recieve data fail!\n");
			return;
		}

		printf("server:%s\n", buf);

		sleep(0.01);  //一秒发送一次消息

		if (index > 1000) break;
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

	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	//ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //注意网络序转换
	ser_addr.sin_port = htons(SERVER_PORT);  //注意网络序转换

	udp_msg_sender(client_fd, (struct sockaddr*)&ser_addr);

	close(client_fd);

	return 0;
}