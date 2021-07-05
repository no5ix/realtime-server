from __future__ import annotations

import asyncio
# import collections
import functools
import typing
from asyncio import futures

import time
from typing import Optional

from bson import ObjectId

import core.util.UtilApi
from ConnBase import ConnBase
from ConnMgr import ConnMgr, PROTO_TYPE_TCP, PROTO_TYPE_RUDP
from core.common.IdManager import IdManager
from core.mobilelog.LogManager import AsyncLogger
from core.util.TimerHub import TimerHub
from core.util.UtilApi import wait_or_not

if typing.TYPE_CHECKING:
    from TcpConn import TcpConn
    from server_entity.ServerEntity import ServerEntity

from common import gv
from core.common import MsgpackSupport
from core.common.EntityFactory import EntityFactory
from core.mobilelog.LogManager import LogManager


RPC_TYPE_NOTIFY = 0
RPC_TYPE_REQUEST = 1
RPC_TYPE_REPLY = 2
RPC_TYPE_HEARTBEAT = 3

REQUEST_RPC_TIMEOUT = 3  # sec


def get_a_rpc_handler_id() -> bytes:
    return IdManager.genid().binary


class RpcReplyFuture:

    def __init__(self, rpc_reply_id: int, rpc_func_name: str, fut: futures.Future):
        self.rpc_reply_id = rpc_reply_id
        self.rpc_func_name = rpc_func_name
        self.fut = fut

    def set_error_and_result(self, error_msg: str, result_val):
        try:
            self.fut.set_result((error_msg, result_val))
        except asyncio.InvalidStateError:
            # 并发情况即使`self.fut`已经done了也无所谓, 不处理即可
            pass


