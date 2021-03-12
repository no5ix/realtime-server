import asyncio

from TcpConn import TcpConn
from core.common.IdManager import IdManager
from ..common.IdManager import IdManager
from ..common.EntityManager import EntityManager, EntityIdOrLocalId
from ..common.EntityFactory import EntityFactory
from ..mobilelog.LogManager import LogManager



# pylint: disable = E1101
class ServerEntity(object):
    """ 游戏中所有服务端对象的父类"""
    def __init__(self, entityid = None):
        super(ServerEntity, self).__init__()
        # if we have entityid passed in, used it, otherwise use generated one
        self.id = (entityid == None) and IdManager.genid() or entityid
        self.localid = -1
        self.logger = LogManager.get_logger("ServerEntity." + self.__class__.__name__)
        self.logger.info("__init__ create entity %s with id %s mem_id=%s", self.__class__.__name__, self.id, id(self))
        # entity所对应的gate proxy, 使用请调_get_gate_proxy方法，不要直接使用此变量
        self._gate_proxy = None
        self._src_mailbox_info = None                              # 缓存自己的src_mailbox_info信息
        EntityManager.addentity(self.id, self, False)
        self._save_timer = None
        self.isdestroty = False
        savetime = self.get_persistent_time()

    def call_server_method(self, remote_mailbox, methodname, parameters={}, callback=None):
        remote_ip = remote_mailbox.ip
        remote_port = remote_mailbox.port
        reader, writer = await asyncio.open_connection(remote_ip, remote_port)
        _tcp_conn = TcpConn(writer.get_extra_info('peername'), writer, reader)
        _tcp_conn.request_rpc(methodname, parameters)
        await _tcp_conn.loop()