//=====================================================================
//
// KCP - A Better ARQ Protocol Implementation
// skywind3000 (at) gmail.com, 2010-2011
//  
// Features:
// + Average RTT reduce 30% - 40% vs traditional ARQ like tcp.
// + Maximum RTT reduce three times vs tcp.
// + Lightweight, distributed as a single source file.
//
//=====================================================================
#include "ikcp.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

//=====================================================================
// KCP BASIC
//=====================================================================
const IUINT32 IKCP_RTO_NDL = 30;		// no delay min rto
const IUINT32 IKCP_RTO_MIN = 100;		// normal min rto
const IUINT32 IKCP_RTO_DEF = 200;
const IUINT32 IKCP_RTO_MAX = 60000;
const IUINT32 IKCP_CMD_PUSH = 81;		// cmd: push data
const IUINT32 IKCP_CMD_ACK  = 82;		// cmd: ack
const IUINT32 IKCP_CMD_WASK = 83;		// cmd: window probe (ask)
const IUINT32 IKCP_CMD_WINS = 84;		// cmd: window size (tell)
const IUINT32 IKCP_ASK_SEND = 1;		// need to send IKCP_CMD_WASK
const IUINT32 IKCP_ASK_TELL = 2;		// need to send IKCP_CMD_WINS
const IUINT32 IKCP_WND_SND = 32;
const IUINT32 IKCP_WND_RCV = 128;   // must >= max fragment size
const IUINT32 IKCP_MTU_DEF = 1400;
const IUINT32 IKCP_ACK_FAST	= 3;
const IUINT32 IKCP_INTERVAL	= 100;
const IUINT32 IKCP_OVERHEAD = 24;		// kcp设计了自己的包结构 IKCPSEG，包头一共24bytes
const IUINT32 IKCP_DEADLINK = 20;
const IUINT32 IKCP_THRESH_INIT = 2;
const IUINT32 IKCP_THRESH_MIN = 2;
const IUINT32 IKCP_PROBE_INIT = 7000;		// 7 secs to probe window size
const IUINT32 IKCP_PROBE_LIMIT = 120000;	// up to 120 secs to probe window


//---------------------------------------------------------------------
// encode / decode
//---------------------------------------------------------------------

/* encode 8 bits unsigned int */
static inline char *ikcp_encode8u(char *p, unsigned char c)
{
	*(unsigned char*)p++ = c;
	return p;
}

/* decode 8 bits unsigned int */
static inline const char *ikcp_decode8u(const char *p, unsigned char *c)
{
	*c = *(unsigned char*)p++;
	return p;
}

/* encode 16 bits unsigned int (lsb) */
static inline char *ikcp_encode16u(char *p, unsigned short w)
{
#if IWORDS_BIG_ENDIAN
	*(unsigned char*)(p + 0) = (w & 255);
	*(unsigned char*)(p + 1) = (w >> 8);
#else
	*(unsigned short*)(p) = w;
#endif
	p += 2;
	return p;
}

/* decode 16 bits unsigned int (lsb) */
static inline const char *ikcp_decode16u(const char *p, unsigned short *w)
{
#if IWORDS_BIG_ENDIAN
	*w = *(const unsigned char*)(p + 1);
	*w = *(const unsigned char*)(p + 0) + (*w << 8);
#else
	*w = *(const unsigned short*)p;
#endif
	p += 2;
	return p;
}

/* encode 32 bits unsigned int (lsb) */
static inline char *ikcp_encode32u(char *p, IUINT32 l)
{
#if IWORDS_BIG_ENDIAN
	*(unsigned char*)(p + 0) = (unsigned char)((l >>  0) & 0xff);
	*(unsigned char*)(p + 1) = (unsigned char)((l >>  8) & 0xff);
	*(unsigned char*)(p + 2) = (unsigned char)((l >> 16) & 0xff);
	*(unsigned char*)(p + 3) = (unsigned char)((l >> 24) & 0xff);
#else
	*(IUINT32*)p = l;
#endif
	p += 4;
	return p;
}

/* decode 32 bits unsigned int (lsb) */
static inline const char *ikcp_decode32u(const char *p, IUINT32 *l)
{
#if IWORDS_BIG_ENDIAN
	*l = *(const unsigned char*)(p + 3);
	*l = *(const unsigned char*)(p + 2) + (*l << 8);
	*l = *(const unsigned char*)(p + 1) + (*l << 8);
	*l = *(const unsigned char*)(p + 0) + (*l << 8);
#else 
	*l = *(const IUINT32*)p;
#endif
	p += 4;
	return p;
}

static inline IUINT32 _imin_(IUINT32 a, IUINT32 b) {
	return a <= b ? a : b;
}

static inline IUINT32 _imax_(IUINT32 a, IUINT32 b) {
	return a >= b ? a : b;
}

static inline IUINT32 _ibound_(IUINT32 lower, IUINT32 middle, IUINT32 upper) 
{
	return _imin_(_imax_(lower, middle), upper);
}

static inline long _itimediff(IUINT32 later, IUINT32 earlier) 
{
	return ((IINT32)(later - earlier));
}

//---------------------------------------------------------------------
// manage segment
//---------------------------------------------------------------------
typedef struct IKCPSEG IKCPSEG;

static void* (*ikcp_malloc_hook)(size_t) = NULL;
static void (*ikcp_free_hook)(void *) = NULL;

// internal malloc
static void* ikcp_malloc(size_t size) {
	if (ikcp_malloc_hook) 
		return ikcp_malloc_hook(size);
	return malloc(size);
}

// internal free
static void ikcp_free(void *ptr) {
	if (ikcp_free_hook) {
		ikcp_free_hook(ptr);
	}	else {
		free(ptr);
	}
}

// redefine allocator
void ikcp_allocator(void* (*new_malloc)(size_t), void (*new_free)(void*))
{
	ikcp_malloc_hook = new_malloc;
	ikcp_free_hook = new_free;
}

// allocate a new kcp segment
static IKCPSEG* ikcp_segment_new(ikcpcb *kcp, int size)
{
	return (IKCPSEG*)ikcp_malloc(sizeof(IKCPSEG) + size);
}

// delete a segment
static void ikcp_segment_delete(ikcpcb *kcp, IKCPSEG *seg)
{
	ikcp_free(seg);
}

// write log
void ikcp_log(ikcpcb *kcp, int mask, const char *fmt, ...)
{
	char buffer[1024];
	va_list argptr;
	if ((mask & kcp->logmask) == 0 || kcp->writelog == 0) return;
	va_start(argptr, fmt);
	vsprintf(buffer, fmt, argptr);
	va_end(argptr);
	kcp->writelog(buffer, kcp, kcp->user);
}

// check log mask
static int ikcp_canlog(const ikcpcb *kcp, int mask)
{
	if ((mask & kcp->logmask) == 0 || kcp->writelog == NULL) return 0;
	return 1;
}

// output segment
static int ikcp_output(ikcpcb *kcp, const void *data, int size)
{
	assert(kcp);
	assert(kcp->output);
	if (ikcp_canlog(kcp, IKCP_LOG_OUTPUT)) {
		ikcp_log(kcp, IKCP_LOG_OUTPUT, "[RO] %ld bytes", (long)size);
	}
	if (size == 0) return 0;
	return kcp->output((const char*)data, size, kcp, kcp->user);
}

// output queue
void ikcp_qprint(const char *name, const struct IQUEUEHEAD *head)
{
#if 0
	const struct IQUEUEHEAD *p;
	printf("<%s>: [", name);
	for (p = head->next; p != head; p = p->next) {
		const IKCPSEG *seg = iqueue_entry(p, const IKCPSEG, node);
		printf("(%lu %d)", (unsigned long)seg->sn, (int)(seg->ts % 10000));
		if (p->next != head) printf(",");
	}
	printf("]\n");
#endif
}


