#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <deque>
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
//#	include <sys/param.h>

#if !defined(BYTE_ORDER) && defined(__BYTE_ORDER)
#define BYTE_ORDER __BYTE_ORDER

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN __LITTLE_ENDIAN
#endif /* LITTLE_ENDIAN */

#ifndef BIG_ENDIAN
#define BIG_ENDIAN __LITTLE_ENDIAN
#endif /* BIG_ENDIAN */
#endif /* BYTE_ORDER */

#	if BYTE_ORDER == LITTLE_ENDIAN

#		define htobe16(x) htons(x)
#		define htole16(x) (x)
#		define be16toh(x) ntohs(x)
#		define le16toh(x) (x)

#		define htobe32(x) htonl(x)
#		define htole32(x) (x)
#		define be32toh(x) ntohl(x)
#		define le32toh(x) (x)

#ifndef htonll
#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#endif

#ifndef ntohll
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

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


namespace kcpsess
{

// a light weight buf.
// thx chensuo, copy from muduo and make it safe to prepend data of any length.

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
	static const size_t kCheapPrepend = 1024;
	static const size_t kInitialSize = 512;

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

	void append(const std::string& str)
	{
		append(str.data(), str.size());
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

	void prepend(const std::string& str)
	{
		prepend(str.data(), str.size());
	}

	void prepend(const void* /*restrict*/ data, size_t len)
	{
		ensurePrependableBytes(len);
		readerIndex_ -= len;
		const char* d = static_cast<const char*>(data);
		std::copy(d, d + len, begin() + readerIndex_);
	}

	void ensurePrependableBytes(size_t len)
	{
		if (len > prependableBytes())
		{
			makeSpaceForPrepend(len);
		}
		assert(prependableBytes() >= len);
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

	void makeSpaceForPrepend(size_t len)
	{
		if (len - readerIndex_ > writableBytes())
		{
			buffer_.resize(writerIndex_ + (len - readerIndex_));
		}
		// move readable data to the end
		size_t readable = readableBytes();
		std::copy(begin() + readerIndex_,
			begin() + writerIndex_,
			begin() + len);
		readerIndex_ = len;
		writerIndex_ = readerIndex_ + readable;
		assert(readable == readableBytes());
	}

private:
	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;

	static const char kCRLF[];
};


class Fec
{
public:
	static const size_t kDataLen = sizeof(int16_t);
	static const size_t kSnLen = sizeof(int32_t);
	static const int kRedundancyCnt_ = 3;
	static const int kMaxDataLen = 1460 - kDataLen - kSnLen;
	typedef std::function<void(char*, int&, int)> RecvFuncion;
public:
	Fec(const RecvFuncion& rcvFunc)
		: count_(0), nextSndSn_(0), nextRcvSn_(0),
		index_(0), rcvFunc_(rcvFunc), isFinishedThisRound_(true)
	{}

	void Output(Buf* oBuf)
	{
		oBuf->prependInt16(static_cast<int16_t>(oBuf->readableBytes()));
		oBuf->prependInt32(nextSndSn_++);
		outputQueue_.push_back(std::string(oBuf->peek(), oBuf->readableBytes()));

		count_ = outputQueue_.size();
		for (index_ = count_ - 1; index_ >= 0; --index_)
		{
			oBuf->prepend(*(outputQueue_.begin() + index_));
		}
		if (count_ == kRedundancyCnt_)
			outputQueue_.pop_front();
	}

	bool Input(char* data, int& len, Buf* iBuf)
	{
		bool hasData = IsThereAnyDataLeft(iBuf);
		if (hasData)
		{
			isFinishedThisRound_ = false;
			int32_t recvSn = iBuf->readInt32();
			if (recvSn >= nextRcvSn_)
			{
				nextRcvSn_ = ++recvSn;
				rcvFunc_(data, len, iBuf->readInt16());
			}
			else
			{
				iBuf->retrieve(iBuf->readInt16());
				len = 0;
			}
		}
		else
		{
			iBuf->retrieveAll();
			isFinishedThisRound_ = true;
		}
		return hasData;
	}

	bool IsFinishedThisRound_() const { return isFinishedThisRound_; }

private:
	bool IsThereAnyDataLeft(const Buf* buf) const
	{
		if (buf->readableBytes() >= Fec::kSnLen)
		{
			int16_t be16 = 0;
			::memcpy(&be16, buf->peek() + Fec::kSnLen, sizeof be16);
			const uint16_t dataLen = be16toh(be16);

			if (dataLen > kMaxDataLen)
				return false;

			return buf->readableBytes() >= (dataLen + Fec::kSnLen + Fec::kDataLen);
		}
		return false;
	}

private:
	RecvFuncion rcvFunc_;
	std::deque<std::string> outputQueue_;
	int32_t nextSndSn_;
	int32_t nextRcvSn_;
	int index_;
	int count_;
	bool isFinishedThisRound_;
};

class KcpSession
{
public:
	struct InputData
	{
		InputData(char *data = nullptr, const int len = 0)
		{
			this->len_ = len;
			if (len > 0)
				this->data_ = data;
			else
				this->data_ = nullptr;
		}

		InputData SetAndReturnSelf(char *data, const int len)
		{
			this->len_ = len;
			if (len > 0)
				this->data_ = data;
			return *this;
		}

		char* data_;
		int len_;
	};

public:
	static const int kMaxSeparatePktSize =
		(1460 - (Fec::kRedundancyCnt_ * (Fec::kSnLen + Fec::kDataLen))) / Fec::kRedundancyCnt_;
	enum ConnectionStateE { kDisconnected, kConnecting, kConnected, kDisconnecting, kResetting };
	enum RoleTypeE { kSrv, kCli };
	enum TransmitModeE { kUnreliable = 88, kReliable };
	enum PktTypeE { kSyn = 66, kAck, kPsh, kFin, kRst };
	enum FecStateE { kFecEnable = 233, kFecDisable };

	typedef std::function<void(const void* data, int len)> OutputFunction;
	typedef std::function<InputData()> InputFunction;
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
		kcpConnState_(kConnecting),
		fec_(std::bind(&KcpSession::DoRecv, this, std::placeholders::_1,
			std::placeholders::_2, std::placeholders::_3)),
		sndWnd_(128),
		rcvWnd_(128),
		nodelay_(1),
		interval_(10),
		resend_(1),
		nc_(1),
		streamMode_(0),
		mtu_(300),
		rx_minrto_(10)
	{}

	void Update() { if (kcp_) ikcp_update(kcp_, curTimeCb_()); }

	// returns below zero for error
	int Send(const void* data, int len, TransmitModeE dataType = kReliable)
	{
		assert(data != nullptr);
		assert(len > 0);
		assert(dataType == kReliable || dataType == kUnreliable);
		if (dataType == kUnreliable)
		{
			outputBuf_.appendInt8(kUnreliable);
			outputBuf_.append(data, len);
			OutputAfterCheckingFec();
			if (!IsKcpConnected() && role_ == kCli)
				SendSyn();
		}
		else if (dataType == kReliable)
		{
			if (!IsKcpConnected() && role_ == kCli)
			{
				SendSyn();
				sndQueueBeforeConned_.push(std::string(static_cast<const char*>(data), len));
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

				int result = ikcp_send(kcp_, static_cast<const char*>(data), len);
				if (result < 0)
					return result; // ikcp_send err
				else
					Update();
			}
		}
		return 0;
	}

	// returns IsAnyDataLeft
	bool Recv(char* data, int& len)
	{
		if (fec_.IsFinishedThisRound_())
		{
			const InputData& rawRecvdata = inputFunc_();
			if (rawRecvdata.len_ <= 0)
			{
				len = -10;
				return false;
			}
			inputBuf_.append(rawRecvdata.data_, rawRecvdata.len_);
		}
		return fec_.Input(data, len, &inputBuf_);
	}

public:
	bool IsKcpConnected() const { return kcpConnState_ == kConnected; }

	// should set before Send()
	void SetKcpConfig(
		const int sndWnd, const int rcvWnd, const int nodelay,
		const int interval, const int resend, const int nc,
		const int streamMode, const int mtu, const int rx_minrto)
	{
		assert(mtu <= kMaxSeparatePktSize);
		sndWnd_ = sndWnd; rcvWnd_ = rcvWnd; nodelay_ = nodelay;
		interval_ = interval; resend_ = resend; nc_ = nc;
		streamMode_ = streamMode; mtu_ = mtu; rx_minrto_ = rx_minrto;
	}

	~KcpSession() { if (kcp_) ikcp_release(kcp_); }

private:
	friend void Fec::Output(Buf*);

	void DoRecv(char* data, int& len, int readableLen)
	{
		auto dataType = inputBuf_.readInt8();
		readableLen -= 1;
		if (dataType == kUnreliable)
		{
			memcpy(data, inputBuf_.peek(), readableLen);
			len = readableLen;
		}
		else if (dataType == kReliable)
		{
			auto pktType = inputBuf_.readInt8();
			readableLen -= 1;
			if (pktType == kSyn)
			{
				assert(role_ == kSrv);
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
				len = 0;
			}
			else if (pktType == kAck)
			{
				if (!IsKcpConnected())
				{
					SetKcpConnectState(kConnected);
					InitKcp(inputBuf_.readInt32());
					readableLen -= 4;
				}
				len = 0;
			}
			else if (pktType == kRst)
			{
				SetKcpConnectState(kResetting);
				assert(role_ == kCli);
				SendSyn();
				len = 0;
			}
			else if (pktType == kPsh)
			{
				if (IsKcpConnected())
				{
					int result = ikcp_input(kcp_, inputBuf_.peek(), readableLen);
					if (result == 0)
						len = KcpRecv(data); // if err, -1, -2, -3
					else // if (result < 0)
						len = result - 3; // ikcp_input err, -4, -5, -6
				}
				else  // pktType == kPsh, but kcp not connected
				{
					if (role_ == kSrv)
						SendRst();
					len = 0;
				}
			}
			else
			{
				len = -8; // pktType err
			}
		}
		else
		{
			len = -9; // dataType err
		}
		inputBuf_.retrieve(readableLen);
	}

	void SendRst()
	{
		assert(role_ == kSrv);
		outputBuf_.appendInt8(kReliable);
		outputBuf_.appendInt8(kRst);
		OutputAfterCheckingFec();
	}

	void SendSyn()
	{
		assert(role_ == kCli);
		outputBuf_.appendInt8(kReliable);
		outputBuf_.appendInt8(kSyn);
		OutputAfterCheckingFec();
	}

	void SendAckAndConv()
	{
		assert(role_ == kSrv);
		outputBuf_.appendInt8(kReliable);
		outputBuf_.appendInt8(kAck);
		outputBuf_.appendInt32(conv_);
		OutputAfterCheckingFec();
	}

	void InitKcp(const IUINT32 conv, bool reinit = false)
	{
		if (reinit) if (kcp_) ikcp_release(kcp_);
		conv_ = conv;
		kcp_ = ikcp_create(conv, this);
		ikcp_wndsize(kcp_, sndWnd_, rcvWnd_);
		ikcp_nodelay(kcp_, nodelay_, interval_, resend_, nc_);
		ikcp_setmtu(kcp_, mtu_);
		kcp_->stream = streamMode_;
		kcp_->rx_minrto = rx_minrto_;
		kcp_->output = KcpSession::KcpPshOutputFuncRaw;
	}

	IUINT32 GetNewConv()
	{
		assert(role_ == kSrv);
		static IUINT32 newConv = 666;
		return newConv++;
	}

	void SetKcpConnectState(ConnectionStateE s) { kcpConnState_ = s; }

	int KcpRecv(char* userBuffer)
	{
		assert(kcp_);
		int msgLen = ikcp_peeksize(kcp_);
		if (msgLen <= 0)
			return 0;
		return ikcp_recv(kcp_, userBuffer, msgLen);
	}

	static int KcpPshOutputFuncRaw(const char* data, int len, IKCPCB* kcp, void* user)
	{
		(void)kcp;
		auto thisPtr = reinterpret_cast<KcpSession *>(user);
		assert(thisPtr->outputFunc_ != nullptr);

		thisPtr->outputBuf_.appendInt8(kReliable);
		thisPtr->outputBuf_.appendInt8(kPsh);
		thisPtr->outputBuf_.append(data, len);
		thisPtr->OutputAfterCheckingFec();

		return 0;
	}

	void OutputAfterCheckingFec()
	{
		fec_.Output(&outputBuf_);
		outputFunc_(outputBuf_.peek(), outputBuf_.readableBytes());
		outputBuf_.retrieveAll();
	}

private:
	ikcpcb* kcp_;
	OutputFunction outputFunc_;
	InputFunction inputFunc_;
	ConnectionStateE kcpConnState_;
	Buf outputBuf_;
	Buf inputBuf_;
	CurrentTimeCallBack curTimeCb_;
	IUINT32 conv_;
	RoleTypeE role_;
	std::queue<std::string> sndQueueBeforeConned_;
	Fec fec_;
	bool isFecEnable_;

private:
	// kcp config...
	int sndWnd_;
	int rcvWnd_;
	int nodelay_;
	int interval_;
	int resend_;
	int nc_;
	int streamMode_;
	int mtu_;
	int rx_minrto_;
};
typedef std::shared_ptr<KcpSession> KcpSessionPtr;

}