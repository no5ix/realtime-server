from __future__ import annotations

import datetime
import typing
import asyncio
from asyncio import transports
from collections import defaultdict
from time import sleep
from typing import Optional, Tuple

from jwt import ExpiredSignatureError

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
    def __init__(self, role_type):
        self._role_type = role_type
        self._conn = None  # type: Optional[TcpConn.TcpConn]

    def connection_made(self, transport: transports.BaseTransport) -> None:
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


RUDP_HANDSHAKE_SYN = b'new_byte_hello_its_me'
RUDP_HANDSHAKE_SYN_ACK_PREFIX = b'new_byte_welcome:'
RUDP_HANDSHAKE_ACK_PREFIX = b'new_byte_ack:'

RUDP_JWT_EXP = 6
RUDP_CONV = 0


class RudpProtocol(asyncio.DatagramProtocol):
    def __init__(self):
        with JWT.initialize() as manager:
            manager.config.secret_key = "new_byte"

        self.transport = None
        self.logger = LogManager.get_logger()

    def connection_made(self, transport):
        self.transport = transport

    def datagram_received(self, data: bytes, addr):
        global RUDP_CONV
        global RUDP_JWT_EXP
        global RUDP_HANDSHAKE_SYN
        global RUDP_HANDSHAKE_SYN_ACK_PREFIX
        global RUDP_HANDSHAKE_ACK_PREFIX

        # print(f'datagram_received: {data=}, {addr=}')
        if data == RUDP_HANDSHAKE_SYN:
            RUDP_CONV += 1
            access_token_jwt: bytes = JWT.create_access_token(
                identity=str(RUDP_CONV), expires_delta=datetime.timedelta(seconds=RUDP_JWT_EXP)).encode()

            exp = int((Token(access_token_jwt.decode()).exp - datetime.datetime.utcnow()).total_seconds())
            # print(f'aaa {exp=}')

            self.transport.sendto(RUDP_HANDSHAKE_SYN_ACK_PREFIX + access_token_jwt, addr)
        elif data.startswith(RUDP_HANDSHAKE_ACK_PREFIX):
            parts = data.split(RUDP_HANDSHAKE_ACK_PREFIX, 2)
            if len(parts) != 2:
                raise Exception(f"Expected value '{RUDP_HANDSHAKE_ACK_PREFIX}<JWT>'")
            # sleep(2)
            raw_jwt = parts[1].decode()
            try:
                token_obj = Token(raw_jwt)
                # exp = int((token_obj.exp - datetime.datetime.utcnow()).total_seconds())
                # print(f'xxx {exp=}')
            except ExpiredSignatureError as e:
                self.logger.warning(str(e))
                return
            except:
                self.logger.log_last_except()
                return

            if token_obj.type != "access":
                raise Exception("Only access tokens are allowed")
            conv = int(token_obj.identity)

            ConnMgr.instance().add_incoming_conn(
                PROTO_TYPE_RUDP, self.transport, addr, rudp_conv=conv
            )

            self.logger.info(f"RUDP ROLE_TYPE_PASSIVE peer_addr={addr} is connected !!!!")
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

            self.logger.info(f"RUDP ROLE_TYPE_ACTIVE peer_addr={addr} is connected !!!!")
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
            rudp_conv: int = None) -> ConnBase:
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
            rpc_handler: RpcHandler = None) -> ConnBase:

        _conn: ConnBase = self._proto_type_2_addr_2_conn[proto_type].get(addr, None)
        if _conn is None:
            if proto_type == PROTO_TYPE_TCP:
                from TcpConn import TcpConn
                conn_cls = TcpConn
            else:
                from RudpConn import RudpConn
                conn_cls = RudpConn
            _conn: ConnBase = conn_cls(
                ROLE_TYPE_ACTIVE, addr,
                rpc_handler=rpc_handler,
                close_cb=lambda ct=proto_type, a=addr: self._remove_conn(ct, a),
                is_proxy=self._is_proxy,
                # transport=transport
            )

            self._proto_type_2_addr_2_conn[proto_type][addr] = _conn
            # if rpc_handler is not None:
            #     _conn.add_rpc_handler(rpc_handler)
            await _conn.try_connect()
            # if not is_conned:
            #     _conn.handle_close(close_reason='try connecting failed')
            #     _conn = None
        else:
            _conn.add_rpc_handler(rpc_handler)  # 不加这个会造成 proxy 类的服务器转发混乱
            # is_conned = _conn.is_connected()
            if _conn.is_disconnected_or_disconnecting():
                await _conn.try_connect()
        return _conn

    def _remove_conn(self, proto_type, addr: typing.Tuple[str, int]):
        self._proto_type_2_addr_2_conn[proto_type].pop(addr, None)
