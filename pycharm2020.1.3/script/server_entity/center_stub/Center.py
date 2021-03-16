# -*- coding: utf-8 -*-
from core.common.RpcMethodArgs import MailBox
from core.common.RpcSupport import rpc_method, SERVER_ONLY
# from core.distserver.game import GameAPI

from ..ServerEntity import ServerEntity
# from ..ServerEntityBase import ServerEntityBase


# class Center(ServerEntityBase):
class Center(ServerEntity):
    """
    Center基类
    负责和各个GameServer的Stub类连接
    如RollCenter, RollStub
    """

    def __init__(self, entity_id=None):
        super(Center, self).__init__(entity_id)
        self._stubs = []

    def init(self):
        pass

    def register_callback(self, succeeded):
        if succeeded:
            self.logger.info('Register global %s succeed', self.__class__.__name__)
        else:
            self.logger.error('Failed to register global %s!!', self.__class__.__name__)

    @rpc_method(SERVER_ONLY)
    # @rpc_method(SERVER_ONLY, (MailBox(),))
    # def register_stub(self, stub_box):
    def register_stub(self):
        stub_box = None
        self.on_stub_connected(stub_box)
        self._stubs.append(stub_box)
        # self.logger.info('Stub is registered to %s %s:%s', self.__class__.__name__, stub_box.serverinfo.ip,
        #                  stub_box.serverinfo.port)
        # self.notify_stubs_update_peers()
        # 通知stub已成功连接到center
        # self.call_server_method(stub_box, 'connected_to_center')
        self.call_server_method('connected_to_center')

    # def refresh_stubs(self, server_list):
    #     """
    #     通过传来的服务器列表，把失效的roll_stub删去
    #     """
    #     online_stubs = []
    #     offline_stubs = []
    #     for stub in self._stubs:
    #         for server in server_list:
    #             if stub.serverinfo.ip == server.ip and stub.serverinfo.port == server.port:
    #                 online_stubs.append(stub)
    #                 break
    #         else:
    #             offline_stubs.append(stub)
    #     self._stubs = online_stubs
    #
    #     for stub in offline_stubs:
    #         self.on_stub_lose_connection(stub)
    #     if len(offline_stubs) > 0:
    #         self.notify_stubs_update_peers()
    #
    # def notify_stubs_update_peers(self):
    #     binary_list = []
    #     for s in self._stubs:
    #         bin_mb = GameAPI.encode_mailbox(s)
    #         binary_list.append(bin_mb)
    #     self.call_all_stub_method('update_peers', {'peers': binary_list})
    #
    # def on_stub_lose_connection(self, stub_box):
    #     pass

    def on_stub_connected(self, stub_box):
        pass

    # def get_stub_num(self):
    #     return len(self._stubs)
    #
    # def call_all_stub_method(self, method_name, args):
    #     for stub in self._stubs:
    #         self.call_server_method(stub, method_name, args)
