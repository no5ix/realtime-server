# # -*- coding: utf-8 -*-
# import random
# # import gr
# # from Stub import Stub
# # from common import const
# # from common.cdata import DungeonConfig
# # from common.utils import collision_util, import_helper
# # from core.common.IdManager import IdManager
# # from core.common.RpcSupport import rpc_method, SERVER_ONLY
# # from core.common.RpcMethodArgs import Dict, MailBox, Uuid, Str, Int, List, Response
# # from core.common.EntityFactory import EntityFactory
# # from core.common.EntityManager import EntityManager
# # from core.common.mobilecommon import asiocore
# # from core.distserver.game import GameServerRepo
# # from const import service_const
# # from core.servercommon.PostmanDelayGuard import PostmanDelayGuard
# # from util.performance.battleserver_load import BattleProcessMetric
# # from util.sunshine import node_executor
# # from core.util.performance.battleserver_load import BattleProcessMetric
# # from core.util.performance.battleserver_load import BattleProcessMetric
# from core.util.performance.cpu_load_handler import AvgCpuLoad
# from server_entity.center_stub.Stub import Stub
#
#
# class BattleAllocatorStub(Stub):
#
#     def __init__(self, entity_id=None):
#         super(BattleAllocatorStub, self).__init__(entity_id)
#         # self._metric = BattleProcessMetric()
#         self._avg_load = AvgCpuLoad()
#         self.timer_hub.call_later(1.0, self.report_metric, repeat_count=-1, repeat_interval_sec=1.0)
#
#     def report_metric(self):
#         # self._metric.refresh()
#         # self.call_center_method('report_metric', {'m': self._metric.to_dict()})
#         # self.call_center_method('report_load', )
#         print(f"avg load: {self._avg_load.get_avg_cpu_by_period(10)}")
#
