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
#ifndef __IKCP_H__
#define __IKCP_H__

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>


//=====================================================================
// 32BIT INTEGER DEFINITION 
//=====================================================================
#ifndef __INTEGER_32_BITS__
#define __INTEGER_32_BITS__
#if defined(_WIN64) || defined(WIN64) || defined(__amd64__) || \
	defined(__x86_64) || defined(__x86_64__) || defined(_M_IA64) || \
	defined(_M_AMD64)
	typedef unsigned int ISTDUINT32;
	typedef int ISTDINT32;
#elif defined(_WIN32) || defined(WIN32) || defined(__i386__) || \
	defined(__i386) || defined(_M_X86)
	typedef unsigned long ISTDUINT32;
	typedef long ISTDINT32;
#elif defined(__MACOS__)
	typedef UInt32 ISTDUINT32;
	typedef SInt32 ISTDINT32;
#elif defined(__APPLE__) && defined(__MACH__)
	#include <sys/types.h>
	typedef u_int32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#elif defined(__BEOS__)
	#include <sys/inttypes.h>
	typedef u_int32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#elif (defined(_MSC_VER) || defined(__BORLANDC__)) && (!defined(__MSDOS__))
	typedef unsigned __int32 ISTDUINT32;
	typedef __int32 ISTDINT32;
#elif defined(__GNUC__)
	#include <stdint.h>
	typedef uint32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#else 
	typedef unsigned long ISTDUINT32; 
	typedef long ISTDINT32;
#endif
#endif


//=====================================================================
// Integer Definition
//=====================================================================
#ifndef __IINT8_DEFINED
#define __IINT8_DEFINED
typedef char IINT8;
#endif

#ifndef __IUINT8_DEFINED
#define __IUINT8_DEFINED
typedef unsigned char IUINT8;
#endif

#ifndef __IUINT16_DEFINED
#define __IUINT16_DEFINED
typedef unsigned short IUINT16;
#endif

#ifndef __IINT16_DEFINED
#define __IINT16_DEFINED
typedef short IINT16;
#endif

#ifndef __IINT32_DEFINED
#define __IINT32_DEFINED
typedef ISTDINT32 IINT32;
#endif

#ifndef __IUINT32_DEFINED
#define __IUINT32_DEFINED
typedef ISTDUINT32 IUINT32;
#endif

#ifndef __IINT64_DEFINED
#define __IINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64 IINT64;
#else
typedef long long IINT64;
#endif
#endif

#ifndef __IUINT64_DEFINED
#define __IUINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int64 IUINT64;
#else
typedef unsigned long long IUINT64;
#endif
#endif

#ifndef INLINE
#if defined(__GNUC__)

#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#define INLINE         __inline__ __attribute__((always_inline))
#else
#define INLINE         __inline__
#endif

#elif (defined(_MSC_VER) || defined(__BORLANDC__) || defined(__WATCOMC__))
#define INLINE __inline
#else
#define INLINE 
#endif
#endif

#if (!defined(__cplusplus)) && (!defined(inline))
#define inline INLINE
#endif


//=====================================================================
// QUEUE DEFINITION                                                  
//=====================================================================
#ifndef __IQUEUE_DEF__
#define __IQUEUE_DEF__

struct IQUEUEHEAD {
	struct IQUEUEHEAD *next, *prev;
};

typedef struct IQUEUEHEAD iqueue_head;


//---------------------------------------------------------------------
// queue init                                                         
//---------------------------------------------------------------------
#define IQUEUE_HEAD_INIT(name) { &(name), &(name) }
#define IQUEUE_HEAD(name) \
	struct IQUEUEHEAD name = IQUEUE_HEAD_INIT(name)

#define IQUEUE_INIT(ptr) ( \
	(ptr)->next = (ptr), (ptr)->prev = (ptr))

#define IOFFSETOF(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define ICONTAINEROF(ptr, type, member) ( \
		(type*)( ((char*)((type*)ptr)) - IOFFSETOF(type, member)) )


