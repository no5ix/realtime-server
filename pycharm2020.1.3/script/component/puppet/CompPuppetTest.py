
from common.component.Component import Component
from core.common.RpcMethodArgs import Dict
from core.common.RpcSupport import rpc_method, CLIENT_ONLY


class CompPuppetTest(Component):

    VAR_NAME = 'CompPuppetTest'

    def __init__(self):
        super().__init__()
        self._cnt = 100
        pass

    @rpc_method(CLIENT_ONLY, (Dict('i'), ))
    def puppet_chat_to_channel(self, chat_info):
        print(chat_info)
        self._cnt -= 1
        print("self._cnt:" + str(self._cnt))
