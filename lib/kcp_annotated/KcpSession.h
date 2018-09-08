#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <algorithm>
#include <vector>
#include <assert.h>
#include <string>
#include "ikcp.h"


// "License": Public Domain
// I, Mathias Panzenböck, place this file hereby into the public domain. Use it at your own risk for whatever you like.
// In case there are jurisdictions that don't support putting things in the public domain you can also consider it to
// be "dual licensed" under the BSD, MIT and Apache licenses, if you want to. This code is trivial anyway. Consider it
// an example on how to get the endian conversion functions on different platforms.

#ifndef PORTABLE_ENDIAN_H__
#define PORTABLE_ENDIAN_H__

#if (defined(_WIN16) || defined(_WIN32) || defined(_WIN64)) && !defined(__WINDOWS__)

#	define __WINDOWS__

#endif

#if defined(__linux__) || defined(__CYGWIN__)

#	include <endian.h>

#elif defined(__APPLE__)

#	include <libkern/OSByteOrder.h>

#	define htobe16(x) OSSwapHostToBigInt16(x)
#	define htole16(x) OSSwapHostToLittleInt16(x)
#	define be16toh(x) OSSwapBigToHostInt16(x)
#	define le16toh(x) OSSwapLittleToHostInt16(x)

#	define htobe32(x) OSSwapHostToBigInt32(x)
#	define htole32(x) OSSwapHostToLittleInt32(x)
#	define be32toh(x) OSSwapBigToHostInt32(x)
#	define le32toh(x) OSSwapLittleToHostInt32(x)

#	define htobe64(x) OSSwapHostToBigInt64(x)
#	define htole64(x) OSSwapHostToLittleInt64(x)
#	define be64toh(x) OSSwapBigToHostInt64(x)
#	define le64toh(x) OSSwapLittleToHostInt64(x)

#	define __BYTE_ORDER    BYTE_ORDER
#	define __BIG_ENDIAN    BIG_ENDIAN
#	define __LITTLE_ENDIAN LITTLE_ENDIAN
#	define __PDP_ENDIAN    PDP_ENDIAN

#elif defined(__OpenBSD__)

#	include <sys/endian.h>

#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)

#	include <sys/endian.h>

#	define be16toh(x) betoh16(x)
#	define le16toh(x) letoh16(x)

#	define be32toh(x) betoh32(x)
#	define le32toh(x) letoh32(x)

#	define be64toh(x) betoh64(x)
#	define le64toh(x) letoh64(x)

#elif defined(__WINDOWS__)

#	include <winsock2.h>
#	include <sys/param.h>

#	if BYTE_ORDER == LITTLE_ENDIAN

#		define htobe16(x) htons(x)
#		define htole16(x) (x)
#		define be16toh(x) ntohs(x)
#		define le16toh(x) (x)

#		define htobe32(x) htonl(x)
#		define htole32(x) (x)
#		define be32toh(x) ntohl(x)
#		define le32toh(x) (x)

#		define htobe64(x) htonll(x)
#		define htole64(x) (x)
#		define be64toh(x) ntohll(x)
#		define le64toh(x) (x)

#	elif BYTE_ORDER == BIG_ENDIAN

/* that would be xbox 360 */
#		define htobe16(x) (x)
#		define htole16(x) __builtin_bswap16(x)
#		define be16toh(x) (x)
#		define le16toh(x) __builtin_bswap16(x)

#		define htobe32(x) (x)
#		define htole32(x) __builtin_bswap32(x)
#		define be32toh(x) (x)
#		define le32toh(x) __builtin_bswap32(x)

#		define htobe64(x) (x)
#		define htole64(x) __builtin_bswap64(x)
#		define be64toh(x) (x)
#		define le64toh(x) __builtin_bswap64(x)

#	else

#		error byte order not supported

#	endif

#	define __BYTE_ORDER    BYTE_ORDER
#	define __BIG_ENDIAN    BIG_ENDIAN
#	define __LITTLE_ENDIAN LITTLE_ENDIAN
#	define __PDP_ENDIAN    PDP_ENDIAN

