import sys

from TcpServer import TcpServer
from common import gr
from core.mobilelog.LogManager import LogManager
from core.common import EntityScanner
from core.common.EntityFactory import EntityFactory
from core.tool import incremental_reload


class BattleServer(object):

    def __init__(self, server_name):
        # game_server_name = sys.argv[1]

        server_json_conf_path = r"../bin/win/conf/battle_server.json"
        self._server = TcpServer(server_name, server_json_conf_path)
        self._logger = LogManager.get_logger()

        self._register_server_entities()
        # self.register_battle_entities()
        self._register_component()

        incremental_reload.init_reload_record()  # 注意!! 一定要放到EntityScanner注册了的代码之后, 不然sys.modules里没相关的模块

    def _register_component(self):
        from common.component.Component import Component
        from common.component import ComponentRegister
        # gameconfig = self.config[self.config_sections.game]
        component_root = gr.game_json_conf.get('component_root')
        if component_root is None:
            self._logger.error('conf file has no component_root!')
            return
        component_classes = EntityScanner.scan_entity_package(component_root, Component)
        component_classes = component_classes.items()
        # component_classes.sort(lambda a, b: cmp(a[0], b[0]))
        for comp_type, comp_cls in component_classes:
            ComponentRegister.register(comp_cls)

    # def register_battle_entities(self):
    #     from BattleEntity import BattleEntity
    #     _ber = gr.game_json_conf.get('battle_entity_root', None)
    #     if _ber is None:
    #         self.logger.error('conf file has no battle_entity_root!')
    #         return
    #     entity_classes = EntityScanner.scan_entity_package(_ber, BattleEntity)
    #     entity_classes = entity_classes.items()
    #
    #     # def cmp(x, y):
    #     #     if x < y:
    #     #         return -1
    #     #     elif x == y:
    #     #         return 0
    #     #     else:
    #     #         return 1
    #     #
    #     # entity_classes.sort(lambda a, b: cmp(a[0], b[0]))
    #     for cls_name, cls in entity_classes:
    #         EntityFactory.instance().register_entity(cls_name, cls)

    @staticmethod
    def _register_server_entities():
        from BattleEntity import BattleEntity
        from LobbyEntity import LobbyEntity
        from server_entity.ServerEntity import ServerEntity
        _ser = gr.game_json_conf.get('server_entity_root', None)
        _ler = gr.game_json_conf.get('lobby_entity_root', None)
        _ber = gr.game_json_conf.get('battle_entity_root', None)
        if _ser is None:
            # self.logger.error('conf file has no server_entity_root!')
            raise Exception('conf file has no server_entity_root!')
        if _ler is None:
            # self.logger.error('conf file has no server_entity_root!')
            raise Exception('conf file has no lobby_entity_root!')
        if _ber is None:
            # self.logger.error('conf file has no server_entity_root!')
            raise Exception('conf file has no battle_entity_root!')
            # return
        _temp = {_ser: ServerEntity,
                 _ler: LobbyEntity,
                 _ber: BattleEntity}
        for _ent_root, _ent_cls in _temp.items():
            entity_classes = EntityScanner.scan_entity_package(
                _ent_root, _ent_cls)
            entity_classes = entity_classes.items()
            for cls_name, cls in entity_classes:
                EntityFactory.instance().register_entity(cls_name, cls)

    def start(self):
        self._server.run()


if __name__ == '__main__':
    game_server_name = sys.argv[1]
    bs = BattleServer(game_server_name)
    bs.start()

