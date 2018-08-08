#pragma once

#include "ikcp.h"
#include <functional>
#include <memory>

typedef std::function<void(const void* data, int len)> OutputFunction;

class KcpSession
{
public:
	KcpSession(IUINT32 conv, OutputFunction outputFunc)
		: kcpcb_(ikcp_create(conv, this)), outputFunc_(outputFunc)
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

	int Input(const char* data, size_t len)
	{
		return ikcp_input(kcpcb_, data, len);
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

	int Send(const char data[], int len)
	{
		return ikcp_send(kcpcb_, data, len);
	}

	void Update(IUINT32 currentTimestamp)
	{
		ikcp_update(kcpcb_, currentTimestamp);
	}

private:
	ikcpcb* kcpcb_;
	OutputFunction outputFunc_;

};
typedef std::shared_ptr<KcpSession> KcpSessionPtr;