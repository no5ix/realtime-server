# from core.EtcdSupport import ServiceNode
from __future__ import annotations

import asyncio
from asyncio import AbstractEventLoop
# from TcpServer import ev_loop
import typing

if typing.TYPE_CHECKING:
# if True:
    from TcpServer import TcpServer
    from core.EtcdSupport import ServiceNode

# usage: r = await AioApi.async_wrap(lambda: requests.request("GET", 'http://baidu.com', timeout=2))
# lambda关键字不可少
# from TcpServer import TcpServer

is_dev_version = True

bind_entity = None
is_client = False

local_ip = None
local_port = None

# 游戏服务器名
game_server_name = None
etcd_tag = None
# game_server_name = 'battle_1'

game_json_conf = None
etcd_service_node = None  # type: typing.Union[ServiceNode, None]

EV_LOOP = None  # type: typing.Union[None, AbstractEventLoop]

server_singletons = {}


def add_server_singleton(entity, postfix=''):
    """添加一个GameServer内唯一的entity"""
    server_singletons[entity.__class__.__name__ + postfix] = entity


def get_server_singleton(entity_name):
    return server_singletons.get(entity_name, None)


def get_cur_server() -> TcpServer:
    return get_server_singleton("TcpServer")


def get_ev_loop():
    global EV_LOOP
    if EV_LOOP is None:
        try:
            EV_LOOP = asyncio.get_running_loop()
        except RuntimeError:
            pass  # 正常情况不可能会发生调用此get_ev_loop比server启动还要早, 所以直接pass
    return EV_LOOP