// 关于 IQUEUE_ENTRY 宏 : 
//
// #1. 先看&((type *)0)->member：
//			把“0”强制转化为指针类型，则该指针一定指向“0”（数据段基址）。
//			因为指针是“type *”型的，所以可取到以“0”为基地址的一个type型变量member域的地址。
//			那么这个地址也就等于member域到结构体基地址的偏移字节数。
// 
// #2. 再来看((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))：
//			(char *)(ptr)使得指针的加减操作步长为一字节，
//			(unsigned long)(&((type *)0)->member)等于ptr指向的member到该member所在结构体基地址的偏移字节数。
//			二者一减便得出该结构体的地址。转换为(type *)型的指针，大功告成。
//
// 比如 ikcp_parse_una 函数中的以下代码
// ```
// struct IQUEUEHEAD *p, *next;
// for (p = kcp->snd_buf.next; p != &kcp->snd_buf; p = next)
// {
//		IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node); 
// ```
// 其中的 ` IKCPSEG *seg = iqueue_entry(p, IKCPSEG, node); ` 展开则为 : 
//		IKCPSEG *seg = ( (IKCPSEG*)( ( (char*)((IKCPSEG*)p) ) - ((size_t) &((IKCPSEG *)0)->node) ) );
// 
// 则可达到以下效果 : 
//		通过一个 IQUEUEHEAD 指针 p 得到 一个 指向该链表节点所在的数据块的 IKCPSEG 指针 seg
//
// 注 : 
//			要把源代码中的宏展开，其实只要使用gcc进行预处理即可。
//			gcc -E source.c > out.txt
//			-E 表示只进行预处理，不进行编译。
//			预处理时会把注释当成空格处理掉，如果想保留其中的注释，可以加上 -C选项，即：
//			gcc -E -C source.c > out.txt
#define IQUEUE_ENTRY(ptr, type, member) ICONTAINEROF(ptr, type, member)


//---------------------------------------------------------------------
// queue operation                     
//---------------------------------------------------------------------

// 插到队列某一元素的后面
// 如 head = rcv_queue, node = A 则变为 rcv_queue <-> A <-> rcv_queue
// 若然后在A后面加个B之后(即head = A, node = B)则变为 rcv_queue <-> A <-> B <-> rcv_queue
#define IQUEUE_ADD(node, head) (\
	(node)->prev = (head), (node)->next = (head)->next, \
	(head)->next->prev = (node), (head)->next = (node))

// 插到队列尾部
// 如 rcv_queue 变为 rcv_queue <-> A <-> rcv_queue
// 然后加个B之后变为 rcv_queue <-> A <-> B <-> rcv_queue
// 然后加个node之后变为 rcv_queue <-> A <-> B <-> node <-> rcv_queue
#define IQUEUE_ADD_TAIL(node, head) (\
	(node)->prev = (head)->prev, (node)->next = (head), \
	(head)->prev->next = (node), (head)->prev = (node))

#define IQUEUE_DEL_BETWEEN(p, n) ((n)->prev = (p), (p)->next = (n))

// 如 A <-> entry <-> C 变为 A <-> C
#define IQUEUE_DEL(entry) (\
	(entry)->next->prev = (entry)->prev, \
	(entry)->prev->next = (entry)->next, \
	(entry)->next = 0, (entry)->prev = 0)

#define IQUEUE_DEL_INIT(entry) do { \
	IQUEUE_DEL(entry); IQUEUE_INIT(entry); } while (0)

#define IQUEUE_IS_EMPTY(entry) ((entry) == (entry)->next)

#define iqueue_init		IQUEUE_INIT
#define iqueue_entry	IQUEUE_ENTRY
#define iqueue_add		IQUEUE_ADD
#define iqueue_add_tail	IQUEUE_ADD_TAIL
#define iqueue_del		IQUEUE_DEL
#define iqueue_del_init	IQUEUE_DEL_INIT
#define iqueue_is_empty IQUEUE_IS_EMPTY

#define IQUEUE_FOREACH(iterator, head, TYPE, MEMBER) \
	for ((iterator) = iqueue_entry((head)->next, TYPE, MEMBER); \
		&((iterator)->MEMBER) != (head); \
		(iterator) = iqueue_entry((iterator)->MEMBER.next, TYPE, MEMBER))

#define iqueue_foreach(iterator, head, TYPE, MEMBER) \
	IQUEUE_FOREACH(iterator, head, TYPE, MEMBER)

#define iqueue_foreach_entry(pos, head) \
	for( (pos) = (head)->next; (pos) != (head) ; (pos) = (pos)->next )
	