class RpcHandler:

    def __init__(
            self, rpc_handler_id: bytes, conn: ConnBase = None,
            entity: ServerEntity = None, conn_proto_type: int = PROTO_TYPE_TCP):
        self.rpc_handler_id = rpc_handler_id
        self._logger = LogManager.get_logger()  # type: AsyncLogger
        # self._logger.info(f'!!! xx {rpc_handler_id=}')
        self._conn = conn  # type: ConnBase
        if conn:
            self._conn_proto_type = conn.get_proto_type()
        else:
            self._conn_proto_type = conn_proto_type
        self._entity = entity  # type: ServerEntity
        self._next_reply_id = 0
        self._pending_requests = {}  # type: typing.Dict[int, RpcReplyFuture]
        self._msg_buffer = []  # type: typing.List[bytes]
        self._timer_hub = TimerHub()
        # self._try_connect_times = 0

        self._is_destroyed = False
        self._addr_2_create_conn_tasks = {}  # type: typing.Dict[typing.Tuple[str, int], asyncio.Task]

    def destroy(self):
        # self._logger.debug(f'{self.rpc_handler_id=} RpcHandler Destroy!')
        for _ct in self._addr_2_create_conn_tasks.values():
            _ct.cancel()
        self._addr_2_create_conn_tasks = {}
        self._timer_hub.destroy()
        # self._conn.handle_close(close_reason=f'RpcHandler destroy, {self.rpc_handler_id=}')
        self._entity = None
        if self._conn:
            self._conn.remove_rpc_handler(self.rpc_handler_id)
        self._logger = None
        self._is_destroyed = True

    def on_conn_close(self):
        # self.fire_all_future_with_result(close_reason)
        # if self._conn.is_active_role():
        #     await self._handle_create_conn()
        self._conn = None

    # def fire_all_future_with_result(self, error: str, result=None):
    #     for _reply_id, _reply_fut_tuple in self._pending_requests.items():
    #         # _reply_fut.set_exception(RpcReplyError(error))
    #         _reply_fut_tuple[1].set_result((error, result))
    #     self._pending_requests.clear()

    def set_conn(self, conn):
        self._conn = conn

    @staticmethod
    def do_encode(msg):
        return MsgpackSupport.encode(msg)

    @staticmethod
    def do_decode(msg):
        return MsgpackSupport.decode(msg)

    # def request_rpc(self, *args, **kwargs):
    #     asyncio.create_task(self.request_rpc_impl(*args, **kwargs))

    def get_reply_id(self):
        self._next_reply_id += 1
        if self._next_reply_id > 60000000:
            self._next_reply_id = 0
        return self._next_reply_id

    async def send_heartbeat(self):
        try:
            # self._logger.info("sennnnnnnnnd heartbeatttt")
            msg = (RPC_TYPE_HEARTBEAT, )
            self._send_rpc_msg(self.do_encode(msg))
        except:
            self._logger.log_last_except()

    @wait_or_not()
    async def request_rpc(
            self,
            rpc_fuc_name: str,
            rpc_fuc_args: typing.Union[typing.Set, typing.List, typing.Tuple] = (),
            rpc_fuc_kwargs: typing.Union[None, typing.Dict] = None,
            rpc_callback: typing.Callable = None,
            rpc_need_reply: bool = True,
            rpc_reply_timeout: typing.Union[None, int, float] = REQUEST_RPC_TIMEOUT,  # None for no timeout
            rpc_remote_entity_type: typing.Union[None, str] = None,
            ip_port_tuple: typing.Tuple[str, int] = None
    ):
        rpc_fuc_kwargs = {} if rpc_fuc_kwargs is None else rpc_fuc_kwargs
        if rpc_need_reply:
            _reply_id = self.get_reply_id()
            msg = (RPC_TYPE_REQUEST, _reply_id, rpc_remote_entity_type, rpc_fuc_name, rpc_fuc_args, rpc_fuc_kwargs)
            _reply_fut = gv.get_ev_loop().create_future()

            def final_fut_cb(fut, rid=_reply_id, cb=rpc_callback):
                # self._logger.info(f"final_fut_cb: rid={rid}")
                self._pending_requests.pop(rid, None)
                if callable(cb):
                    cb(*fut.result())

            _reply_fut.add_done_callback(final_fut_cb)
            _rpc_reply_fut_obj = RpcReplyFuture(_reply_id, rpc_fuc_name, _reply_fut)
            self._pending_requests[_reply_id] = _rpc_reply_fut_obj
            self._send_rpc_msg(self.do_encode(msg), ip_port_tuple)
            try:
                # print(f"deaaaaaaa before{time.time()=}")
                return await asyncio.wait_for(asyncio.shield(_reply_fut), timeout=rpc_reply_timeout)
            except asyncio.exceptions.TimeoutError:
                # print(f"afterrrr {time.time()=}")
                # self._logger.error(f"rpc_fuc_name={rpc_fuc_name}, asyncio.exceptions.TimeoutError")
                # _reply_fut.set_exception(e)
                # if self._conn is None:
                #     self._logger.error(
                #         f"request rpc(`{rpc_fuc_name}`) timeout because conn lost, try reconnect ...")
                #     return await asyncio.wait_for(asyncio.shield(_reply_fut), timeout=None)
                # else:
                # if not _reply_fut.done():
                _rpc_reply_fut_obj.set_error_and_result(
                    f"request rpc timeout, {rpc_fuc_name=}, {rpc_fuc_args=}, {rpc_fuc_kwargs=}",
                    None)
                # try:
                #     _reply_fut.set_result((f"request rpc timeout: {rpc_fuc_name}", None))
                # except asyncio.InvalidStateError:
                #     pass
                return await _reply_fut
        else:
            msg = (RPC_TYPE_NOTIFY, rpc_remote_entity_type, rpc_fuc_name, rpc_fuc_args, rpc_fuc_kwargs)
            self._send_rpc_msg(self.do_encode(msg), ip_port_tuple)
            return None

    # @wait_or_not()
    def _send_rpc_msg(self, msg: bytes, ip_port_tuple: typing.Tuple[str, int] = None):
        # assert ((self._conn is None and ip_port_tuple) or (not self._conn.is_connected()) or self._conn)
        # if ip_port_tuple:
        #     _cur_addr = ip_port_tuple
        # else:
        #     _cur_addr = self._conn.get_addr()
        if self._conn is None or not self._conn.is_connected():
            self._msg_buffer.append(msg)
            # print(f"_send_rpc_msg  _try_connect_times={self._try_connect_times}, id(self._conn)= {id(self._conn)}")
            # if self._try_connect_times > 0 or ip_port_tuple is None:  # `self._try_connect_times > 0` means connecting
            #     return
            if ip_port_tuple is None:
                    # or ip_port_tuple in self._addr_2_create_conn_tasks:  # means connecting
                return
            # self._create_conn_tasks.append((gv.get_ev_loop().create_task(self._handle_create_conn(ip_port_tuple))))
            self._addr_2_create_conn_tasks[ip_port_tuple] = self._handle_create_conn(ip_port_tuple)

            return
        self._conn.send_data_and_count(self.rpc_handler_id, msg)

    @wait_or_not()
    async def _handle_create_conn(self, addr: typing.Tuple[str, int] = None):
        try:
            if self._conn:
                if not self._conn.is_active_role() or self._conn.is_connected():
                    return
                # addr = self._conn.get_addr()
            # self._try_connect_times += 1
            # self._conn, is_conned = await ConnMgr.instance().open_conn_by_addr(CONN_TYPE_TCP, addr, self)
            self._logger.debug(f'_handle_create_conn: {addr=}')
            _conn: ConnBase = await ConnMgr.instance().open_conn_by_addr(
                self._conn_proto_type, addr, self)
            self._addr_2_create_conn_tasks.pop(addr, None)
            if _conn.is_connected():
                self._logger.debug(f'_handle_create_conn: _conn.is_connected() {addr=}')
                if self._conn is None:
                    self._conn = _conn
                    for _msg in self._msg_buffer:
                        self._conn.send_data_and_count(self.rpc_handler_id, _msg)
                    self._msg_buffer.clear()
            elif _conn.is_disconnected_or_disconnecting():
                self._logger.debug(f'_handle_create_conn: _conn.is_disconnected_or_disconnecting() {addr=}')
                if self._conn is None:
                    for _msg in self._msg_buffer:
                        if _msg[0] == RPC_TYPE_REQUEST:
                            _reply_id = _msg[1]
                            if _reply_id not in self._pending_requests:
                                continue
                            self._logger.warning(
                                f"fire pending requests future, rpc func: {_msg[3]}, reply_id: {_reply_id}")
                            self._pending_requests[_reply_id].set_error_and_result(
                                f"cant connect remote addr: {addr}", None)
            elif _conn.is_connecting():
                self._logger.debug(f'_handle_create_conn: _conn.is_connecting() {addr=}')
                pass
        except asyncio.CancelledError:
            LogManager.get_logger().warning(f'cancel _handle_create_conn task {addr=}')
            raise

    @staticmethod
    async def handle_request_notify_rpc(_method, _method_name, _method_args, _method_kwargs):
        if _method is None:
            raise Exception(f"{_method_name} is not found")
        _is_rpc_func = getattr(_method, "is_rpc_func", False)
        if not _is_rpc_func:
            raise Exception(f"{_method_name} is not rpc func")
        _method_res = _method(*_method_args, **_method_kwargs)
        if asyncio.iscoroutine(_method_res):
            _method_res = await _method_res
        # if _method_res is None:
        #     return
        return _method_res

    @wait_or_not()
    async def handle_rpc(self, rpc_msg):
        try:
            _rpc_msg_tuple = self.do_decode(rpc_msg)
            _rpc_type = _rpc_msg_tuple[0]
            if _rpc_type == RPC_TYPE_REQUEST or _rpc_type == RPC_TYPE_NOTIFY:
                try:
                    # msg = (RPC_TYPE_REQUEST, _reply_id, rpc_remote_entity_type, rpc_fuc_name, rpc_fuc_args, rpc_fuc_kwargs)
                    _entity_type_str, _method_name, _method_args, _method_kwargs = _rpc_msg_tuple[-4:]
                    if self._entity is None:
                        self._entity = core.util.UtilApi.get_server_singleton(_entity_type_str)
                        if self._entity is None:
                            self._entity = EntityFactory.instance().create_entity(_entity_type_str)
                        self._entity.set_rpc_handler(self)
                        # self._conn.set_entity(_entity)
                    _method = getattr(self._entity, _method_name, None)

                    # _comp_method_list = _method_name.split(".")
                    # if len(_comp_method_list) == 1:
                    #     _method = getattr(self._entity, _method_name, None)
                    # else:
                    #     _method = getattr(
                    #         self._entity.get_component(_comp_method_list[0]),
                    #         _comp_method_list[1], None)
                    # if _method is None:
                    #     raise Exception(f"{_method_name} is not found")
                    # _is_rpc_func = getattr(_method, "is_rpc_func", False)
                    # if not _is_rpc_func:
                    #     raise Exception(f"{_method_name} is not rpc func")
                    # _method_res = _method(*_method_args, **_method_kwargs)
                    # if asyncio.iscoroutine(_method_res):
                    #     _method_res = await _method_res
                    # if _method_res is None:
                    #     return
                    _method_res = await self.handle_request_notify_rpc(
                        _method, _method_name, _method_args, _method_kwargs)

                    if _rpc_type == RPC_TYPE_REQUEST:
                        _rpc_reply = (RPC_TYPE_REPLY, _rpc_msg_tuple[1], None, _method_res)
                except Exception as e:
                    self._logger.error("Exception %r in call handler %r", e, _method_name)
                    self._logger.log_last_except()
                    if _rpc_type == RPC_TYPE_REQUEST:
                        _rpc_reply = (RPC_TYPE_REPLY, _rpc_msg_tuple[1], str(e), None)
                if _rpc_type == RPC_TYPE_REQUEST:
                    self._send_rpc_msg(self.do_encode(_rpc_reply))
            # elif _rpc_type == RPC_TYPE_NOTIFY:
            #     pass
            elif _rpc_type == RPC_TYPE_REPLY:
                _reply_id, _error, _reply_result = _rpc_msg_tuple[-3:]
                _rpc_reply_fut_obj = self._pending_requests.get(_reply_id, None)
                if _rpc_reply_fut_obj is None:
                    # self._logger.warning(f"{_rpc_func_name} _reply_future already fired or timeout")
                    return
                if _error:
                    self._logger.error(f"{_rpc_reply_fut_obj.rpc_func_name=}: {_error=}")
                #     _reply_fut.set_exception(RpcReplyError(_error))
                # else:
                _rpc_reply_fut_obj.set_error_and_result(_error, _reply_result)
            elif _rpc_type == RPC_TYPE_HEARTBEAT:
                # print("remote_heart_beatttttttt")
                self._conn.remote_heart_beat()
            else:
                self._logger.error(f"unknown RPC type: {_rpc_type}")
                return
            # _entity_type_str, _method_name, _parameters = self.do_decode(rpc_msg)
            # if self._entity is None:
            #     self._entity = gv.get_server_singleton(_entity_type_str)
            #     if self._entity is None:
            #         self._entity = EntityFactory.instance().create_entity(_entity_type_str)
            #     self._entity.set_rpc_handler(self)
            #     # self._conn.set_entity(_entity)
            # _comp_method_list = _method_name.split(".")
            # if len(_comp_method_list) == 1:
            #     _method = getattr(self._entity, _method_name)
            # else:
            #     _method = getattr(
            #         self._entity.get_component(_comp_method_list[0]), _comp_method_list[1])
            # _method(_parameters)
        except:
            self._logger.log_last_except()


def rpc_func(func):

    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        # func_for_reload = func
        return func(*args, **kwargs)

    wrapper.is_rpc_func = True
    return wrapper
