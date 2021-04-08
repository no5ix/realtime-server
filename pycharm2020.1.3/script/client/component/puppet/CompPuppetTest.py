# from RpcHandler import RpcReplyError
import asyncio
import functools
from asyncio import Future

from common.component.Component import Component
from core.common.RpcMethodArgs import Dict
from core.common.RpcSupport import rpc_method, CLI_TO_SRV, SRV_TO_CLI
from core.util.UtilApi import wait_or_not


class CompPuppetTest(Component):

    VAR_NAME = 'CompPuppetTest'

    def __init__(self):
        super().__init__()
        self._cnt = 1000000

    def puppet_chat_to_channel(self, chat_info):
        print("call puppet_chat_to_channel")
        self.call_server_comp_method(
            self.VAR_NAME, 'puppet_chat_to_channel', {'i': "mmp"})

    def puppet_chat_to_ppt(self, chat_info):
        print("call puppet_chat_to_ppt")
        self.call_server_comp_method(
            self.VAR_NAME, 'puppet_chat_to_ppt', {'p': chat_info})

    def test_reload(self):
        print("call test_reload")
        self.call_server_comp_method(
            self.VAR_NAME, 'test_reload')

    def make_server_reload(self):
        print("call make server reload")
        self.call_server_comp_method(
            self.VAR_NAME, 'make_server_reload')

    # @rpc_method(CLIENT_STUB, (Dict('i'),))
    # @rpc_method(CLIENT_STUB, Dict('i'))
    @rpc_method(SRV_TO_CLI, [Dict('i')])
    # @rpc_method(CLIENT_STUB, {Dict('i')})
    def puppet_chat_from_srv(self, chat_info):
        print(chat_info)
        self._cnt -= 1
        if self._cnt > 0:
            self.puppet_chat_to_ppt({'content': 'puppet_chat_to_ppt'})

    async def test_timer_async(self, msg):
        await asyncio.sleep(1)
        print(msg)
        # asyncio.coroutine

    @wait_or_not
    async def test_response_rpc(self, msg):
        print("callll test_response_rpc")

        timer_key = self.entity.timer_hub.call_later(3, lambda: self.test_timer_async(msg), repeat_count=2)

        self.remote_comp.make_server_reload()

        def mmp(error, result):
            print("mmmmmmmmmmmp")
            print(f"err={error}")
            print(f"res={result}")
            # print(_fut.result())

        self.remote_entity.CompPuppetTest.test_response_rpc2(
            997,
            rpc_callback=mmp
        )

        err, hh = await self.remote_comp.test_response_rpc1(
            886, c=11,
            # need_reply=True,
            rpc_reply_timeout=10,
            rpc_callback=mmp
        )
        print(f"hh={hh}, err={err}")
