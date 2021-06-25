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
from RpcHandler import RpcHandler
from RpcHandler import RpcReplyFuture
from common.service_const import ETCD_TAG_LOBBY_SRV
from core.util import EnhancedJson
from core.util.TimerHub import TimerHub
from core.util.UtilApi import wait_or_not

if typing.TYPE_CHECKING:
    from TcpConn import TcpConn
    from server_entity.ServerEntity import ServerEntity

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

    def compress_n_encode(self, rpc_msg):
        # todo
        return rpc_msg

    @wait_or_not()
    def handle_rpc(self, rpc_msg):
        try:
            rpc_msg = self.compress_n_encode(rpc_msg)
            self.proxy_cli_rpc_handler._send_rpc_msg(rpc_msg)  # todo
        except:
            self._logger.log_last_except()


class ProxyCliRpcHandler(RpcHandler):

    def __init__(
            self, conn: TcpConn = None,
            entity: ServerEntity = None):
        super(ProxyCliRpcHandler, self).__init__(conn, entity)
        # cli_2_lobby_map = {}
        # lobby_2_cli_map = {}
        # self.cli_conn = conn
        # self._conn = None  # 这里指的是 和大厅的连接
        self.proxy_lobby_rpc_handler = None  # type: Optional[ProxyLobbyRpcHandler]

    def uncompress_n_decode(self, rpc_msg):
        # todo
        return rpc_msg

    @wait_or_not()
    def handle_rpc(self, rpc_msg):
        try:
            rpc_msg = self.uncompress_n_decode(rpc_msg)
            if self.proxy_lobby_rpc_handler is None:
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
                _err, _lobby_addr = await temp_se.call_remote_method(
                    "pick_lowest_load_service_addr",
                    # [gv.etcd_tag],
                    # ["battle_server"],
                    # ["lobby_server"],
                    [ETCD_TAG_LOBBY_SRV],
                    # rpc_reply_timeout=None,
                    # rpc_remote_entity_type="LoadCollector", ip_port_tuple=dispatcher_service_addr
                    # rpc_callback=lambda err, res: self.logger.info(f"pick_lowest_load_service_addr: {err=} {res=}"),
                    rpc_remote_entity_type="LoadCollector",
                    ip_port_tuple=rand_dispatcher_service_addr)
                if _err:
                    self._logger.error(f'{_err=}')
                    return

                plrh = ProxyLobbyRpcHandler(self)
                await ConnMgr.instance().get_conn_by_addr(addr=_lobby_addr, rpc_handler=plrh)
                self.proxy_lobby_rpc_handler = plrh
                # await self._send_rpc_msg(msg=rpc_msg, ip_port_tuple=_lobby_addr)
            self.proxy_lobby_rpc_handler._send_rpc_msg(rpc_msg)  # todo
        except:
            self._logger.log_last_except()

# class Tcp























