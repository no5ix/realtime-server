from __future__ import annotations
import asyncio
import functools
# import json
import random
import signal
import platform
import sys
# from random import random
import socket

# from PuppetBindEntity import PuppetBindEntity
# from battle_entity.Puppet import Puppet
# from core.common import MsgpackSupport
from asyncio import events, tasks, transports

import typing

# from core.util.performance.cpu_load_handler import CpuLoad
from typing import Optional

import ConnBase
# from ConnBase import ROLE_TYPE_PASSIVE, ROLE_TYPE_ACTIVE
from ConnMgr import ConnMgr, CONN_TYPE_TCP, TcpProtocol, ROLE_TYPE_PASSIVE
from ProxyRpcHandler import ProxyCliRpcHandler
from ServerBase import ServerBase
from core.util import UtilApi
from core.util.UtilApi import wait_or_not, Singleton
from RpcHandler import get_a_rpc_handler_id

if typing.TYPE_CHECKING:
    from RpcHandler import RpcHandler

import TcpConn
# from common import gv
from core.EtcdSupport import ServiceNode
# from core.common import EntityScanner
# from core.common.EntityFactory import EntityFactory
from core.mobilelog.LogManager import LogManager
# from core.util import EnhancedJson
from core.util.TimerHub import TimerHub
# from util.SingletonEntityManager import SingletonEntityManager
from common import gv
from core.common import EntityScanner
from core.common.EntityFactory import EntityFactory
from core.tool import incremental_reload
# from core.tool import incremental_reload

# TCP_SERVER = None


# class TcpClientProtocol(asyncio.Protocol):
#     def __init__(self):
#     # def __init__(self):
#         # self._role_type = role_type
#         # self._create_tcp_conn_cb = create_tcp_conn_cb
#         self._conn = None  # type: Optional[TcpConn.TcpConn]
#
#     def connection_made(self, transport: transports.BaseTransport) -> None:
#         # assert callable(self._create_tcp_conn_cb)
#         # self._conn = self._create_tcp_conn_cb(self._role_type, transport)
#         self._conn = ConnMgr.instance().add_incoming_conn(ROLE_TYPE_PASSIVE, CONN_TYPE_TCP, transport)
#
#     def data_received(self, data: bytes) -> None:
#         self._conn.handle_read(data)
#
#     def connection_lost(self, exc: Optional[Exception]) -> None:
#         self._conn.handle_close(str(exc))


class TcpServer(ServerBase):

    # def _create_conn(self, role_type, transport: transports.BaseTransport) -> TcpConn.TcpConn:
    #     addr = transport.get_extra_info('peername')  # type: typing.Tuple[str, int]
    #     tcp_conn = TcpConn.TcpConn(
    #         role_type, addr, close_cb=self._on_conn_close, is_proxy=self._is_proxy,
    #         transport=transport)
    #     self._addr_2_conn_map[addr] = tcp_conn
    #     return tcp_conn
    #
    # async def open_conn_by_addr(
    #         self, addr: typing.Tuple[str, int], rpc_handler: RpcHandler = None) -> TcpConn:
    #     _conn = self._addr_2_conn_map.get(addr, None)
    #     if _conn is None:
    #         transport, protocol = await self._ev_loop.create_connection(
    #             lambda: TcpServerProtocol(ROLE_TYPE_ACTIVE, self._create_conn),
    #             addr[0], addr[1])
    #     if rpc_handler is not None:
    #         _conn.add_rpc_handler(rpc_handler)
    #     return _conn

    async def _start_server_task(self):
        try:
            # _ev_loop = gv.get_ev_loop()
            server = await self._ev_loop.create_server(
                lambda: TcpProtocol(ROLE_TYPE_PASSIVE),
                gv.local_ip, gv.local_port)

            addr = server.sockets[0].getsockname()
            self._logger.info(f'TCP Server on {addr}')
            async with server:
                await server.serve_forever()
        except KeyboardInterrupt:
            self._logger.info(f"\nShutting Down Server: {gv.server_name}...\n")
            # _loop = asyncio.get_running_loop()
            # _loop.stop()
            # _loop.close()
            # server.close()

            return
        except:
            # self.logger.info("Unexpected error:", sys.exc_info()[0])
            self._logger.log_last_except()
            raise


if __name__ == '__main__':
    game_server_name = sys.argv[1]
    server_json_conf_path = r"../bin/win/conf/battle_server.json"
    tcp_server = TcpServer(game_server_name, server_json_conf_path)
    # TCP_SERVER = tcp_server
    tcp_server.run()

    # loop = asyncio.get_event_loop()
    # loop.run_until_complete(main())
