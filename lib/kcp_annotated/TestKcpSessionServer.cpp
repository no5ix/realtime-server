//=====================================================================
//
// TestKcpSessionServer.cpp - KcpSession 测试用例
//
// 说明：
// g++ TestKcpSessionServer.cpp -o ServerTestKcpSession -std=c++11
//
//=====================================================================


#include <stdio.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <unistd.h>

#include <random>

#include "test.h" // for iclock
#include "ikcp.c"
#include "KcpSession.h"


#define SERVER_PORT 8888

#define SND_BUFF_LEN 888
#define RCV_BUFF_LEN 1500


void udp_output(const void *buf, int len, int fd, struct sockaddr_in dst)
{
	sendto(fd, buf, len, 0, (struct sockaddr*)&dst, sizeof(*(struct sockaddr*)&dst));
}

float GetRandomFloat()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution< float > dis(0.f, 1.f);
	return dis(gen);
}

void handle_udp_msg(int fd)
{
	struct sockaddr_in clientAddr;  //clent_addr用于记录发送方的地址信息

	KcpSession kcpServer(
		std::bind(udp_output, std::placeholders::_1, std::placeholders::_2, fd,
			//clientAddr),
			std::ref(clientAddr)),
		std::bind(iclock));

	char sndBuf[SND_BUFF_LEN];
	char rcvBuf[RCV_BUFF_LEN];  //接收缓冲区，1024字节
	socklen_t clientAddrLen = sizeof(clientAddr);
	int len = 0;
	uint32_t nextSn = 11;
	bool lost = false;

	while (1)
	{
		memset(rcvBuf, 0, RCV_BUFF_LEN);
		memset(sndBuf, 0, SND_BUFF_LEN);

		//recvfrom是拥塞函数，没有数据就一直拥塞
		len = ::recvfrom(fd, rcvBuf, RCV_BUFF_LEN, 0, (struct sockaddr*)&clientAddr, &clientAddrLen);

		printf(" kcpServer.IsKcpConnected() = %d\n",(kcpServer.IsKcpConnected() ? 1 : 0));
		printf("recvfrom() = %d \n", len);
		//uint32_t sn = *(uint32_t*)(rcvBuf + 0);
		//printf("sn = %d \n", sn);

		if (len > 0)
		{
			// 模拟丢包
			lost = GetRandomFloat() > 0.8 ? true : false;
			//lost = false;
			if (lost && kcpServer.IsKcpConnected())
			{
				sprintf(sndBuf, "packet lost!!\n");  //回复client
				printf("server:%s\n", sndBuf);
			}
			else
			{
				len = kcpServer.Recv(rcvBuf, len);
				printf("kcpClient.Recv() = %d \n", len);
				if (len < 0)
				{
					printf("kcpSession Recv failed, Recv() = %d \n", len);
					return;
				}
				else if (len > 0)
				{
					uint32_t sn = *(uint32_t*)(rcvBuf + 0);
					printf("client:%d\n", (int)sn);  //打印client发过来的信息
					if (kcpServer.IsKcpConnected() && sn != nextSn)
					{
						// 如果收到的包不连续
						printf("ERROR sn<->nextSn : %d<->%d, kcpServer.IsKcpConnected() = %d\n",
							(int)sn, (int)nextSn, (kcpServer.IsKcpConnected() ? 1 : 0));
						return;
					}
					++nextSn;
				}
			}
		}
		else if (len < 0)
		{
			printf("recieve data fail!\n");
		}

		//发送信息给client，注意使用了clientAddr结构体指针
		//sprintf(sndBuf, "I have recieved %d bytes data!\n", len);  //回复client
		//len = ::sendto(fd, sndBuf, SND_BUFF_LEN, 0, (struct sockaddr*)&clientAddr, clientAddrLen);
		//printf("sendto() = %d \n", len);

		{
			printf("server:I have recieved %d bytes data!\n\n", len);  //打印自己发送的信息给

			sprintf(sndBuf, "I have recieved sn = %d!\n", nextSn - 1);  //回复client
			((uint32_t*)sndBuf)[0] = nextSn - 1;

			int result = kcpServer.Send(sndBuf, SND_BUFF_LEN);
			printf("kcpServer.Send() = %d \n", result);
			if (result < 0)
			{
				printf("kcpSession Send failed\n");
				return;
			}
		}
	}
}


/*
		server:
						socket-->bind-->recvfrom-->sendto-->close
*/

int main(int argc, char* argv[])
{
	srand(static_cast<uint32_t>(time(nullptr)));

	int server_fd, ret;
	struct sockaddr_in ser_addr;

	server_fd = socket(AF_INET, SOCK_DGRAM, 0); //AF_INET:IPV4;SOCK_DGRAM:UDP
	if (server_fd < 0)
	{
		printf("create socket fail!\n");
		return -1;
	}


	//// set socket non-blocking
	//{
	//	int flags = fcntl(server_fd, F_GETFL, 0);
	//	fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
	//}

	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP地址，需要进行网络序转换，INADDR_ANY：本地地址
	ser_addr.sin_port = htons(SERVER_PORT);  //端口号，需要网络序转换

	ret = ::bind(server_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
	if (ret < 0)
	{
		printf("socket bind fail!\n");
		return -1;
	}

	handle_udp_msg(server_fd);   //处理接收到的数据

	close(server_fd);
	return 0;
}