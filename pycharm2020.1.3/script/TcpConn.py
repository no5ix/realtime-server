from __future__ import annotations

import time
import asyncio
import struct
import typing
from asyncio import transports
from asyncio.exceptions import CancelledError

from ConnBase import HEARTBEAT_TIMEOUT, HEARTBEAT_INTERVAL, ConnBase, RPC_HANDLER_ID_LEN, RECONNECT_MAX_TIMES, \
    RECONNECT_INTERVAL, CONN_STATE_CONNECTED, CONN_STATE_CONNECTING
from ConnMgr import PROTO_TYPE_TCP, ROLE_TYPE_ACTIVE
from common import gv
from core.common.IdManager import IdManager
from core.util.TimerHub import TimerHub
from core.util.UtilApi import wait_or_not

if typing.TYPE_CHECKING:
    from RpcHandler import RpcHandler

# from common import gr
# from core.common import MsgpackSupport
# from core.common.EntityFactory import EntityFactory
from core.mobilelog.LogManager import LogManager


class TcpConn(ConnBase):

    def __init__(
            self,
            role_type: int,
            addr: typing.Tuple[str, int],
            # asyncio_writer: asyncio.StreamWriter,
            # asyncio_reader: asyncio.StreamReader,
            rpc_handler: RpcHandler = None,
            close_cb: typing.Callable = lambda: None,
            is_proxy: bool = False,
            transport: transports.BaseTransport = None
    ):
        super(TcpConn, self).__init__(role_type, addr, rpc_handler, close_cb, is_proxy, transport)
        self._proto_type = PROTO_TYPE_TCP

    # @wait_or_not()
    async def try_connect(self) -> bool:
        self._conn_state = CONN_STATE_CONNECTING
        while self._try_connect_times < RECONNECT_MAX_TIMES:
            try:
                from TcpServer import TcpProtocol
                self._try_connect_times += 1
                transport, protocol = await gv.get_ev_loop().create_connection(
                    lambda: TcpProtocol(ROLE_TYPE_ACTIVE),
                    self._addr[0], self._addr[1])
            except Exception as e:
                self._logger.error(str(e))
                await asyncio.sleep(RECONNECT_INTERVAL)
                # if self._try_connect_times < RECONNECT_MAX_TIMES:
                self._logger.warning(f"try reconnect tcp: {str(self._addr)} ... {self._try_connect_times}")
            else:
                self._try_connect_times = 0
                self.set_connection_state(CONN_STATE_CONNECTED)
                self._transport = transport
                return True
        else:
            pass  # todo
            self._logger.error(f"try {RECONNECT_MAX_TIMES} times , still can't connect tcp remote addr: {self._addr}")
            self._try_connect_times = 0
            return False
