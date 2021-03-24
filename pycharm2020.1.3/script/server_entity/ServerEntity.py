# import asyncio

# from TcpConn import TcpConn
import asyncio

import typing

from TcpConn import TcpConn
from core.common.EntityManager import EntityManager
from core.common.IdManager import IdManager
from core.mobilelog.LogManager import LogManager
from core.util.TimerHub import TimerHub


class ServerEntity(object):
    def __init__(self, entity_id=None):
        super(ServerEntity, self).__init__()
        self.id = (entity_id is None) and IdManager.genid() or entity_id
        self.local_id = -1
        self._logger = LogManager.get_logger("ServerEntity." + self.__class__.__name__)
        self._logger.info("__init__ create entity %s with id %s mem_id=%s", self.__class__.__name__, self.id, id(self))
        # entity所对应的gate proxy, 使用请调_get_gate_proxy方法，不要直接使用此变量
        self._gate_proxy = None
        self._src_mailbox_info = None                              # 缓存自己的src_mailbox_info信息
        EntityManager.instance().addentity(self.id, self, False)
        self._save_timer = None
        self.is_destroy = False
        # save_time = self.get_persistent_time()

        self._conn = None
        self.timer_hub = TimerHub()

    def set_connection(self, conn):
        self._conn = conn

    def call_client_method(
            self, method_name, parameters=None, remote_entity_type: typing.Union[None, str] = None):
        self._conn.request_rpc(remote_entity_type or self.__class__.__name__, method_name, parameters)

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

    def call_server_method(
            self, method_name, parameters=None, remote_entity_type: typing.Union[None, str] = None):
        self._conn.request_rpc(remote_entity_type or self.__class__.__name__, method_name, parameters)

    async def call_server_method_with_ip_port(
            self, server_ip_port_tuple, method_name, parameters=None,
            remote_entity_type: typing.Union[None, str] = None):
        if server_ip_port_tuple:
            remote_ip = server_ip_port_tuple[0]
            remote_port = server_ip_port_tuple[1]
            reader, writer = await asyncio.open_connection(remote_ip, remote_port)
            _tcp_conn = TcpConn(writer.get_extra_info('peername'), writer, reader)
            self.set_connection(_tcp_conn)
            self._conn.set_entity(self)
            self._conn.request_rpc(remote_entity_type or self.__class__.__name__, method_name, parameters)
            # self._conn.request_rpc(self.__class__.__name__, method_name, parameters)
            self._conn.loop()
            # asyncio.create_task(_tcp_conn.loop())
