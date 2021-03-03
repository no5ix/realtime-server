
from common.component.Component import Component
from core.common.RpcMethodArgs import Dict
from core.common.RpcSupport import rpc_method, CLIENT_ONLY, CLIENT_STUB


class CompPuppetTest(Component):

    VAR_NAME = 'CompPuppetTest'

    def __init__(self):
        super().__init__()
        self._cnt = 1000000

    def puppet_chat_to_channel(self, chat_info):
        self.call_server_comp_method(
            self.VAR_NAME, 'puppet_chat_to_channel', {'i': chat_info})

    def puppet_chat_to_ppt(self, chat_info):
        self.call_server_comp_method(
            self.VAR_NAME, 'puppet_chat_to_ppt', {'p': chat_info})

    @rpc_method(CLIENT_STUB, (Dict('i'),))
    def puppet_chat_from_srv(self, chat_info):
        print(chat_info)
        self._cnt -= 1
        if self._cnt > 0:
            self.puppet_chat_to_ppt({'content': 'puppet_chat_to_ppt'})