
from common.component.Component import Component
from core.common.RpcMethodArgs import Dict
from core.common.RpcSupport import rpc_method, CLIENT_ONLY


class CompPuppetTest(Component):

    def __init__(self):
        super().__init__()
        pass

    @rpc_method(CLIENT_ONLY, (Dict('i'), ))
    def puppet_chat_to_channel(self, chat_info):
        print(chat_info)
