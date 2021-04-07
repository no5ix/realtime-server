from __future__ import annotations

import asyncio
import collections
import typing
from asyncio import futures

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

RECONNECT_MAX_TIMES = 6
RECONNECT_INTERVAL = 0.6  # sec


class RpcHandler:

    def __init__(
            self, conn: typing.Optional[TcpConn] = None,
            entity: typing.Optional[ServerEntity] = None):
        self._logger = LogManager.get_logger()
        self._conn = conn  # type: typing.Optional[TcpConn]
        self._entity = entity  # type: typing.Optional[ServerEntity]
        self._next_reply_id = 0
        self._pending_requests = {}  # type: typing.Dict[int, typing.Tuple[str, futures.Future]]
        self._msg_buffer = []  # type: typing.List[typing.Tuple]
        self._timer_hub = TimerHub()
        self._connect_fail_times = 0

    def on_conn_close(self, close_reason):
        self.fire_all_future_with_result(close_reason)
        self._conn = None

    def fire_all_future_with_result(self, error: str, result=None):
        for _reply_id, _reply_fut_tuple in self._pending_requests.items():
            # _reply_fut.set_exception(RpcReplyError(error))
            _reply_fut_tuple[1].set_result((error, result))
        self._pending_requests.clear()

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
            print("sennnnnnnnnd heartbeatttt")
            msg = (RPC_TYPE_HEARTBEAT, )
            await self._send_rpc_msg(msg)
        except:
            self._logger.log_last_except()

    @wait_or_not
    async def request_rpc(
            self,
            rpc_fuc_name: str,
            rpc_fuc_args: typing.Union[typing.Set, typing.List, typing.Tuple] = (),
            rpc_fuc_kwargs: typing.Union[None, typing.Dict] = None,
            rpc_callback: typing.Callable = None,
            rpc_need_reply: bool = True,
            rpc_reply_timeout: typing.Union[int, float] = 2,
            rpc_remote_entity_type: typing.Union[None, str] = None,
            ip_port_tuple: typing.Tuple[str, int] = None
    ):
        try:
            rpc_fuc_kwargs = {} if rpc_fuc_kwargs is None else rpc_fuc_kwargs
            if rpc_need_reply:
                _reply_id = self.get_reply_id()
                msg = (RPC_TYPE_REQUEST, _reply_id, rpc_remote_entity_type, rpc_fuc_name, rpc_fuc_args, rpc_fuc_kwargs)
                _reply_fut = gv.get_ev_loop().create_future()

                def final_fut_cb(fut, rid=_reply_id, cb=rpc_callback):
                    self._pending_requests.pop(rid, None)
                    if callable(cb):
                        cb(*fut.result())

                _reply_fut.add_done_callback(final_fut_cb)
                self._pending_requests[_reply_id] = (rpc_fuc_name, _reply_fut)
                await self._send_rpc_msg(msg, ip_port_tuple)
                try:
                    return await asyncio.wait_for(asyncio.shield(_reply_fut), timeout=rpc_reply_timeout)
                except asyncio.exceptions.TimeoutError:
                    # self._logger.error(f"rpc_fuc_name={rpc_fuc_name}, asyncio.exceptions.TimeoutError")
                    # _reply_fut.set_exception(e)
                    if self._conn is None:
                        self._logger.error(
                            f"request rpc({rpc_fuc_name}) timeout because conn lost, try reconnect ...")
                        return
                    else:
                        _reply_fut.set_result((f"request rpc timeout: {rpc_fuc_name}", None))
                        return await _reply_fut
            else:
                msg = (RPC_TYPE_NOTIFY, rpc_remote_entity_type, rpc_fuc_name, rpc_fuc_args, rpc_fuc_kwargs)
                await self._send_rpc_msg(msg, ip_port_tuple)
                return None
        except:
            self._logger.log_last_except()

    # @wait_or_not
    async def _send_rpc_msg(self, msg, ip_port_tuple=None):
        if self._conn is None:
            self._msg_buffer.append(msg)
            await self._handle_create_conn(ip_port_tuple)
            return
        self._conn.send_data_and_count(self.do_encode(msg))

    # @wait_or_not
    async def _handle_create_conn(self, addr: typing.Tuple[str, int]):
        try:
            self._conn = await gv.get_cur_server().get_conn_by_addr(addr, self)
        except ConnectionRefusedError:
            self._connect_fail_times += 1
            if self._connect_fail_times <= RECONNECT_MAX_TIMES:
                print(f"try reconnect {str(addr)} ... {self._connect_fail_times}")
                self._timer_hub.call_later(RECONNECT_INTERVAL, lambda: self._handle_create_conn(addr))
            else:
                self._logger.error(f"try {self._connect_fail_times} times , still cant connect remote addr: {addr}")
                for _msg in self._msg_buffer:
                    if _msg[0] == RPC_TYPE_REQUEST:
                        # self._pending_requests.get
                        self._pending_requests[_msg[1]][1].set_result(
                            (f"cant connect remote addr: {addr}", None))
                self._connect_fail_times = 0
            return
        else:
            for _msg in self._msg_buffer:
                self._conn.send_data_and_count(self.do_encode(_msg))
            self._msg_buffer.clear()

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
        if _method_res is None:
            return
        return _method_res

    @wait_or_not
    async def handle_rpc(self, rpc_msg):
        try:
            _rpc_msg_tuple = self.do_decode(rpc_msg)
            _rpc_type = _rpc_msg_tuple[0]
            if _rpc_type == RPC_TYPE_REQUEST or _rpc_type == RPC_TYPE_NOTIFY:
                try:
                    _entity_type_str, _method_name, _method_args, _method_kwargs = _rpc_msg_tuple[-4:]
                    if self._entity is None:
                        self._entity = gv.get_server_singleton(_entity_type_str)
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
                    if _rpc_type == RPC_TYPE_REQUEST:
                        _rpc_reply = (RPC_TYPE_REPLY, _rpc_msg_tuple[1], str(e), None)
                if _rpc_type == RPC_TYPE_REQUEST:
                    await self._send_rpc_msg(_rpc_reply)
            # elif _rpc_type == RPC_TYPE_NOTIFY:
            #     pass
            elif _rpc_type == RPC_TYPE_REPLY:
                _reply_id, _error, _reply_result = _rpc_msg_tuple[-3:]
                _rpc_func_name, _reply_fut = self._pending_requests.get(_reply_id, (None, None))
                if _reply_fut is None:
                    # self._logger.warning(f"{_rpc_func_name} _reply_future already fired or timeout")
                    return
                if _error:
                    self._logger.error(f"{_rpc_func_name}: {_error}")
                #     _reply_fut.set_exception(RpcReplyError(_error))
                # else:
                _reply_fut.set_result((_error, _reply_result))
            elif _rpc_type == RPC_TYPE_HEARTBEAT:
                print("remote_heart_beatttttttt")
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

    async def send_heartbeat_rpc(self):
        msg = (RPC_TYPE_HEARTBEAT, )
        await self._send_rpc_msg(msg)
        print("calllll send_heartbeat_rpc")


def rpc_func(func):
    def wrapper(*args, **kwargs):
        # func_for_reload = func
        return func(*args, **kwargs)
    wrapper.is_rpc_func = True
    return wrapper