#else

#	error platform not supported

#endif

#endif



// a light weight buf.
// thx chensuo, copy from muduo.

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class Buf
{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;

	explicit Buf(size_t initialSize = kInitialSize)
		: buffer_(kCheapPrepend + initialSize),
		readerIndex_(kCheapPrepend),
		writerIndex_(kCheapPrepend)
	{
		assert(readableBytes() == 0);
		assert(writableBytes() == initialSize);
		assert(prependableBytes() == kCheapPrepend);
	}

	// implicit copy-ctor, move-ctor, dtor and assignment are fine
	// NOTE: implicit move-ctor is added in g++ 4.6

	void swap(Buf& rhs)
	{
		buffer_.swap(rhs.buffer_);
		std::swap(readerIndex_, rhs.readerIndex_);
		std::swap(writerIndex_, rhs.writerIndex_);
	}

	size_t readableBytes() const
	{ return writerIndex_ - readerIndex_; }

	size_t writableBytes() const
	{ return buffer_.size() - writerIndex_; }

	size_t prependableBytes() const
	{ return readerIndex_; }

	const char* peek() const
	{ return begin() + readerIndex_; }

	const char* findCRLF() const
	{
		// FIXME: replace with memmem()?
		const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
		return crlf == beginWrite() ? NULL : crlf;
	}

	const char* findCRLF(const char* start) const
	{
		assert(peek() <= start);
		assert(start <= beginWrite());
		// FIXME: replace with memmem()?
		const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
		return crlf == beginWrite() ? NULL : crlf;
	}

	const char* findEOL() const
	{
		const void* eol = memchr(peek(), '\n', readableBytes());
		return static_cast<const char*>(eol);
	}

	const char* findEOL(const char* start) const
	{
		assert(peek() <= start);
		assert(start <= beginWrite());
		const void* eol = memchr(start, '\n', beginWrite() - start);
		return static_cast<const char*>(eol);
	}

	// retrieve returns void, to prevent
	// std::string str(retrieve(readableBytes()), readableBytes());
	// the evaluation of two functions are unspecified
	void retrieve(size_t len)
	{
		assert(len <= readableBytes());
		if (len < readableBytes())
		{
			readerIndex_ += len;
		}
		else
		{
			retrieveAll();
		}
	}

	void retrieveUntil(const char* end)
	{
		assert(peek() <= end);
		assert(end <= beginWrite());
		retrieve(end - peek());
	}

	void retrieveInt64()
	{
		retrieve(sizeof(int64_t));
	}

	void retrieveInt32()
	{
		retrieve(sizeof(int32_t));
	}

	void retrieveInt16()
	{
		retrieve(sizeof(int16_t));
	}

	void retrieveInt8()
	{
		retrieve(sizeof(int8_t));
	}

	void retrieveAll()
	{
		readerIndex_ = kCheapPrepend;
		writerIndex_ = kCheapPrepend;
	}

	std::string retrieveAllAsString()
	{
		return retrieveAsString(readableBytes());
	}

	std::string retrieveAsString(size_t len)
	{
		assert(len <= readableBytes());
		std::string result(peek(), len);
		retrieve(len);
		return result;
	}

	void append(const char* /*restrict*/ data, size_t len)
	{
		ensureWritableBytes(len);
		std::copy(data, data + len, beginWrite());
		hasWritten(len);
	}

	void append(const void* /*restrict*/ data, size_t len)
	{
		append(static_cast<const char*>(data), len);
	}

	void ensureWritableBytes(size_t len)
	{
		if (writableBytes() < len)
		{
			makeSpace(len);
		}
		assert(writableBytes() >= len);
	}

	char* beginWrite()
	{ return begin() + writerIndex_; }

	const char* beginWrite() const
	{ return begin() + writerIndex_; }

	void hasWritten(size_t len)
	{
		assert(len <= writableBytes());
		writerIndex_ += len;
	}

	void unwrite(size_t len)
	{
		assert(len <= readableBytes());
		writerIndex_ -= len;
	}

	///
	/// Append int64_t using network endian
	///
	void appendInt64(int64_t x)
	{
		int64_t be64 = htobe64(x);
		append(&be64, sizeof be64);
	}

	///
	/// Append int32_t using network endian
	///
	void appendInt32(int32_t x)
	{
		int32_t be32 = htobe32(x);
		append(&be32, sizeof be32);
	}

	void appendInt16(int16_t x)
	{
		int16_t be16 = htobe16(x);
		append(&be16, sizeof be16);
	}

	void appendInt8(int8_t x)
	{
		append(&x, sizeof x);
	}

	///
	/// Read int64_t from network endian
	///
	/// Require: buf->readableBytes() >= sizeof(int32_t)
	int64_t readInt64()
	{
		int64_t result = peekInt64();
		retrieveInt64();
		return result;
	}

	///
	/// Read int32_t from network endian
	///
	/// Require: buf->readableBytes() >= sizeof(int32_t)
	int32_t readInt32()
	{
		int32_t result = peekInt32();
		retrieveInt32();
		return result;
	}

	int16_t readInt16()
	{
		int16_t result = peekInt16();
		retrieveInt16();
		return result;
	}

	int8_t readInt8()
	{
		int8_t result = peekInt8();
		retrieveInt8();
		return result;
	}

	///
	/// Peek int64_t from network endian
	///
	/// Require: buf->readableBytes() >= sizeof(int64_t)
	int64_t peekInt64() const
	{
		assert(readableBytes() >= sizeof(int64_t));
		int64_t be64 = 0;
		::memcpy(&be64, peek(), sizeof be64);
		return be64toh(be64);
	}

	///
	/// Peek int32_t from network endian
	///
	/// Require: buf->readableBytes() >= sizeof(int32_t)
	int32_t peekInt32() const
	{
		assert(readableBytes() >= sizeof(int32_t));
		int32_t be32 = 0;
		::memcpy(&be32, peek(), sizeof be32);
		return be32toh(be32);
	}

	int16_t peekInt16() const
	{
		assert(readableBytes() >= sizeof(int16_t));
		int16_t be16 = 0;
		::memcpy(&be16, peek(), sizeof be16);
		return be16toh(be16);
	}

	int8_t peekInt8() const
	{
		assert(readableBytes() >= sizeof(int8_t));
		int8_t x = *peek();
		return x;
	}

	///
	/// Prepend int64_t using network endian
	///
	void prependInt64(int64_t x)
	{
		int64_t be64 = htobe64(x);
		prepend(&be64, sizeof be64);
	}

	///
	/// Prepend int32_t using network endian
	///
	void prependInt32(int32_t x)
	{
		int32_t be32 = htobe32(x);
		prepend(&be32, sizeof be32);
	}

	void prependInt16(int16_t x)
	{
		int16_t be16 = htobe16(x);
		prepend(&be16, sizeof be16);
	}

	void prependInt8(int8_t x)
	{
		prepend(&x, sizeof x);
	}

	void prepend(const void* /*restrict*/ data, size_t len)
	{
		assert(len <= prependableBytes());
		readerIndex_ -= len;
		const char* d = static_cast<const char*>(data);
		std::copy(d, d + len, begin() + readerIndex_);
	}

	size_t internalCapacity() const
	{
		return buffer_.capacity();
	}

private:

	char* begin()
	{ return &*buffer_.begin(); }

	const char* begin() const
	{ return &*buffer_.begin(); }

	void makeSpace(size_t len)
	{
		if (writableBytes() + prependableBytes() < len + kCheapPrepend)
		{
			// FIXME: move readable data
			buffer_.resize(writerIndex_ + len);
		}
		else
		{
			// move readable data to the front, make space inside buffer
			assert(kCheapPrepend < readerIndex_);
			size_t readable = readableBytes();
			std::copy(begin() + readerIndex_,
				begin() + writerIndex_,
				begin() + kCheapPrepend);
			readerIndex_ = kCheapPrepend;
			writerIndex_ = readerIndex_ + readable;
			assert(readable == readableBytes());
		}
	}

private:
	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;

	static const char kCRLF[];
};



