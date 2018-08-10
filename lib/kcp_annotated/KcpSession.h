#pragma once

#include "ikcp.h"
#include "Buf.h"
#include <functional>
#include <memory>

typedef std::function<void(const void* data, int len)> OutputFunction;
typedef std::function<IUINT32()> CurrentTimeCallBack;

class KcpSession
{
public:
	enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
	static const IUINT8 DATA_TYPE_UNRELIABLE = 0;
	static const IUINT8 DATA_TYPE_RELIABLE = 1;

public:
	KcpSession(OutputFunction outputFunc, CurrentTimeCallBack currentTimeCb, IUINT32 conv = 0)
		: kcpcb_(ikcp_create(conv, this)), outputFunc_(outputFunc), curTimeCb_(currentTimeCb)
	{
		ikcp_wndsize(kcpcb_, 128, 128);
		ikcp_nodelay(kcpcb_, 1, 10, 2, 1);
		kcpcb_->rx_minrto = 10;
		kcpcb_->fastresend = 1;
		kcpcb_->output = KcpSession::OutputFuncRaw;
	}

	~KcpSession() { ikcp_release(kcpcb_); }

	static int OutputFuncRaw(const char* buf, int len, IKCPCB* kcp, void* user)
	{
		(void)kcp;

		auto thisPtr = reinterpret_cast<KcpSession *>(user);
		assert(thisPtr->outputFunc_ != nullptr);
		thisPtr->outputFunc_(buf, len);

		return 0;
	}

	// returns below zero for error
	int Feed(char* data, size_t len)
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
			if (dataType == DATA_TYPE_RELIABLE)
			{
				int result = ikcp_input(kcpcb_, data + 1, len);
				if (result == 0)
				{
					if (!IsKcpConnected())
						SetKcpConnectState(kConnected);
					return Recv(data);
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

	int Recv(char* userBuffer)
	{
		int msgLen = ikcp_peeksize(kcpcb_);
		if (msgLen <= 0)
		{
			return 0;
		}
		return ikcp_recv(kcpcb_, userBuffer, msgLen);
	}

	// returns below zero for error
	int Send(const void* data, int len, IUINT8 dataType = DATA_TYPE_RELIABLE)
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
			else
				ikcp_update(kcpcb_, curTimeCb_());
		}
		else
		{
			outputFunc_(output_buf_.peek(), output_buf_.readableBytes());
			output_buf_.retrieveAll();
		}
		return 0;
	}

	void Update(IUINT32 currentTimestamp) { ikcp_update(kcpcb_, currentTimestamp); }

	bool IsKcpConnected() const { return kcpConnectState_ == kConnected; }
	bool IsKcpDisconnected() const { return kcpConnectState_ == kDisconnected; }
	void SetKcpConnectState(StateE s) { kcpConnectState_ = s; }

private:
	ikcpcb* kcpcb_;
	OutputFunction outputFunc_;
	StateE kcpConnectState_;
	realtime_srv::Buf output_buf_;
	CurrentTimeCallBack curTimeCb_;
};
typedef std::shared_ptr<KcpSession> KcpSessionPtr;