#define __iqueue_splice(list, head) do {	\
		iqueue_head *first = (list)->next, *last = (list)->prev; \
		iqueue_head *at = (head)->next; \
		(first)->prev = (head), (head)->next = (first);		\
		(last)->next = (at), (at)->prev = (last); }	while (0)

#define iqueue_splice(list, head) do { \
	if (!iqueue_is_empty(list)) __iqueue_splice(list, head); } while (0)

#define iqueue_splice_init(list, head) do {	\
	iqueue_splice(list, head);	iqueue_init(list); } while (0)


#ifdef _MSC_VER
#pragma warning(disable:4311)
#pragma warning(disable:4312)
#pragma warning(disable:4996)
#endif

#endif


//---------------------------------------------------------------------
// WORD ORDER
//---------------------------------------------------------------------
#ifndef IWORDS_BIG_ENDIAN
    #ifdef _BIG_ENDIAN_
        #if _BIG_ENDIAN_
            #define IWORDS_BIG_ENDIAN 1
        #endif
    #endif
    #ifndef IWORDS_BIG_ENDIAN
        #if defined(__hppa__) || \
            defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
            (defined(__MIPS__) && defined(__MIPSEB__)) || \
            defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
            defined(__sparc__) || defined(__powerpc__) || \
            defined(__mc68000__) || defined(__s390x__) || defined(__s390__)
            #define IWORDS_BIG_ENDIAN 1
        #endif
    #endif
    #ifndef IWORDS_BIG_ENDIAN
        #define IWORDS_BIG_ENDIAN  0
    #endif
#endif



//=====================================================================
//  SEGMENT
//	KCP数据包
//	
// # 1. KCP Segment定义
//
//		- KCP.Segment.conv 发送端与接收端通信时的匹配数字，发送端发送的数据包中此值与接收端的conv值匹配一致时，接收端才会接受此包。
//		- KCP.Segment.cmd cmd是command的缩写, 指明Segment类型。 
//				KCP中会有四种Segment数据包类型，分别是
//				- 1. 数据包（IKCP_CMD_PUSH）： 
//						最基础的Segment，用于发送应用层数据给远端。
//						每个数据包会有自己的sn， 发送出去后不会立即从缓存池中删除，
//						而是会等收到远端返回回来的ack包时才会从缓存中移除（两端通过sn确认哪些包已收到）
//				- 2. ACK包（IKCP_CMD_ACK）：
//						告诉远端自己已收到了远端发送的哪个编号的数据
//				- 3. 窗口大小探测包（IKCP_CMD_WASK）：
//						询问远端的接收窗口大小。
//						本地发送数据时，会根据远端的窗口大小来控制发送的数据量。
//						每个数据包的包头中都会带有远端当前的接收窗口大小。
//						但是当远端的接收窗口大小为0时，本机将不会再向远端发送数据，
//						此时也就不会有远端的回传数据从而导致无法更新远端窗口大小。
//						因此需要单独的一类远端窗口大小探测包，在远端接收窗口大小为0时，
//						隔一段时间询问一次，从而让本地有机会再开始重新传数据。
//				- 4. 窗口大小回应包（IKCP_CMD_WINS）：
//						回应远端自己的数据接收窗口大小window size
//		- KCP.Segment.frg frg是fragment的缩小，是一个Segment在一次Send的data中的倒序序号。 
//				在让KCP发送数据时，KCP会加入snd_queue的Segment分配序号，标记Segment是这次发送数据中的倒数第几个Segment。
//				数据在发送出去时，由于mss的限制，数据可能被分成若干个Segment发送出去。在分segment的过程中，相应的序号就会被记录到frg中。
//				接收端在接收到这些segment时，就会根据frg将若干个segment合并成一个，再返回给应用层。
//		- KCP.Segment.wnd wnd是window的缩写； 滑动窗口大小，用于流控（Flow Control）
//			- 当Segment做为发送数据时，此wnd为本机滑动窗口大小，用于告诉远端自己窗口剩余多少
//			- 当Segment做为接收到数据时，此wnd为远端滑动窗口大小，本机知道了远端窗口剩余多少后，可以控制自己接下来发送数据的大小
//		- KCP.Segment.ts 即timestamp, 当前Segment发送时的时间戳
//		- KCP.Segment.resendts 即resend timestamp, 指定重发的时间戳，当当前时间超过这个时间时，则再重发一次这个包。
//		- KCP.Segment.sn 即Sequence Number, Segment的编号
//		- KCP.Segment.una una即unacknowledged, 表示此编号前的所有包都已收到了。
//		- KCP.Segment.fastack 用于以数据驱动的快速重传机制；
//		- KCP.Segment.rto rto即Retransmission TimeOut，即超时重传时间，在发送出去时根据之前的网络情况进行设置
//		- KCP.Segment.xmit 基本类似于Segment发送的次数，每发送一次会自加一。用于统计该Segment被重传了几次，用于参考，进行调节
//		- KCP.Segment.length 数据的长度
//		- KCP.Segment.data 数据段，应用层要发送出去的数据。
//
//
//	# 2. kcp包头结构分析
//	
//	kcp发送的数据包设计了自己的包结构，包头一共24bytes，包含了一些必要的信息，具体内容和大小如下：
//	
//	|<------------ 4 bytes ------------>|
//	+--------+--------+--------+--------+
//	|  conv                             | conv：Conversation, 会话序号，用于标识收发数据包是否一致
//	+--------+--------+--------+--------+ cmd: Command, 指令类型，代表这个Segment的类型
//	|  cmd   |  frg   |			  wnd       | frg: Fragment, 分段序号，分段从大到小，0代表数据包接收完毕
//	+--------+--------+--------+--------+ wnd: Window, 窗口大小
//	|								 ts									| ts: Timestamp, 发送的时间戳
//	+--------+--------+--------+--------+
//	|								 sn									| sn: Sequence Number, Segment序号
//	+--------+--------+--------+--------+
//	|							   una								| una: Unacknowledged, 当前未收到的序号，
//	+--------+--------+--------+--------+      即代表这个序号之前的包均收到
//	|								 len                | len: Length, 后续数据的长度
//	+--------+--------+--------+--------+
//
//	包的结构可以在函数ikcp_encode_seg函数的编码过程中看出来
//=====================================================================
struct IKCPSEG
{
	struct IQUEUEHEAD node; // 节点用来串接多个 KCP segment，也就是前向后向指针；
	// 通用链表实现的队列.
	// node是一个通用链表，用于管理Segment队列，
	// 通用链表可以支持在不同类型的链表中做转移，
	// 通用链表实际上管理的就是一个最小的链表节点，
	// 具体该链表节点所在的数据块可以通过该数据块在链表中的位置反向解析出来。见 iqueue_entry 宏

