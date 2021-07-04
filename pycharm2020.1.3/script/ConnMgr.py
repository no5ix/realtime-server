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

from sanic_jwt_extended import JWT
from sanic_jwt_extended.tokens import Token

if typing.TYPE_CHECKING:
    from RpcHandler import RpcHandler
    import TcpConn


PROTO_TYPE_TCP = 0
PROTO_TYPE_RUDP = 1

ROLE_TYPE_ACTIVE = 0
ROLE_TYPE_PASSIVE = 1


class TcpProtocol(asyncio.Protocol):
    # def __init__(self, role_type, create_tcp_conn_cb):
    def __init__(self, role_type):
    # def __init__(self, rpc_handler=None):
        self._role_type = role_type
        # self._create_tcp_conn_cb = create_tcp_conn_cb
        self._conn = None  # type: Optional[TcpConn.TcpConn]
        # self._rpc_handler = rpc_handler  # type: Optional[RpcHandler]

    def connection_made(self, transport: transports.BaseTransport) -> None:
        # assert callable(self._create_tcp_conn_cb)
        # self._conn = self._create_tcp_conn_cb(self._role_type, transport)
        addr = transport.get_extra_info('peername')  # type: typing.Tuple[str, int]
        if self._role_type == ROLE_TYPE_PASSIVE:
            self._conn = ConnMgr.instance().add_incoming_conn(
                PROTO_TYPE_TCP, transport, addr
                # self._rpc_handler
            )
            LogManager.get_logger().info(f"TCP ROLE_TYPE_PASSIVE peer_addr={addr} is connected !!!!")
        else:
            self._conn = ConnMgr.instance().get_conn(addr, PROTO_TYPE_TCP)
            assert self._conn
            LogManager.get_logger().info(f"TCP ROLE_TYPE_ACTIVE peer_addr={addr} is connected !!!!")

    def data_received(self, data: bytes) -> None:
        self._conn.handle_read(data)

    def connection_lost(self, exc: Optional[Exception]) -> None:
        self._conn.handle_close(str(exc))


# {kSyn = 66, kAck, kPsh, kRst};
RUDP_HANDSHAKE_SYN = b'new_byte_hello_its_me'
RUDP_HANDSHAKE_SYN_ACK_PREFIX = b'new_byte_welcome:'
RUDP_HANDSHAKE_ACK_PREFIX = b'new_byte_ack:'

RUDP_CONV = 0


class RudpProtocol(asyncio.DatagramProtocol):
    # def __init__(self, create_kcp_conn_cb):
    def __init__(self):
        # self._create_kcp_conn_cb = create_kcp_conn_cb
        with JWT.initialize() as manager:
            manager.config.secret_key = "new_byte"

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
        # print(f'datagram_received: {data=}, {addr=}')
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
            conv = int(token_obj.identity)

            ConnMgr.instance().add_incoming_conn(
                PROTO_TYPE_RUDP, self.transport, addr, rudp_conv=conv
                # self._rpc_handler
            )
            # addr = transport.get_extra_info('peername')  # type: typing.Tuple[str, int]
            LogManager.get_logger().info(f"RUDP ROLE_TYPE_PASSIVE peer_addr={addr} is connected !!!!")
        elif data.startswith(RUDP_HANDSHAKE_SYN_ACK_PREFIX):
            parts = data.split(RUDP_HANDSHAKE_SYN_ACK_PREFIX, 2)
            if len(parts) != 2:
                raise Exception(f"Expected value '{RUDP_HANDSHAKE_SYN_ACK_PREFIX}<JWT>'")

            raw_jwt = parts[1].decode()
            token_obj = Token(raw_jwt)
            if token_obj.type != "access":
                raise Exception("Only access tokens are allowed")
            self.transport.sendto(RUDP_HANDSHAKE_ACK_PREFIX + raw_jwt.encode(), addr)
            conv = int(token_obj.identity)
            ConnMgr.instance().set_fut_result(addr, conv)

            # ConnMgr.instance().add_incoming_conn(
            #     ROLE_TYPE_ACTIVE, CONN_TYPE_RUDP, self.transport,
            #     rudp_conv=conv, rudp_peer_addr=addr
            #     # self._rpc_handler
            # )
            # addr = transport.get_extra_info('peername')  # type: typing.Tuple[str, int]
            LogManager.get_logger().info(f"RUDP ROLE_TYPE_ACTIVE peer_addr={addr} is connected !!!!")
        else:
            _cur_conn = ConnMgr.instance().get_conn(addr, PROTO_TYPE_RUDP)
            assert _cur_conn
            _cur_conn.handle_read(data)


