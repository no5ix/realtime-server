from typing import Union
# from core.EtcdSupport import ServiceNode


bind_entity = None
is_client = False

local_ip = None
local_port = None

# 游戏服务器名
game_server_name = 'battle_0'
# game_server_name = 'battle_1'

game_json_conf = None
etcd_service_node = None  # type: Union[ServiceNode, None]


server_singletons = {}
reload_listeners = set()


def add_server_singleton(entity, postfix=''):
    """添加一个GameServer内唯一的entity"""
    server_singletons[entity.__class__.__name__ + postfix] = entity


def get_server_singleton(entity_name):
    return server_singletons.get(entity_name)
