from __future__ import annotations
import typing
import asyncio
from asyncio import transports
from typing import Optional

import ConnBase
from ConnBase import ROLE_TYPE_ACTIVE
from common import gv
import TcpConn
from core.mobilelog.LogManager import LogManager
from core.util.UtilApi import Singleton

if typing.TYPE_CHECKING:
    from RpcHandler import RpcHandler


CONN_TYPE_TCP = 0
CONN_TYPE_RUDP = 1


@Singleton
class ConnMgr:

    def __init__(self):
        self._addr_2_conn_map = {}  # type: typing.Dict[typing.Tuple[str, int], TcpConn.TcpConn]
        self._logger = LogManager.get_logger()
        self._is_proxy = False

    def set_is_proxy(self, is_proxy):
        self._is_proxy = is_proxy

    def create_conn(
            self, role_type, conn_type, transport: transports.BaseTransport,
            # rpc_handler: Optional[RpcHandler] = None
    ) -> TcpConn.TcpConn:
        addr = transport.get_extra_info('peername')  # type: typing.Tuple[str, int]

        if conn_type == CONN_TYPE_TCP:
            tcp_conn = TcpConn.TcpConn(
                role_type, addr, close_cb=self._remove_conn, is_proxy=self._is_proxy,
                transport=transport)
        else:
            pass  # todo: rudp
        self._addr_2_conn_map[addr] = tcp_conn
        return tcp_conn

    async def get_conn_by_addr(
            self, conn_type, addr: typing.Tuple[str, int],
            rpc_handler: RpcHandler = None
    ) -> TcpConn:
        from TcpServer import TcpServerProtocol
        _conn = self._addr_2_conn_map.get(addr, None)
        if _conn is None:
            if conn_type == CONN_TYPE_TCP:
                # reader, writer = await asyncio.open_connection(addr[0], addr[1])
                transport, protocol = await gv.EV_LOOP.create_connection(
                    lambda: TcpServerProtocol(),
                    # lambda: TcpServerProtocol(rpc_handler),
                    addr[0], addr[1])
                _conn = self._addr_2_conn_map[addr]
            else:
                pass  # todo: rudp
        if rpc_handler is not None:
            _conn.add_rpc_handler(rpc_handler)
        return _conn

    def _remove_conn(self, addr: typing.Tuple[str, int]):
        self._addr_2_conn_map.pop(addr, None)

    # def add_conn(
    #         self,
    #         role_type: int,
    #         writer: asyncio.StreamWriter,
    #         reader: asyncio.StreamReader,
    #         rpc_handler: RpcHandler = None,
    #         # is_proxy: bool = False
    # ):
    #     addr = writer.get_extra_info("peername")
    #     conn = TcpConn.TcpConn(
    #         role_type, addr, writer, reader, rpc_handler,
    #         close_cb=lambda: self._remove_conn(addr),
    #         is_proxy=self._is_proxy
    #     )
    #     self._addr_2_conn_map[addr] = conn
    #     return conn
    #
    # async def get_conn_by_addr(
    #         self, addr: typing.Tuple[str, int], rpc_handler: RpcHandler = None) -> TcpConn:
    #     _conn = self._addr_2_conn_map.get(addr, None)
    #     if _conn is None:
    #         reader, writer = await asyncio.open_connection(addr[0], addr[1])
    #         _conn = self.add_conn(ConnBase.ROLE_TYPE_ACTIVE, writer, reader, rpc_handler)
    #     if rpc_handler is not None:
    #         _conn.add_rpc_handler(rpc_handler)
    #     return _conn

