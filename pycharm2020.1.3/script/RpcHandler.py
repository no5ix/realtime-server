from __future__ import annotations

import asyncio
import typing
if typing.TYPE_CHECKING:
    from TcpConn import TcpConn

from common import gr
from core.common import MsgpackSupport
from core.common.EntityFactory import EntityFactory
from core.mobilelog.LogManager import LogManager


class RpcHandler:

    def __init__(self, conn: typing.Optional[TcpConn] = None):
        self._logger = LogManager.get_logger()
        self._conn = conn  # type: typing.Optional[TcpConn]

    @staticmethod
    def do_encode(msg):
        return MsgpackSupport.encode(msg)

    @staticmethod
    def do_decode(msg):
        return MsgpackSupport.decode(msg)

    def request_rpc(self, *args, **kwargs):
        asyncio.create_task(self.request_rpc_impl(*args, **kwargs))

    async def request_rpc_impl(
            self, from_entity, method_name, param=None, remote_entity_type: typing.Union[None, str] = None,
            ip_port_tuple: typing.Tuple[str, int] = None):
        msg = [remote_entity_type, method_name, param]
        try:
            encoded_msg = self.do_encode(msg)
            # msg_len = len(encoded_msg) if encoded_msg else 0
            # header_data = struct.pack("i", msg_len)
            # final_data = header_data + encoded_msg
            if self._conn is None:
                self._conn = await gr.get_cur_server().get_conn_by_addr(ip_port_tuple)
                self._conn.set_entity(from_entity)
            self._conn.send_data_and_count(encoded_msg)
        except:
            self._logger.log_last_except()

    def handle_rpc(self, rpc_msg):
        try:
            _entity_type_str, _method_name, _parameters = self.do_decode(rpc_msg)
            _entity = self._conn.get_entity()
            if _entity is None:
                _entity = gr.get_server_singleton(_entity_type_str)
                if _entity is None:
                    _entity = EntityFactory.instance().create_entity(_entity_type_str)
                _entity.set_rpc_handler(self)
                self._conn.set_entity(_entity)

            _method = getattr(_entity, _method_name, None)
            _method(_parameters)
        except:
            self._logger.log_last_except()
