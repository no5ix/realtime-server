# -*- coding: utf-8 -*-
import random
# import gr
# from Stub import Stub
# from common import const
# from common.cdata import DungeonConfig
# from common.utils import collision_util, import_helper
# from core.common.IdManager import IdManager
# from core.common.RpcSupport import rpc_method, SERVER_ONLY
# from core.common.RpcMethodArgs import Dict, MailBox, Uuid, Str, Int, List, Response
# from core.common.EntityFactory import EntityFactory
# from core.common.EntityManager import EntityManager
# from core.common.mobilecommon import asiocore
# from core.distserver.game import GameServerRepo
# from const import service_const
# from core.servercommon.PostmanDelayGuard import PostmanDelayGuard
# from util.performance.battleserver_load import BattleProcessMetric
# from util.sunshine import node_executor

from server_entity.center_stub.Stub import Stub


class BattleAllocatorStub(Stub):

    def __init__(self, entity_id=None):
        super(BattleAllocatorStub, self).__init__(entity_id)
        # self._metric = BattleProcessMetric()
        # self._delay_calls.callback('metric', 1.0, self.report_metric, repeat_num=-1, first_delay=5.0)
        # try:
        #     self.init_collision()
        # except:
        #     self.logger.error('init_collision failed!!!')
        #     self.logger.log_last_except()
        # asiocore.set_use_condition(False)

    # def reload_script(self):
    #     super(BattleAllocatorStub, self).reload_script()
    #     # 清理ETS文件缓存
    #     node_executor.cache.clear()
    #
    # def call_server_method(self, remote_mailbox, methodname, parameters=None, callback=None):
    #     if parameters is None:
    #         parameters = {}
    #     return self.call_server_method_direct(remote_mailbox, methodname, parameters=parameters)
    #
    # def report_metric(self):
    #     self._metric.refresh()
    #     self.call_center_method('report_metric', {'m': self._metric.to_dict()})
