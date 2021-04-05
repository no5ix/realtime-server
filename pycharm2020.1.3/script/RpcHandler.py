from __future__ import annotations

import asyncio
import typing
from asyncio import futures, shield

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


class RpcHandler:

    def __init__(
            self, conn: typing.Optional[TcpConn] = None,
            entity: typing.Optional[ServerEntity] = None):
        self._logger = LogManager.get_logger()
        self._conn = conn  # type: typing.Optional[TcpConn]
        self._entity = entity  # type: typing.Optional[ServerEntity]
        self._next_reply_id = 0
        self._pending_requests = {}  # type: typing.Dict[int, futures.Future]

    # def bind_entity(self, entity: ServerEntity):
    #     self._entity = entity

    def fire_all_future_with_error(self, error: str):
        for _reply_id, _reply_fut in self._pending_requests.items():
            # _reply_fut.set_exception(RpcReplyError(error))
            _reply_fut.set_result((error, None))
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

    @wait_or_not
    async def request_rpc(
            self,
            method_name: str,
            params: typing.Union[typing.Set, typing.List, typing.Tuple] = (),
            need_reply: bool = True, reply_timeout: typing.Union[int, float] = 2,
            remote_entity_type: typing.Union[None, str] = None,
            ip_port_tuple: typing.Tuple[str, int] = None):
        try:
            if need_reply:
                _reply_id = self.get_reply_id()
                msg = (RPC_TYPE_REQUEST, _reply_id, remote_entity_type, method_name, params)
                _reply_fut = gv.get_ev_loop().create_future()
                _reply_fut.add_done_callback(
                    lambda fut, rid=_reply_id: self._pending_requests.pop(rid, None))
                self._pending_requests[_reply_id] = _reply_fut
                self._send_rpc_msg(msg, ip_port_tuple)
                try:
                    return await asyncio.wait_for(asyncio.shield(_reply_fut), timeout=reply_timeout)
                except asyncio.exceptions.TimeoutError:
                    # self._logger.error(f"method_name={method_name}, asyncio.exceptions.TimeoutError")
                    # _reply_fut.set_exception(e)
                    _reply_fut.set_result((f"request rpc timeout: {method_name}", None))
                    # self._pending_requests.pop(_reply_id, None)
                    # return None
                    return await _reply_fut
            else:
                msg = (RPC_TYPE_NOTIFY, remote_entity_type, method_name, params)
                self._send_rpc_msg(msg, ip_port_tuple)
                return None
        except:
            self._logger.log_last_except()

    @wait_or_not
    async def _send_rpc_msg(self, msg, ip_port_tuple=None):
        encoded_msg = self.do_encode(msg)
        # msg_len = len(encoded_msg) if encoded_msg else 0
        # header_data = struct.pack("i", msg_len)
        # final_data = header_data + encoded_msg
        if self._conn is None:
            self._conn = await gv.get_cur_server().get_conn_by_addr(
                ip_port_tuple, self)
            # self._conn.set_entity(from_entity)
        self._conn.send_data_and_count(encoded_msg)

    @wait_or_not
    async def handle_rpc(self, rpc_msg):
        try:
            _rpc_msg_tuple = self.do_decode(rpc_msg)
            _rpc_type = _rpc_msg_tuple[0]
            if _rpc_type == RPC_TYPE_REQUEST or _rpc_type == RPC_TYPE_NOTIFY:
                _entity_type_str, _method_name, _parameters = _rpc_msg_tuple[-3:]
                if self._entity is None:
                    self._entity = gv.get_server_singleton(_entity_type_str)
                    if self._entity is None:
                        self._entity = EntityFactory.instance().create_entity(_entity_type_str)
                    self._entity.set_rpc_handler(self)
                    # self._conn.set_entity(_entity)
                _comp_method_list = _method_name.split(".")
                if len(_comp_method_list) == 1:
                    _method = getattr(self._entity, _method_name)
                else:
                    _method = getattr(
                        self._entity.get_component(_comp_method_list[0]), _comp_method_list[1])
                try:
                    _method_res = _method(_parameters)
                    if asyncio.iscoroutine(_method_res):
                        _method_res = await _method_res
                    if _method_res is None:
                        return
                    if _rpc_type == RPC_TYPE_REQUEST:
                        _rpc_reply = (RPC_TYPE_REPLY, _rpc_msg_tuple[1], None, _method_res)
                except Exception as e:
                    self._logger.error("Exception %r in call handler %r", e, _method_name)
                    if _rpc_type == RPC_TYPE_REQUEST:
                        _rpc_reply = (RPC_TYPE_REPLY, _rpc_msg_tuple[1], str(e), None)
                if _rpc_type == RPC_TYPE_REQUEST:
                    self._send_rpc_msg(_rpc_reply)
            # elif _rpc_type == RPC_TYPE_NOTIFY:
            #     pass
            elif _rpc_type == RPC_TYPE_REPLY:
                _reply_id, _error, _reply_result = _rpc_msg_tuple[-3:]
                _reply_fut = self._pending_requests.get(_reply_id, None)
                if _reply_fut is None:
                    self._logger.warning("_reply_future already fired or timeout")
                    return
                # if _error:
                #     _reply_fut.set_exception(RpcReplyError(_error))
                # else:
                _reply_fut.set_result((_error, _reply_result))
            elif _rpc_type == RPC_TYPE_HEARTBEAT:
                pass  # TODO
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


# class RpcException(Exception):
#     pass
#
#
# class RpcReplyError(RpcException):
#     pass
