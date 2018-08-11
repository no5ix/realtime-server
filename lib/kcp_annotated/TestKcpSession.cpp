//=====================================================================
//
// test.cpp - kcp 测试用例
//
// 说明：
// g++ TestKcpSession.cpp -o TestKcpSession -std=c++11
//
//=====================================================================

#include <stdio.h>
#include <stdlib.h>

#include "test.h"
#include "ikcp.c"
#include "KcpSession.h"

// 模拟网络
LatencySimulator *vnet;

//// 模拟网络：模拟发送一个 udp包
//int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
//{
//	union { int id; void *ptr; } parameter;
//	parameter.ptr = user;
//	vnet->send(parameter.id, buf, len);
//	return 0;
//}

// 模拟网络：模拟发送一个 udp包
int udp_output(const void *buf, int len, void *user)
{
	union { int id; void *ptr; } parameter;
	parameter.ptr = user;
	vnet->send(parameter.id, buf, len);
	return 0;
}

// 测试用例
void test()
{
	// 创建模拟网络：丢包率10%，Rtt 60ms~125ms
	vnet = new LatencySimulator(60, 60, 125);

	// 创建两个端点的 kcp对象，第一个参数 conv是会话编号，同一个会话需要相同
	// 最后一个是 user参数，用来传递标识
	//ikcpcb *kcp1 = ikcp_create(0x11223344, (void*)0);
	//ikcpcb *kcp2 = ikcp_create(0x11223344, (void*)1);



	// 创建两个端点的 kcp会话对象，第一个参数设置kcp的下层输出，这里为 udp_output，模拟udp网络输出函数
	// 第二个是获取当前时间的函数
	// 最后一个参数是conv是会话编号，同一个会话需要相同
	KcpSession kcp1(
		std::bind(udp_output, std::placeholders::_1, std::placeholders::_2, (void*)0),
		std::bind(iclock),
		0x11223344);
	KcpSession kcp2(
		std::bind(udp_output, std::placeholders::_1, std::placeholders::_2, (void*)1),
		std::bind(iclock),
		0x11223344);

	////kcp1->output = udp_output;
	//kcp2->output = udp_output;

	IUINT32 current = iclock();
	IUINT32 slap = current + 20;
	IUINT32 index = 0;
	IUINT32 next = 0;
	IINT64 sumrtt = 0;
	int count = 0;
	int maxrtt = 0;


	char buffer[2000];
	int hr;

	IUINT32 ts1 = iclock();

	while (1)
	{
		isleep(1);
		current = iclock();
		//ikcp_update(kcp1, iclock());
		//ikcp_update(kcp2, iclock());

		kcp1.Update();
		kcp2.Update();

		// 每隔 20ms，kcp1发送数据
		for (; current >= slap; slap += 20)
		{
			((IUINT32*)buffer)[0] = index++;
			((IUINT32*)buffer)[1] = current;

			// 发送上层协议包
			//ikcp_send(kcp1, buffer, 8);
			kcp1.Send(buffer, 8, KcpSession::DATA_TYPE_RELIABLE, false);
		}

		// 处理虚拟网络：检测是否有udp包从p1->p2
		while (1)
		{
			hr = vnet->recv(1, buffer, 2000);
			if (hr < 0) break;
			// 如果 p2收到udp，则作为下层协议输入到kcp2
			//ikcp_input(kcp2, buffer, hr);
			kcp2.Recv(buffer, hr, true);
			//if (hr < 0) break;
			//kcp2.Send(buffer, hr, KcpSession::DATA_TYPE_RELIABLE, false);
		}

		// 处理虚拟网络：检测是否有udp包从p2->p1
		while (1)
		{
			hr = vnet->recv(0, buffer, 2000);
			if (hr < 0) break;
			// 如果 p1收到udp，则作为下层协议输入到kcp1
			//ikcp_input(kcp1, buffer, hr);
			kcp1.Recv(buffer, hr);
		}

		// kcp2接收到任何包都返回回去
		while (1)
		{
			//hr = ikcp_recv(kcp2, buffer, 10);
			hr = kcp2.KcpRecv(buffer);
			// 没有收到包就退出
			if (hr < 0) break;
			// 如果收到包就回射
			//ikcp_send(kcp2, buffer, hr);
			kcp2.Send(buffer, hr, KcpSession::DATA_TYPE_RELIABLE, false);
		}

		// kcp1收到kcp2的回射数据
		while (1)
		{
			hr = kcp1.KcpRecv(buffer);
			// 没有收到包就退出
			if (hr < 0) break;
			IUINT32 sn = *(IUINT32*)(buffer + 0);
			IUINT32 ts = *(IUINT32*)(buffer + 4);
			IUINT32 rtt = current - ts;

			if (sn != next)
			{
				// 如果收到的包不连续
				//printf("ERROR sn %d<->%d\n", (int)count, (int)next);
				printf("ERROR sn<->next : %d<->%d\n", (int)sn, (int)next);
				//return;
			}

			next++;
			sumrtt += rtt;
			count++;
			if (rtt > (IUINT32)maxrtt) maxrtt = rtt;

			printf("[RECV] sn=%d rtt=%d\n", (int)sn, (int)rtt);
		}
		if (next > 1000) break;
	}

	ts1 = iclock() - ts1;

	const char *names[3] = { "default", "normal", "fast" };
	printf("fast mode result (%dms):\n", (int)ts1);
	printf("avgrtt=%d maxrtt=%d tx=%d\n", (int)(sumrtt / count), (int)maxrtt, (int)vnet->tx1);
	printf("press enter to next ...\n");
	char ch; scanf("%c", &ch);
}

int main()
{
	test();	// 快速模式，所有开关都打开，且关闭流控
	return 0;
}

