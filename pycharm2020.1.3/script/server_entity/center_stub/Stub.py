# -*- coding: utf-8 -*-
import asyncio

from core.common.RpcMethodArgs import List
from core.common.RpcSupport import rpc_method, SERVER_ONLY
from core.common.IdManager import IdManager
# from core.common.proto_python.common_pb2 import EntityMailbox
# from core.distserver.game import GameAPI
# from core.distserver.game import GameServerRepo
# from ..ServerEntityBase import ServerEntityBase
# import gr

from core.util import UtilApi
from ..ServerEntity import ServerEntity


class Stub(ServerEntity):
    """
    Stub基类
    负责和Center类连接
    如RollCenter, RollStub
    """

    def __init__(self, entity_id=None):
        super(Stub, self).__init__(entity_id)
        self._connected = False
        self._connect_counter = 0
        self.connect_cb = None
        self._center_name = ''
        self._peers = []

    # def start_connect(self, center_name, cb):
    def start_connect(self, center_name):
        """center_name 要连接的center_name"""
        self._center_name = center_name
        # self.connect_cb = cb
        # gr.callback(1.0, lambda: self.connect_to_center())
        asyncio.get_running_loop().call_later(1, self.connect_to_center)

    def connect_to_center(self):
        self._connect_counter += 1
        # center = UtilApi.get_global_entity_mailbox(self._center_name)
        center_ip_port_tuple = UtilApi.get_service_info(self._center_name)
        if center_ip_port_tuple:
            center_ip_port = center_ip_port_tuple[0]
            asyncio.create_task(self.call_server_method_with_ip_port(
                center_ip_port, 'register_stub', remote_entity_type=self._center_name))
            self._connected = True
            if self.connect_cb is not None:
                self.connect_cb()
                self.connect_cb = None
        else:
            self.logger.info('Stub cannot connect to %s, try again after 1 sec... (%d)', self._center_name,
                             self._connect_counter)
            # gr.callback(1.0, lambda: self.connect_to_center())
            asyncio.get_running_loop().call_later(1, self.connect_to_center)

    # def call_center_method(self, method_name, args):
    #     if not self._connected:
    #         self.logger.error('call_center_method error: not connected to center method:%s', method_name)
    #     else:
    #         center = GameAPI.get_global_entity_mailbox(self._center_name)
    #         if center:
    #             self.call_server_method(center, method_name, args)
    #         else:
    #             self.logger.error('call_center_method error: cannot find center mailbox method:%s', method_name)
    #
    # def call_peer_method(self, method_name, args):
    #     """调用除自己之外的其他stubs方法"""
    #     for p in self._peers:
    #         self.call_server_method(p, method_name, args)
    #
    # def call_all_stub_method(self, method_name, args):
    #     """调用所有stub的方法，含自己"""
    #     # call self's method
    #     self._call_self_rpc_method(method_name, args)
    #     # call peer method
    #     self.call_peer_method(method_name, args)
    #
    # def _call_self_rpc_method(self, method_name, args):
    #     rpc_func = getattr(self, method_name, None)
    #     if rpc_func is None:
    #         self.logger.error("cannot find rpc_method=%s in stub=%s", method_name, self.__class__.__name__)
    #         return
    #     self_mb = self.get_mailbox()
    #     rpc_func(self_mb, args)
    #
    # @rpc_method(SERVER_ONLY, (List('peers'),))
    # def update_peers(self, peers):
    #     self._peers = []
    #     self_mb = self.get_mailbox()
    #     for bin_peers in peers:
    #         p = GameAPI.decode_mailbox(bin_peers)
    #         if not GameAPI.is_same_mailbox(self_mb, p):
    #             self._peers.append(p)

    @rpc_method(SERVER_ONLY, ())
    def connected_to_center(self):
        self.on_connected_to_center()

    def on_connected_to_center(self):
        pass

    # def get_mailbox(self):
    #     mb = EntityMailbox()
    #     mb.entityid = IdManager.id2bytes(self.id)
    #     mb.serverinfo.CopyFrom(GameServerRepo.game_server_info)
    #     return mb
