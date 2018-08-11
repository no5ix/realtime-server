#pragma once

#include <functional>
#include <memory>
#include "ikcp.h"
#include "Buf.h"

typedef std::function<void(const void* data, int len)> OutputFunction;
typedef std::function<IUINT32()> CurrentTimeCallBack;

class KcpSession
{
public:
	enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
	static const IUINT8 DATA_TYPE_UNRELIABLE = 0;
	static const IUINT8 DATA_TYPE_RELIABLE = 1;

public:
	// 创建两个端点的 kcp会话对象，第一个参数设置kcp的下层输出
	// 第二个是获取当前时间的函数
	// 最后一个参数是conv是会话编号，同一个会话需要相同
	KcpSession(OutputFunction outputFunc, CurrentTimeCallBack currentTimeCb,
		IUINT32 conv = 0, StateE kcpConnectState = kConnecting)
		:
		kcpcb_(ikcp_create(conv, this)),
		outputFunc_(outputFunc),
		curTimeCb_(currentTimeCb),
		kcpConnectState_(kcpConnectState)
	{
		ikcp_wndsize(kcpcb_, 128, 128);
		ikcp_nodelay(kcpcb_, 1, 10, 2, 1);
		kcpcb_->rx_minrto = 10;
		kcpcb_->fastresend = 1;
		kcpcb_->output = KcpSession::OutputFuncRaw;
	}

	~KcpSession() { ikcp_release(kcpcb_); }

	bool IsKcpConnected() const { return kcpConnectState_ == kConnected; }
	bool IsKcpDisconnected() const { return kcpConnectState_ == kDisconnected; }
	void SetKcpConnectState(StateE s) { kcpConnectState_ = s; }

	void Update() { ikcp_update(kcpcb_, curTimeCb_()); }

	// returns below zero for error
	int Recv(char* data, size_t len, bool justInput = false)
	{
		if (len > 0)
		{
			IUINT8 dataType = *(unsigned char*)data;
			--len;

			if (dataType == DATA_TYPE_UNRELIABLE)
			{
				memcpy(data, data + 1, len);
				return len;
			}
			else if (dataType == DATA_TYPE_RELIABLE)
			{
				int result = ikcp_input(kcpcb_, data + 1, len);
				if (result == 0)
				{
					if (!IsKcpConnected())
						SetKcpConnectState(kConnected);
					return justInput ? 0 : KcpRecv(data);
				}
				else if (result < 0)
				{
					if (IsKcpConnected()) // ikcp_input error
						return result;
					else // kcp not connected
					{
						memcpy(data, data + 1, len);
						return len;
					}
				}
				else
					return -6; // impossible
			}
			else
				return -4; // DATA_TYPE err
		}
		else
			return -5; // len err
	}

	// returns below zero for error
	int Send(const void* data, int len, IUINT8 dataType = DATA_TYPE_RELIABLE, bool update = true)
	{
		output_buf_.appendInt8(dataType);
		output_buf_.append(data, len);

		if (dataType == DATA_TYPE_RELIABLE && IsKcpConnected())
		{
			int result = ikcp_send(
				kcpcb_,
				static_cast<const char*>(output_buf_.peek()),
				output_buf_.readableBytes());
			output_buf_.retrieveAll();

			if (result < 0)
				return result;
			else if (update)
				ikcp_update(kcpcb_, curTimeCb_());
		}
		else
		{
			outputFunc_(output_buf_.peek(), output_buf_.readableBytes());
			output_buf_.retrieveAll();
		}
		return 0;
	}

	int KcpRecv(char* userBuffer)
	{
		int msgLen = ikcp_peeksize(kcpcb_);
		if (msgLen <= 0)
		{
			return 0;
		}
		return ikcp_recv(kcpcb_, userBuffer, msgLen);
	}

private:

	static int OutputFuncRaw(const char* buf, int len, IKCPCB* kcp, void* user)
	{
		(void)kcp;
		auto thisPtr = reinterpret_cast<KcpSession *>(user);
		assert(thisPtr->outputFunc_ != nullptr);
		thisPtr->outputFunc_(buf, len);
		return 0;
	}

private:
	ikcpcb* kcpcb_;
	OutputFunction outputFunc_;
	StateE kcpConnectState_;
	realtime_srv::Buf output_buf_;
	CurrentTimeCallBack curTimeCb_;
};
typedef std::shared_ptr<KcpSession> KcpSessionPtr;