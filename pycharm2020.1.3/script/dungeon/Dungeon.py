from core.common.RpcMethodArgs import Uuid
from core.common.RpcSupport import rpc_method, CLIENT_ONLY


class Dungeon(object):

    def __init__(self):
        pass

    @rpc_method(CLIENT_ONLY, (Uuid('a'), ))
    def prepare_dungeon_finish(self, aid):
        pass