//---------------------------------------------------------------------
// create a new kcpcb
// 首先需要创建一个kcp用于管理接下来的工作过程，
// 在创建的时候，默认的发送、接收以及远端的窗口大小均为32，
// mtu大小为1400bytes，mss为1400-24=1376bytes，
// 超时重传时间为200毫秒，最小重传时间为100毫秒，
// kcp内部间隔最小时间为100毫秒(kcp->interval = IKCP_INTERVAL;)，
// 最大重发次数 dead_link 为IKCP_DEADLINK即20。
//---------------------------------------------------------------------
ikcpcb* ikcp_create(IUINT32 conv, void *user)
{
	ikcpcb *kcp = (ikcpcb*)ikcp_malloc(sizeof(struct IKCPCB));
	if (kcp == NULL) return NULL;
	kcp->conv = conv;
	kcp->user = user;
	kcp->snd_una = 0;
	kcp->snd_nxt = 0;
	kcp->rcv_nxt = 0;
	kcp->ts_recent = 0;
	kcp->ts_lastack = 0;
	kcp->ts_probe = 0;
	kcp->probe_wait = 0;
	kcp->snd_wnd = IKCP_WND_SND;
	kcp->rcv_wnd = IKCP_WND_RCV;
	kcp->rmt_wnd = IKCP_WND_RCV;
	kcp->cwnd = 0;
	kcp->incr = 0;
	kcp->probe = 0;
	kcp->mtu = IKCP_MTU_DEF;
	kcp->mss = kcp->mtu - IKCP_OVERHEAD;
	kcp->stream = 0;

	kcp->buffer = (char*)ikcp_malloc((kcp->mtu + IKCP_OVERHEAD) * 3);
	if (kcp->buffer == NULL) {
		ikcp_free(kcp);
		return NULL;
	}

	iqueue_init(&kcp->snd_queue);
	iqueue_init(&kcp->rcv_queue);
	iqueue_init(&kcp->snd_buf);
	iqueue_init(&kcp->rcv_buf);
	kcp->nrcv_buf = 0;
	kcp->nsnd_buf = 0;
	kcp->nrcv_que = 0;
	kcp->nsnd_que = 0;
	kcp->state = 0;
	kcp->acklist = NULL;
	kcp->ackblock = 0;
	kcp->ackcount = 0;
	kcp->rx_srtt = 0;
	kcp->rx_rttval = 0;
	kcp->rx_rto = IKCP_RTO_DEF;
	kcp->rx_minrto = IKCP_RTO_MIN;
	kcp->current = 0;
	kcp->interval = IKCP_INTERVAL;
	kcp->ts_flush = IKCP_INTERVAL;
	kcp->nodelay = 0;
	kcp->updated = 0;
	kcp->logmask = 0;
	kcp->ssthresh = IKCP_THRESH_INIT;
	kcp->fastresend = 0;
	kcp->nocwnd = 0;
	kcp->xmit = 0;
  kcp->dead_link = IKCP_DEADLINK;
	kcp->output = NULL;
	kcp->writelog = NULL;

	return kcp;
}


//---------------------------------------------------------------------
// release a new kcpcb
//---------------------------------------------------------------------
void ikcp_release(ikcpcb *kcp)
{
	assert(kcp);
	if (kcp) {
		IKCPSEG *seg;
		while (!iqueue_is_empty(&kcp->snd_buf)) {
			seg = iqueue_entry(kcp->snd_buf.next, IKCPSEG, node);
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
		}
		while (!iqueue_is_empty(&kcp->rcv_buf)) {
			seg = iqueue_entry(kcp->rcv_buf.next, IKCPSEG, node);
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
		}
		while (!iqueue_is_empty(&kcp->snd_queue)) {
			seg = iqueue_entry(kcp->snd_queue.next, IKCPSEG, node);
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
		}
		while (!iqueue_is_empty(&kcp->rcv_queue)) {
			seg = iqueue_entry(kcp->rcv_queue.next, IKCPSEG, node);
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
		}
		if (kcp->buffer) {
			ikcp_free(kcp->buffer);
		}
		if (kcp->acklist) {
			ikcp_free(kcp->acklist);
		}

		kcp->nrcv_buf = 0;
		kcp->nsnd_buf = 0;
		kcp->nrcv_que = 0;
		kcp->nsnd_que = 0;
		kcp->ackcount = 0;
		kcp->buffer = NULL;
		kcp->acklist = NULL;
		ikcp_free(kcp);
	}
}


//---------------------------------------------------------------------
// set output callback, which will be invoked by kcp
//---------------------------------------------------------------------
void ikcp_setoutput(ikcpcb *kcp, int (*output)(const char *buf, int len,
	ikcpcb *kcp, void *user))
{
	kcp->output = output;
}


//---------------------------------------------------------------------
// user/upper level recv: returns size, returns below zero for EAGAIN
// kcp_recv函数，用户获取接收到数据（去除kcp头的用户数据）。
// 该函数根据frg，把kcp包数据进行组合返回给用户。
//
// 上层调用kcp的receive函数，
// 会将rcv_queue中的数据分段整理好填入用户数据区(即 ikcp_recv 函数中的形参char *buffer)中，
// 然后删除对应的Segment，在做数据转移前会先计算一遍本次数据包的总大小，
// 只有大小合适时才会用户才会收到数据。
//
// 然后在接收缓冲区中寻找下一个需要接收的Segment，
// 如果找到则将该Segment转移到rcv_queue中等待下次用户再调用receive接收数据 。
//
// 需要注意的是，Segment在从buf转到queue中时会确保转移的Segment的sn号为下次需要接收的，
// 否则将不做转移，rcv_queue 的数据是连续的，rcv_buf 可能是间隔的
//
// 之后根据用户接收数据后的窗口变化来告诉远端进行窗口恢复。
//---------------------------------------------------------------------
int ikcp_recv(ikcpcb *kcp, char *buffer, int len)
{
	struct IQUEUEHEAD *p;
	int ispeek = (len < 0)? 1 : 0;
	int peeksize;
	int recover = 0;
	IKCPSEG *seg;
	assert(kcp);

	if (iqueue_is_empty(&kcp->rcv_queue))
		return -1;

	if (len < 0) len = -len;

	peeksize = ikcp_peeksize(kcp);

	if (peeksize < 0) 
		return -2;

	if (peeksize > len) 
		return -3;

	// 首先检测一下本次接收数据之后，是否需要进行窗口恢复。
	// 在前面的内容中解释过，KCP 协议在远端窗口为0的时候将会停止发送数据，
	// 此时如果远端调用 ikcp_recv 将数据从 rcv_queue 中移动到应用层 buffer 中之后，
	// 表明其可以再次接受数据，为了能够恢复数据的发送，
	// 远端可以主动发送 IKCP_ASK_TELL 来告知窗口大小
	if (kcp->nrcv_que >= kcp->rcv_wnd) // 判断当前是否可用窗口为0
		recover = 1; // 标记可以开始窗口恢复

	// merge fragment
	// 拷贝rcv_queue到用户buffer
	// 先将 rcv_queue 中的数据根据分片编号 frg merge 起来，
	// 然后拷贝到用户的 buffer 中。循环遍历 rcv_queue，
	// 按序拷贝数据，当碰到某个 segment 的 frg 为 0 时跳出循环，
	// 表明本次数据接收结束。这点应该很好理解，经过 ikcp_send 发送的数据会进行分片，
	// 分片编号为倒序序号，因此 frg 为 0 的数据包标记着完整接收到了一次 send 发送过来的数据；
	for (len = 0, p = kcp->rcv_queue.next; p != &kcp->rcv_queue; ) {
		int fragment;
		seg = iqueue_entry(p, IKCPSEG, node);
		p = p->next;

		if (buffer) {
			memcpy(buffer, seg->data, seg->len);
			buffer += seg->len;
		}

		len += seg->len;
		fragment = seg->frg;

		if (ikcp_canlog(kcp, IKCP_LOG_RECV)) {
			ikcp_log(kcp, IKCP_LOG_RECV, "recv sn=%lu", seg->sn);
		}

		if (ispeek == 0) {
			iqueue_del(&seg->node);
			ikcp_segment_delete(kcp, seg);
			kcp->nrcv_que--;
		}

		if (fragment == 0) 
			break;
	}

	assert(len == peeksize);

	// move available data from rcv_buf -> rcv_queue
	// 下一步将 rcv_buf 中的数据转移到 rcv_queue 中，
	// 这个过程根据报文的 sn 编号来确保转移到 rcv_queue 中的数据一定是按序的：
	while (! iqueue_is_empty(&kcp->rcv_buf)) {
		IKCPSEG *seg = iqueue_entry(kcp->rcv_buf.next, IKCPSEG, node);
		if (seg->sn == kcp->rcv_nxt && kcp->nrcv_que < kcp->rcv_wnd) {
			iqueue_del(&seg->node);
			kcp->nrcv_buf--;
			iqueue_add_tail(&seg->node, &kcp->rcv_queue);
			kcp->nrcv_que++;
			kcp->rcv_nxt++;
		}	else {
			break;
		}
	}

	// fast recover
	// 最后进行窗口恢复。此时如果 recover 标记为1，表明在此次接收之前，
	// 可用接收窗口为0，如果经过本次接收之后，可用窗口大于0，
	// 将主动发送 IKCP_ASK_TELL 数据包来通知对方已可以接收数据：
	if (kcp->nrcv_que < kcp->rcv_wnd && recover) {
		// ready to send back IKCP_CMD_WINS in ikcp_flush
		// tell remote my window size
		kcp->probe |= IKCP_ASK_TELL;
	}

	return len;
}


