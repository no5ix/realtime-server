from __future__ import annotations

import time
import asyncio
import struct
import typing
from asyncio import transports
from asyncio.exceptions import CancelledError

from ConnBase import ConnBase, RECONNECT_MAX_TIMES, RECONNECT_INTERVAL, \
    CONN_STATE_CONNECTING, CONN_STATE_CONNECTED, STRUCT_PACK_FORMAT, CONN_STATE_DISCONNECTED
from ConnMgr import ConnMgr, ROLE_TYPE_PASSIVE
from core.common.protocol_def import PROTO_TYPE_TCP, PROTO_TYPE_RUDP, RUDP_HANDSHAKE_SYN, RudpProtocol
from common import gv
from core.common import rudp
from core.common.IdManager import IdManager
from core.util.TimerHub import TimerHub
from core.util.UtilApi import wait_or_not

if typing.TYPE_CHECKING:
    from RpcHandler import RpcHandler

# from common import gr
# from core.common import MsgpackSupport
# from core.common.EntityFactory import EntityFactory
from core.mobilelog.LogManager import LogManager


KCP_UPDATE_TIMER_KEY = 'kcp_update_timer_key'


class RudpConn(ConnBase):

    def __init__(
            self,
            role_type: int,
            addr: typing.Tuple[str, int],
            # asyncio_writer: asyncio.StreamWriter,
            # asyncio_reader: asyncio.StreamReader,
            rpc_handler: RpcHandler = None,
            close_cb: typing.Callable = lambda: None,
            is_proxy: bool = False,
            transport: transports.BaseTransport = None,
            conv: int = None,
    ):
        super(RudpConn, self).__init__(role_type, addr, rpc_handler, close_cb, is_proxy, transport)
        self._proto_type = PROTO_TYPE_RUDP
        self._conv = conv
        if self._role_type == ROLE_TYPE_PASSIVE:
            self.init_kcp(conv)

    def init_kcp(self, conv):
        self._timer_hub.cancel_timer(key=KCP_UPDATE_TIMER_KEY)
        self._conv = conv
        self._kcp = rudp.Kcp(self._conv, self.send_data_internal)
        self._kcp.set_nodelay(nodelay=True, interval=10, resend=2, nocwnd=True)
        self.tick_kcp_update()

    async def try_connect(self) -> bool:
        self.set_connection_state(CONN_STATE_CONNECTING)
        self._transport, protocol = await gv.get_ev_loop().create_datagram_endpoint(
            lambda: RudpProtocol(),
            remote_addr=self._addr)
        while self._try_connect_times < RECONNECT_MAX_TIMES:
            self._try_connect_times += 1
            self._transport.sendto(RUDP_HANDSHAKE_SYN)

            fut = ConnMgr.instance().create_rudp_conned_fut(self._addr)
            try:
                conv = await asyncio.wait_for(
                    asyncio.shield(fut), timeout=RECONNECT_INTERVAL)
            except asyncio.exceptions.TimeoutError:
                self._logger.warning(f"try reconnect rudp: {str(self._addr)} ... {self._try_connect_times}")
            except Exception as e:
                self._logger.warning(f"try reconnect rudp: {str(e)} {str(self._addr)} ... {self._try_connect_times}")
            else:
                self.set_connection_state(CONN_STATE_CONNECTED)
                self.init_kcp(conv)
                self._try_connect_times = 0
                return True
        else:
            self._logger.error(f"try {RECONNECT_MAX_TIMES} times , still can't connect rudp remote addr: {self._addr}")
            self._try_connect_times = 0
            self.set_connection_state(CONN_STATE_DISCONNECTED)
            return False

    # @wait_or_not()
    def tick_kcp_update(self):
        now_ms = int(time.time() * 1000)
        self._kcp.update(now_ms)
        wait_sec = self._kcp.check(now_ms) / 1000
        # print(f"tickkkkkkkk, {wait_sec-now=}")

        self._timer_hub.call_at(wait_sec, self.tick_kcp_update, key=KCP_UPDATE_TIMER_KEY)

    def send_data_internal(self, kcp, data):
        assert self._transport
        self._transport.sendto(data, self._addr)

    def handle_read(self, _data):
        _input_res = self._kcp.input(_data)
        if _input_res < 0:
            self.handle_close(close_reason=f'kcp input error {_input_res=}')
            return
        elif _input_res == 0:
            while (_recv_data := self._kcp.recv()) is not None:
                super(RudpConn, self).handle_read(_recv_data)
            self._kcp.update(int(time.time() * 1000))

    def send_data_and_count(self, rpc_handler_id: bytes, data: bytes):
        self._send_cnt += 1
        rh_id_data = struct.pack(STRUCT_PACK_FORMAT, rpc_handler_id)  # type: bytes
        data = rh_id_data + data
        data_len = len(data) if data else 0
        header_data = struct.pack("i", data_len)
        data = header_data + data
        self._kcp.send(data)
