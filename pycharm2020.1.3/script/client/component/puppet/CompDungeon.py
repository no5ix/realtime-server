from core.common.EntityFactory import EntityFactory
from common.component.Component import Component
from common import gr


class CompDungeon(Component):
    VAR_NAME = 'CompDungeon'

    def __init__(self):
        super(CompDungeon, self).__init__()

        self.ip = None
        self.port = None
        self.bind_id = None
        self.token = None
        self.invalid_token = None
        self.battle_retry_time = 0

    def connect_to_battle_server_imp(self, reconnect=False):
        # gr.player.CompNewbie.close_newbie_check_confirm()
        # gr.logger.warn('connect_to_battle_server_imp %d %s %s %s %s', self.battle_retry_time, str(self.ip), str(self.port), str(self.bind_id), str(self.token))
        if not gr.bind_entity or gr.bind_entity.id != self.bind_id or gr.bind_entity.expired:
            if gr.bind_entity:
                gr.bind_entity.destroy()
            _entity = EntityFactory.instance().create_entity('PuppetBindEntity', self.bind_id)
        else:
            _entity = gr.bind_entity
        gr.bind_entity = _entity
        # _entity.server, is_old = create_direct_proxy(
        #     _entity,
        #     self.bind_id,
        #     (self.ip, self.port),
        #     self.CON_TYPE,
        #     key_content=cconst.RSA_PUB_KEY,
        #     bind_token=self.token,
        #     cb=self.battle_server_bind_result,
        # )