//---------------------------------------------------------------------
// peek data size
//---------------------------------------------------------------------
int ikcp_peeksize(const ikcpcb *kcp)
{
	struct IQUEUEHEAD *p;
	IKCPSEG *seg;
	int length = 0;

	assert(kcp);

	if (iqueue_is_empty(&kcp->rcv_queue))
		return -1;

	seg = iqueue_entry(kcp->rcv_queue.next, IKCPSEG, node);
	if (seg->frg == 0) return seg->len;

	if (kcp->nrcv_que < seg->frg + 1)
		return -1;

	for (p = kcp->rcv_queue.next; p != &kcp->rcv_queue; p = p->next) {
		seg = iqueue_entry(p, IKCPSEG, node);
		length += seg->len;
		if (seg->frg == 0) break;
	}

	return length;
}


//---------------------------------------------------------------------
// user/upper level send, returns below zero for error
//
// 该函数的功能非常简单，把用户发送的数据根据MSS进行分片。
// 用户发送1900字节的数据，MTU为1400byte。
// 因此，该函数会把1900byte的用户数据分成两个包，一个数据大小为1400，头frg设置为1，
// len设置为1400；第二个包，头frg设置为0，len设置为500。
// 切好KCP包之后，放入到名为snd_queue的待发送队列中。
// 注：
// - 流模式情况下，kcp会把两次发送的数据衔接为一个完整的kcp包。
// - 非流模式下，用户数据%MSS的包，也会作为一个包发送出去。
//
// 当设置好输出函数之后，上层应用可以调用 ikcp_send 来发送数据。
// ikcpcb 中定义了发送相关的缓冲队列和 buf，分别是 snd_queue 和 snd_buf。
// 应用层调用 ikcp_send 后，数据将会进入到 snd_queue 中，
// 而下层函数 ikcp_flush 将会决定将多少数据从 snd_queue 中移到 snd_buf 中，
// 进行发送。
//
// 我们首先来看 ikcp_send 的主要功能 :
//
// kcp发送的数据包分为2种模式，包模式和流模式。
// 
// - 在包模式下 : 
//		数据按照用户单次的send数据分界，记录Segment到send_queue中，
//		单次数据量超过mss大小将进行分片处理，
//		分片内的frg记录分片序号，从大到小，0代表本次数据的结束。
// - 在流模式下 : 
//		kcp会将用户的数据全部拼接在一起，
//		上一次send的数据Segment后如果有空间就将新数据补充进末尾，
//		剩余数据再创建新的Segment。send的过程就是将用户数据转移到Segment，
//		然后添加到发送队列中。
//
// 以mss为依据对用户数据分segment (即分片过程fragment) : 
// - 消息模式，数据分片赋予独立id，依次放入snd_queue，接收方按照id解分片数据，分片大小 <= mss
// - 流模式，检测上一个分片是否达到mss，如未达到则填充，利用率高一些
//---------------------------------------------------------------------
int ikcp_send(ikcpcb *kcp, const char *buffer, int len)
{
	IKCPSEG *seg;
	int count, i;

	assert(kcp->mss > 0);
	if (len < 0) return -1;

	// append to previous segment in streaming mode (if possible)
	// 1. 如果当前的 KCP 开启流模式，取出 `snd_queue` 中的最后一个报文(即 kcp->snd_queue.prev)
	// 将其填充到 mss 的长度，并设置其 frg 为 0.
	if (kcp->stream != 0) {
		if (!iqueue_is_empty(&kcp->snd_queue)) {
			IKCPSEG *old = iqueue_entry(kcp->snd_queue.prev, IKCPSEG, node);
			if (old->len < kcp->mss) {
				int capacity = kcp->mss - old->len;
				int extend = (len < capacity)? len : capacity;
				seg = ikcp_segment_new(kcp, old->len + extend);
				assert(seg);
				if (seg == NULL) {
					return -2;
				}
				iqueue_add_tail(&seg->node, &kcp->snd_queue);
				memcpy(seg->data, old->data, old->len);
				if (buffer) {
					memcpy(seg->data + old->len, buffer, extend);
					buffer += extend;
				}
				seg->len = old->len + extend;
				seg->frg = 0;
				len -= extend;
				iqueue_del_init(&old->node);
				ikcp_segment_delete(kcp, old);
			}
		}
		if (len <= 0) {
			return 0;
		}
	}

	// 2. 计算剩下的数据需要分成几段
	if (len <= (int)kcp->mss) count = 1;
	else count = (len + kcp->mss - 1) / kcp->mss;

	if ((IUINT32)count >= IKCP_WND_RCV) return -2;

	if (count == 0) count = 1;

	// fragment
	// 3. 为剩下的数据创建 KCP segment
	for (i = 0; i < count; i++) {
		int size = len > (int)kcp->mss ? (int)kcp->mss : len;
		seg = ikcp_segment_new(kcp, size);
		assert(seg);
		if (seg == NULL) {
			return -2;
		}
		if (buffer && len > 0) {
			memcpy(seg->data, buffer, size);
		}
		seg->len = size;
		// frg用来表示被分片的序号，从大到小递减; 流模式情况下分片编号不用填写
		seg->frg = (kcp->stream == 0)? (count - i - 1) : 0;
		iqueue_init(&seg->node);
		iqueue_add_tail(&seg->node, &kcp->snd_queue); // 加入到 snd_queue 中
		kcp->nsnd_que++;
		if (buffer) {
			buffer += size;
		}
		len -= size;
	}

	return 0;
}


//---------------------------------------------------------------------
// parse ack
//** 更新ack
//** 此处实际上是在更新rto, 
//** 因为此时收到远端的ack，所以我们知道远端的包到本机的时间，因此可统计当前的网速如何，进行调整
//---------------------------------------------------------------------
static void ikcp_update_ack(ikcpcb *kcp, IINT32 rtt)
{
	IINT32 rto = 0;
	if (kcp->rx_srtt == 0) {
		kcp->rx_srtt = rtt;
		kcp->rx_rttval = rtt / 2;
	}	else {
		long delta = rtt - kcp->rx_srtt;
		if (delta < 0) delta = -delta;
		kcp->rx_rttval = (3 * kcp->rx_rttval + delta) / 4;
		kcp->rx_srtt = (7 * kcp->rx_srtt + rtt) / 8;
		if (kcp->rx_srtt < 1) kcp->rx_srtt = 1;
	}
	rto = kcp->rx_srtt + _imax_(kcp->interval, 4 * kcp->rx_rttval);
	kcp->rx_rto = _ibound_(kcp->rx_minrto, rto, IKCP_RTO_MAX);
}

//** 更新本地 snd_una 数据，如snd_buf为空，snd_una 指向 snd_nxt，否则指向 snd_buf 首端
static void ikcp_shrink_buf(ikcpcb *kcp)
{
	struct IQUEUEHEAD *p = kcp->snd_buf.next;
	if (p != &kcp->snd_buf) { // 若snd_buff不为空
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		kcp->snd_una = seg->sn; // snd_una指向snd_buf首端
	}	else { // 如snd_buf为空，snd_una指向snd_nxt
		kcp->snd_una = kcp->snd_nxt;
	}
}

