from __future__ import annotations

import asyncio
# import collections
import functools
import typing
from asyncio import futures

import time
from typing import Optional

import core.util.UtilApi
from ConnMgr import ConnMgr
from RpcHandler import RpcHandler, RPC_TYPE_HEARTBEAT, RPC_TYPE_NOTIFY, RPC_TYPE_REQUEST, RPC_TYPE_REPLY
from RpcHandler import RpcReplyFuture
from common.service_const import ETCD_TAG_LOBBY_SRV
from core.util import EnhancedJson
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
            self, proxy_cli_rpc_handler: ProxyCliRpcHandler, conn: TcpConn = None,
            entity: ServerEntity = None, ):
        super(ProxyLobbyRpcHandler, self).__init__(conn, entity)
        self.proxy_cli_rpc_handler = proxy_cli_rpc_handler  # type: Optional[ProxyCliRpcHandler]
        self._lobby_addr = None  # type: Optional[tuple]
        self.try_retrieve_lobby_addr()

    def compress_n_encode(self, rpc_msg):
        # todo
        return rpc_msg

    @wait_or_not()
    async def handle_rpc(self, rpc_msg):
        try:
            rpc_msg = self.compress_n_encode(rpc_msg)

            _rpc_msg_tuple = self.do_decode(rpc_msg)  # todo: 不应该解出来的
            _rpc_type = _rpc_msg_tuple[0]

            if _rpc_type == RPC_TYPE_HEARTBEAT:
                self._conn.remote_heart_beat()
            self.proxy_cli_rpc_handler._conn.send_data_and_count(rpc_msg)  # todo
            # elif _rpc_type in (RPC_TYPE_NOTIFY, RPC_TYPE_REQUEST, RPC_TYPE_REPLY,):
            #     await self.proxy_cli_rpc_handler._conn.send_data_and_count(rpc_msg)  # todo
            # else:
            #     self._logger.error(f"unknown RPC type: {_rpc_type}")
            #     return
        except:
            self._logger.log_last_except()

    async def forward_to_lobby_server(self, rpc_msg):
        self.try_retrieve_lobby_addr()
        await self._send_rpc_msg(msg=rpc_msg, ip_port_tuple=self._lobby_addr)

    @wait_or_not()
    @async_lock
    async def try_retrieve_lobby_addr(self):
        if self._lobby_addr is None:
            dispatcher_json_conf_path = r"../bin/win/conf/dispatcher_service.json"

            # dispatcher_json_conf = None
            with open(dispatcher_json_conf_path) as conf_file:
                dispatcher_json_conf = EnhancedJson.load(conf_file)

            # UtilApi.parse_json_conf(json_conf_path)

            rand_dispatcher_service_addr = None
            for _svr_name, _svr_info in dispatcher_json_conf.items():
                if type(_svr_info) is dict and _svr_name.startswith("dispatcher"):
                    rand_dispatcher_service_addr = (_svr_info["ip"], _svr_info["port"])
                    break

            temp_se = ServerEntity()
            # _err, _lobby_addr = self.request_rpc(
            start_time = time.time()
            # self._logger.info(f'start: {start_time=}')
            print(f'ETCD_TAG_LOBBY_SRV success0000 !!!{rand_dispatcher_service_addr=}')

            # todo: uncomment
            # _err, _lobby_addr_info = await temp_se.call_remote_method(
            #     "pick_lowest_load_service_addr",
            #     # [gv.etcd_tag],
            #     # ["battle_server"],
            #     # ["lobby_server"],
            #     [ETCD_TAG_LOBBY_SRV],
            #     rpc_reply_timeout=None,  # todo: 时常会timeout
            #     # rpc_remote_entity_type="LoadCollector", ip_port_tuple=dispatcher_service_addr
            #     # rpc_callback=lambda err, res: self.logger.info(f"pick_lowest_load_service_addr: {err=} {res=}"),
            #     rpc_remote_entity_type="LoadCollector",
            #     ip_port_tuple=rand_dispatcher_service_addr)
            _err, _lobby_addr_info = None, (None, '127.0.0.1', 10001)
            end_time = time.time()
            offset = end_time - start_time
            self._logger.info(f'end: {offset=}')
            print(f'ETCD_TAG_LOBBY_SRV success0.5 !!! {_lobby_addr_info=}')

            if _err:
                self._logger.error(f'{_err=}')
                # return
            if not _lobby_addr_info:
                self._logger.error(f'lobby addr empty')
                # if rpc_msg is not None:
                #     self._msg_buffer.append(rpc_msg)
                return

            self._lobby_addr = _lobby_addr_info[1:]
            print(f'ETCD_TAG_LOBBY_SRV success111 !!! {_lobby_addr_info=}')
            print(f'ETCD_TAG_LOBBY_SRV success333 !!! {_lobby_addr_info=}')


class ProxyCliRpcHandler(RpcHandler):

    def __init__(
            self, conn: TcpConn = None,
            entity: ServerEntity = None):
        super(ProxyCliRpcHandler, self).__init__(conn, entity)
        # cli_2_lobby_map = {}
        # lobby_2_cli_map = {}
        # self.cli_conn = conn
        # self._conn = None  # 这里指的是 和大厅的连接
        self._proxy_lobby_rpc_handler = ProxyLobbyRpcHandler(self)  # type: ProxyLobbyRpcHandler

    def uncompress_n_decode(self, rpc_msg):
        # todo
        return rpc_msg

    @wait_or_not()
    async def handle_rpc(self, rpc_msg):
        try:
            rpc_msg = self.uncompress_n_decode(rpc_msg)

            _rpc_msg_tuple = self.do_decode(rpc_msg)  # todo: 不应该解出来的
            _rpc_type = _rpc_msg_tuple[0]

            if _rpc_type == RPC_TYPE_HEARTBEAT:
                self._conn.remote_heart_beat()

            await self._proxy_lobby_rpc_handler.forward_to_lobby_server(rpc_msg)

            # self._proxy_lobby_rpc_handler._conn.send_data_and_count(rpc_msg)  # todo
            # elif _rpc_type in (RPC_TYPE_NOTIFY, RPC_TYPE_REQUEST, RPC_TYPE_REPLY,):
            #
            #     print(f'ETCD_TAG_LOBBY_SRV success444 !!!')
            #     await self._proxy_lobby_rpc_handler._conn.send_data_and_count(rpc_msg)  # todo
            # else:
            #     self._logger.error(f"unknown RPC type: {_rpc_type}")
            #     return
        except:
            self._logger.log_last_except()
        # return self._proxy_lobby_rpc_handler






















