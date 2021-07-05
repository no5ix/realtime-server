from __future__ import annotations

import typing
import asyncio
from asyncio import transports
from collections import defaultdict
from typing import Optional, Tuple

import ConnBase
from common import gv
from core.common.protocol_def import PROTO_TYPE_TCP, PROTO_TYPE_RUDP
from core.mobilelog.LogManager import LogManager
from core.util.TimerHub import TimerHub
from core.util.UtilApi import Singleton

if typing.TYPE_CHECKING:
    from RpcHandler import RpcHandler
    import TcpConn
    from RudpConn import RudpConn


ROLE_TYPE_ACTIVE = 0
ROLE_TYPE_PASSIVE = 1


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
            is_conned = await _conn.try_connect()
        else:
            _conn.add_rpc_handler(rpc_handler)  # 不加这个会造成 proxy 类的服务器转发混乱
            is_conned = _conn.is_connected()
            if _conn.is_disconnected_or_disconnecting():
                is_conned = await _conn.try_connect()
        if not is_conned and proto_type == PROTO_TYPE_RUDP:
            _conn = await self.open_conn_by_addr(PROTO_TYPE_TCP, addr, rpc_handler)
        #     _conn.handle_close(close_reason='try connecting failed')
        #     _conn = None
        return _conn

    def _remove_conn(self, proto_type, addr: typing.Tuple[str, int]):
        self._proto_type_2_addr_2_conn[proto_type].pop(addr, None)
