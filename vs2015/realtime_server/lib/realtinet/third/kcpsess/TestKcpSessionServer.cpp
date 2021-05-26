#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <random>
#include <string>

#include "kcpsess.h"


#define SERVER_PORT 8888

// because `int result = kcpServer.Send(sndBuf, SND_BUFF_LEN, KcpSession::DataTypeE::kUnreliable);`
// in `KcpSession::DataTypeE::kUnreliable` mode, SND_BUFF_LEN should less than `KcpSession::mtu_`
#define SND_BUFF_LEN KcpSession::kMaxSeparatePktSize
#define RCV_BUFF_LEN 1500

using kcpsess::KcpSession;


IUINT32 iclock()
{
	long s, u;
	IUINT64 value;

	struct timeval time;
	gettimeofday(&time, NULL);
	s = time.tv_sec;
	u = time.tv_usec;

	value = ((IUINT64)s) * 1000 + (u / 1000);
	return (IUINT32)(value & 0xfffffffful);
}

float GetRandomFloatFromZeroToOne()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution< float > dis(0.f, 1.f);
	return dis(gen);
}

void udp_output(const void *buf, int len, int fd, struct sockaddr_in* dst)
{
	sendto(fd, buf, len, 0, (struct sockaddr*)dst, sizeof(*(struct sockaddr*)dst));
}

const uint32_t testPassIndex = 222;
uint32_t nextRcvIndex = 11;
bool isSimulatingPackageLoss = false;
KcpSession::InputData udp_input(char* buf, int len, int fd, struct sockaddr_in* from)
{
	socklen_t fromAddrLen = sizeof(*from);
	int recvLen = ::recvfrom(fd, buf, len, 0,
		(struct sockaddr*)from, &fromAddrLen);
	//printf("recvfrom() = %d \n", static_cast<int>(recvLen));
	if (recvLen <= 0)
	{
		printf("recieve data fail!\n");
	}
	else
	{
		isSimulatingPackageLoss = 
			GetRandomFloatFromZeroToOne() > 0.8 ? true : false; // simulate package loss rate 80%
		if (isSimulatingPackageLoss && nextRcvIndex <= testPassIndex)
		{
			printf("server: simulate package loss!!\n");
			recvLen = 0;
		}
	}
	return KcpSession::InputData(buf, recvLen);
}

void handle_udp_msg(int fd)
{
	char sndBuf[SND_BUFF_LEN];
	char rcvBuf[RCV_BUFF_LEN];

	struct sockaddr_in* clientAddr = new struct sockaddr_in;  //clent_addr用于记录发送方的地址信息

	KcpSession kcpServer(
		KcpSession::RoleTypeE::kSrv,
		std::bind(udp_output, std::placeholders::_1, std::placeholders::_2, fd, clientAddr),
		std::bind(udp_input, rcvBuf, RCV_BUFF_LEN, fd, clientAddr),
		std::bind(iclock));

	int len = 0;

	while (1)
	{
		memset(rcvBuf, 0, RCV_BUFF_LEN);
		memset(sndBuf, 0, SND_BUFF_LEN);

		while (kcpServer.Recv(rcvBuf, len))
		{
			//len = kcpServer.Recv(rcvBuf);
			//printf("IsKcpConnected() = %d\n", kcpServer.IsKcpConnected());
			//printf("len = kcpServer.Recv(rcvBuf) = %d\n", len);
			if (len < 0 && !isSimulatingPackageLoss)
			{
				printf("kcpSession Recv failed, Recv() = %d \n", len);
			}
			else if (len > 0)
			{
				uint32_t index = *(uint32_t*)(rcvBuf + 0);

				if (index == testPassIndex)
				{
					printf("client:%d\n", (int)index);
					//printf("when server have recieved the max index >= %d, test passes, yay! \n", maxIndex);  //打印server发过来的信息
					printf("test passes, yay! \n");
				}
				else if (index < testPassIndex)
				{
					printf("client:%d\n", (int)index);
				}

				if (kcpServer.IsKcpConnected() && index != nextRcvIndex)
				{
					// 如果收到的包不连续
					printf("ERROR index<->nextRcvIndex : %d<->%d, kcpServer.IsKcpConnected() = %d\n",
						(int)index, (int)nextRcvIndex, (kcpServer.IsKcpConnected() ? 1 : 0));
					return;
				}
				++nextRcvIndex;

				((uint32_t*)sndBuf)[0] = nextRcvIndex - 1;
				int result = kcpServer.Send(sndBuf, SND_BUFF_LEN, KcpSession::TransmitModeE::kUnreliable);
				//printf("kcpServer.Sendddddd()d\n");
				if (result < 0)
				{
					printf("kcpSession Send failed\n");
					return;
				}
			}
		}
		//printf("kcpServer.Uuuuuuuuuupdate()d\n");
		kcpServer.Update();
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

	server_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (server_fd < 0)
	{
		printf("create socket fail!\n");
		return -1;
	}

	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ser_addr.sin_port = htons(SERVER_PORT);

	ret = ::bind(server_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
	if (ret < 0)
	{
		printf("socket bind fail!\n");
		return -1;
	}

	handle_udp_msg(server_fd);

	close(server_fd);
	return 0;
}