class KcpSession
{
public:
	enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting, kResetting };
	enum DataTypeE { kUnreliable = 88, kReliable };
	enum PktTypeE { kSyn = 66, kAck, kPsh, kFin, kRst };
	enum RoleTypeE { kSrv, kCli };

	typedef std::function<void(const void* data, int len)> OutputFunction;
	typedef std::function<ssize_t()> InputFunction;
	typedef std::function<IUINT32()> CurrentTimeCallBack;

public:
	KcpSession(const RoleTypeE role,
		const OutputFunction& outputFunc,
		const InputFunction& inputFunc,
		const CurrentTimeCallBack& currentTimeCb)
		:
		role_(role),
		conv_(0),
		outputFunc_(outputFunc),
		inputFunc_(inputFunc),
		curTimeCb_(currentTimeCb),
		kcp_(nullptr),
		kcpConnState_(kConnecting)
	{}

	void Update() { if (kcp_) ikcp_update(kcp_, curTimeCb_()); }

	// returns below zero for error
	int Send(const void* data, int len, DataTypeE dataType = kReliable)
	{
		outputBuf_.retrieveAll();
		assert(dataType == kReliable || dataType == kUnreliable);
		if (dataType == kUnreliable)
		{
			outputBuf_.appendInt8(kUnreliable);
			outputBuf_.append(data, len);
			outputFunc_(outputBuf_.peek(), outputBuf_.readableBytes());
			if (!IsKcpConnected() && role_ == kCli)
				SendSyn();
			return 0;
		}
		else if (dataType == kReliable)
		{
			if (!IsKcpConnected() && role_ == kCli)
			{
				SendSyn();
				sndQueueBeforeConned_.push(std::string(static_cast<const char*>(data), len));
				return 0;
			}
			else if (IsKcpConnected())
			{
				while (sndQueueBeforeConned_.size() > 0)
				{
					std::string msg = sndQueueBeforeConned_.front();
					int sendRet = ikcp_send(kcp_, msg.c_str(), msg.size());
					if (sendRet < 0)
						return sendRet; // ikcp_send err
					else
						Update();
					sndQueueBeforeConned_.pop();
				}

				int result = ikcp_send(
					kcp_,
					static_cast<const char*>(data),
					len);
				if (result < 0)
					return result; // ikcp_send err
				else
					Update();
				return 0;
			}
		}
		return 0;
	}

	// returns size, returns below zero for err
	int Recv(char* data)
	{
		ssize_t rawRecvlen = inputFunc_();
		if (rawRecvlen <= 0)
			return -10;

		inputBuf_.retrieveAll();
		inputBuf_.append(data, static_cast<size_t>(rawRecvlen));
		auto dataType = inputBuf_.readInt8();
		if (dataType == kUnreliable)
		{
			auto readableBytes = inputBuf_.readableBytes();
			memcpy(data, inputBuf_.peek(), readableBytes);
			return readableBytes;
		}
		else if (dataType == kReliable)
		{
			auto pktType = inputBuf_.readInt8();
			if (pktType == kSyn)
			{
				if (!IsKcpConnected())
				{
					SetKcpConnectState(kConnected);
					InitKcp(GetNewConv());
				}
				else
				{
					InitKcp(conv_, true);
				}
				SendAckAndConv();
				return 0;
			}
			else if (pktType == kAck)
			{
				if (!IsKcpConnected())
				{
					SetKcpConnectState(kConnected);
					InitKcp(inputBuf_.readInt32());
				}
				return 0;
			}
			else if (pktType == kRst)
			{
				SetKcpConnectState(kResetting);
				SendSyn();
				return 0;
			}
			else if (pktType == kPsh)
			{
				if (IsKcpConnected())
				{
					auto readableBytes = inputBuf_.readableBytes();
					int result = ikcp_input(kcp_, inputBuf_.peek(), readableBytes);
					if (result == 0)
						return KcpRecv(data); // if err, -1, -2, -3
					else // if (result < 0)
						return result - 3; // ikcp_input err, -4, -5, -6
				}
				else  // pktType == kPsh, but kcp not connected
				{
					SendRst();
					return 0;
				}
			}
			else
			{
				return -8; // pktType err
			}
		}
		else
		{
			return -9; // dataType err
		}
	}

