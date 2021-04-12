from __future__ import annotations

import time
import asyncio
import struct
import typing
from asyncio.exceptions import CancelledError

from common import gv
from core.util.TimerHub import TimerHub
from core.util.UtilApi import wait_or_not, async_lock

if typing.TYPE_CHECKING:
    from server_entity.ServerEntity import ServerEntity
    from RpcHandler import RpcHandler

# from common import gr
# from core.common import MsgpackSupport
# from core.common.EntityFactory import EntityFactory
from core.mobilelog.LogManager import LogManager


HEAD_LEN = 4
MAX_BODY_LEN = 4294967296

HEARTBEAT_TIMEOUT = 8
HEARTBEAT_INTERVAL = 6

ROLE_TYPE_ACTIVE = 0
ROLE_TYPE_PASSIVE = 1

CONN_STATE_CONNECTING = 0
CONN_STATE_CONNECTED = 1
CONN_STATE_DISCONNECTING = 2
CONN_STATE_DISCONNECTED = 3


class TcpConn(object):

    def __init__(
            self,
            role_type: int,
            addr: typing.Tuple[str, int],
            asyncio_writer: asyncio.StreamWriter,
            asyncio_reader: asyncio.StreamReader,
            rpc_handler: RpcHandler = None,
            close_cb: typing.Callable = lambda: None,
    ):
        if role_type == ROLE_TYPE_ACTIVE:
            assert(rpc_handler is not None)
        self._is_connected = True

        self._role_type = role_type
        self._addr = addr  # type: typing.Tuple[str, int]
        self._asyncio_writer = asyncio_writer  # type: asyncio.StreamWriter
        self._asyncio_reader = asyncio_reader  # type: asyncio.StreamReader
        self._close_cb = close_cb

        self._send_cnt = 0
        self._recv_cnt = 0
        self._recv_data = b''

        self._timer_hub = TimerHub()
        self._logger = LogManager.get_logger(self.__class__.__name__)
        if rpc_handler:
            rpc_handler.set_conn(self)
            self._rpc_handler = rpc_handler
        else:
            from RpcHandler import RpcHandler
            self._rpc_handler = RpcHandler(self)

        self.loop()

        self._last_heartbeat_ts = 0
        self._timer_hub.call_later(HEARTBEAT_TIMEOUT, self.handle_remote_heartbeat_timeout, repeat_count=-1)
        self._timer_hub.call_later(HEARTBEAT_INTERVAL, self.heartbeat, repeat_count=-1)

    def get_addr(self):
        return self._addr

    def is_connected(self):
        return self._is_connected

    def set_connection_state(self, is_conned):
        self._is_connected = is_conned

    def is_active(self):
        return self._role_type == ROLE_TYPE_ACTIVE

    async def handle_remote_heartbeat_timeout(self):
        # print("ckkkkcheck handle_remote_heartbeat_timeout")
        if time.time() - self._last_heartbeat_ts > HEARTBEAT_TIMEOUT:
            self.handle_close(close_reason="heartbeat timeout")

    def remote_heart_beat(self):
        self._last_heartbeat_ts = time.time()

    async def heartbeat(self):
        await self._rpc_handler.send_heartbeat()

    # def set_entity(self, entity: ServerEntity):
    #     self._entity = entity
    #
    # def get_entity(self) -> ServerEntity:
    #     return self._entity

    # def set_asyncio_writer(self, _asyncio_writer):
    #     self._asyncio_writer = _asyncio_writer

    # def send_msg(self, msg):
    #     self._asyncio_writer.write(MsgpackSupport.encode(msg))

    def get_rpc_handler(self):
        return self._rpc_handler

    # def loop(self):
    #     return asyncio.create_task(self._loop())

    @wait_or_not()
    async def loop(self):
        while True:
            try:
                # self._asyncio_writer
                _data = await self._asyncio_reader.read(8192)
                # self.logger.debug("_data")
                if _data == b"":
                    self.handle_close("the peer has performed an orderly shutdown (recv 0 byte).")
                    return
                self._recv_data += _data
                while True:
                    _len_recv_data = len(self._recv_data)
                    if _len_recv_data < HEAD_LEN:
                        break
                    _body_len, = struct.unpack('i', self._recv_data[:HEAD_LEN])
                    _input_data_len = HEAD_LEN + _body_len
                    if _body_len > MAX_BODY_LEN or _body_len < 0:
                        self.handle_close("body too big, Close the connection")
                        return
                    elif _len_recv_data >= _input_data_len:
                        _body_data = self._recv_data[HEAD_LEN:_input_data_len]
                        self._recv_cnt += 1
                        # self.logger.debug("self._recv_cnt:" + str(self._recv_cnt))
                        self.handle_message(_body_data)
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

            # message = MsgpackSupport.decode(_data)
            # self.forward(self._asyncio_writer, addr, message)
            # await self._asyncio_writer.drain()
            # if message == "exit":
            #     message = f"{addr!r} wants to close the connection."
            #     self.logger.debug(message)
            #     self.forward(self._asyncio_writer, "Server", message)
            #     break
        # self._asyncio_writer.close()

    # @wait_or_not
    def handle_close(self, close_reason: str):
        if not self.is_connected():
            return
        self._logger.warning(close_reason)
        self.set_connection_state(False)
        self._close_cb()
        # await self._asyncio_writer.drain()
        self._asyncio_writer.close()
        self._rpc_handler.on_conn_close()
        self._timer_hub.destroy()
        # gv.get_cur_server().remove_conn(self._addr)

    def handle_message(self, msg_data):
        try:
            # rpc_message = self.do_decode(msg_data)
            self._rpc_handler.handle_rpc(msg_data)
        except:
            self._logger.log_last_except()

    # def handle_rpc(self, rpc_msg):
    #     _entity_type_str, _method_name, _parameters = rpc_msg
    #     if self._entity is None:
    #         self._entity = gr.get_server_singleton(_entity_type_str)
    #         if self._entity is None:
    #             self._entity = EntityFactory.instance().create_entity(_entity_type_str)
    #         self._entity.set_connection(self)
    #     _method = getattr(self._entity, _method_name, None)
    #
    #     if not _method:
    #         self._logger.error("entity:%s  method:%s not exist", self._entity, _method_name)
    #         return
    #     try:
    #         _method(_parameters)
    #     except:
    #         self._logger.log_last_except()

    # def request_rpc(
    #         # self, address, service_id, method_name, args=[], service_id_type=0, method_name_type=0,
    #         # self, entity_type,
    #         # method_name, args=None,
    #         # method_name_type=0,
    #         # need_reply=False, timeout=2
    #         self, *args, **kwargs
    # ):
    #     self._rpc_handler.request_rpc(*args, **kwargs)
        # message = [RPC_REQUEST, service_id_type, service_id, method_name_type, method_name, args]
        # message = [entity_type, method_name, args]
        # try:
        #     data = self.do_encode(message)
        #     # if conn is None:
        #     #     conn = gr.get_cur_server().get_conn_by_addr(ip_port_tuple)
        # except:
        #     # self.logger.error("encode request message error")
        #     self._logger.log_last_except()
        #     # self.handle_traceback()
        #     self._logger.debug("encode request message error")
        # else:
        #     # con.send_data_and_count(data)
        #     # if gr.flow_backups:
        #     #     gr.flow_msg('[BATTLE] NET UP ', len(data), message)
        #     # await self.send_data_and_count(data)
        #     self.send_data_and_count(data)
        #     # _task = asyncio.create_task(self.send_data_and_count(data))
        # # return _task

    # @staticmethod
    # def do_decode(msg):
    #     return MsgpackSupport.decode(msg)
    #
    # @staticmethod
    # def do_encode(message):
    #     return MsgpackSupport.encode(message)

    # @async_lock
    # async def send_data_and_count(self, data: bytes):
    def send_data_and_count(self, data: bytes):
        self._send_cnt += 1
        data_len = len(data) if data else 0
        header_data = struct.pack("i", data_len)
        data = header_data + data

        self._asyncio_writer.write(data)
        # await self._asyncio_writer.drain()
