from __future__ import annotations

import datetime
import typing
import asyncio
from asyncio import transports
from collections import defaultdict
from typing import Optional, Tuple

import ConnBase
from common import gv
from core.mobilelog.LogManager import LogManager
from core.util.TimerHub import TimerHub
from core.util.UtilApi import Singleton

from core.common.sanic_jwt_extended import JWT, refresh_jwt_required, jwt_required, jwt_optional
from core.common.sanic_jwt_extended.tokens import Token

if typing.TYPE_CHECKING:
    from RpcHandler import RpcHandler, RPC_TYPE_REQUEST
    import TcpConn


CONN_TYPE_TCP = 0
CONN_TYPE_RUDP = 1

RECONNECT_MAX_TIMES = 6
RECONNECT_INTERVAL = 0.6  # sec

ROLE_TYPE_ACTIVE = 0
ROLE_TYPE_PASSIVE = 1


class TcpServerProtocol(asyncio.Protocol):
    # def __init__(self, role_type, create_tcp_conn_cb):
    def __init__(self):
    # def __init__(self, rpc_handler=None):
        # self._role_type = role_type
        # self._create_tcp_conn_cb = create_tcp_conn_cb
        self._conn = None  # type: Optional[TcpConn.TcpConn]
        # self._rpc_handler = rpc_handler  # type: Optional[RpcHandler]

    def connection_made(self, transport: transports.BaseTransport) -> None:
        # assert callable(self._create_tcp_conn_cb)
        # self._conn = self._create_tcp_conn_cb(self._role_type, transport)
        self._conn = ConnMgr.instance().add_conn(
            ROLE_TYPE_PASSIVE, CONN_TYPE_TCP, transport,
            # self._rpc_handler
        )

        addr = transport.get_extra_info('peername')  # type: typing.Tuple[str, int]
        LogManager.get_logger().info(f"{addr!r} is connected !!!!")

    def data_received(self, data: bytes) -> None:
        self._conn.handle_read(data)

    def connection_lost(self, exc: Optional[Exception]) -> None:
        self._conn.handle_close(str(exc))


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

            ConnMgr.instance().add_conn(
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


@Singleton
class ConnMgr:

    def __init__(self):
        self._timer_hub = TimerHub()
        self._logger = LogManager.get_logger()
        self._is_proxy = False
        self._conn_type_2_addr_2_conn = {CONN_TYPE_TCP: {}, CONN_TYPE_RUDP: {}}
        self._addr_2_try_connect_times = defaultdict(int)

    def set_is_proxy(self, is_proxy):
        self._is_proxy = is_proxy

    def get_conn(self, addr, conn_type=None) -> ConnBase:
        if conn_type is not None:
            return self._conn_type_2_addr_2_conn[conn_type].get(addr, None)
        _conn = self._conn_type_2_addr_2_conn[CONN_TYPE_RUDP].get(addr, None)
        if _conn is None:
            _conn = self._conn_type_2_addr_2_conn[CONN_TYPE_TCP].get(addr, None)
        return _conn

    def add_conn(
            self, role_type, conn_type, transport: transports.BaseTransport,
            rudp_conv: int = 0, rudp_peer_addr: Optional[Tuple[str, int]] = None
            # rpc_handler: Optional[RpcHandler] = None
    ) -> ConnBase:
        if conn_type == CONN_TYPE_TCP:
            import TcpConn
            addr = transport.get_extra_info('peername')  # type: typing.Tuple[str, int]
            _conn = TcpConn.TcpConn(
                role_type, addr,
                close_cb=lambda ct=conn_type, a=addr: self._remove_conn(ct, a),
                is_proxy=self._is_proxy,
                transport=transport)
        else:
            pass  # todo: rudp
            from RudpConn import RudpConn
            assert rudp_conv > 0
            addr = rudp_peer_addr
            _conn = RudpConn(
                rudp_conv, role_type, addr,
                close_cb=lambda ct=conn_type, a=addr: self._remove_conn(ct, a),
                is_proxy=self._is_proxy,
                transport=transport)

        self._conn_type_2_addr_2_conn[conn_type][addr] = _conn
        return _conn

    async def create_conn_by_addr(
            self, conn_type, addr: typing.Tuple[str, int],
            rpc_handler: RpcHandler = None
    ) -> ConnBase:
        _conn = self._conn_type_2_addr_2_conn[conn_type].get(addr, None)
        if _conn is None:
            if conn_type == CONN_TYPE_RUDP:
                _conn = await self._try_create_rudp_conn(addr)
            if _conn is None:
                _conn = await self._try_create_tcp_conn(addr)
        if _conn is not None and rpc_handler is not None:
            _conn.add_rpc_handler(rpc_handler)
        return _conn

    async def _try_create_rudp_conn(self, addr):
        pass

    async def _try_create_tcp_conn(self, addr):
        while self._addr_2_try_connect_times[addr] < RECONNECT_MAX_TIMES:
            try:
                from TcpServer import TcpServerProtocol
                self._addr_2_try_connect_times[addr] += 1
                transport, protocol = await gv.get_ev_loop().create_connection(
                    lambda: TcpServerProtocol(),
                    addr[0], addr[1])
            except Exception as e:
                self._logger.error(str(e))
                await asyncio.sleep(RECONNECT_INTERVAL)
                # if self._addr_2_try_connect_times[addr] < RECONNECT_MAX_TIMES:
                self._logger.warning(f"try reconnect tcp: {str(addr)} ... {self._addr_2_try_connect_times[addr]}")
                await self._try_create_tcp_conn(addr)
            else:
                self._addr_2_try_connect_times[addr] = 0
                return self._conn_type_2_addr_2_conn[CONN_TYPE_TCP][addr]
        else:
            pass  # todo
            self._logger.error(f"try {RECONNECT_MAX_TIMES} times , still can't connect remote addr: {addr}")
            self._addr_2_try_connect_times[addr] = 0
            return None

    def _remove_conn(self, conn_type, addr: typing.Tuple[str, int]):
        self._conn_type_2_addr_2_conn[conn_type].pop(addr, None)

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
    # async def create_conn_by_addr(
    #         self, addr: typing.Tuple[str, int], rpc_handler: RpcHandler = None) -> TcpConn:
    #     _conn = self._addr_2_conn_map.get(addr, None)
    #     if _conn is None:
    #         reader, writer = await asyncio.open_connection(addr[0], addr[1])
    #         _conn = self.add_conn(ConnBase.ROLE_TYPE_ACTIVE, writer, reader, rpc_handler)
    #     if rpc_handler is not None:
    #         _conn.add_rpc_handler(rpc_handler)
    #     return _conn

