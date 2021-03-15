# from TcpServer import TCP_SERVER
import asyncio

from common.component.Component import Component
from core.common.RpcMethodArgs import Dict
from core.common.RpcSupport import rpc_method, CLIENT_ONLY

import random
import typing


class CompPuppetTest(Component):

    VAR_NAME = 'CompPuppetTest'

    def __init__(self):
        super().__init__()
        self._cnt = random.randint(0, 10)
        pass

    @rpc_method(CLIENT_ONLY, (Dict('i'), ))
    def puppet_chat_to_channel(self, chat_info):
        print(chat_info)
        # self._cnt -= 1
        print("self._cnt:" + str(self._cnt))
        # TCP_SERVER.call_later()

        loop = asyncio.get_event_loop()
        # loop.call_later(4, self.test_delay_func)

    def test_delay_func(self):
        print('test_delay_func')

    @rpc_method(CLIENT_ONLY, (Dict('p'), ))
    def puppet_chat_to_ppt(self, chat_info: typing.Dict):
        print(chat_info)
        # self._cnt -= 1
        print("self._cnt:" + str(self._cnt))
        chat_info.update({'cnt': self._cnt})
        self.call_client_comp_method(self.VAR_NAME, 'puppet_chat_from_srv', {'i': chat_info})

