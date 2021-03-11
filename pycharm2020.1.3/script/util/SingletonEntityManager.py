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
        pass
    #     """注册当前server进程上需要启动的center，并且注册所有server_singleton，全部启动完毕后进行回调"""
    #     self._finish_centers_stubs_cb = cb
    #     self.register_server_singleton(game_server_name)
    #     self.register_relevant_centers(
    #         game_server_name,
    #         lambda flag: self._register_relevant_centers_cb(flag, game_server_name))
    #
    # def register_relevant_centers(self, game_server_name, finish_cb):
    #     self._finish_centers_cb = finish_cb
    #     if game_server_name in self.center_name_map:
    #         name_dict = self.center_name_map[game_server_name]
    #         non_persistent_names = name_dict.get('non_persistent', [])
    #         persistent_names = name_dict.get('persistent', [])
    #     else:
    #         self._register_centers_success()
    #         return
    #     # 非持久化center
    #     for name in non_persistent_names:
    #         if UtilApi.get_global_entity_mailbox(name) is not None:
    #             self.logger.error("re-register global entity name=%s", name)
    #         # self._register_centers_failed()
    #         # break
    #         entity = EntityFactory.instance().create_entity(name)
    #         entity.init_from_dict({})
    #         UtilApi.register_entity_globally(
    #             name, entity,
    #             lambda flag, c=entity: self._register_global_entity_cb(flag, c))
    #         gr.add_server_singleton(entity)
    #         self.waiting_center_names.append(name)
    #
    #     # 持久化center
    #     for name in persistent_names:
    #         # 从center collection中找
    #         DBOperation.db_find_center(name, lambda s, doc, n=name: self._center_find_id_cb(s, doc, n))
    #         self.waiting_center_names.append(name)
    #
    #     if len(self.waiting_center_names) == 0:
    #         self._register_centers_success()
    #
    # def _register_relevant_centers_cb(self, status, game_server_name):
    #     self.logger.debug('_register_relevant_centers_cb, status:%s', status)
    #     self.register_stubs(lambda flag: self._register_stubs_cb(flag), game_server_name)
    #
    # def register_stubs(self, finish_cb, game_server_name):
    #     self._finish_stubs_cb = finish_cb
    #     game_server_prefix = game_server_name.split('_')[0]
    #     stub_names = self.stub_names.get(game_server_prefix, [])
    #     for name in stub_names:
    #         stub = EntityFactory.instance().create_entity(name)
    #         gr.add_server_singleton(stub)
    #         name_prefix = name[:-4]
    #         center_name = name_prefix + 'Center'
    #         stub.start_connect(center_name, lambda stub=stub: self._stub_connect_center_cb(stub))
    #     self._stubs_connect_success()
    #
    # def _register_stubs_cb(self, status):
    #     self.logger.debug('_register_stubs_cb, status:%s', status)
    #     if self._finish_centers_stubs_cb is not None:
    #         self._finish_centers_stubs_cb(status)
    #         self._finish_centers_stubs_cb = None
    #
    # def register_server_singleton(self, game_server_name):
    #     """加入GameServer唯一的entity"""
    #     game_server_prefix = game_server_name.split('_')[0]
    #     singleton_names = self.singleton_names.get(game_server_name, [])
    #     extra_singleton_name = self.singleton_names.get(game_server_prefix, [])
    #     singleton_names.extend(extra_singleton_name)
    #     for name in singleton_names:
    #         gr.add_server_singleton(EntityFactory.instance().create_entity(name))
    #     self._create_immortal_dungeon()
    #
    # def _create_immortal_dungeon(self):
    #     dun_mgr = gr.get_server_singleton('DungeonManager')
    #     if not dun_mgr:
    #         return
    #     dun_no_list = dun_mgr.get_immortal_dungeon(GameServerRepo.game_server_name)
    #     for no in dun_no_list:
    #         flag, dun_id = dun_mgr.apply_dungeon(no, None)
    #         if not flag:
    #             self.logger.error('apply_local_dungeon no = %s failed in _create_immortal_dungeon' % no)
    #             continue
    #         dun = EntityManager.getentity(dun_id)
    #         if dun is None:
    #             self.logger.error('cannot find dun no=%s id=%s in _create_immortal_dungeon' % (no, dun_id))
    #             continue
    #         dun.disable_auto_destroy()
    #         dun.is_immortal = True
    #
    #         gr.add_server_singleton(dun, '_' + str(no))
    #
    # def refresh_center_stubs(self, serverlist):
    #     # 清理断线的GameServer
    #     for c in self.center_list:
    #         c.refresh_stubs(serverlist)
    #
    # def _center_find_id_cb(self, status, docs, name):
    #     assert status, 'Db error when create center=%s' % name
    #     if docs is not None and len(docs) == 1:
    #         # 存在的话直接从数据库创建
    #         cb = lambda ent, n=name: self._center_find_ent_cb(ent, n)
    #         UtilApi.create_entity_from_db(name, docs[0]['entityid'], cb)
    #     else:
    #         # 不存在的话从内存创建
    #         entity = EntityFactory.instance().create_entity(name)
    #         entity.enable_persistent(False)
    #         # 初始化
    #         entity.init_from_dict({})
    #         cb = lambda f, e=entity, n=name: self._save_center_cb(f, e, n)
    #         if not UtilApi.save_entity(entity, cb):
    #             self.logger.error('save_entity failed in _center_find_id_cb for name=%s' % name)
    #
    # def _save_center_cb(self, flag, entity, name):
    #     """向entities collection写入的回调"""
    #     if not flag:
    #         self._register_centers_failed()
    #         return
    #     # 向collections写入成功的话再向center collection保存
    #     create_data = {'entityid': entity.id}
    #     cb = lambda status, ret_id, e=entity, n=name: self._save_to_center_collection_cb(status, e, n)
    #     ret_val = UtilApi.insert_data_record(db_const.COLLECTION_CENTER, name, create_data, cb)
    #     if not ret_val:
    #         self._register_centers_failed()
    #
    # def _save_to_center_collection_cb(self, flag, entity, name):
    #     """向center collection保存的回调"""
    #     if not flag:
    #         self._register_centers_failed()
    #         return
    #     entity.enable_persistent(True)
    #     cb = lambda flag, c=entity: self._register_global_entity_cb(flag, c)
    #     UtilApi.register_entity_globally(name, entity, cb)
    #     gr.add_server_singleton(entity)
    #
    # def _center_find_ent_cb(self, entity, name):
    #     """从数据库创建entity的回调"""
    #     if entity is None:
    #         self.logger.error("Create cluster from db error: %s" % name)
    #         self._register_centers_failed()
    #         return
    #
    #     if UtilApi.get_global_entity_mailbox(name) is not None:
    #         self.logger.error("re-register global entity name=%s", name)
    #     # self._register_centers_failed()
    #     # return
    #
    #     cb = lambda flag, c=entity: self._register_global_entity_cb(flag, c)
    #     UtilApi.register_entity_globally(name, entity, cb)
    #     gr.add_server_singleton(entity)
    #
    # def _register_global_entity_cb(self, flag, center):
    #     """注册全局entity的回调"""
    #     center.register_callback(flag)
    #     if not flag:
    #         self._register_centers_failed()
    #         return
    #     name = center.__class__.__name__
    #     if name in self.waiting_center_names:
    #         self.center_list.append(center)
    #         self.waiting_center_names.remove(name)
    #         if len(self.waiting_center_names) == 0:
    #             # 所有center注册完毕
    #             self._register_centers_success()
    #     else:
    #         self.logger.warn('name:%s not in waiting_center_names:%s', name, self.waiting_center_names)
    #
    # def _register_centers_failed(self):
    #     """遇到问题，失败"""
    #     if self._finish_centers_cb:
    #         self._finish_centers_cb(False)
    #         self._finish_centers_cb = None
    #
    # def _register_centers_success(self):
    #     """注册成功"""
    #     if self._finish_centers_cb:
    #         self._finish_centers_cb(True)
    #         self._finish_centers_cb = None
    #
    # def _stub_connect_center_cb(self, stub):
    #     name = stub.__class__.__name__
    #     self.logger.info("%s connected to center" % name)
    #
    # def _stubs_connect_success(self):
    #     """连接成功"""
    #     if self._finish_stubs_cb:
    #         self._finish_stubs_cb(True)
    #         self._finish_stubs_cb = None
    #
    # def get_register_and_subscript_service(self):
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
    #     game_server_name = GameServerRepo.game_server_name
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