//** 分析具体是哪个segment被收到了，将其从snd_buf中移除
static void ikcp_parse_ack(ikcpcb *kcp, IUINT32 sn)
{
	struct IQUEUEHEAD *p, *next;

	// sn小于snd_una或大于等于snd_nxt，忽略该包，snd_una之前是完备的，snd_nxt之后未发送，不应收到ack
	if (_itimediff(sn, kcp->snd_una) < 0 || _itimediff(sn, kcp->snd_nxt) >= 0)
		return;

	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = next) {
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		next = p->next;
		if (sn == seg->sn) {
			iqueue_del(p);
			ikcp_segment_delete(kcp, seg);
			kcp->nsnd_buf--;
			break;
		}
		if (_itimediff(sn, seg->sn) < 0) {
			break;
		}
	}
}

// 分析una，看哪些segment远端收到了，删除send_buf中小于una的segment
static void ikcp_parse_una(ikcpcb *kcp, IUINT32 una)
{
	struct IQUEUEHEAD *p, *next;
	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = next) {

		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);

		next = p->next;
		if (_itimediff(una, seg->sn) > 0) {
			iqueue_del(p);
			ikcp_segment_delete(kcp, seg);
			kcp->nsnd_buf--;
		}	else {
			break;
		}
	}
}

// 根据遍历snd_buf队列更新各个Segment中ack跳过的次数，
// 也就是说, 若Segment的sn小于接收到的ack包的sn, 则Segment的fastack ++，
// 用于之后判断是否需要快速重传, 
// 若fastack超过指定阈值，则启动快速重传
static void ikcp_parse_fastack(ikcpcb *kcp, IUINT32 sn)
{
	struct IQUEUEHEAD *p, *next;

	if (_itimediff(sn, kcp->snd_una) < 0 || _itimediff(sn, kcp->snd_nxt) >= 0)
		return;

	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = next) {
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		next = p->next;
		if (_itimediff(sn, seg->sn) < 0) {
			break;
		}
		else if (sn != seg->sn) { // 若seg的sn小于接收到的所有ack包中的最大sn
			seg->fastack++;
		}
	}
}


//---------------------------------------------------------------------
// ack append
//** push当前包的ack给远端（会在flush中发送ack出去)
// 调用 ikcp_ack_push 将对该报文的确认 ACK 报文放入 ACK 列表acklist中
//---------------------------------------------------------------------
static void ikcp_ack_push(ikcpcb *kcp, IUINT32 sn, IUINT32 ts)
{
	size_t newsize = kcp->ackcount + 1;
	IUINT32 *ptr;

	if (newsize > kcp->ackblock) {
		IUINT32 *acklist;
		size_t newblock;

		for (newblock = 8; newblock < newsize; newblock <<= 1); // newblock <<= 1 等价于 newblock *= 2;
		acklist = (IUINT32*)ikcp_malloc(newblock * sizeof(IUINT32) * 2);

		if (acklist == NULL) {
			assert(acklist != NULL);
			abort();
		}

		if (kcp->acklist != NULL) {
			size_t x;
			for (x = 0; x < kcp->ackcount; x++) {
				acklist[x * 2 + 0] = kcp->acklist[x * 2 + 0];
				acklist[x * 2 + 1] = kcp->acklist[x * 2 + 1];
			}
			ikcp_free(kcp->acklist);
		}

		kcp->acklist = acklist;
		kcp->ackblock = newblock;
	}

	ptr = &kcp->acklist[kcp->ackcount * 2];
	ptr[0] = sn;
	ptr[1] = ts;
	kcp->ackcount++;
}

static void ikcp_ack_get(const ikcpcb *kcp, int p, IUINT32 *sn, IUINT32 *ts)
{
	if (sn) sn[0] = kcp->acklist[p * 2 + 0];
	if (ts) ts[0] = kcp->acklist[p * 2 + 1];
}


//---------------------------------------------------------------------
// parse data
// 首先会在rcv_buf中遍历一次，判断是否已经接收过这个数据包，
// 如果数据包不存在则添加到rcv_buf中，之后将可用的Segment再转移到rcv_queue中
//---------------------------------------------------------------------
void ikcp_parse_data(ikcpcb *kcp, IKCPSEG *newseg)
{
	struct IQUEUEHEAD *p, *prev;
	IUINT32 sn = newseg->sn;
	int repeat = 0;
	
	// 超出接收窗口大小了 或 rcv_queue已经接收过这个sn的数据包了
	if (_itimediff(sn, kcp->rcv_nxt + kcp->rcv_wnd) >= 0 || _itimediff(sn, kcp->rcv_nxt) < 0) {
		ikcp_segment_delete(kcp, newseg);
		return;
	}

	// rcv_buf 从后往前遍历，判断是否已经接收过这个数据包, 并且找到新数据newseg应该插入到 rcv_buf 的位置
	for (p = kcp->rcv_buf.prev; p != &kcp->rcv_buf; p = prev) {
		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node);
		prev = p->prev;

		// 检测是否为重复数据包
		if (seg->sn == sn) {
			repeat = 1;
			break;
		}
		if (_itimediff(sn, seg->sn) > 0) {
			break;
		}
	}

	if (repeat == 0) {
		iqueue_init(&newseg->node);
		iqueue_add(&newseg->node, p); // 新数据newseg插入到p的后面
		kcp->nrcv_buf++;
	}	else {
		// 如果已经接收过了，则丢弃
		ikcp_segment_delete(kcp, newseg);
	}

#if 0
	ikcp_qprint("rcvbuf", &kcp->rcv_buf);
	printf("rcv_nxt=%lu\n", kcp->rcv_nxt);
#endif

	// move available data from rcv_buf to rcv_queue
	// 扫描rcv_buf，segment的id等于rcv_nxt，则rcv_nxt右移，
	// 同时segment移出rcv_buf移入rcv_queue，rcv_nxt的连续性保证rcv_queue的完备性
	while (! iqueue_is_empty(&kcp->rcv_buf)) {
		IKCPSEG *seg = iqueue_entry(kcp->rcv_buf.next, IKCPSEG, node);
		if (seg->sn == kcp->rcv_nxt && kcp->nrcv_que < kcp->rcv_wnd) {
			iqueue_del(&seg->node);
			kcp->nrcv_buf--;
			iqueue_add_tail(&seg->node, &kcp->rcv_queue);
			kcp->nrcv_que++;
			kcp->rcv_nxt++;
		}	else {
			break;
		}
	}

#if 0
	ikcp_qprint("queue", &kcp->rcv_queue);
	printf("rcv_nxt=%lu\n", kcp->rcv_nxt);
#endif

#if 1
//	printf("snd(buf=%d, queue=%d)\n", kcp->nsnd_buf, kcp->nsnd_que);
//	printf("rcv(buf=%d, queue=%d)\n", kcp->nrcv_buf, kcp->nrcv_que);
#endif
}


