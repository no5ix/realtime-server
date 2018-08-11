//=====================================================================
//
// TestKcpSessionServer.cpp - KcpSession 测试用例
//
// 说明：
// g++ TestKcpSessionServer.cpp -o TestKcpSessionServer -std=c++11
//
//=====================================================================


#include <stdio.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <unistd.h>

#include <random>

#include "test.h" // for iclock
#include "ikcp.c"
#include "KcpSession.h"


#define SERVER_PORT 8888
#define BUFF_LEN 1024


void udp_output(const void *buf, int len, int fd, struct sockaddr* dst)
{
	sendto(fd, buf, len, 0, dst, sizeof(*dst));
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
		std::bind(udp_output, std::placeholders::_1, std::placeholders::_2, fd, (struct sockaddr*)&clientAddr),
		std::bind(iclock),
		0x11223344);

	char buf[BUFF_LEN];  //接收缓冲区，1024字节
	socklen_t clientAddrLen = sizeof(clientAddr);
	int len;
	IUINT32 nextSn = 0;
	bool lost = false;

	while (1)
	{
		memset(buf, 0, BUFF_LEN);

		//recvfrom是拥塞函数，没有数据就一直拥塞
		len = recvfrom(fd, buf, BUFF_LEN, 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
		if (len > 0)
		{
			// 模拟丢包
			lost = GetRandomFloat() > 0.5 ? true : false;

			if (lost)
			{
				memset(buf, 0, BUFF_LEN);
				sprintf(buf, "packet lost!!\n");  //回复client
				printf("server:%s\n", buf);  //打印自己发送的信息给
			}
			else
			{
				len = kcpServer.Recv(buf, len);
				if (len < 0)
				{
					printf("kcpSession Recv failed\n");
					return;
				}
				else if (len > 0)
				{
					IUINT32 sn = *(IUINT32*)(buf + 0);
					//if (kcpServer.IsKcpConnected() && sn != nextSn)
					{
						// 如果收到的包不连续
						printf("ERROR sn<->nextSn : %d<->%d, kcpServer.IsKcpConnected() = %d\n",
							(int)sn, (int)nextSn, (kcpSession_->IsKcpConnected() ? 1 : 0));
						//return;
					}
					++nextSn;
					printf("client:%d\n", (int)sn);  //打印client发过来的信息

					memset(buf, 0, BUFF_LEN);
					sprintf(buf, "I have recieved %d bytes data!\n", len);  //回复client
					printf("server:%s\n", buf);  //打印自己发送的信息给
				}
			}
		}
		else
		{
			printf("recieve data fail!\n");
			return;
		}

		//发送信息给client，注意使用了clientAddr结构体指针
		//sendto(fd, buf, BUFF_LEN, 0, (struct sockaddr*)&clientAddr, clientAddrLen);
		len = kcpServer.Send(buf, BUFF_LEN);
		if (len < 0)
		{
			printf("kcpSession Send failed\n");
			return;
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

	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP地址，需要进行网络序转换，INADDR_ANY：本地地址
	ser_addr.sin_port = htons(SERVER_PORT);  //端口号，需要网络序转换

	ret = bind(server_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
	if (ret < 0)
	{
		printf("socket bind fail!\n");
		return -1;
	}

	handle_udp_msg(server_fd);   //处理接收到的数据

	close(server_fd);
	return 0;
}