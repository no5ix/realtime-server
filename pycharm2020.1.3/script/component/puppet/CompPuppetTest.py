# from TcpServer import TCP_SERVER
# import asyncio
import time

from common.component.Component import Component
from component.puppet import test_reload_const
from component.puppet.test_reload_const import TEST_CONST_STR
from core.common.RpcMethodArgs import Dict, Str
from core.common.RpcSupport import rpc_method, CLIENT_ONLY

import random
import typing

from core.mobilelog.LogManager import LogManager
from core.tool import incremental_reload
from core.util.UtilApi import log


class CompPuppetTest(Component):

    VAR_NAME = 'CompPuppetTest'

    def __init__(self):
        super().__init__()
        self._cnt = random.randint(0, 10)

    @rpc_method(CLIENT_ONLY, (Str('i'), ))
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

    @rpc_method(CLIENT_ONLY, (Dict('p'), ))
    def puppet_chat_to_ppt(self, chat_info: typing.Dict):
        # print(chat_info)
        # self._cnt -= 1
        # print("self._cnt:" + str(self._cnt))
        chat_info.update({'cnt': self._cnt})

        self.call_client_comp_method(self.VAR_NAME, 'puppet_chat_from_srv', {'i': chat_info})

    @rpc_method(CLIENT_ONLY)
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

    @rpc_method(CLIENT_ONLY)
    def test_reload(self):
        print("22call test_reload")
        # print("test_reload  after")
        print(TEST_CONST_STR)
        self.test_reload_impl()

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

    @rpc_method(CLIENT_ONLY)
    def test_response_rpc(self, a=0):
        b = 1 + a
        print(f"test_response_rpc: a={a}, b={b}")
        # print(f"test_response_rpc: a={a}")
        time.sleep(2)
        1/0
        return b