//---------------------------------------------------------------------
// ikcp_input负责接收用户传入的底层网络数据(比如udp协议传过来的报文)，
// 然后把底层网络数据解码成kcp报文进行缓存。
// kcp不负责网络端数据的接收，
// 需要用户自己调用相关的网络操作函数进行数据包的接收，将接收到数据通过input传入kcp中。
//
// 相关联的成员变量有以下几个：
// - UInt32 rcv_nxt 下一个要接收的数据包的编号。也就是说此序号之前的包都已经按顺序全部收到了，
//			下面期望收到这个序号的包（已保证数据包的连续性、顺序性）
// - UInt32 rcv_wnd 接收窗口的大小
// - Segment[] rcv_buf 接收到的数据会先存放到rcv_buf中。
//			因为数据可能是乱序到达本地的，所以接受到的数据会按sn顺序依次放入到对应的位置中。 
//			当sn从低到高连续的数据包都收到了，则将这批连续的数据包转移到 rcv_queue 中。
//			这样就保证了数据包的顺序性。
// - Segment[] rcv_queue 缓存接收到、连续的数据包
// - UInt32[] acklist 收到包后要发送的回传确认。 
//			在收到包时先将要回传ack的sn放入此队列中，在 ikcp_flush 函数中再发出去。 
//			acklist中，一个ack以(sn, timestampe)为一组的方式存储。
//			即[{sn1, ts1}, { sn2,ts2 } …] 即[sn1, ts1, sn2, ts2 …]
//
//	对于用户传入的数据，kcp会先对数据头部进行解包，判断数据包的大小、会话序号等信息，
//	同时更新远端窗口大小。通过调用 parse_una 来确认远端收到的数据包，
//	将接收到的数据包从 snd_buf 中移除。然后调用shrink_buf来更新kcp中snd_una信息，
//	用于告诉远端自己已经确认被接收的数据包信息。
//
//	之后根据不同的数据包cmd类型分别处理对应的数据包 : 
//
//	- IKCP_CMD_ACK  : 对应ack包，
//			kcp通过判断当前接收到ack的时间戳和ack包内存储的发送时间戳来更新rtt和rto的时间。
//	- IKCP_CMD_PUSH : 对应数据包，kcp首先会判断sn号是否超出了当前窗口所能接收的范围，
//			如果超出范围将直接丢弃这个数据包，
//			如果是已经确认接收过的重复包也直接丢弃，然后将数据转移到新的Segment中，
//			通过 parse_data 将Segment放入rcv_buf中，
//			在 parse_data 中首先会在rcv_buf中遍历一次，判断是否已经接收过这个数据包，
//			如果数据包不存在则添加到rcv_buf中，之后将可用的Segment再转移到rcv_queue中。
//	- IKCP_CMD_WASK : 对应远端的窗口探测包，设置probe标志，在之后发送本地窗口大小。
//	- IKCP_CMD_WINS : 对应远端的窗口更新包，无需做额外的操作。
//
//	然后根据接收到的ack遍历 snd_buf 队列更新各个Segment中ack跳过的次数，用于之后判断是否需要快速重传。
//	最后进行窗口慢启动的恢复。
//---------------------------------------------------------------------
int ikcp_input(ikcpcb *kcp, const char *data, long size)
{
	IUINT32 una = kcp->snd_una; // 缓存一下当前的 snd_una
	IUINT32 maxack = 0;
	int flag = 0;

	if (ikcp_canlog(kcp, IKCP_LOG_INPUT)) {
		ikcp_log(kcp, IKCP_LOG_INPUT, "[RI] %d bytes", size);
	}

	if (data == NULL || (int)size < (int)IKCP_OVERHEAD) return -1;

	// Part 1 逐步解析data中的数据
	while (1) {
		IUINT32 ts, sn, len, una, conv;
		IUINT16 wnd;
		IUINT8 cmd, frg;
		IKCPSEG *seg;

		//** Part 1.1
		//** 解析出数据中的KCP头部
		//
		//		KCP Header Format :
		//
		//		4               1   1     2 (Byte)
		//		+---+---+---+---+---+---+---+---+
		//		|     conv      |cmd|frg|  wnd  |
		//		+---+---+---+---+---+---+---+---+
		//		|      ts       |      sn       |
		//		+---+---+---+---+---+---+---+---+
		//		|      una      |      len      |
		//		+---+---+---+---+---+---+---+---+
		//		|                               |
		//		+             DATA              +
		//		|                               |
		//		+---+---+---+---+---+---+---+---+
		//
		if (size < (int)IKCP_OVERHEAD) break;

		data = ikcp_decode32u(data, &conv);
		if (conv != kcp->conv) return -1;

		data = ikcp_decode8u(data, &cmd);
		data = ikcp_decode8u(data, &frg);
		data = ikcp_decode16u(data, &wnd);
		data = ikcp_decode32u(data, &ts);
		data = ikcp_decode32u(data, &sn);
		data = ikcp_decode32u(data, &una);
		data = ikcp_decode32u(data, &len);

		// kcp包头一共24个字节, size减去IKCP_OVERHEAD即24个字节应该不小于len
		size -= IKCP_OVERHEAD;
		if ((long)size < (long)len) return -2;

		if (cmd != IKCP_CMD_PUSH && cmd != IKCP_CMD_ACK &&
			cmd != IKCP_CMD_WASK && cmd != IKCP_CMD_WINS) 
			return -3;

		//** Part 1.2
		//** 获得远端的窗口大小
		kcp->rmt_wnd = wnd;

		//** Part 1.3 
		//** 分析una，看哪些segment远端收到了，删除send_buf中小于una的segment
		ikcp_parse_una(kcp, una);

		//** 更新本地 snd_una 数据，如snd_buff为空，snd_una指向snd_nxt，否则指向send_buff首端
		ikcp_shrink_buf(kcp);

		//** Part 1.4 
		//** 如果收到的是远端发来的ACK包
		if (cmd == IKCP_CMD_ACK) {
			if (_itimediff(kcp->current, ts) >= 0) {
				//** 更新ack
				//** 此处实际上是在更新rto, 
				//** 因为此时收到远端的ack，所以我们知道远端的包到本机的时间，因此可统计当前的网速如何，进行调整
				ikcp_update_ack(kcp, _itimediff(kcp->current, ts));
			}

			//** 分析具体是哪个segment被收到了，将其从snd_buf中移除
			ikcp_parse_ack(kcp, sn);

			//** 因为snd_buf可能改变了，更新当前的 snd_una
			ikcp_shrink_buf(kcp);

			// 记录最大的ack包的sn值
			if (flag == 0) {
				flag = 1;
				maxack = sn;
			}	else {
				if (_itimediff(sn, maxack) > 0) {
					maxack = sn;
				}
			}

			// 记录sn, rtt, rto
			if (ikcp_canlog(kcp, IKCP_LOG_IN_ACK)) {
				ikcp_log(kcp, IKCP_LOG_IN_DATA, 
					"input ack: sn=%lu rtt=%ld rto=%ld", sn, 
					(long)_itimediff(kcp->current, ts),
					(long)kcp->rx_rto);
			}
		}
		//** Part 1.5
		//** 如果收到的是远端发来的数据包
		else if (cmd == IKCP_CMD_PUSH) {
			if (ikcp_canlog(kcp, IKCP_LOG_IN_DATA)) {
				ikcp_log(kcp, IKCP_LOG_IN_DATA, 
					"input psh: sn=%lu ts=%lu", sn, ts);
			}

			//** 如果还有足够多的接收窗口
			if (_itimediff(sn, kcp->rcv_nxt + kcp->rcv_wnd) < 0) {
				//** push当前包的ack给远端（会在flush中发送ack出去)
				// 调用 ikcp_ack_push 将对该报文的确认 ACK 报文放入 ACK 列表acklist中
				ikcp_ack_push(kcp, sn, ts);

				if (_itimediff(sn, kcp->rcv_nxt) >= 0) {
					seg = ikcp_segment_new(kcp, len);
					seg->conv = conv;
					seg->cmd = cmd;
					seg->frg = frg;
					seg->wnd = wnd;
					seg->ts = ts;
					seg->sn = sn;
					seg->una = una;
					seg->len = len;

					if (len > 0) {
						memcpy(seg->data, data, len);
					}

					// 解析data, 
					// 首先会在rcv_buf中遍历一次，判断是否已经接收过这个数据包，
					// 如果数据包不存在则添加到rcv_buf中，之后将可用的Segment再转移到rcv_queue中
					ikcp_parse_data(kcp, seg);
				}
			}
		}
		//** Part 1.6
		//** 如果收到的包是远端发过来询问窗口大小的包
		else if (cmd == IKCP_CMD_WASK) {
			// ready to send back IKCP_CMD_WINS in ikcp_flush
			// tell remote my window size
			kcp->probe |= IKCP_ASK_TELL;
			if (ikcp_canlog(kcp, IKCP_LOG_IN_PROBE)) {
				ikcp_log(kcp, IKCP_LOG_IN_PROBE, "input probe");
			}
		}
		// wins告知窗口大小
		else if (cmd == IKCP_CMD_WINS) {
			// do nothing
			if (ikcp_canlog(kcp, IKCP_LOG_IN_WINS)) {
				ikcp_log(kcp, IKCP_LOG_IN_WINS,
					"input wins: %lu", (IUINT32)(wnd));
			}
		}
		else {
			return -3;
		}

		data += len;
		size -= len;
	}

	if (flag != 0) {
		// 根据记录的最大ack的snd值，扫描 snd_buf ，小于max ack的segment的fastack ++，
		// 在 ikcp_flush 函数中会判断是否超过指定快速重传次数阈值，超过了就会启动快速重传
		ikcp_parse_fastack(kcp, maxack);
	}

	// snd_una与之前缓存的 una 比较 见本函数第一行代码，
	// 若 snd_una>una，说明收到了有效的una或ack之后已经更新了snd_una了,
	// 接下来要做流量控制和拥塞控制
	// 情况1 : cwnd < ssthresh, 慢启动阶段，cwnd++，可发送最大数据量+mss
	// 情况2 : 拥塞控制阶段
	if (_itimediff(kcp->snd_una, una) > 0) {
		if (kcp->cwnd < kcp->rmt_wnd) {
			IUINT32 mss = kcp->mss;
			if (kcp->cwnd < kcp->ssthresh) { // 慢启动阶段
				kcp->cwnd++;
				kcp->incr += mss;
			}	else { // 拥塞控制阶段
				if (kcp->incr < mss)
					kcp->incr = mss;
				kcp->incr += (mss * mss) / kcp->incr + (mss / 16);
				if ((kcp->cwnd + 1) * mss <= kcp->incr)
					kcp->cwnd++;
			}
			if (kcp->cwnd > kcp->rmt_wnd) {
				kcp->cwnd = kcp->rmt_wnd;
				kcp->incr = kcp->rmt_wnd * mss;
			}
		}
	}

	return 0;
}


