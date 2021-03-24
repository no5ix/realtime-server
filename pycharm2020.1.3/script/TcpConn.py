import asyncio
import typing

from struct import unpack as s_unpack, pack as s_pack
from core.common import MsgpackSupport
from core.common.EntityFactory import EntityFactory
from core.mobilelog.LogManager import LogManager

HEAD_LEN = 4
MAX_BODY_LEN = 4294967296


class TcpConn(object):

    def __init__(self, addr_str, asyncio_writer, asyncio_reader):
        self.addr_str = addr_str
        # self.entity = entity  # type: typing.Type[ServerEntity]
        from server_entity.ServerEntity import ServerEntity
        self._entity = None  # type: typing.Union[ServerEntity, None]
        self.asyncio_writer = asyncio_writer
        self.asyncio_reader = asyncio_reader

        self.send_cnt = 0
        self._recv_cnt = 0

        self._recv_data = b''
        self._logger = LogManager.get_logger(self.__class__.__name__)

    def set_entity(self, entity):
        self._entity = entity

    def set_asyncio_writer(self, asyncio_writer):
        self.asyncio_writer = asyncio_writer

    def send_msg(self, msg):
        self.asyncio_writer.write(MsgpackSupport.encode(msg))

    def loop(self):
        return asyncio.create_task(self._loop())

    async def _loop(self):
        while True:
            try:
                # self.asyncio_writer
                _data = await self.asyncio_reader.read(8192)
                # self._logger.debug("_data")
                if _data == b"":
                    self.asyncio_writer.close()
                    self._logger.debug("the peer has performed an orderly shutdown (recv 0 byte).")
                    break
                self._recv_data += _data
                while True:
                    _len_recv_data = len(self._recv_data)
                    if _len_recv_data < HEAD_LEN:
                        break
                    _body_len, = s_unpack('i', self._recv_data[:HEAD_LEN])
                    _input_data_len = HEAD_LEN + _body_len
                    if _body_len > MAX_BODY_LEN or _body_len < 0:
                        self._logger.debug("body too big, Close the connection")
                        self.asyncio_writer.close()
                        return
                    elif _len_recv_data >= _input_data_len:
                        _body_data = self._recv_data[HEAD_LEN:_input_data_len]
                        self._recv_cnt += 1
                        # self._logger.debug("self._recv_cnt:" + str(self._recv_cnt))
                        self.handle_message(_body_data)
                        self._recv_data = self._recv_data[_input_data_len:]
                    else:
                        break
            except (ConnectionResetError, ):
                self.asyncio_writer.close()
                # TODO: not safe, handle conn closed
                self._logger.debug("connection is closed by remote client..with ConnectionResetError")
                break
            except ConnectionAbortedError:
                self.asyncio_writer.close()
                # TODO: not safe, handle conn closed
                self._logger.debug("connection is closed by remote client..with ConnectionAbortedError")
                break
            # except ConnectionError:
            #     self.asyncio_writer.close()
            #     # TODO: not safe, handle conn closed
            #     self._logger.debug("connection is closed by remote client..")
            #     break
            except ConnectionRefusedError:
                self.asyncio_writer.close()
                # TODO: not safe, handle conn closed
                self._logger.debug("connection is closed by remote client..with ConnectionRefusedError")
                break
            except:
                self._logger.log_last_except()

            # message = MsgpackSupport.decode(_data)
            # self.forward(self.asyncio_writer, addr, message)
            # await self.asyncio_writer.drain()
            # if message == "exit":
            #     message = f"{addr!r} wants to close the connection."
            #     self._logger.debug(message)
            #     self.forward(self.asyncio_writer, "Server", message)
            #     break
        # self.asyncio_writer.close()

    def handle_message(self, msg_data):
        try:
            rpc_message = MsgpackSupport.decode(msg_data)
            self.handle_rpc(rpc_message)
        except:
            self._logger.log_last_except()

    def handle_rpc(self, rpc_msg):
        _entity_type, _method_name, _parameters = rpc_msg
        if self._entity is None:
            self._entity = EntityFactory.instance().create_entity(_entity_type)
            self._entity.set_connection(self)
        _method = getattr(self._entity, _method_name, None)

        if not _method:
            self._logger.error("entity:%s  method:%s not exist", self._entity, _method_name)
            return
        try:
            _method(_parameters)
        except:
            self._logger.log_last_except()

    def request_rpc(
            # self, address, service_id, method_name, args=[], service_id_type=0, method_name_type=0,
            self, entity_type,
            method_name, args=None,
            # method_name_type=0,
            # need_reply=False, timeout=2
    ):

        # message = [RPC_REQUEST, service_id_type, service_id, method_name_type, method_name, args]
        message = [entity_type, method_name, args]
        try:
            data = self.do_encode(message)
        except:
            # self.logger.error("encode request message error")
            self._logger.log_last_except()
            # self.handle_traceback()
            self._logger.debug("encode request message error")
        else:
            # con.send_data_and_count(data)
            # if gr.flow_backups:
            #     gr.flow_msg('[BATTLE] NET UP ', len(data), message)
            # await self.send_data_and_count(data)
            self.send_data_and_count(data)
            # _task = asyncio.create_task(self.send_data_and_count(data))
        # return _task

    @staticmethod
    def do_encode(message):
        return MsgpackSupport.encode(message)

    def send_data_and_count(self, data):
    # async def send_data_and_count(self, data):
        self.send_cnt += 1
        # _len = len(data)
        data_len = len(data) if data else 0
        header_data = s_pack("i", data_len)
        data = header_data + data

        self.asyncio_writer.write(data)
        # await self.asyncio_writer.drain()
        # self.asyncio_writer.drain()