@Singleton
class ConnMgr:

    def __init__(self):
        self._timer_hub = TimerHub()
        self._logger = LogManager.get_logger()
        self._is_proxy = False
        self._proto_type_2_addr_2_conn = {PROTO_TYPE_TCP: {}, PROTO_TYPE_RUDP: {}}
        self._addr_2_try_connect_times = defaultdict(int)
        self._addr_2_rudp_conned_fut = {}

    def create_rudp_conned_fut(self, addr):
        _fut = gv.get_ev_loop().create_future()
        self._addr_2_rudp_conned_fut[addr] = _fut

        def final_fut_cb(fut, _addr=addr):
            # self._logger.info(f"final_fut_cb: rid={rid}")
            self._addr_2_rudp_conned_fut.pop(_addr, None)

        _fut.add_done_callback(final_fut_cb)
        return _fut

    def set_fut_result(self, addr, conv):
        _fut = self._addr_2_rudp_conned_fut.get(addr, None)
        if _fut is None:
            return
        try:
            _fut.set_result(conv)
        except asyncio.InvalidStateError:
            # 并发情况即使`fut`已经done了也无所谓, 不处理即可
            pass

    def set_is_proxy(self, is_proxy):
        self._is_proxy = is_proxy

    def get_conn(self, addr, proto_type=None) -> ConnBase:
        if proto_type is not None:
            return self._proto_type_2_addr_2_conn[proto_type].get(addr, None)
        _conn = self._proto_type_2_addr_2_conn[PROTO_TYPE_RUDP].get(addr, None)
        if _conn is None:
            _conn = self._proto_type_2_addr_2_conn[PROTO_TYPE_TCP].get(addr, None)
        return _conn

    def add_incoming_conn(
            self, proto_type, transport: transports.BaseTransport,
            peer_addr: Optional[Tuple[str, int]] = None,
            rudp_conv: int = None
            # rpc_handler: Optional[RpcHandler] = None
    ) -> ConnBase:
        if proto_type == PROTO_TYPE_TCP:
            from TcpConn import TcpConn
            _conn = TcpConn(
                ROLE_TYPE_PASSIVE, peer_addr,
                close_cb=lambda ct=proto_type, a=peer_addr: self._remove_conn(ct, a),
                is_proxy=self._is_proxy,
                transport=transport)
        else:
            from RudpConn import RudpConn
            assert rudp_conv > 0
            _conn = RudpConn(
                ROLE_TYPE_PASSIVE, peer_addr,
                close_cb=lambda ct=proto_type, a=peer_addr: self._remove_conn(ct, a),
                is_proxy=self._is_proxy,
                transport=transport,
                conv=rudp_conv)

        self._proto_type_2_addr_2_conn[proto_type][peer_addr] = _conn
        return _conn

    async def open_conn_by_addr(
            self, proto_type, addr: typing.Tuple[str, int],
            rpc_handler: RpcHandler = None
    ) -> (ConnBase, bool):

        _conn = self._proto_type_2_addr_2_conn[proto_type].get(addr, None)
        if _conn is None:
            if proto_type == PROTO_TYPE_TCP:
                from TcpConn import TcpConn
                conn_cls = TcpConn
                # addr = transport.get_extra_info('peername')  # type: typing.Tuple[str, int]
                # _conn = TcpConn.TcpConn(
                #     ROLE_TYPE_ACTIVE, addr,
                #     close_cb=lambda ct=proto_type, a=addr: self._remove_conn(ct, a),
                #     is_proxy=self._is_proxy,
                #     # transport=transport
                # )
            else:
                pass  # todo: rudp
                from RudpConn import RudpConn
                conn_cls = RudpConn

                # assert rudp_conv > 0
                # addr = rudp_peer_addr
            _conn = conn_cls(
                ROLE_TYPE_ACTIVE, addr,
                rpc_handler=rpc_handler,
                close_cb=lambda ct=proto_type, a=addr: self._remove_conn(ct, a),
                is_proxy=self._is_proxy,
                # transport=transport
            )

            self._proto_type_2_addr_2_conn[proto_type][addr] = _conn
            # if rpc_handler is not None:
            #     _conn.add_rpc_handler(rpc_handler)
            is_conned = await _conn.try_connect()
        else:
            _conn.add_rpc_handler(rpc_handler)  # 不加这个会造成 proxy 类的服务器转发混乱
            is_conned = _conn.is_connected()
        return _conn, is_conned

    def _remove_conn(self, proto_type, addr: typing.Tuple[str, int]):
        self._proto_type_2_addr_2_conn[proto_type].pop(addr, None)

    # def add_incoming_conn(
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
    # async def open_conn_by_addr(
    #         self, addr: typing.Tuple[str, int], rpc_handler: RpcHandler = None) -> TcpConn:
    #     _conn = self._addr_2_conn_map.get(addr, None)
    #     if _conn is None:
    #         reader, writer = await asyncio.open_connection(addr[0], addr[1])
    #         _conn = self.add_incoming_conn(ConnBase.ROLE_TYPE_ACTIVE, writer, reader, rpc_handler)
    #     if rpc_handler is not None:
    #         _conn.add_rpc_handler(rpc_handler)
    #     return _conn