	IUINT32 conv;     // Conversation, 会话序号: 接收到的数据包与发送的一致才接收此数据包
	IUINT32 cmd;      // Command, 指令类型: 代表这个Segment的类型
	IUINT32 frg;      // Fragment 记录了分片时的倒序序号, 当输出数据大于 MSS 时，需要将数据进行分片；
	IUINT32 wnd;      // Window, 己方的可用窗口大小, 也就是 rcv_queue 的可用大小即 rcv_wnd - nrcv_que, 见 ikcp_wnd_unused 函数
	IUINT32 ts;       // Timestamp, 记录了发送时的时间戳，用来估计 RTT
	IUINT32 sn;       // Sequence Number, Segment序号
	IUINT32 una;      // Unacknowledged, 当前未收到的序号: 即代表这个序号之前的包均收到
	IUINT32 len;      // Length, 数据长度
	IUINT32 resendts;	// 即 resend timestamp, 指定重发的时间戳，当当前时间超过这个时间时，则再重发一次这个包。
	IUINT32 rto;			// 即 Retransmit Timeout, 用于记录超时重传的时间间隔
	IUINT32 fastack;	// 记录ack跳过的次数，用于快速重传, 由函数 ikcp_parse_fastack 更新
	IUINT32 xmit;			// 记录发送的次数
	char data[1];			// 应用层要发送出去的数据
};


