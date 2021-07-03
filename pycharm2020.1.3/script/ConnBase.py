from __future__ import annotations

import time
import asyncio
import struct
import typing
from asyncio import transports
from asyncio.exceptions import CancelledError

from ConnBase import HEARTBEAT_TIMEOUT, HEARTBEAT_INTERVAL, ROLE_TYPE_ACTIVE, ConnBase
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

HEARTBEAT_TIMEOUT = 8
HEARTBEAT_INTERVAL = 6

ROLE_TYPE_ACTIVE = 0
ROLE_TYPE_PASSIVE = 1

CONN_STATE_CONNECTING = 0
CONN_STATE_CONNECTED = 1
CONN_STATE_DISCONNECTING = 2
CONN_STATE_DISCONNECTED = 3

HEAD_LEN = 4
RPC_HANDLER_ID_LEN = 12
STRUCT_PACK_FORMAT = '12s'
MAX_BODY_LEN = 4294967296


class ConnBase:

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
        if role_type == ROLE_TYPE_ACTIVE:
            assert (rpc_handler is not None)
        self._is_connected = True

        self._role_type = role_type
        self._is_proxy = is_proxy
        self._addr = addr  # type: typing.Tuple[str, int]
        # self._asyncio_writer = asyncio_writer  # type: # asyncio.StreamWriter
        # self._asyncio_reader = asyncio_reader  # type: asyncio.StreamReader
        self._close_cb = close_cb

        self._transport = transport

        self._send_cnt = 0
        self._recv_cnt = 0
        self._recv_data = b''

        self._timer_hub = TimerHub()
        self._logger = LogManager.get_logger(self.__class__.__name__)
        self._rpc_handlers_map = {}  # type: typing.Dict[int, RpcHandler]

        if rpc_handler:
            rpc_handler.set_conn(self)
            self._rpc_handlers_map = {rpc_handler.rpc_handler_id: rpc_handler}

        # self.loop()

        self._last_heartbeat_ts = time.time()
        # todo:
        if not gv.is_dev_version:
            self._timer_hub.call_later(HEARTBEAT_TIMEOUT, self.handle_remote_heartbeat_timeout, repeat_count=-1)
            self._timer_hub.call_later(HEARTBEAT_INTERVAL, self.heartbeat, repeat_count=-1)

    def add_rpc_handler(self, rpc_handler: RpcHandler):
        self._rpc_handlers_map[rpc_handler.rpc_handler_id] = rpc_handler
        rpc_handler.set_conn(self)

    def remove_rpc_handler(self, rpc_handler_id):
        self._rpc_handlers_map.pop(rpc_handler_id, None)
        # if not self._rpc_handlers_map:
        #     self.handle_close(close_reason=f'has no rpc handler')

    def get_addr(self):
        return self._addr

    def is_connected(self):
        return self._is_connected

    def set_connection_state(self, is_conned):
        self._is_connected = is_conned

    def is_active_role(self):
        return self._role_type == ROLE_TYPE_ACTIVE

    def handle_remote_heartbeat_timeout(self):
        _now = time.time()
        _offset = _now - self._last_heartbeat_ts
        # self._logger.info(f"{self._addr=} { _offset=} {_now=}ckkkkcheck handle_remote_heartbeat_timeout")
        if _offset > HEARTBEAT_TIMEOUT:
            self.handle_close(close_reason="heartbeat timeout")

    def remote_heart_beat(self):
        self._last_heartbeat_ts = time.time()
        # self._logger.info(
        #     f"{self._addr=}, {self._last_heartbeat_ts}, remote_heart_beat")

    async def heartbeat(self):
        # self._logger.info(
        #     f"{self._addr=} heartbeat")
        for _, _rh in self._rpc_handlers_map.items():
            await _rh.send_heartbeat()

    # @wait_or_not()
    def handle_read(self, _data):
        # while True:
        try:
            # self._asyncio_writer
            # _data = await self._asyncio_reader.read(8192)
            # self.logger.info("_data")
            if _data == b"":
                self.handle_close("the peer has performed an orderly shutdown (recv 0 byte).")
                return
            self._recv_data += _data
            while True:
                _len_recv_data = len(self._recv_data)
                if _len_recv_data < (HEAD_LEN + RPC_HANDLER_ID_LEN):
                    break
                _body_len, = struct.unpack('i', self._recv_data[:HEAD_LEN])
                _input_data_len = HEAD_LEN + _body_len
                if _body_len > MAX_BODY_LEN or _body_len < 0:
                    self.handle_close("body too big, Close the connection")
                    return
                elif _len_recv_data >= _input_data_len:
                    _rpc_handler_id, = struct.unpack(
                        STRUCT_PACK_FORMAT, self._recv_data[HEAD_LEN: HEAD_LEN + RPC_HANDLER_ID_LEN])
                    _body_data = self._recv_data[HEAD_LEN + RPC_HANDLER_ID_LEN:_input_data_len]
                    self._recv_cnt += 1
                    # self.logger.info("self._recv_cnt:" + str(self._recv_cnt))
                    self.handle_message(_rpc_handler_id, _body_data)
                    self._recv_data = self._recv_data[_input_data_len:]
                else:
                    break
        except (
                ConnectionResetError,
                ConnectionAbortedError,
                # ConnectionRefusedError
        ) as e:
            self.handle_close(f"connection is closed by error: {str(e)}")
            return
        except CancelledError as e:
            self._logger.error(str(e))  # TODO
        except:
            self._logger.log_last_except()

    # @wait_or_not
    def handle_close(self, close_reason: str):
        if not self.is_connected():
            return
        self._logger.warning(f'peer addr: {self._addr}, {close_reason=}')
        self.set_connection_state(False)
        self._close_cb()
        # await self._asyncio_writer.drain()

        self._transport.close()
        for _, _rh in self._rpc_handlers_map.items():
            _rh.on_conn_close()
            # _rh.destroy()
        self._timer_hub.destroy()
        # gv.get_cur_server().remove_conn(self._addr)

    def handle_message(self, rpc_handler_id: bytes, msg_data: bytes):
        try:
            # rpc_message = self.do_decode(msg_data)
            _rh = self._rpc_handlers_map.get(rpc_handler_id, None)
            if _rh is None:
                try:
                    IdManager.bytes2id(rpc_handler_id)
                except:
                    self.handle_close(f"{rpc_handler_id=} err, Close the connection")
                    return
                from RpcHandler import RpcHandler
                from ProxyRpcHandler import ProxyCliRpcHandler
                _rh = ProxyCliRpcHandler(rpc_handler_id, self) if self._is_proxy else RpcHandler(rpc_handler_id, self)
                self._rpc_handlers_map[rpc_handler_id] = _rh
            _rh.handle_rpc(msg_data)
        except:
            self._logger.log_last_except()

    def send_data_and_count(self, rpc_handler_id: bytes, data: bytes):
        self._send_cnt += 1
        rh_id_data = struct.pack(STRUCT_PACK_FORMAT, rpc_handler_id)  # type: bytes
        data = rh_id_data + data
        data_len = len(data) if data else 0
        header_data = struct.pack("i", data_len)
        data = header_data + data

        # self._asyncio_writer.write(data)
        self._transport.write(data)
        # await self._asyncio_writer.drain()