//---------------------------------------------------------------------
// ikcp_encode_seg
//---------------------------------------------------------------------
static char *ikcp_encode_seg(char *ptr, const IKCPSEG *seg)
{
	ptr = ikcp_encode32u(ptr, seg->conv);
	ptr = ikcp_encode8u(ptr, (IUINT8)seg->cmd);
	ptr = ikcp_encode8u(ptr, (IUINT8)seg->frg);
	ptr = ikcp_encode16u(ptr, (IUINT16)seg->wnd);
	ptr = ikcp_encode32u(ptr, seg->ts);
	ptr = ikcp_encode32u(ptr, seg->sn);
	ptr = ikcp_encode32u(ptr, seg->una);
	ptr = ikcp_encode32u(ptr, seg->len);
	return ptr;
}

static int ikcp_wnd_unused(const ikcpcb *kcp)
{
	if (kcp->nrcv_que < kcp->rcv_wnd) {
		return kcp->rcv_wnd - kcp->nrcv_que; // 剩余接收窗口大小(接收窗口大小-接收队列大小)
	}
	return 0;
}


//---------------------------------------------------------------------
//	ikcp_flush
//	KCP.flush之发包
//
//	KCP中，与发包相关联的成员变量有以下几个：
//	- UInt32 snd_una 当前未收到确认回传的发送出去的包的最小编号。也就是此编号前的包都已经收到确认回传了。
//	- UInt32 snd_nxt 下一个要发送出去的包编号
//	- UInt32 snd_wnd 发送窗口大小
//	- UInt32 rmt_wnd 远端的接收窗口大小
//	- Segment[] snd_queue 发送队列。
//			应用层的数据（在调用KCP.Send后）
//			会进入此队列中，KCP在flush的时候根据发送窗口的大小，
//			再决定将多少个Segment放入到snd_buf中进行发送
//	- Segment[] snd_buf 发送缓存池。
//			发送出去的数据将会呆在这个池子中，
//			等待远端的回传确认，等收到远端确认此包收到后再从snd_buf移出去。 
//			KCP在每次flush的时候都会检查这个缓存池中的每个Segment，如果超时或者判定丢包就会重发。
//
//	Tips :
//	- snd是send或者sender的缩写，发送端
//	- rmt是remote的缩写，远端
//	- nxt是next的缩写
//
//	kcp在flush的时候将snd_queue中的内容移动到snd_buf中，然后才真正将数据发送出去。
//	
//	首先kcp会发送所有ack信息，每个ack信息占用一个kcp数据包头的大小，用于存储ack的sn和ts，
//	这里的ts是在接收到数据包时存储进acklist中的ts，也就是远端发送数据包的ts。
//	
//	发送完ack信息后，kcp还会检查当前是否需要对远端窗口进行探测。
//	因为kcp的流量控制依赖于远端通知其可接受窗口的大小，一旦远端接受窗口为0，
//	本地将不会再向远端发送数据，就无法从远端接收ack包，从而没有机会更新远端窗口大小。
//	在这种情况下，kcp需要发送窗口探测包到远端，待远端回复窗口大小后，后续传输才可以继续。
//	然后kcp分别根据之前的判断，选择是否发送窗口探测包和窗口更新包。
//	
//	接着kcp将设置当前的发送窗口大小，如果未开启非退让流控，
//	窗口大小将有发送缓存大小、远端窗口大小以及慢启动所计算出的窗口大小决定，
//	如果开启非退让流控，则不受慢启动窗口大小的限制。
//	按照发送窗口所能容纳的大小，kcp将需要发送的Segment从snd_queue移动到snd_buf中。
//	
//	然后更新快速重传的次数和重传时间延迟，为之后的数据发送做准备。
//	
//	kcp将遍历snd_buf队列，将发送缓冲区中需要发送的Segment逐一发送出去，有3种情况数据包需要发送：
//	
//	- 第一次发送：设置重传时间信息。
//	- 超过重传时间：更新重传时间信息，根据kcp的设置选择rto * 2或rto*1.5，并记录lost标志。
//	- ack跳过指定次数：立即重传，重置跳过次数并更新重传时间，记录change标志。
//
//	然后将这3种情况下的数据包进行发送。当一个数据包重传超过dead_link次数时，
//	kcp的state将设置为 - 1。这里的state在其他函数中并没有使用到，
//	猜测可能是当kcp发送缓冲区内的数据丢失无法得到ack时，kcp将不断重传，
//	如果重传多次仍然失败，这时可以通过判断state来清理snd_buf中这些僵死的数据包，
//	放在永远占用在snd_buf中限制正常的发送窗口大小。
//	
//	最后，kcp将更新慢启动的窗口大小。
//---------------------------------------------------------------------
void ikcp_flush(ikcpcb *kcp)
{
	IUINT32 current = kcp->current;
	char *buffer = kcp->buffer;
	char *ptr = buffer;
	int count, size, i;
	IUINT32 resent, cwnd;
	IUINT32 rtomin;
	struct IQUEUEHEAD *p;
	int change = 0; // 标识快重传发生
	int lost = 0; // 记录出现了报文丢失
	IKCPSEG seg;

	// 'ikcp_update' haven't been called. 
	// 检查 kcp->update 是否更新，未更新直接返回。
	// kcp->update 由 ikcp_update 更新，
	// 上层应用需要每隔一段时间（10-100ms）调用 ikcp_update 来驱动 KCP 发送数据；
	if (kcp->updated == 0) return;

	seg.conv = kcp->conv;
	seg.cmd = IKCP_CMD_ACK;
	seg.frg = 0;
	seg.wnd = ikcp_wnd_unused(kcp);
	seg.una = kcp->rcv_nxt;
	seg.len = 0;
	seg.sn = 0;
	seg.ts = 0;

	// flush acknowledges
	// 我们先看一下 KCP 对于 ack 报文的管理。KCP 控制块 ikcpcb 中有如下几个成员：
	// - acklist： 当收到一个数据报文时，将其对应的 ACK 报文的 sn 号以及
	//			时间戳 ts 同时加入到acklist 中，即形成如[sn1, ts1, sn2, ts2 …] 的列表；
	// - ackcount：记录 acklist 中存放的 ACK 报文的数量；
	// - ackblock：acklist 数组的可用长度，当 acklist 的容量不足时，需要进行扩容；
	// 以下代码表示 : 
	// 准备将 acklist 中记录的 ACK 报文发送出去，即从 acklist 中填充 ACK 报文的 sn 和 ts 字段；
	count = kcp->ackcount;
	for (i = 0; i < count; i++) {
		size = (int)(ptr - buffer);
		if (size + (int)IKCP_OVERHEAD > (int)kcp->mtu) {
			ikcp_output(kcp, buffer, size);
			ptr = buffer;
		}
		ikcp_ack_get(kcp, i, &seg.sn, &seg.ts);
		ptr = ikcp_encode_seg(ptr, &seg);
	}

	kcp->ackcount = 0;

	// probe window size (if remote window size equals zero)
	// 检查当前是否需要对远端窗口进行探测。
	// 由于 KCP 流量控制依赖于远端通知其可接受窗口的大小，
	// 一旦远端接受窗口 kcp->rmt_wnd 为0，那么本地将不会再向远端发送数据，
	// 因此就没有机会从远端接受 ACK 报文，从而没有机会更新远端窗口大小。
	// 在这种情况下，KCP 需要发送窗口探测报文到远端，待远端回复窗口大小后，后续传输才可以继续
	if (kcp->rmt_wnd == 0) {
		if (kcp->probe_wait == 0) {
			kcp->probe_wait = IKCP_PROBE_INIT;
			kcp->ts_probe = kcp->current + kcp->probe_wait; // 初始化探测间隔和下一次探测时间
		}	
		else {
			if (_itimediff(kcp->current, kcp->ts_probe) >= 0) { //当前时间 > 下一次探查窗口的时间
				if (kcp->probe_wait < IKCP_PROBE_INIT)
					kcp->probe_wait = IKCP_PROBE_INIT;
				kcp->probe_wait += kcp->probe_wait / 2;   //等待时间变为之前的1.5倍
				if (kcp->probe_wait > IKCP_PROBE_LIMIT)
					kcp->probe_wait = IKCP_PROBE_LIMIT;   //若超过上限，设置为上限值
				kcp->ts_probe = kcp->current + kcp->probe_wait;  //计算下次探查窗口的时间戳
				kcp->probe |= IKCP_ASK_SEND;         //设置探查变量。IKCP_ASK_TELL表示告知远端窗口大小。IKCP_ASK_SEND表示请求远端告知窗口大小
			}
		}
	}	else {
		kcp->ts_probe = 0;
		kcp->probe_wait = 0;
	}

	// flush window probing commands
	// 将窗口探测报文发送出去
	if (kcp->probe & IKCP_ASK_SEND) {
		seg.cmd = IKCP_CMD_WASK;
		size = (int)(ptr - buffer);
		if (size + (int)IKCP_OVERHEAD > (int)kcp->mtu) {
			ikcp_output(kcp, buffer, size);
			ptr = buffer;
		}
		ptr = ikcp_encode_seg(ptr, &seg);
	}

	// flush window probing commands
	// 将窗口回复报文发送出去
	if (kcp->probe & IKCP_ASK_TELL)
	{
		seg.cmd = IKCP_CMD_WINS;
		size = (int)(ptr - buffer);
		if (size + (int)IKCP_OVERHEAD > (int)kcp->mtu) {
			ikcp_output(kcp, buffer, size);
			ptr = buffer;
		}
		ptr = ikcp_encode_seg(ptr, &seg);
	}

	kcp->probe = 0;

	// calculate window size
	// 计算本次发送可用的窗口大小，这里 KCP 采用了可以配置的策略，
	// 正常情况下，KCP 的窗口大小由
	// 发送窗口 snd_wnd 和 远端接收窗口 rmt_wnd 以及 根据拥塞控制计算得到的 kcp->cwnd 三者共同决定；
	// 但是当开启了 nocwnd 模式时，窗口大小仅由前两者决定
	cwnd = _imin_(kcp->snd_wnd, kcp->rmt_wnd);
	if (kcp->nocwnd == 0) cwnd = _imin_(kcp->cwnd, cwnd);

	// move data from snd_queue to snd_buf
	// 将缓存在 snd_queue 中的数据移到 snd_buf 中等待发送
	// 移动的包的数量不会超过snd_una+cwnd-snd_nxt，确保发送的数据不会让接收方的接收队列溢出。
	// 该功能类似于TCP协议中的滑动窗口。
	while (_itimediff(kcp->snd_nxt, kcp->snd_una + cwnd) < 0) {
		IKCPSEG *newseg;
		if (iqueue_is_empty(&kcp->snd_queue))
			break;

		newseg = iqueue_entry(kcp->snd_queue.next, IKCPSEG, node);  //snd_queue：发送消息的队列

		iqueue_del(&newseg->node);                      //从发送消息队列中，删除节点
		iqueue_add_tail(&newseg->node, &kcp->snd_buf);  //然后把删除的节点，加入到kcp的发送缓存队列中
		kcp->nsnd_que--;
		kcp->nsnd_buf++;

		newseg->conv = kcp->conv;     //会话id
		newseg->cmd = IKCP_CMD_PUSH;
		newseg->wnd = seg.wnd;
		newseg->ts = current;
		newseg->sn = kcp->snd_nxt++;  //下一个待发报的序号
		newseg->una = kcp->rcv_nxt;   //待接收的下一个消息序号
		newseg->resendts = current;   //下次超时重传的时间戳
		newseg->rto = kcp->rx_rto;    //由ack接收延迟计算出来的重传超时时间
		newseg->fastack = 0;          //收到ack时计算的该分片被跳过的累计次数
		newseg->xmit = 0;             //发送分片的次数，每发送一次加一
	}

	// calculate resent
	// 在发送数据之前，先设置快重传的次数和重传间隔；
	// KCP 允许设置快重传的次数，即 fastresend 参数。
	// 例如设置 fastresend 为2，并且发送端发送了1,2,3,4,5几个包，
	// 收到远端的ACK: 1, 3, 4, 5，当收到ACK3时，KCP知道2被跳过1次，
	// 收到ACK4时，知道2被“跳过”了2次，此时可以认为2号丢失，不用等超时，直接重传2号包；
	// 每个报文的 fastack 记录了该报文被跳过了几次，
	// 由函数 ikcp_parse_fastack 更新。于此同时，KCP 也允许设置 nodelay 参数,
	// 当激活该参数时，每个报文的超时重传时间将由 x2 变为 x1.5，即加快报文重传：
	resent = (kcp->fastresend > 0)? (IUINT32)kcp->fastresend : 0xffffffff; // 是否设置了快重传次数
	rtomin = (kcp->nodelay == 0)? (kcp->rx_rto >> 3) : 0; // 是否开启了 nodelay

	// flush data segments
	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = p->next) {
		IKCPSEG *segment = iqueue_entry(p, IKCPSEG, node);
		int needsend = 0;

		// 1. xmit为0，第一次发送，赋值rto及resendts
		if (segment->xmit == 0) {
			needsend = 1;
			segment->xmit++;
			segment->rto = kcp->rx_rto;
			segment->resendts = current + segment->rto + rtomin;
		}
		// 2. 超过segment重发时间，却仍在send_buf中，说明长时间未收到ack，认为丢失，重发
		else if (_itimediff(current, segment->resendts) >= 0) {
			needsend = 1;
			segment->xmit++;
			kcp->xmit++;
			// 更新重传时间信息，根据kcp的设置选择rto*2或rto*1.5，并记录lost标志。
			if (kcp->nodelay == 0) {
				segment->rto += kcp->rx_rto; // 以2倍的方式来增长(TCP的RTO默认也是2倍增长)
			}	else {
				segment->rto += kcp->rx_rto / 2; // 可以以1.5倍的速度增长
			}
			segment->resendts = current + segment->rto;
			lost = 1; // 记录出现了报文丢失
		}
		// 3. 达到快速重传阈值，重新发送
		else if (segment->fastack >= resent) {
			needsend = 1;
			segment->xmit++;
			segment->fastack = 0;
			segment->resendts = current + segment->rto;
			change++;  // 标识快重传发生
		}

		if (needsend) {
			int size, need;
			segment->ts = current;
			segment->wnd = seg.wnd;
			segment->una = kcp->rcv_nxt;

			size = (int)(ptr - buffer);
			need = IKCP_OVERHEAD + segment->len; //segment报文默认大小 + segment的长度

			if (size + need > (int)kcp->mtu) {
				ikcp_output(kcp, buffer, size);
				ptr = buffer;
			}

			ptr = ikcp_encode_seg(ptr, segment);

			if (segment->len > 0) {
				// 因为ptr初始值是指向buffer的, 而buffer是指向kcp->buffer的, 
				// 所以这里实际上是把segment->data拷贝到kcp->buffer中, 
				// 之后再求ptr与buffer的差值是否大于mtu决定是否要ikcp_output
				memcpy(ptr, segment->data, segment->len);
				ptr += segment->len;
			}

			if (segment->xmit >= kcp->dead_link) {
				kcp->state = -1;
			}
		}
	}

	// flush remain segments
	size = (int)(ptr - buffer);
	if (size > 0) {
		ikcp_output(kcp, buffer, size);
	}

	// update ssthresh
	// 如发生快速重传，将拥塞窗口阈值ssthresh调整为当前发送窗口的一半，
	// 将拥塞窗口调整为 ssthresh + resent，resent是触发快速重传的丢包的次数，
	// resent的值代表的意思在被弄丢的包后面收到了resent个数的包的ack。
	// 这样调整后kcp就进入了拥塞控制状态。
	if (change) {
		IUINT32 inflight = kcp->snd_nxt - kcp->snd_una;
		kcp->ssthresh = inflight / 2; // 将拥塞窗口阈值ssthresh调整为当前发送窗口的一半
		if (kcp->ssthresh < IKCP_THRESH_MIN)
			kcp->ssthresh = IKCP_THRESH_MIN;
		kcp->cwnd = kcp->ssthresh + resent;
		kcp->incr = kcp->cwnd * kcp->mss;
	}

	// update ssthresh
	// 当出现超时重传的时候，说明网络很可能死掉了，因为超时重传会出现，
	// 原因是有包丢失了，并且该包之后的包也没有收到，这很有可能是网络死了，
	// 这时候，拥塞窗口直接变为1。
	if (lost) {
		kcp->ssthresh = cwnd / 2;
		if (kcp->ssthresh < IKCP_THRESH_MIN)
			kcp->ssthresh = IKCP_THRESH_MIN;
		kcp->cwnd = 1;
		kcp->incr = kcp->mss;
	}

	if (kcp->cwnd < 1) {
		kcp->cwnd = 1;
		kcp->incr = kcp->mss;
	}
}