//---------------------------------------------------------------------
// IKCPCB
//
//	conv 会话ID
//	mtu	最大传输单元
//	mss	最大分片大小
//	state 连接状态（0xFFFFFFFF表示断开连接）
//	snd_una 第一个未确认的包
//	snd_nxt 下一个待分配的包的序号
//	rcv_nxt 待接收的下一个消息序号, 当把segment移出rcv_buf移入rcv_queue时, rcv_nxt会自增 
//	ssthresh 拥塞窗口阈值
//	rx_rttval	ack接收rtt浮动值
//	rx_srtt ack接收rtt静态值
//	rx_rto 由ack接收延迟计算出来的重传超时时间
//	rx_minrto 最小重传超时时间
//	snd_wnd	发送窗口大小, 一旦设置之后就不会变了, 默认32
//	rcv_wnd	接收窗口大小, 一旦设置之后就不会变了, 默认128
//	rmt_wnd, 远端接收窗口大小
//	cwnd, 拥塞窗口大小
//	probe 探查变量，IKCP_ASK_TELL表示告知远端窗口大小。IKCP_ASK_SEND表示请求远端告知窗口大小
//	interval	内部flush刷新间隔
//	ts_flush 下次flush刷新时间戳
//	nodelay	是否启动无延迟模式
//	updated 是否调用过update函数的标识
//	ts_probe 下次探查窗口的时间戳
//	probe_wait 探查窗口需要等待的时间
//	dead_link	最大重传次数
//	incr 可发送的最大数据量
//	
//	fastresend 触发快速重传的重复ack个数
//	nocwnd	取消拥塞控制
//	stream 是否采用流传输模式
//
//  nrcv_buf, nsnd_buf; // 收发缓存区中的Segment数量
//
//  nsnd_que // 发送队列snd_queue中的Segment数量
//	snd_queue	发送消息的队列
//
//	rcv_queue	接收消息的队列, rcv_queue的数据是连续的，rcv_buf可能是间隔的
//  nrcv_que // 接收队列rcv_queue中的Segment数量, 需要小于 rcv_wnd
//  rcv_queue 如下图所示
//	+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
//	...	|	2 |	3 |	4 |	............................................... 
//	+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
//							^												^												^				
//							|												|												|				
//					 rcv_nxt						rcv_nxt + nrcv_que			rcv_nxt + rcv_wnd		
//
//	snd_buf 发送消息的缓存
//  snd_buf 如下图所示
//	+---+---+---+---+---+---+---+---+---+---+---+---+---+
//	...	|	2 |	3 |	4 |	5 |	6 |	7 |	8 |	9 |	...........
//	+---+---+---+---+---+---+---+---+---+---+---+---+---+
//							^								^								^
//							|								|								|
//					 snd_una				 snd_nxt		snd_una + snd_wnd	
//
//
//	rcv_buf 接收消息的缓存
//  rcv_buf 如下图所示, rcv_queue的数据是连续的，rcv_buf可能是间隔的
//	+---+---+---+---+---+---+---+---+---+---+---+---+---+
//	...	|	2 |	4 |	6 |	7 |	8 |	9 |	...........
//	+---+---+---+---+---+---+---+---+---+---+---+---+---+	
//
//	acklist 待发送的ack列表
//	
//	buffer 存储消息字节流的内存
//	output udp发送消息的回调函数
//
//  IKCPCB用于管理整个kcp的工作过程，
//  内部维护了4条队列分别用于管理收发的数据，
//  以及一个ack数组记录ack的数据包。
//	内部还有一些关于重传RTO、流控、窗口大小的信息，
//---------------------------------------------------------------------
struct IKCPCB
{
	IUINT32 conv, mtu, mss, state;
	IUINT32 snd_una, snd_nxt, rcv_nxt;
	IUINT32 ts_recent, ts_lastack, ssthresh;
	IINT32 rx_rttval, rx_srtt, rx_rto, rx_minrto;
	IUINT32 snd_wnd, rcv_wnd, rmt_wnd, cwnd, probe;
	IUINT32 current, interval, ts_flush, xmit;
	IUINT32 nrcv_buf, nsnd_buf; // 收发缓存区中的Segment数量
	IUINT32 nrcv_que, nsnd_que; // 收发队列中的Segment数量
	IUINT32 nodelay, updated; // 非延迟ack，是否update(kcp需要上层通过不断的ikcp_update和ikcp_check来驱动kcp的收发过程)
	IUINT32 ts_probe, probe_wait;
	IUINT32 dead_link, incr;
	struct IQUEUEHEAD snd_queue; // 发送队列：send时将Segment放入
	struct IQUEUEHEAD rcv_queue; // 接收队列：recv时将接收缓冲区rcv_buf中的Segment移入接收队列
	struct IQUEUEHEAD snd_buf; // 发送缓冲区：update时将Segment从发送队列放入缓冲区
	struct IQUEUEHEAD rcv_buf; // 接收缓冲区：存放底层接收的数据Segment
	IUINT32 *acklist; // ack列表，所有收到的包ack将放在这里，依次存放sn和ts
	IUINT32 ackcount; // ack数量
	IUINT32 ackblock; // acklist大小
	void *user;
	char *buffer; // 存储消息字节流的内存
	int fastresend; // 触发快速重传的重复ack个数
	int nocwnd, stream; // 非退让流控、流模式
	int logmask;
	int(*output)(const char *buf, int len, struct IKCPCB *kcp, void *user); // 底层网络传输函数
	void(*writelog)(const char *log, struct IKCPCB *kcp, void *user);
};


