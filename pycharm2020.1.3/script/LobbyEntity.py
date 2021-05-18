from common.PuppetEntity import PuppetEntity
# from server_entity.ServerEntity import ServerEntity


class LobbyEntity(PuppetEntity):

    def __init__(self):
        super().__init__()

    def should_db_save(self):
        return True

    def get_db_save_dict(self):
        return {}
