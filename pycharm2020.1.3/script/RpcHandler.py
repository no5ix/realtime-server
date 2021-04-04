from __future__ import annotations

import asyncio
import typing


if typing.TYPE_CHECKING:
    from TcpConn import TcpConn
    from server_entity.ServerEntity import ServerEntity

from common import gv
from core.common import MsgpackSupport
from core.common.EntityFactory import EntityFactory
from core.mobilelog.LogManager import LogManager


class RpcHandler:

    def __init__(
            self, conn: typing.Optional[TcpConn] = None,
            entity: typing.Optional[ServerEntity] = None):
        self._logger = LogManager.get_logger()
        self._conn = conn  # type: typing.Optional[TcpConn]
        self._entity = entity  # type: typing.Optional[ServerEntity]

    # def bind_entity(self, entity: ServerEntity):
    #     self._entity = entity

    def set_conn(self, conn):
        self._conn = conn

    @staticmethod
    def do_encode(msg):
        return MsgpackSupport.encode(msg)

    @staticmethod
    def do_decode(msg):
        return MsgpackSupport.decode(msg)

    def request_rpc(self, *args, **kwargs):
        asyncio.create_task(self.request_rpc_impl(*args, **kwargs))

    async def request_rpc_impl(
            self, method_name, params=(), remote_entity_type: typing.Union[None, str] = None,
            ip_port_tuple: typing.Tuple[str, int] = None):
        msg = [remote_entity_type, method_name, params]
        try:
            encoded_msg = self.do_encode(msg)
            # msg_len = len(encoded_msg) if encoded_msg else 0
            # header_data = struct.pack("i", msg_len)
            # final_data = header_data + encoded_msg
            if self._conn is None:
                self._conn = await gv.get_cur_server().get_conn_by_addr(
                    ip_port_tuple, self)
                # self._conn.set_entity(from_entity)
            self._conn.send_data_and_count(encoded_msg)
        except:
            self._logger.log_last_except()

    def handle_rpc(self, rpc_msg):
        try:
            _entity_type_str, _method_name, _parameters = self.do_decode(rpc_msg)
            # _entity = self._conn.get_entity()
            if self._entity is None:
                self._entity = gv.get_server_singleton(_entity_type_str)
                if self._entity is None:
                    self._entity = EntityFactory.instance().create_entity(_entity_type_str)
                self._entity.set_rpc_handler(self)
                # self._conn.set_entity(_entity)
            _comp_method_list = _method_name.split(".")
            if len(_comp_method_list) == 1:
                _method = getattr(self._entity, _method_name)
            else:
                _method = getattr(
                    self._entity.get_component(_comp_method_list[0]), _comp_method_list[1])
            _method(_parameters)
        except:
            self._logger.log_last_except()
