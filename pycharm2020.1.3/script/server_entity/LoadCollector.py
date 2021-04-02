# from common import gr
from common import gv
from core.common.RpcMethodArgs import RpcMethodArg, Float, Str
from core.common.RpcSupport import rpc_method, SERVER_ONLY
from core.mobilelog.LogManager import LogManager
from server_entity.ServerEntity import ServerEntity

import redis
import time


class LoadCollector(ServerEntity):

    def __init__(self):
        super().__init__()
        gv.add_server_singleton(self)

        pool = redis.ConnectionPool(host='localhost', port=6379, decode_responses=True)
        self._pipe = redis.Redis(connection_pool=pool).pipeline(transaction=False)

    @rpc_method(SERVER_ONLY, [Str("sn"), Float("l")])
    def report_load(self, etcd_tag, server_name, load):
        print(f"etcd_tag: {etcd_tag} server_name: {server_name} load: {load}")
        self._pipe.zadd(etcd_tag, )  # TODO

    @rpc_method(SERVER_ONLY)
    def pick_battle_server(self):
        return
