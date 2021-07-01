# from RpcHandler import RpcReplyError
import asyncio
import functools
import time
from asyncio import Future

from RpcHandler import rpc_func
from common.component.Component import Component
from core.common.RpcMethodArgs import Dict
from core.common.RpcSupport import rpc_method, CLI_TO_SRV, SRV_TO_CLI
from core.util.UtilApi import wait_or_not
import time


last_ts = time.time()


class CompAvatarTest(Component):

    VAR_NAME = 'CompAvatarTest'

    def __init__(self):
        super().__init__()
        self._cnt = 1

    # def puppet_chat_to_channel(self, chat_info):
    #     print("call puppet_chat_to_channel")
    #     self.call_server_comp_method(
    #         self.VAR_NAME, 'puppet_chat_to_channel', {'i': "mmp"})

    def puppet_chat_to_ppt(self, chat_info):
        global last_ts
        last_ts = time.time()
        print("call puppet_chat_to_ppt")
        self.remote_comp.puppet_chat_to_ppt(chat_info)
        # self.call_server_comp_method(
        #     self.VAR_NAME, 'puppet_chat_to_ppt', {'p': chat_info})

    # @rpc_method(CLIENT_STUB, (Dict('i'),))
    # @rpc_method(CLIENT_STUB, Dict('i'))
    # @rpc_method(SRV_TO_CLI, [Dict('i')])
    # @rpc_method(CLIENT_STUB, {Dict('i')})
    @rpc_func
    def puppet_chat_from_srv(self, chat_info):
        global last_ts
        print(f'{time.time()-last_ts=}')
        print(f"call puppet_chat_from_srv {chat_info=}, {time.time()=}")
        self._cnt -= 1
        # if self._cnt > 0:
        #     self.remote_comp.puppet_chat_to_ppt({'content': 'puppet_chat_to_ppt'})
        # last_ts = time.time()
