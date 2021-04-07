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

        # msg = "calll test_timer_async"
        # timer_key = self.entity.timer_hub.call_later(3, lambda m=msg: self.test_timer_async(m), repeat_count=2)
        timer_key = self.timer_hub.call_later(3, lambda: self.test_timer_async(msg), repeat_count=2)
        # self.entity.timer_hub.cancel_timer(timer_key)
        # msg = "calll test_timer_async nonono"

        self.remote_comp.make_server_reload()
        # try:
        # err = None
        # # gg = await self.remote_entity.CompPuppetTest.test_response_rpc(997)
        # err, gg = await\

        # def mmp(_fut: Future):
        def mmp(error, result):
            print("mmmmmmmmmmmp")
            print(f"err={error}")
            print(f"res={result}")
            # print(_fut.result())

        _fut = self.remote_entity.CompPuppetTest.test_response_rpc2(
        # _fut = self.remote_comp.test_response_rpc2(
            997,
            # need_reply=False, reply_timeout=3
            # reply_timeout=6
            rpc_callback=mmp
        )
        # _fut.add_done_callback(mmp)
        # print(await _fut)
        # # except RpcReplyError as e:
        # #     print("rpc reply errr")
        # #     print(e)
        # # self.remote_entity.CompPuppetTest.test_response_rpc(997)
        # print(f"gg={gg}, err={err}")

        err, hh = await self.remote_comp.test_response_rpc1(
            886, c=11,
            # need_reply=True,
            rpc_reply_timeout=10,
            rpc_callback=mmp
        )
        print(f"hh={hh}, err={err}")

        # self.remote_comp.test_response_rpc(886)
        # print("testttt")
        # final_rpc_name = "CompPuppetTest.test_response_rpc"
        # args = [110]
        # need_reply = False
        # reply_timeout = 2
        # self.entity.call_remote_method(
        #     final_rpc_name, [*args], need_reply, reply_timeout)
