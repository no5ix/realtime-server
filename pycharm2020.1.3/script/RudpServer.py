from __future__ import annotations
import asyncio
import datetime
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
from ConnMgr import ConnMgr, CONN_TYPE_RUDP
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
from core.common import EntityScanner, rudp
from core.common.EntityFactory import EntityFactory
from core.tool import incremental_reload
# from core.tool import incremental_reload

# TCP_SERVER = None
from ServerBase import ServerBase
from core.common.sanic_jwt_extended import JWT, refresh_jwt_required, jwt_required, jwt_optional
from core.common.sanic_jwt_extended.tokens import Token


# {kSyn = 66, kAck, kPsh, kRst};
RUDP_HANDSHAKE_SYN = b'new_byte_hello_its_me'
RUDP_HANDSHAKE_SYN_ACK_PREFIX = b'new_byte_welcome:'
RUDP_HANDSHAKE_ACK_PREFIX = b'new_byte_ack:'

RUDP_CONV = 0


class RudpServerProtocol(asyncio.DatagramProtocol):
    # def __init__(self, create_kcp_conn_cb):
    def __init__(self):
        # self._create_kcp_conn_cb = create_kcp_conn_cb
        self.transport = None

    def connection_made(self, transport):
        self.transport = transport

    def datagram_received(self, data: bytes, addr):
        global RUDP_CONV
        global RUDP_HANDSHAKE_SYN
        global RUDP_HANDSHAKE_SYN_ACK_PREFIX
        global RUDP_HANDSHAKE_ACK_PREFIX
        # assert callable(self._create_kcp_conn_cb)
        # self._create_kcp_conn_cb(addr)
        if data == RUDP_HANDSHAKE_SYN:
            RUDP_CONV += 1
            access_token_jwt: bytes = JWT.create_access_token(
                identity=str(RUDP_CONV), expires_delta=datetime.timedelta(seconds=6)).encode()
            self.transport.sendto(RUDP_HANDSHAKE_SYN_ACK_PREFIX + access_token_jwt, addr)
        elif data.startswith(RUDP_HANDSHAKE_ACK_PREFIX):
            parts = data.split(RUDP_HANDSHAKE_ACK_PREFIX, 2)
            if len(parts) != 2:
                raise Exception(f"Expected value '{RUDP_HANDSHAKE_ACK_PREFIX}<JWT>'")
            raw_jwt = parts[1].decode()
            token_obj = Token(raw_jwt)
            if token_obj.type != "access":
                raise Exception("Only access tokens are allowed")
            conv = token_obj.identity

            ConnMgr.instance().create_conn(
                ROLE_TYPE_PASSIVE, CONN_TYPE_RUDP, self.transport,
                rudp_conv=conv, rudp_peer_addr=addr
                # self._rpc_handler
            )
            # addr = transport.get_extra_info('peername')  # type: typing.Tuple[str, int]
            LogManager.get_logger().info(f"{addr!r} is connected !!!!")
        else:
            _cur_conn = ConnMgr.instance().get_conn(addr, CONN_TYPE_RUDP)
            if _cur_conn:
                _cur_conn.handle_read(data)


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
