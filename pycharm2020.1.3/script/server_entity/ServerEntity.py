from __future__ import annotations
import typing

from core.util import DbUtil

if typing.TYPE_CHECKING:
    pass

import RpcHandler

from common import gv
from core.common.EntityManager import EntityManager
from core.common.IdManager import IdManager
from core.mobilelog.LogManager import LogManager
from core.util.TimerHub import TimerHub


class ServerEntity:
    def __init__(self, entity_id=None):
        self.id = (entity_id is None) and IdManager.genid() or entity_id
        self.local_id = -1
        self.logger = LogManager.get_logger("ServerEntity." + self.__class__.__name__)
        self.logger.debug("__init__ create entity %s with id %s mem_id=%s", self.__class__.__name__, self.id, id(self))
        EntityManager.instance().addentity(self.id, self, False)
        self.is_destroy = False

        self._rpc_handler = RpcHandler.RpcHandler(entity=self)
        self.timer_hub = TimerHub()
        self.timer_hub.call_later(gv.db_save_interval_sec, lambda: self.db_save(), repeat_count=-1)

    def destroy(self):
        self.timer_hub.destroy()
        self.timer_hub = None
        self.is_destroy = True

    def should_db_save(self):
        return False

    def get_db_save_dict(self):
        return {}

    def db_save(self):
        if not self.should_db_save():
            return
        DbUtil.save_entity(self)

    def set_rpc_handler(self, rpc_handler):
        self._rpc_handler = rpc_handler

    def get_rpc_handle(self):
        return self._rpc_handler

    # def call_client_method(
    #         self, method_name, parameters=None, remote_entity_type: typing.Union[None, str] = None):
    #     self._rpc_handler.request_rpc(
    #         remote_entity_type or self.__class__.__name__, method_name, parameters)

    def call_other_client_method(self):
        pass

    def call_all_client_method(self):
        pass

    def call_server_method_direct(self):
        pass

    # def call_server_method_(self, remote_mailbox, method_name, parameters=None):
    #     remote_ip = remote_mailbox.ip
    #     remote_port = remote_mailbox.port
    #     reader, writer = await asyncio.open_connection(remote_ip, remote_port)
    #     _tcp_conn = TcpConn(writer.get_extra_info('peername'), writer, reader)
    #     self.set_connection(_tcp_conn)
    #     self._conn.request_rpc(method_name, parameters)
    #     await _tcp_conn.loop()

    def call_remote_method(
            self,
            rpc_fuc_name: str,
            rpc_fuc_args: typing.Union[typing.Set, typing.List, typing.Tuple] = (),
            rpc_fuc_kwargs: typing.Union[None, typing.Dict] = None,
            rpc_callback: typing.Callable = None,
            rpc_need_reply: bool = True,
            rpc_reply_timeout: typing.Union[None, int, float] = RpcHandler.REQUEST_RPC_TIMEOUT,
            rpc_remote_entity_type: typing.Union[None, str] = None,
            ip_port_tuple: typing.Tuple[str, int] = None
    ):
        # if self._rpc_handler is None:
        #     if ip_port_tuple is None:
        #         self.logger.error("self._conn is None and ip_port_tuple is None")
        #         return
        #     self._rpc_handler = RpcHandler()
        try:
            return self._rpc_handler.request_rpc(
                rpc_fuc_name, rpc_fuc_args, rpc_fuc_kwargs, rpc_callback, rpc_need_reply, rpc_reply_timeout,
                rpc_remote_entity_type or self.__class__.__name__, ip_port_tuple
                # *args, **kwargs
            )
        except:
            self.logger.log_last_except()

    # async def call_server_fuc_with_ip_port(
    #         self, server_ip_port_tuple, method_name, parameters=None,
    #         remote_entity_type: typing.Union[None, str] = None):
    #     if server_ip_port_tuple:
    #         remote_ip = server_ip_port_tuple[0]
    #         remote_port = server_ip_port_tuple[1]
    #         reader, writer = await asyncio.open_connection(remote_ip, remote_port)
    #         _tcp_conn = TcpConn(writer.get_extra_info('peername'), writer, reader)
    #         self.set_connection(_tcp_conn)
    #         tcp_srv = gr.get_cur_server()
    #         tcp_srv.add_conn(server_ip_port_tuple, _tcp_conn)
    #
    #         self._conn.set_entity(self)
    #         self._conn.request_rpc(remote_entity_type or self.__class__.__name__, method_name, parameters)
    #         # self._conn.request_rpc(self.__class__.__name__, method_name, parameters)
    #         # self._conn.loop()
    #         # asyncio.create_task(_tcp_conn.loop())
