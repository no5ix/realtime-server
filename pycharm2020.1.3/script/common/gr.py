# from core.EtcdSupport import ServiceNode
from asyncio import AbstractEventLoop
# from TcpServer import ev_loop
import typing
# usage: r = await AioApi.async_wrap(lambda: requests.request("GET", 'http://baidu.com', timeout=2))
# lambda关键字不可少


is_dev_version = True

bind_entity = None
is_client = False

local_ip = None
local_port = None

# 游戏服务器名
game_server_name = 'battle_0'
# game_server_name = 'battle_1'

game_json_conf = None
etcd_service_node = None  # type: typing.Union[ServiceNode, None]

EV_LOOP = None  # type: typing.Union[None, AbstractEventLoop]

server_singletons = {}


def add_server_singleton(entity, postfix=''):
    """添加一个GameServer内唯一的entity"""
    server_singletons[entity.__class__.__name__ + postfix] = entity


def get_server_singleton(entity_name):
    return server_singletons.get(entity_name)