//---------------------------------------------------------------------
// update state (call it repeatedly, every 10ms-100ms), or you can ask 
// ikcp_check when to call it again (without ikcp_input/_send calling).
// 'current' - current timestamp in millisec. 
//
//  ikcp_update函数的作用是 : 
//			设置下一次flush刷新时间戳, 并调用 ikcp_flush 函数
//
// kcp需要上层通过update来驱动kcp数据包的发送，每次驱动的时间间隔由interval来决定，
// interval可以通过函数ikcp_interval来设置，间隔时间在10毫秒到5秒之间，
// 初始默认值为100毫秒。
// 
// 另外注意到一点是，updated参数只有在第一次调用ikcp_update函数时设置为1，
// 源码中没有找到重置为0的地方，目测就是一个标志参数，
// 用于区别第一次驱动和之后的驱动所需要选择的时间。
//---------------------------------------------------------------------
void ikcp_update(ikcpcb *kcp, IUINT32 current)
{
	IINT32 slap;

	kcp->current = current;

	if (kcp->updated == 0) {
		kcp->updated = 1;
		kcp->ts_flush = kcp->current; // 设置下一次flush刷新时间戳
	}

	slap = _itimediff(kcp->current, kcp->ts_flush);

	if (slap >= 10000 || slap < -10000) {
		kcp->ts_flush = kcp->current;
		slap = 0;
	}

	if (slap >= 0) {
		kcp->ts_flush += kcp->interval; // 设置下一次flush刷新时间戳
		if (_itimediff(kcp->current, kcp->ts_flush) >= 0) {
			kcp->ts_flush = kcp->current + kcp->interval;
		}
		ikcp_flush(kcp);
	}
}


