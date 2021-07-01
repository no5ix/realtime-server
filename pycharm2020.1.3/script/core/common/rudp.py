import struct
from collections import deque

IKCP_RTO_NDL = 30  # no delay min rto
IKCP_RTO_MIN = 100  # normal min rto
IKCP_RTO_DEF = 200
IKCP_RTO_MAX = 60000
IKCP_CMD_PUSH = 81  # cmd: push data
IKCP_CMD_ACK = 82  # cmd: ack
IKCP_CMD_WASK = 83  # cmd: window probe (ask)
IKCP_CMD_WINS = 84  # cmd: window size (tell)
IKCP_ASK_SEND = 1  # need to send IKCP_CMD_WASK
IKCP_ASK_TELL = 2  # need to send IKCP_CMD_WINS
IKCP_WND_SND = 32
IKCP_WND_RCV = 128  # must >= max fragment size
IKCP_MTU_DEF = 1400
IKCP_ACK_FAST = 3
IKCP_INTERVAL = 100
IKCP_OVERHEAD = 24
IKCP_DEADLINK = 20
IKCP_THRESH_INIT = 2
IKCP_THRESH_MIN = 2
IKCP_PROBE_INIT = 7000  # 7 secs to probe window size
IKCP_PROBE_LIMIT = 120000  # up to 120 secs to probe window

IKCP_PACKET_HEAD_FORMAT = '<IBBHIIII'


class KcpSeg(object):
    """
    KCP segment
    """

    # pylint: disable=too-many-instance-attributes
    # pylint: disable=too-few-public-methods
    # pylint: disable=invalid-name

    __slots__ = (
        'conv', 'cmd', 'frg', 'wnd', 'ts', 'sn',
        'una', 'len', 'resendts', 'rto', 'fastack',
        'xmit', 'data'
    )

    def __init__(self, conv):
        self.conv = conv
        self.cmd = 0
        self.frg = 0
        self.wnd = 0
        self.ts = 0
        self.sn = 0
        self.una = 0
        self.len = 0
        self.resendts = 0
        self.rto = 0
        self.fastack = 0
        self.xmit = 0
        self.data = None

    def encode(self):
        """
        Encode KCP packet head
        """
        return struct.pack(IKCP_PACKET_HEAD_FORMAT, self.conv, self.cmd, self.frg, \
                           self.wnd, self.ts, self.sn, self.una, self.len)

    @classmethod
    def decode(cls, data):
        """
        Decode KCP packet head
        """
        assert len(data) >= IKCP_OVERHEAD
        conv, cmd, frg, wnd, ts, sn, una, length = \
            struct.unpack(IKCP_PACKET_HEAD_FORMAT, data[:IKCP_OVERHEAD])
        seg = cls(conv)
        seg.cmd = cmd
        seg.frg = frg
        seg.wnd = wnd
        seg.ts = ts
        seg.sn = sn
        seg.una = una
        seg.len = length
        return seg


