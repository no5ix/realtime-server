from core.common.RpcMethodArgs import Uuid
from core.common.RpcSupport import rpc_method, CLI_TO_SRV


class Dungeon(object):

    def __init__(self):
        pass

    @rpc_method(CLI_TO_SRV, (Uuid('a'),))
    def prepare_dungeon_finish(self, aid):
        pass
