from __future__ import annotations
import typing
import asyncio

from common import gv
import TcpConn
from core.mobilelog.LogManager import LogManager
from core.util.UtilApi import Singleton

if typing.TYPE_CHECKING:
    from RpcHandler import RpcHandler


@Singleton
class ConnMgr:

    def __init__(self):
        self._addr_2_conn_map = {}  # type: typing.Dict[typing.Tuple[str, int], TcpConn.TcpConn]
        self._logger = LogManager.get_logger()

    def add_conn(
            self,
            role_type: int,
            writer: asyncio.StreamWriter,
            reader: asyncio.StreamReader,
            rpc_handler: RpcHandler = None,
            is_proxy: bool = False
    ):
        addr = writer.get_extra_info("peername")
        conn = TcpConn.TcpConn(
            role_type, addr, writer, reader, rpc_handler,
            close_cb=lambda: self._remove_conn(addr),
            is_proxy=is_proxy
        )
        self._addr_2_conn_map[addr] = conn
        return conn

    def _remove_conn(self, addr: typing.Tuple[str, int]):
        self._addr_2_conn_map.pop(addr, None)

    async def get_conn_by_addr(
            self, addr: typing.Tuple[str, int], rpc_handler: RpcHandler = None) -> TcpConn:
        _conn = self._addr_2_conn_map.get(addr, None)
        if _conn is None:
            reader, writer = await asyncio.open_connection(addr[0], addr[1])
            _conn = self.add_conn(TcpConn.ROLE_TYPE_ACTIVE, writer, reader, rpc_handler)
        return _conn

