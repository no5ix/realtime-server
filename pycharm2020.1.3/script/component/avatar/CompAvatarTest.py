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
from common.component.Component import Component


class CompAvatarTest(Component):

    # VAR_NAME = "CompAvatarTest"

    def __init__(self):
        super().__init__()
        pass
        self._cnt = random.randint(0, 10)

    # @rpc_method(CLI_TO_SRV, (Dict('p'),))
    @rpc_func
    def puppet_chat_to_ppt(self, chat_info: typing.Dict):
        # print(chat_info)
        # self._cnt -= 1
        print("self._cnt:" + str(self._cnt))
        chat_info.update({'cnt': self._cnt})
        self.remote_comp.puppet_chat_from_srv(chat_info)