public:

	bool IsKcpConnected() const { return kcpConnState_ == kConnected; }

	void SetStreamMode(bool enable) { kcp_->stream = enable ? 1 : 0; }

	int SetNoDelay(int nodelay, int interval, int resend, int nc)
	{ return ikcp_nodelay(kcp_, nodelay, interval, resend, nc); }

	int SetWndSize(int sndwnd, int rcvwnd)
	{ return ikcp_wndsize(kcp_, sndwnd, rcvwnd); }

	int SetMtu(int mtu) { return ikcp_setmtu(kcp_, mtu); }

	~KcpSession() { if (kcp_) ikcp_release(kcp_); }


private:

	void SendRst()
	{
		outputBuf_.retrieveAll();
		outputBuf_.appendInt8(kReliable);
		outputBuf_.appendInt8(kRst);
		outputFunc_(outputBuf_.peek(), outputBuf_.readableBytes());
		outputBuf_.retrieveAll();
	}

	void SendSyn()
	{
		outputBuf_.retrieveAll();
		outputBuf_.appendInt8(kReliable);
		outputBuf_.appendInt8(kSyn);
		outputFunc_(outputBuf_.peek(), outputBuf_.readableBytes());
		outputBuf_.retrieveAll();
	}

	void SendAckAndConv()
	{
		outputBuf_.retrieveAll();
		outputBuf_.appendInt8(kReliable);
		outputBuf_.appendInt8(kAck);
		outputBuf_.appendInt32(conv_);
		outputFunc_(outputBuf_.peek(), outputBuf_.readableBytes());
		outputBuf_.retrieveAll();
	}

	void InitKcp(IUINT32 conv, bool reinit = false)
	{
		if (reinit)
		{
			if (kcp_) ikcp_release(kcp_);
		}
		conv_ = conv;
		kcp_ = ikcp_create(conv, this);
		ikcp_wndsize(kcp_, 128, 128);
		ikcp_nodelay(kcp_, 1, 5, 1, 1); // 设置成1次ACK跨越直接重传, 这样反应速度会更快. 内部时钟5毫秒.
		kcp_->rx_minrto = 5;
		kcp_->output = KcpSession::KcpPshOutputFuncRaw;
	}

	IUINT32 GetNewConv()
	{
		static IUINT32 newConv = 666;
		return newConv++;
	}

	void SetKcpConnectState(StateE s) { kcpConnState_ = s; }

	int KcpRecv(char* userBuffer)
	{
		assert(kcp_);
		int msgLen = ikcp_peeksize(kcp_);
		if (msgLen <= 0)
		{
			return 0;
		}
		return ikcp_recv(kcp_, userBuffer, msgLen);
	}

	static int KcpPshOutputFuncRaw(const char* data, int len, IKCPCB* kcp, void* user)
	{
		(void)kcp;
		auto thisPtr = reinterpret_cast<KcpSession *>(user);
		assert(thisPtr->outputFunc_ != nullptr);

		thisPtr->outputBuf_.retrieveAll();
		thisPtr->outputBuf_.appendInt8(kReliable);
		thisPtr->outputBuf_.appendInt8(kPsh);
		thisPtr->outputBuf_.append(data, len);
		thisPtr->outputFunc_(thisPtr->outputBuf_.peek(), thisPtr->outputBuf_.readableBytes());
		thisPtr->outputBuf_.retrieveAll();

		return 0;
	}

private:
	ikcpcb* kcp_;
	OutputFunction outputFunc_;
	InputFunction inputFunc_;
	StateE kcpConnState_;
	Buf outputBuf_;
	Buf inputBuf_;
	CurrentTimeCallBack curTimeCb_;
	IUINT32 conv_;
	RoleTypeE role_;
	std::queue<std::string> sndQueueBeforeConned_;
};
typedef std::shared_ptr<KcpSession> KcpSessionPtr;