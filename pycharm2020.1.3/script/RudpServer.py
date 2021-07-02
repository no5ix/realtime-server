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
import ConnBase
from ConnBase import ROLE_TYPE_PASSIVE
from ConnMgr import ConnMgr
from ProxyRpcHandler import ProxyCliRpcHandler
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
from ServerBase import ServerBase


class RudpServerProtocol(asyncio.DatagramProtocol):
    def __init__(self, create_kcp_conn_cb):
        self._create_kcp_conn_cb = create_kcp_conn_cb
        self.transport = None

    def connection_made(self, transport):
        self.transport = transport

    def datagram_received(self, data, addr):
        assert callable(self._create_kcp_conn_cb)
        self._create_kcp_conn_cb(addr)

        message = data.decode()
        print('Received %r from %s' % (message, addr))
        print('Send %r to %s' % (message, addr))
        self.transport.sendto(data, addr)


class RudpServer(ServerBase):

    def _create_conn(self, transport: transports.BaseTransport) -> TcpConn.TcpConn:
        addr = transport.get_extra_info('peername')  # type: typing.Tuple[str, int]
        tcp_conn = TcpConn.TcpConn(
            ROLE_TYPE_PASSIVE, addr, close_cb=self._on_conn_close, is_proxy=self._is_proxy,
            transport=transport)
        self._addr_2_conn_map[addr] = tcp_conn
        return tcp_conn

    async def _start_server_task(self):
        try:
            _ev_loop = gv.get_ev_loop()
            _transport, _protocol = await _ev_loop.create_datagram_endpoint(
                lambda: RudpServerProtocol(self._create_conn), local_addr=(gv.local_ip, gv.local_port))

            # server = await asyncio.start_server(
            #     self._handle_client_connected, gv.local_ip, gv.local_port)
            # addr = server.sockets[0].getsockname()
            self._logger.info(f'Server on {gv.local_ip, gv.local_port}')

            # async with server:
            #     await server.serve_forever()
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