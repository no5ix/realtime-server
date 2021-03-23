from common import gr
import sys
from collections import namedtuple
# from const import db_const
# from const import service_const
from core.util import UtilApi
from common import gr
from core.mobilelog.LogManager import LogManager
# from util import DBOperation
from core.common.EntityFactory import EntityFactory
# from core.common.EntityManager import EntityManager
from server_entity.center_stub.BattleAllocatorStub import BattleAllocatorStub


class SingletonEntityManager(object):
    _instance = None

    def __init__(self):
        self.logger = LogManager.get_logger("SingletonEntityManager")
        # 注册全部center，并且stub成功连接所有center的处理结果回调
        self._finish_centers_stubs_cb = None
        # 注册全部center的处理结果回调
        self._finish_centers_cb = None
        # 注册全部stub的处理结果回调
        self._finish_stubs_cb = None
        # 中心列表
        self.center_list = []
        self.center_name_map = self._generate_center_name_map()
        self.stub_names = self._generate_stub_names()
        self.singleton_names = self._generate_singleton_entity_names()
        # 等待创建的center名字列表
        self.waiting_center_names = []
        # 等待连接的stub名字列表
        self.waiting_stub_names = []

    @classmethod
    def instance(cls):
        if cls._instance is None:
            cls._instance = SingletonEntityManager()
            return cls._instance

    @staticmethod
    def _generate_center_name_map():
        """
    将center根据需要部署到不同进程上, game_0, game_1则是config中game项的game_server_name
    """
        dic = {
            'game_0': {
                'non_persistent': [
                    'ForwardCenter', 'GMCenter', 'GameCenter',
                ],
                'persistent': [
                ],
            },
            'game_1': {
                'non_persistent': [
                ],
                'persistent': [
                ]
            },
            'battle_0': {
                'non_persistent': [
                    'BattleAllocatorCenter',
                ],
                'persistent': [
                ]
            },
        }
        # # 在windows平台上，不划分不同进程
        # if sys.platform.startswith('win'):
        #     _merge_on_windows(dic, 'game')
        return dic

    def _generate_stub_names(self):
        return {
            'game': [
                # 'RollStub'
                'GameStub',
            ],
            'battle': [
                'BattleAllocatorStub',
            ],
        }

    def _generate_singleton_entity_names(self):
        return {
            'game_0': [
                'GlobalInitManager',
            ],
            'game': [
                'DungeonManager', 'GMManager', 'DevHelper', 'MessageSender', 'GlobalCompensation', 'MailSender',
                'OnlineAvatarManager', 'GuildManager', 'ChatManager', 'CCLiveManager',
            ],
            'battle': [
                'DungeonManager', 'GMManager', 'DevHelper', 'MessageSender', 'DetourCache', 'AoiDelayDestroyer'
            ],
        }

    def register_centers_and_stubs(self, game_server_name, cb):
        self._finish_centers_stubs_cb = cb
        # self.register_server_singleton(game_server_name)
        self.register_relevant_centers(
            game_server_name,)
            # lambda flag: self._register_relevant_centers_cb(flag, game_server_name))
        self.register_stubs(game_server_name)
        # self.register_and_subscript_service()

    def register_relevant_centers(self, game_server_name):
        conf = gr.game_json_conf
        game_conf_info = conf.get(game_server_name, None)
        if game_conf_info is None:
            self.logger.error(f"has no {game_server_name} game_conf_info")
            return
        if game_conf_info.get("is_center", False):
            EntityFactory.instance().create_entity('BattleAllocatorCenter')
        # else:
        #     EntityFactory.instance().create_entity('BattleAllocatorStub')
        self.logger.debug('_register_relevant_centers_cb, status:%s' % 'good')

    # def _register_relevant_centers_cb(self, status, game_server_name):
    #     self.register_stubs(lambda flag: self._register_stubs_cb(flag), game_server_name)

    def register_stubs(self, game_server_name):
        # conf = gr.game_json_conf
        game_conf_info = gr.game_json_conf.get(game_server_name, None)
        if not game_conf_info.get("is_center", False):
            # EntityFactory.instance().create_entity('BattleAllocatorCenter')
        # else:
            name = 'BattleAllocatorStub'
            stub = EntityFactory.instance().create_entity(name)  # type: BattleAllocatorStub
            gr.add_server_singleton(stub)
            name_prefix = name[:-4]
            center_name = name_prefix + 'Center'
            stub.start_connect(center_name)
            self.logger.info("%s connected to center" % stub.__class__.__name__)
        # self._stubs_connect_success()
        self.logger.debug('_register_stubs_cb, status:%s' % 'good')

        # # self._finish_stubs_cb = finish_cb
        # game_server_prefix = game_server_name.split('_')[0]
        # stub_names = self.stub_names.get(game_server_prefix, [])
        # for name in stub_names:
        #     stub = EntityFactory.instance().create_entity(name)
        #     gr.add_server_singleton(stub)
        #     name_prefix = name[:-4]
        #     center_name = name_prefix + 'Center'
        #     stub.start_connect(center_name)
        #     self.logger.info("%s connected to center" % stub.__class__.__name__)
        # # self._stubs_connect_success()
        # self.logger.debug('_register_stubs_cb, status:%s', 'good')

    # @staticmethod
    # def get_register_and_subscript_service():
    #     return {
    #         'reg': {
    #             'battle_0': [RegInfo('BattleAllocatorCenter', '%s-bac' % str(GameServerRepo.hostid),
    #                                  service_const.TAG_SERVICE_BATTLE_CREATOR)],
    #             'game_0': [RegInfo('ForwardCenter', str(GameServerRepo.hostnum), service_const.TAG_SERVICE_GAME)],
    #         },
    #         'sub': {
    #             'battle_0': [
    #                 service_const.TAG_SERVICE_GAME,
    #                 service_const.TAG_SERVICE_MATCH,
    #             ],
    #             'game': [
    #                 service_const.TAG_SERVICE_BATTLE_CREATOR,
    #                 service_const.TAG_SERVICE_MATCH,
    #                 service_const.TAG_SERVICE_ROOM,
    #             ],
    #         }
    #     }
    #
    # def register_and_subscript_service(self):
    #     game_server_name = gr.game_server_name
    #     game_server_prefix = game_server_name.split('_')[0]
    #
    #     all_rss = self.get_register_and_subscript_service()
    #     # register service
    #     reg_dict = all_rss.get('reg', {})
    #     reg_list = reg_dict.get(game_server_prefix, [])
    #     reg_list.extend(reg_dict.get(game_server_name, []))
    #
    #     for reg_info in reg_list:
    #         center = gr.get_server_singleton(reg_info.entity_name)
    #         UtilApi.register_entity_to_etcd(center, reg_info.etcd_name, tag=reg_info.tag)
    #
    #     # subscript tag service
    #     sub_dict = all_rss.get('sub', {})
    #     sub_list = sub_dict.get(game_server_prefix, [])
    #     sub_list.extend(sub_dict.get(game_server_name, []))
    #     for sub_tag in sub_list:
    #         UtilApi.subscribe_entity_tag(sub_tag)
