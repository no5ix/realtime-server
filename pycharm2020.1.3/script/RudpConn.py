from __future__ import annotations

import time
import asyncio
import struct
import typing
from asyncio import transports
from asyncio.exceptions import CancelledError

from ConnBase import HEARTBEAT_TIMEOUT, HEARTBEAT_INTERVAL, ROLE_TYPE_ACTIVE, ConnBase
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


HEAD_LEN = 4
RPC_HANDLER_ID_LEN = 12
STRUCT_PACK_FORMAT = '12s'
MAX_BODY_LEN = 4294967296


class RudpConn(ConnBase):

    def __init__(
            self,
            conv: int,
            role_type: int,
            addr: typing.Tuple[str, int],
            # asyncio_writer: asyncio.StreamWriter,
            # asyncio_reader: asyncio.StreamReader,
            rpc_handler: RpcHandler = None,
            close_cb: typing.Callable = lambda: None,
            is_proxy: bool = False,
            transport: transports.BaseTransport = None
    ):
        super(RudpConn, self).__init__(role_type, addr, rpc_handler, close_cb, is_proxy, transport)
        self._conv = conv

        self._kcp = rudp.Kcp(conv, self.send_data_internal)
        self._kcp.set_nodelay(nodelay=True, interval=10, resend=2, nocwnd=True)
        self.tick_kcp_update()

    # @wait_or_not()
    def tick_kcp_update(self):
        now = time.time()
        self._kcp.update(int(time.time() * 1000))
        wait_sec = self._kcp.check(int(time.time() * 1000)) / 1000
        # print(f"tickkkkkkkk, {wait_sec-now=}")

        self._timer_hub.call_at(wait_sec, self.tick_kcp_update)

    def send_data_internal(self, kcp, data):
        assert self._transport
        self._transport.sendto(data, self._addr)
