# from core.EtcdSupport import ServiceNode
from __future__ import annotations

import asyncio
from asyncio import AbstractEventLoop
# from TcpServer import ev_loop
import typing
from typing import Optional


if typing.TYPE_CHECKING:
    from core.EtcdSupport import ServiceNode
    from ServerBase import ServerBase


is_dev_version = True


db_save_interval_sec = 8
# entity_db_name = "entity_db"
# entity_collection_name = "entity_collection"

server_inst_arr = []  # todo: 后期要支持udp/kcp共存, udp连不上就tcp
server_inst = None  # type: Optional[ServerBase]

bind_entity = None
is_client = False

local_ip = None
local_port = None

# 服务器名
server_name = None
etcd_tag = None
# server_name = 'battle_1'

game_json_conf = None  # type: typing.Union[typing.Dict, None]
etcd_service_node = None  # type: typing.Union[ServiceNode, None]


_EV_LOOP = None  # type: typing.Union[None, AbstractEventLoop]


def get_ev_loop() -> asyncio.AbstractEventLoop:
    global _EV_LOOP
    if _EV_LOOP is None:
        try:
            _EV_LOOP = asyncio.get_running_loop()
        except RuntimeError:
            # pass  # 正常情况不可能会发生调用此get_ev_loop比server启动还要早
            _EV_LOOP = asyncio.new_event_loop()
            asyncio.set_event_loop(_EV_LOOP)
        _EV_LOOP.set_debug(is_dev_version)
    return _EV_LOOP
