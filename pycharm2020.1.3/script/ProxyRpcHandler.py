from __future__ import annotations

import asyncio
# import collections
import functools
import typing
from asyncio import futures

import time
from typing import Optional

import core.util.UtilApi
from ConnBase import ConnBase
from ConnMgr import ConnMgr
from core.common.protocol_def import PROTO_TYPE_TCP
from RpcHandler import RpcHandler, RPC_TYPE_HEARTBEAT, RPC_TYPE_NOTIFY, RPC_TYPE_REQUEST, RPC_TYPE_REPLY, \
    get_a_rpc_handler_id
from RpcHandler import RpcReplyFuture
from common.service_const import ETCD_TAG_LOBBY_SRV
from core.util import EnhancedJson, UtilApi
from core.util.TimerHub import TimerHub
from core.util.UtilApi import wait_or_not, async_lock
from server_entity.ServerEntity import ServerEntity

if typing.TYPE_CHECKING:
    from TcpConn import TcpConn

from common import gv
from core.common import MsgpackSupport
from core.common.EntityFactory import EntityFactory
from core.mobilelog.LogManager import LogManager


class ProxyLobbyRpcHandler(RpcHandler):

    def __init__(
            self, proxy_cli_rpc_handler: ProxyCliRpcHandler,
            rpc_handler_id: bytes, conn: TcpConn = None,
            entity: ServerEntity = None):
        super(ProxyLobbyRpcHandler, self).__init__(rpc_handler_id, conn, entity)
        self.proxy_cli_rpc_handler = proxy_cli_rpc_handler  # type: Optional[ProxyCliRpcHandler]
        self._lobby_addr = None  # type: Optional[tuple]
        self.try_retrieve_lobby_addr()

    def try_retrieve_lobby_addr(self):
        _info = UtilApi.get_lowest_load_service_info(ETCD_TAG_LOBBY_SRV)
        if _info is None:
            self._timer_hub.call_later(1, self.try_retrieve_lobby_addr)
        else:
            self._lobby_addr = _info[1:]

    def compress_n_encode(self, rpc_msg):
        # todo
        return rpc_msg

    @wait_or_not()
    async def handle_rpc(self, rpc_msg):
        try:
            rpc_msg = self.compress_n_encode(rpc_msg)

            _rpc_msg_tuple = self.do_decode(rpc_msg)  # todo: 不应该解出来的
            _rpc_type = _rpc_msg_tuple[0]

            # _entity_type_str, _method_name, _method_args, _method_kwargs = _rpc_msg_tuple[-4:]
            # self._logger.debug(f'{_entity_type_str=}, {_method_name=}, {_method_args=}, {_method_kwargs=}')

            if _rpc_type == RPC_TYPE_HEARTBEAT:
                self._conn.remote_heart_beat()
            self.proxy_cli_rpc_handler._conn.send_data_and_count(self.rpc_handler_id, rpc_msg)  # todo

            # self._logger.debug(f'handle_rpcxxxxx, {self.rpc_handler_id=}')

            # elif _rpc_type in (RPC_TYPE_NOTIFY, RPC_TYPE_REQUEST, RPC_TYPE_REPLY,):
            #     await self.proxy_cli_rpc_handler._conn.send_data_and_count(rpc_msg)  # todo
            # else:
            #     self._logger.error(f"unknown RPC type: {_rpc_type}")
            #     return
        except:
            self._logger.log_last_except()

    async def forward_to_lobby_server(self, rpc_msg):
        # await self.try_retrieve_lobby_addr()
        self._send_rpc_msg(msg=rpc_msg, ip_port_tuple=self._lobby_addr)
        # self._logger.debug(f'forward_to_lobby_serverxxxxxxxxx, {self.rpc_handler_id=}, {self._lobby_addr=}')


class ProxyCliRpcHandler(RpcHandler):

    def __init__(
            self, rpc_handler_id: bytes, conn: ConnBase = None,
            entity: ServerEntity = None, conn_proto_type: int = PROTO_TYPE_TCP):
        super(ProxyCliRpcHandler, self).__init__(rpc_handler_id, conn, entity, conn_proto_type)
        # cli_2_lobby_map = {}
        # lobby_2_cli_map = {}
        # self.cli_conn = conn
        self._proxy_lobby_rpc_handler = ProxyLobbyRpcHandler(self, rpc_handler_id)  # type: ProxyLobbyRpcHandler

    def uncompress_n_decode(self, rpc_msg):
        # todo
        return rpc_msg

    @wait_or_not()
    async def handle_rpc(self, rpc_msg):
        try:
            rpc_msg = self.uncompress_n_decode(rpc_msg)

            _rpc_msg_tuple = self.do_decode(rpc_msg)  # todo: 不应该解出来的
            _rpc_type = _rpc_msg_tuple[0]

            # _entity_type_str, _method_name, _method_args, _method_kwargs = _rpc_msg_tuple[-4:]
            # self._logger.debug(f'{_entity_type_str=}, {_method_name=}, {_method_args=}, {_method_kwargs=}')

            if _rpc_type == RPC_TYPE_HEARTBEAT:
                self._conn.remote_heart_beat()

            await self._proxy_lobby_rpc_handler.forward_to_lobby_server(rpc_msg)
        except:
            self._logger.log_last_except()






















