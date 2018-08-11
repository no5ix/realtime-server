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
	KcpSession(const OutputFunction& outputFunc,
		const CurrentTimeCallBack& currentTimeCb,
		IUINT8 conv = 0,
		StateE kcpConnectState = kConnecting)
		:
		kcpcb_(ikcp_create(conv, this)),
		outputFunc_(outputFunc),
		curTimeCb_(currentTimeCb),
		kcpConnState_(kcpConnectState)
	{
		ikcp_wndsize(kcpcb_, 128, 128);
		ikcp_nodelay(kcpcb_, 1, 10, 2, 1);
		kcpcb_->rx_minrto = 10;
		kcpcb_->fastresend = 1;
		kcpcb_->output = KcpSession::OutputFuncRaw;
	}

	~KcpSession() { ikcp_release(kcpcb_); }

	bool IsKcpConnected() const { return kcpConnState_ == kConnected; }
	bool IsKcpDisconnected() const { return kcpConnState_ == kDisconnected; }
	void SetKcpConnectState(StateE s) { kcpConnState_ = s; }

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

		outputBuf_.appendUInt8(dataType);
		outputBuf_.append(data, len);

		if (dataType == DATA_TYPE_RELIABLE && IsKcpConnected())
		{
			int result = ikcp_send(
				kcpcb_,
				static_cast<const char*>(outputBuf_.peek()),
				outputBuf_.readableBytes());
			outputBuf_.retrieveAll();

			if (result < 0)
				return result;
			else if (update)
				ikcp_update(kcpcb_, curTimeCb_());
		}
		else
		{
			outputFunc_(outputBuf_.peek(), outputBuf_.readableBytes());
			outputBuf_.retrieveAll();
		}
		return 0;
	}



private:

	int KcpRecv(char* userBuffer)
	{
		int msgLen = ikcp_peeksize(kcpcb_);
		if (msgLen <= 0)
		{
			return 0;
		}
		return ikcp_recv(kcpcb_, userBuffer, msgLen);
	}

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
	StateE kcpConnState_;
	realtime_srv::Buf outputBuf_;
	CurrentTimeCallBack curTimeCb_;
};
typedef std::shared_ptr<KcpSession> KcpSessionPtr;