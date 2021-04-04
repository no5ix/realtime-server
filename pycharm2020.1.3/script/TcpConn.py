from __future__ import annotations
import asyncio
import struct
import typing
from asyncio.exceptions import CancelledError

from RpcHandler import RpcHandler
from core.util.UtilApi import wait_or_not

if typing.TYPE_CHECKING:
    from server_entity.ServerEntity import ServerEntity

# from common import gr
# from core.common import MsgpackSupport
# from core.common.EntityFactory import EntityFactory
from core.mobilelog.LogManager import LogManager


HEAD_LEN = 4
MAX_BODY_LEN = 4294967296


class TcpConn(object):

    def __init__(
            self, addr_str,
            asyncio_writer: asyncio.StreamWriter, asyncio_reader: asyncio.StreamReader,
            rpc_handler: RpcHandler = None):
        self.addr_str = addr_str
        # self.entity = entity  # type: typing.Type[ServerEntity]
        # self._entity = None  # type: typing.Union[ServerEntity, None]
        self.asyncio_writer = asyncio_writer  # type: asyncio.StreamWriter
        self.asyncio_reader = asyncio_reader  # type: asyncio.StreamReader

        self.send_cnt = 0
        self._recv_cnt = 0

        self._recv_data = b''
        self._logger = LogManager.get_logger(self.__class__.__name__)
        if rpc_handler:
            rpc_handler.set_conn(self)
            self._rpc_handler = rpc_handler
        else:
            self._rpc_handler = RpcHandler(self)

        # await self._loop()
        self.loop()

    # def set_entity(self, entity: ServerEntity):
    #     self._entity = entity
    #
    # def get_entity(self) -> ServerEntity:
    #     return self._entity

    # def set_asyncio_writer(self, asyncio_writer):
    #     self.asyncio_writer = asyncio_writer

    # def send_msg(self, msg):
    #     self.asyncio_writer.write(MsgpackSupport.encode(msg))

    def get_rpc_handler(self):
        return self._rpc_handler

    # def loop(self):
    #     return asyncio.create_task(self._loop())

    @wait_or_not
    async def loop(self):
        while True:
            try:
                # self.asyncio_writer
                _data = await self.asyncio_reader.read(8192)
                # self.logger.debug("_data")
                if _data == b"":
                    self._logger.debug("the peer has performed an orderly shutdown (recv 0 byte).")
                    self.handle_close()
                    break
                self._recv_data += _data
                while True:
                    _len_recv_data = len(self._recv_data)
                    if _len_recv_data < HEAD_LEN:
                        break
                    _body_len, = struct.unpack('i', self._recv_data[:HEAD_LEN])
                    _input_data_len = HEAD_LEN + _body_len
                    if _body_len > MAX_BODY_LEN or _body_len < 0:
                        self._logger.debug("body too big, Close the connection")
                        self.handle_close()
                        return
                    elif _len_recv_data >= _input_data_len:
                        _body_data = self._recv_data[HEAD_LEN:_input_data_len]
                        self._recv_cnt += 1
                        # self.logger.debug("self._recv_cnt:" + str(self._recv_cnt))
                        self.handle_message(_body_data)
                        self._recv_data = self._recv_data[_input_data_len:]
                    else:
                        break
            except (ConnectionResetError, ConnectionAbortedError, ConnectionRefusedError):
                self._logger.debug("connection is closed by remote client..with ConnectionResetError")
                self.handle_close()
                break
            except CancelledError:
                pass
            except:
                self._logger.log_last_except()

            # message = MsgpackSupport.decode(_data)
            # self.forward(self.asyncio_writer, addr, message)
            # await self.asyncio_writer.drain()
            # if message == "exit":
            #     message = f"{addr!r} wants to close the connection."
            #     self.logger.debug(message)
            #     self.forward(self.asyncio_writer, "Server", message)
            #     break
        # self.asyncio_writer.close()

    def handle_close(self, ):
        self.asyncio_writer.close()
        self._rpc_handler.fire_all_future_with_error("connection_closed")

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

    def send_data_and_count(self, data):
    # async def send_data_and_count(self, data):
        self.send_cnt += 1
        data_len = len(data) if data else 0
        header_data = struct.pack("i", data_len)
        data = header_data + data

        self.asyncio_writer.write(data)
        # await self.asyncio_writer.drain()
        # self.asyncio_writer.drain()
