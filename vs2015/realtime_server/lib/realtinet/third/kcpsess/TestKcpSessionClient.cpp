#include <stdio.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <string.h>
#include <unistd.h>

#include "kcpsess.h"

#define SERVER_PORT 8888

#define SND_BUFF_LEN 1500
#define RCV_BUFF_LEN 1500

// #define SERVER_IP "172.96.239.56"
#define SERVER_IP "127.0.0.1"

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

void udp_output(const void *buf, int len, int fd, struct sockaddr* dst)
{
	::sendto(fd, buf, len, 0, dst, sizeof(*dst));
}

KcpSession::InputData kcpsessInputData;
KcpSession::InputData udp_input(char *buf, int len, int fd, struct sockaddr_in from)
{
	socklen_t fromAddrLen = sizeof(from);
	int recvLen = ::recvfrom(fd, buf, len, 0,
		(struct sockaddr*)&from, &fromAddrLen);
	//printf("recvfrom() = %d \n", static_cast<int>(recvLen));
	if (recvLen <= 0)
	{
		printf("recieve data fail!\n");
	}
	return kcpsessInputData.SetAndReturnSelf(buf, recvLen);
}



void udp_msg_sender(int fd, struct sockaddr* dst)
{
	char sndBuf[SND_BUFF_LEN];
	char rcvBuf[RCV_BUFF_LEN];

	struct sockaddr_in from;
	int len = 0;
	uint32_t index = 11;
	const uint32_t testPassIndex = 222;

	KcpSession kcpClient(
		KcpSession::RoleTypeE::kCli,
		std::bind(udp_output, std::placeholders::_1, std::placeholders::_2, fd, dst),
		std::bind(udp_input, rcvBuf, RCV_BUFF_LEN, fd, std::ref(from)),
		std::bind(iclock));

	while (1)
	{
		memset(rcvBuf, 0, RCV_BUFF_LEN);
		memset(sndBuf, 0, SND_BUFF_LEN);

		((uint32_t*)sndBuf)[0] = index++;

		//printf("client:%d\n", ((uint32_t*)sndBuf)[0]);  //打印自己发送的信息

		len = kcpClient.Send(sndBuf, SND_BUFF_LEN);
		//printf("kcpClient.Sendddddd()d\n");
		if (len < 0)
		{
			printf("kcpSession Send failed\n");
			return;
		}

		while (kcpClient.Recv(rcvBuf, len))
		{
			//int result = kcpClient.Recv(rcvBuf);
			//printf("IsKcpConnected() = %d\n", kcpClient.IsKcpConnected());
			if (len < 0)
			{
				printf("kcpSession Recv failed, Recv() = %d \n", len);
			}
			else if (len > 0)
			{
				uint32_t srvRcvMaxIndex = *(uint32_t*)(rcvBuf + 0);
				printf("server: have recieved the max index = %d\n", (int)srvRcvMaxIndex);
				if (srvRcvMaxIndex >= testPassIndex)
				{
					//printf("when server have recieved the max index >= %d, test passes, yay! \n", maxIndex);
					printf("test passes, yay! \n");
					return;
				}
			}
		}

		//printf("kcpClient.Uuuuuuuuuupdate()d\n");
		kcpClient.Update();
		usleep(16666); // 60fps
		//sleep(1);
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
	ser_addr.sin_port = htons(SERVER_PORT);

	udp_msg_sender(client_fd, (struct sockaddr*)&ser_addr);

	close(client_fd);

	return 0;
}