typedef struct IKCPCB ikcpcb;

#define IKCP_LOG_OUTPUT			1
#define IKCP_LOG_INPUT			2
#define IKCP_LOG_SEND			4
#define IKCP_LOG_RECV			8
#define IKCP_LOG_IN_DATA		16
#define IKCP_LOG_IN_ACK			32
#define IKCP_LOG_IN_PROBE		64
#define IKCP_LOG_IN_WINS		128
#define IKCP_LOG_OUT_DATA		256
#define IKCP_LOG_OUT_ACK		512
#define IKCP_LOG_OUT_PROBE		1024
#define IKCP_LOG_OUT_WINS		2048

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------
// interface
//---------------------------------------------------------------------

// create a new kcp control object, 'conv' must equal in two endpoint
// from the same connection. 'user' will be passed to the output callback
// output callback can be setup like this: 'kcp->output = my_udp_output'
ikcpcb* ikcp_create(IUINT32 conv, void *user);

// release kcp control object
void ikcp_release(ikcpcb *kcp);

// set output callback, which will be invoked by kcp
void ikcp_setoutput(ikcpcb *kcp, int (*output)(const char *buf, int len, 
	ikcpcb *kcp, void *user));

// user/upper level recv: returns size, returns below zero for EAGAIN
int ikcp_recv(ikcpcb *kcp, char *buffer, int len);

// user/upper level send, returns below zero for error
int ikcp_send(ikcpcb *kcp, const char *buffer, int len);

// update state (call it repeatedly, every 10ms-100ms), or you can ask 
// ikcp_check when to call it again (without ikcp_input/_send calling).
// 'current' - current timestamp in millisec. 
void ikcp_update(ikcpcb *kcp, IUINT32 current);

// Determine when should you invoke ikcp_update:
// returns when you should invoke ikcp_update in millisec, if there 
// is no ikcp_input/_send calling. you can call ikcp_update in that
// time, instead of call update repeatly.
// Important to reduce unnacessary ikcp_update invoking. use it to 
// schedule ikcp_update (eg. implementing an epoll-like mechanism, 
// or optimize ikcp_update when handling massive kcp connections)
IUINT32 ikcp_check(const ikcpcb *kcp, IUINT32 current);

// when you received a low level packet (eg. UDP packet), call it
int ikcp_input(ikcpcb *kcp, const char *data, long size);

// flush pending data
void ikcp_flush(ikcpcb *kcp);

// check the size of next message in the recv queue
int ikcp_peeksize(const ikcpcb *kcp);

// change MTU size, default is 1400
int ikcp_setmtu(ikcpcb *kcp, int mtu);

// set maximum window size: sndwnd=32, rcvwnd=32 by default
int ikcp_wndsize(ikcpcb *kcp, int sndwnd, int rcvwnd);

// get how many packet is waiting to be sent
int ikcp_waitsnd(const ikcpcb *kcp);

// fastest: ikcp_nodelay(kcp, 1, 20, 2, 1)
// nodelay: 0:disable(default), 1:enable
// interval: internal update timer interval in millisec, default is 100ms 
// resend: 0:disable fast resend(default), 1:enable fast resend
// nc: 0:normal congestion control(default), 1:disable congestion control
int ikcp_nodelay(ikcpcb *kcp, int nodelay, int interval, int resend, int nc);


void ikcp_log(ikcpcb *kcp, int mask, const char *fmt, ...);

// setup allocator
void ikcp_allocator(void* (*new_malloc)(size_t), void (*new_free)(void*));

// read conv
IUINT32 ikcp_getconv(const void *ptr);


#ifdef __cplusplus
}
#endif

#endif