//---------------------------------------------------------------------
// Determine when should you invoke ikcp_update:
// returns when you should invoke ikcp_update in millisec, if there 
// is no ikcp_input/_send calling. you can call ikcp_update in that
// time, instead of call update repeatly.
// Important to reduce unnacessary ikcp_update invoking. use it to 
// schedule ikcp_update (eg. implementing an epoll-like mechanism, 
// or optimize ikcp_update when handling massive kcp connections)
//
// check函数用于获取下次update的时间。
// 具体的时间由上次update后更新的下次时间和snd_buf中的超时重传时间决定。
// check过程会寻找snd_buf中是否有超时重传的数据，如果有需要重传的Segment，
// 将返回当前时间，立即进行一次update来进行重传，如果全都不需要重传，
// 则会根据最小的重传时间来判断下次update的时间。
//---------------------------------------------------------------------
IUINT32 ikcp_check(const ikcpcb *kcp, IUINT32 current)
{
	IUINT32 ts_flush = kcp->ts_flush;
	IINT32 tm_flush = 0x7fffffff;
	IINT32 tm_packet = 0x7fffffff;
	IUINT32 minimal = 0;
	struct IQUEUEHEAD *p;

	if (kcp->updated == 0) {
		return current;
	}

	if (_itimediff(current, ts_flush) >= 10000 ||
		_itimediff(current, ts_flush) < -10000) {
		ts_flush = current;
	}

	if (_itimediff(current, ts_flush) >= 0) {
		return current;
	}

	tm_flush = _itimediff(ts_flush, current);

	for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = p->next) {
		const IKCPSEG *seg = iqueue_entry(p, const IKCPSEG, node);
		IINT32 diff = _itimediff(seg->resendts, current);
		if (diff <= 0) {
			return current;
		}
		if (diff < tm_packet) tm_packet = diff;
	}

	minimal = (IUINT32)(tm_packet < tm_flush ? tm_packet : tm_flush);
	if (minimal >= kcp->interval) minimal = kcp->interval;

	return current + minimal;
}



int ikcp_setmtu(ikcpcb *kcp, int mtu)
{
	char *buffer;
	if (mtu < 50 || mtu < (int)IKCP_OVERHEAD) 
		return -1;
	buffer = (char*)ikcp_malloc((mtu + IKCP_OVERHEAD) * 3);
	if (buffer == NULL) 
		return -2;
	kcp->mtu = mtu;
	kcp->mss = kcp->mtu - IKCP_OVERHEAD;
	ikcp_free(kcp->buffer);
	kcp->buffer = buffer;
	return 0;
}

int ikcp_interval(ikcpcb *kcp, int interval)
{
	if (interval > 5000) interval = 5000;
	else if (interval < 10) interval = 10;
	kcp->interval = interval;
	return 0;
}

//nodelay:   0 不启用，1启用nodelay模式(即使用更小的 rx_minrto, 以更快检测到丢包)
//interval： 内部flush刷新时间
//resend:    0（默认）表示关闭。可以自己设置值，若设置为2（则2次ACK跨越将会直接重传）
//nc:        即 no congest 的缩写, 是否关闭拥塞控制，0（默认）代表不关闭，1代表关闭
int ikcp_nodelay(ikcpcb *kcp, int nodelay, int interval, int resend, int nc)
{
	if (nodelay >= 0)
	{
		kcp->nodelay = nodelay;
		if (nodelay)
		{
			kcp->rx_minrto = IKCP_RTO_NDL;  //最小重传超时时间（如果需要可以设置更小）
		}
		else
		{
			kcp->rx_minrto = IKCP_RTO_MIN;
		}
	}
	if (interval >= 0)
	{
		if (interval > 5000)
			interval = 5000;
		else if (interval < 10)
			interval = 10;
		kcp->interval = interval; //内部flush刷新时间
	}
	if (resend >= 0) // ACK被跳过resend次数后直接重传该包, 而不等待超时
	{                     
		kcp->fastresend = resend; // fastresend : 触发快速重传的重复ack个数
	}
	if (nc >= 0)
	{
		kcp->nocwnd = nc;
	}
	return 0;
}


int ikcp_wndsize(ikcpcb *kcp, int sndwnd, int rcvwnd)
{
	if (kcp) {
		if (sndwnd > 0) {
			kcp->snd_wnd = sndwnd;
		}
		if (rcvwnd > 0) {   // must >= max fragment size
			kcp->rcv_wnd = _imax_(rcvwnd, IKCP_WND_RCV);
		}
	}
	return 0;
}

int ikcp_waitsnd(const ikcpcb *kcp)
{
	return kcp->nsnd_buf + kcp->nsnd_que;
}


// read conv
IUINT32 ikcp_getconv(const void *ptr)
{
	IUINT32 conv;
	ikcp_decode32u((const char*)ptr, &conv);
	return conv;
}


