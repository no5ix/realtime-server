from server_entity.ServerEntity import ServerEntity


class LobbyEntity(ServerEntity):
    """ 游戏中所有服务端对象的父类"""
    def __init__(self, entity_id=None):
        super().__init__()


