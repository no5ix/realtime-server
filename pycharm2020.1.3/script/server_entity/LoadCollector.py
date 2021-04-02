# from common import gr
from common import gv
from core.common.RpcMethodArgs import RpcMethodArg, Float, Str
from core.common.RpcSupport import rpc_method, SERVER_ONLY
from core.mobilelog.LogManager import LogManager
from server_entity.ServerEntity import ServerEntity


class LoadCollector(ServerEntity):

    def __init__(self):
        super().__init__()
        gv.add_server_singleton(self)

    @rpc_method(SERVER_ONLY, [Str("sn"), Float("l")])
    def report_load(self, server_name, load):
        print(f"{server_name} load: {load}")

    @rpc_method(SERVER_ONLY)
    def pick_battle_server(self):
        return
