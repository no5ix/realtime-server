# from TcpServer import TCP_SERVER
# import asyncio
import asyncio
import time

from RpcHandler import rpc_func
from common.component.Component import Component
from component.puppet import test_reload_const
from component.puppet.test_reload_const import TEST_CONST_STR
from core.common.RpcMethodArgs import Dict, Str
from core.common.RpcSupport import rpc_method, CLI_TO_SRV

import random
import typing

from core.mobilelog.LogManager import LogManager
from core.tool import incremental_reload
# from core.util.UtilApi import log


class CompPuppetTest(Component):

    VAR_NAME = 'CompPuppetTest'

    def __init__(self):
        super().__init__()
        self._cnt = random.randint(0, 10)

    @rpc_method(CLI_TO_SRV, (Str('i'),))
    def puppet_chat_to_channel(self, chat_info):
        # import sys
        # isin = "component.puppet.CompPuppetTest" in sys.modules  # TODO del
        # self.entity.logger.debug(str(sys.modules))

        print(chat_info)
        # self._cnt -= 1
        # print("self._cnt:" + str(self._cnt))
        # TCP_SERVER.call_later()

        # loop = asyncio.get_event_loop()
        # loop.call_later(4, self.test_delay_func)
        # chat_info.update({'cnt': self._cnt})

        self.call_client_comp_method(self.VAR_NAME, 'puppet_chat_from_srv', {'i': {"lk": 8}})
        print("call client puppet_chat_from_srv")

    def test_delay_func(self):
        print('test_delay_func')

    # @rpc_method(CLI_TO_SRV, (Dict('p'),))
    @rpc_func
    def puppet_chat_to_ppt(self, chat_info: typing.Dict):
        # print(chat_info)
        # self._cnt -= 1
        # print("self._cnt:" + str(self._cnt))
        chat_info.update({'cnt': self._cnt})
        self.remote_comp.puppet_chat_from_srv(chat_info)
        # self.call_client_comp_method(self.VAR_NAME, 'puppet_chat_from_srv', {'i': chat_info})

    # @rpc_method(CLI_TO_SRV)

    @rpc_func
    def make_server_reload(self):
        # print("before reload")
        # print(test_reload_const.TEST_CONST_STR)
        print("start make reload")
        incremental_reload.reload_script()
        print("fin make reload")
        # self.test_reload_impl()

        # self.call_client_comp_method(
        #     self.VAR_NAME, 'puppet_chat_from_srv', {'i': {"reload_state": "success"}})

        # self.test_timer_circle()

    @rpc_method(CLI_TO_SRV)
    def test_reload(self):
        print("22call test_reload")
        # print("test_reload  after")
        print(TEST_CONST_STR)
        self.test_reload_impl()

    def log(text):
        def decorator(func):
            # @functools.wraps(func)
            def wrapper(*args, **kw):
                print('%s %s():' % (text, func.__name__))
                return func(*args, **kw)

            return wrapper

        return decorator

    @log("execute")
    def test_reload_impl(self):
        print("call test_reload_impllll222112")
        pass

        self.test_cls_log()
        self.test_static_log()
        CompPuppetTest.test_cls_log()
        CompPuppetTest.test_static_log()
        # print("test_reload_impl after")

    def test_timer_circle(self):
        print("test_timer_circle")
        self.entity.timer_hub.call_later(
            0, lambda: self.test_timer_circle(), "test_timer_circle"
            # , repeat_count=2, repeat_interval_sec=0
        )

    @classmethod
    def test_cls_log(cls):
        xxd = LogManager.get_logger()

    @staticmethod
    def test_static_log():
        mm = LogManager.get_logger()

    @rpc_func
    async def test_response_rpc1(self, a: int = 0, c: int = 3):
        b = 1 + a + c
        print(f"test_response_rpc: a={a}, b={b}, c={c}")
        # print(f"test_response_rpc: a={a}")
        # time.sleep(2)
        await asyncio.sleep(6)
        # 1/0
        return b

    @rpc_func
    async def test_response_rpc2(self, a: int = 0):
        b = 2 + a
        print(f"test_response_rpc: a={a}, b={b}")
        # print(f"test_response_rpc: a={a}")
        # time.sleep(4)
        await asyncio.sleep(4)
        # 1/0
        # return b