class Kcp(object):
    """
    KCP
    """

    # pylint: disable=too-many-instance-attributes
    # pylint: disable=too-many-public-methods
    # pylint: disable=invalid-name

    __slots__ = (
        'conv', 'mtu', 'mss', 'state',
        'snd_una', 'snd_nxt', 'rcv_nxt',
        'ts_recent', 'ts_lastack', 'ssthresh',
        'rx_rttval', 'rx_srtt', 'rx_rto', 'rx_minrto',
        'snd_wnd', 'rcv_wnd', 'rmt_wnd', 'cwnd', 'probe',
        'current', 'interval', 'ts_flush', 'xmit',
        'nrcv_buf', 'nsnd_buf',
        'nrcv_que', 'nsnd_que',
        'nodelay', 'updated',
        'ts_probe', 'probe_wait',
        'dead_link', 'incr',
        'snd_queue',
        'rcv_queue',
        'snd_buf',
        'rcv_buf',
        'acklist',
        'ackcount',
        'ackblock',
        'fastresend',
        'nocwnd', 'stream',
        'output_func'
    )

    def __init__(self, conv, output):
        self.conv = conv
        self.snd_una = 0
        self.snd_nxt = 0
        self.rcv_nxt = 0
        self.ts_recent = 0
        self.ts_lastack = 0
        self.ts_probe = 0
        self.probe_wait = 0
        self.snd_wnd = IKCP_WND_SND
        self.rcv_wnd = IKCP_WND_RCV
        self.rmt_wnd = IKCP_WND_RCV
        self.cwnd = 0
        self.incr = 0
        self.probe = 0
        self.mtu = IKCP_MTU_DEF
        self.mss = self.mtu - IKCP_OVERHEAD
        self.stream = False
        self.snd_queue = deque()
        self.rcv_queue = deque()
        self.snd_buf = deque()
        self.rcv_buf = deque()
        self.nrcv_buf = 0
        self.nsnd_buf = 0
        self.nrcv_que = 0
        self.nsnd_que = 0
        self.state = 0
        self.acklist = []
        self.ackblock = 0
        self.ackcount = 0
        self.rx_srtt = 0
        self.rx_rttval = 0
        self.rx_rto = IKCP_RTO_DEF
        self.rx_minrto = IKCP_RTO_MIN
        self.current = 0
        self.interval = IKCP_INTERVAL
        self.ts_flush = IKCP_INTERVAL
        self.nodelay = False
        self.updated = False
        self.ssthresh = IKCP_THRESH_INIT
        self.fastresend = 0
        self.nocwnd = False
        self.xmit = 0
        self.dead_link = IKCP_DEADLINK
        assert callable(output)
        self.output_func = output

    def peeksize(self):
        """
        peek data size
        """
        if not self.rcv_queue:
            return -1

        seg = self.rcv_queue[0]

        if seg.frg == 0:
            return seg.len

        if self.nrcv_que < seg.frg + 1:
            return -1

        length = 0
        for seg in self.rcv_queue:
            length += seg.len
            if seg.frg == 0:
                break
        return length

    def recv(self):
        """
        recv
        """
        data = b''
        if not self.rcv_queue:
            return None

        peek_size = self.peeksize()
        if peek_size < 0:
            return None

        recover = self.nrcv_que >= self.rcv_wnd

        # merge fragment
        length = 0
        fragment = 0
        while self.rcv_queue:
            seg = self.rcv_queue.popleft()
            length += seg.len
            data += seg.data
            self.nrcv_que -= 1
            fragment = seg.frg
            del seg
            if fragment == 0:
                break

        assert length == peek_size

        # move available data from rcv_buf to rcv_queue
        while self.rcv_buf:
            seg = self.rcv_buf.popleft()
            if seg.sn == self.rcv_nxt and self.nrcv_que < self.rcv_wnd:
                self.nrcv_buf -= 1
                self.rcv_queue.append(seg)
                self.nrcv_que += 1
                self.rcv_nxt += 1
            else:
                self.rcv_buf.appendleft(seg)
                break

        # fast recover
        if self.nrcv_que < self.rcv_wnd and recover:
            self.probe |= IKCP_ASK_TELL

        return data

    def send(self, data):
        """
        send
        """
        assert self.mss > 0
        assert isinstance(data, bytes), 'Send must be bytes'

        length = len(data)

        # append to previous segment in streaming mode if possible
        if self.stream:
            if self.snd_queue:
                seg = self.snd_queue[-1]
                if seg.len < int(self.mss):
                    capacity = int(self.mss) - seg.len
                    extend = capacity
                    if length < capacity:
                        extend = length
                    seg.data += data
                    seg.len += extend
                    seg.frg = 0
                    length -= extend

            if length <= 0:
                return None

        count = 1
        if length > int(self.mss):
            count = int((length + int(self.mss) - 1) / int(self.mss))

        if count >= IKCP_WND_RCV:
            return -2

        if count == 0:
            count = 1

        for i in range(count):
            new_seg = KcpSeg(self.conv)
            new_seg.len = length
            if length > int(self.mss):
                new_seg.len = int(self.mss)
            new_seg.data = data[:new_seg.len]
            new_seg.frg = 0
            if not self.stream:
                new_seg.frg = count - i - 1
            self.snd_queue.append(new_seg)
            self.nsnd_que += 1
            length -= new_seg.len

        return 0

    def update_ack(self, rtt):
        """
        Parse ack
        """
        rto = 0
        if self.rx_srtt == 0:
            self.rx_srtt = rtt
            self.rx_rttval = int(rtt / 2)
        else:
            delta = abs(rtt - self.rx_srtt)
            self.rx_rttval = int((3 * self.rx_rttval + delta) / 4)
            self.rx_srtt = int((7 * self.rx_srtt + rtt) / 8)
            if self.rx_srtt < 1:
                self.rx_srtt = 1

        rto = self.rx_srtt + max(self.interval, 4 * self.rx_rttval)
        self.rx_rto = min(max(self.rx_minrto, rto), IKCP_RTO_MAX)

    def shrink_buf(self):
        """
        Parse ack
        """
        if self.snd_buf:
            self.snd_una = self.snd_buf[0].sn
        else:
            self.snd_una = self.snd_nxt

    def parse_ack(self, sn):
        """
        Parse ACK
        """
        if sn - self.snd_una < 0 or sn - self.snd_nxt >= 0:
            return

        rm_seg = None
        for seg in self.snd_buf:
            if sn == seg.sn:
                rm_seg = seg
                self.nsnd_buf -= 1
                break
            if sn - seg.sn < 0:
                break
        if rm_seg:
            self.snd_buf.remove(rm_seg)

    def parse_una(self, una):
        """
        Parse UNA
        """
        while self.snd_buf:
            seg = self.snd_buf.popleft()
            if una - seg.sn > 0:
                del seg
                self.nsnd_buf -= 1
            else:
                self.snd_buf.appendleft(seg)
                break

    def parse_fastack(self, sn):
        """
        Parse fast ACK
        """
        if sn - self.snd_una < 0 or sn - self.snd_nxt >= 0:
            return

        for seg in self.snd_buf:
            if sn - seg.sn < 0:
                break
            elif sn != seg.sn:
                seg.fastack += 1

    def parse_data(self, newseg):
        """
        Parse data
        """
        sn = newseg.sn
        repeat = False
        if sn - (self.rcv_nxt + self.rcv_wnd) >= 0 or sn - self.rcv_nxt < 0:
            del newseg
            return

        tmp_deque = deque()
        while self.rcv_buf:
            seg = self.rcv_buf.pop()
            tmp_deque.appendleft(seg)
            if seg.sn == sn:
                repeat = True
                break
            if sn - seg.sn > 0:
                break

        if repeat:
            del newseg
        else:
            tmp_deque.appendleft(newseg)
            self.nrcv_buf += 1

        if tmp_deque:
            self.rcv_buf.extend(tmp_deque)

        while self.rcv_buf:
            seg = self.rcv_buf.popleft()
            if seg.sn == self.rcv_nxt and self.nrcv_que < self.rcv_wnd:
                self.nrcv_buf -= 1
                self.rcv_queue.append(seg)
                self.nrcv_que += 1
                self.rcv_nxt += 1
            else:
                break

    def output(self, data):
        """
        Output
        """
        self.output_func(self, data)

    def update(self, current):
        """
        update
        """
        current &= 0xffffffff
        self.current = current
        if not self.updated:
            self.updated = True
            self.ts_flush = self.current

        slap = self.current - self.ts_flush

        if slap >= 10000 or slap < -10000:
            self.ts_flush = self.current
            slap = 0

        if slap >= 0:
            self.ts_flush += self.interval
            if self.current - self.ts_flush >= 0:
                self.ts_flush = self.current + self.interval
            self.flush()

    def check(self, now):
        """
        check
        """
        current = now & 0xffffffff
        ts_flush = self.ts_flush
        tm_flush = 0x7fffffff
        tm_packet = 0x7fffffff

        if not self.updated:
            return now

        tm_flush = current - ts_flush
        if tm_flush >= 10000 or tm_flush < -10000:
            ts_flush = current

        if tm_flush >= 0:
            return now

        tm_flush = ts_flush - current

        for seg in self.snd_buf:
            diff = seg.resendts - current
            if diff <= 0:
                return now
            if diff < tm_packet:
                tm_packet = diff

        return now + min(tm_flush, tm_packet, self.interval)

    def input(self, data):
        """
        input
        """

        # pylint: disable=too-many-branches
        # pylint: disable=too-many-statements

        assert isinstance(data, bytes), 'Input must be bytes'

        una = self.snd_una
        maxack = 0
        flag = False
        size = len(data)

        if not data or size < IKCP_OVERHEAD:
            return -1

        while True:
            if size < IKCP_OVERHEAD:
                break

            seg = KcpSeg.decode(data)

            if seg.conv != self.conv:
                return -1

            size -= IKCP_OVERHEAD
            if size < seg.len or seg.len < 0:
                return -2

            if seg.cmd not in (IKCP_CMD_PUSH, IKCP_CMD_ACK, IKCP_CMD_WASK, IKCP_CMD_WINS):
                return -3

            self.rmt_wnd = seg.wnd
            self.parse_una(seg.una)
            self.shrink_buf()

            if seg.cmd == IKCP_CMD_ACK:
                if self.current - seg.ts >= 0:
                    self.update_ack(self.current - seg.ts)
                self.parse_ack(seg.sn)
                self.shrink_buf()
                if not flag:
                    flag = True
                    maxack = seg.sn
                elif seg.sn - maxack > 0:
                    maxack = seg.sn

            elif seg.cmd == IKCP_CMD_PUSH:
                if seg.sn - (self.rcv_nxt + self.rcv_wnd) < 0:
                    self.acklist.append((seg.sn, seg.ts))
                    if seg.sn - self.rcv_nxt >= 0:
                        seg.data = data[IKCP_OVERHEAD:IKCP_OVERHEAD + seg.len]
                        self.parse_data(seg)

            elif seg.cmd == IKCP_CMD_WASK:
                self.probe |= IKCP_ASK_TELL

            elif seg.cmd == IKCP_CMD_WINS:
                pass

            else:
                return -3

            data = data[IKCP_OVERHEAD + seg.len:]
            size -= seg.len

        if flag:
            self.parse_fastack(maxack)

        if self.snd_una - una > 0:
            if self.cwnd < self.rmt_wnd:
                mss = self.mss
                if self.cwnd < self.ssthresh:
                    self.cwnd += 1
                    self.incr += mss
                else:
                    if self.incr < mss:
                        self.incr = mss
                    self.incr += int(mss * mss) / self.incr + int(mss / 16)
                    if (self.cwnd + 1) * mss <= self.incr:
                        self.cwnd += 1
                if self.cwnd > self.rmt_wnd:
                    self.cwnd = self.rmt_wnd
                    self.incr = self.rmt_wnd * mss

        return 0

    def wnd_unused(self):
        """
        WND unused
        """
        return max(self.rcv_wnd - self.nrcv_que, 0)

    def flush(self):
        """
        flush
        """

        # pylint: disable=too-many-branches
        # pylint: disable=too-many-statements

        current = self.current
        change = False
        lost = False
        if not self.updated:
            return

        seg = KcpSeg(self.conv)
        seg.cmd = IKCP_CMD_ACK
        seg.frg = 0
        seg.wnd = self.wnd_unused()
        seg.una = self.rcv_nxt
        seg.len = 0
        seg.sn = 0
        seg.ts = 0

        data = b''
        for sn, ts in self.acklist:
            seg.sn = sn
            seg.ts = ts
            data += seg.encode()
            if len(data) + IKCP_OVERHEAD > self.mtu:
                self.output(data)
                data = b''

        self.acklist = []

        if self.rmt_wnd == 0:
            if self.probe_wait == 0:
                self.probe_wait = IKCP_PROBE_INIT
                self.ts_probe = self.current + self.probe_wait
            else:
                if self.current - self.ts_probe >= 0:
                    self.probe_wait = min(
                        self.probe_wait + int(max(self.probe_wait, IKCP_PROBE_INIT) / 2),
                        IKCP_PROBE_LIMIT)
                    self.ts_probe = self.current + self.probe_wait
                    self.probe |= IKCP_ASK_SEND
        else:
            self.ts_probe = 0
            self.probe_wait = 0

        if self.probe & IKCP_ASK_SEND != 0:
            seg.cmd = IKCP_CMD_WASK
            data += seg.encode()
            if len(data) + IKCP_OVERHEAD > self.mtu:
                self.output(data)
                data = b''

        self.probe = 0

        cwnd = min(self.snd_wnd, self.rmt_wnd)
        if not self.nocwnd:
            cwnd = min(self.cwnd, cwnd)

        while self.snd_nxt - (self.snd_una + cwnd) < 0:
            if not self.snd_queue:
                break
            newseg = self.snd_queue.popleft()
            self.snd_buf.append(newseg)
            self.nsnd_que -= 1
            self.nsnd_buf += 1

            newseg.conv = self.conv
            newseg.cmd = IKCP_CMD_PUSH
            newseg.wnd = seg.wnd
            newseg.ts = current
            newseg.sn = self.snd_nxt
            self.snd_nxt += 1
            newseg.una = self.rcv_nxt
            newseg.resendts = current
            newseg.rto = self.rx_rto
            newseg.fastack = 0
            newseg.xmit = 0

        resent = 0xffffffff
        if self.fastresend > 0:
            resent = self.fastresend

        rtomin = 0
        if not self.nodelay:
            rtomin = self.rx_rto >> 3

        for segment in self.snd_buf:
            needsend = False
            if segment.xmit == 0:
                needsend = True
                segment.xmit += 1
                segment.rto = self.rx_rto
                segment.resendts = current + segment.rto + rtomin
            elif current - segment.resendts >= 0:
                needsend = True
                segment.xmit += 1
                self.xmit += 1
                if not self.nodelay:
                    segment.rto += self.rx_rto
                else:
                    segment.rto += int(self.rx_rto / 2)
                segment.resendts = current + segment.rto
                lost = True
            elif segment.fastack >= resent:
                needsend = True
                segment.xmit += 1
                segment.fastack = 0
                segment.resendts = current + segment.rto
                change = True

            if needsend:
                segment.ts = current
                segment.wnd = seg.wnd
                segment.una = self.rcv_nxt
                if len(data) + segment.len + IKCP_OVERHEAD > self.mtu:
                    self.output(data)
                    data = b''

                data += segment.encode()
                data += segment.data

                if segment.xmit >= self.dead_link:
                    self.state = -1

        if data:
            self.output(data)

        if change:
            inflight = self.snd_nxt - self.snd_una
            self.ssthresh = int(inflight / 2)
            if self.ssthresh < IKCP_THRESH_MIN:
                self.ssthresh = IKCP_THRESH_MIN
            self.cwnd = self.ssthresh + resent
            self.incr = self.cwnd * self.mss

        if lost:
            self.ssthresh = int(cwnd / 2)
            if self.ssthresh < IKCP_THRESH_MIN:
                self.ssthresh = IKCP_THRESH_MIN
            self.cwnd = 1
            self.incr = self.mss

        if self.cwnd < 1:
            self.cwnd = 1
            self.incr = self.mss

    def set_mut(self, mtu):
        """
        Set mut
        """
        if mtu < 50 or mtu < IKCP_OVERHEAD:
            raise ValueError
        self.mtu = mtu
        self.mss = self.mtu - IKCP_OVERHEAD

    def set_interval(self, interval):
        """
        Set interval
        """
        # interval = max(10, min(5000, interval))  # todo: uncomment
        self.interval = interval

    def set_wndsize(self, sndwnd=32, rcvwnd=32):
        """
        set maximum window size: sndwnd=32, rcvwnd=32 by default
        """
        if sndwnd > 0:
            self.snd_wnd = sndwnd
        if rcvwnd > 0:
            self.rcv_wnd = max(rcvwnd, IKCP_WND_RCV)

    def waitsnd(self):
        """
        get how many packet is waiting to be sent
        """
        return self.nsnd_buf + self.nsnd_que

    def set_nodelay(self, nodelay=None, interval=100, resend=0, nocwnd=False):
        """
        nodelay: False=disable; True=enable; default=False
        interval: internal update timer interval in millisec, default is 100ms
        resend: 0=disable fast resend; 1=enable fast resend; default=0
        nocwnd: False=normal congestion control;\
                True=disable congestion control; default=False
        """
        if nodelay is not None:
            self.nodelay = nodelay
            if nodelay:
                self.rx_minrto = 1  # todo: del
                # self.rx_minrto = IKCP_RTO_NDL
            else:
                self.rx_minrto = IKCP_RTO_MIN

        if interval >= 0:
            # self.set_interval(interval)
            self.set_interval(1)  # todo: del

        if resend >= 0:
            self.fastresend = resend

        if nocwnd is not None:
            self.nocwnd = nocwnd
