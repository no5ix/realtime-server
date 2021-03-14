# -*- coding: utf-8 -*-
import random
import time

import gr
import collections
from Center import Center
from common.cdata import DungeonConfig
from core.common.ProtoEncoder import ProtoEncoder
from util.SALoggerManager import SALoggerManager
from util.performance.battleserver_load import BattleCenterMetric
from core.distserver.game import GameServerRepo
from core.distserver.game import GameAPI
from const import service_const, server_const
from core.common.rpcdecorator import rpc_method, SERVER_ONLY
from core.common.RpcMethodArgs import Response, Dict, Str, Int, Uuid, MailBox, List
from server_entity.center_stub import Center

_encoder = ProtoEncoder('compact')


class BattleAllocatorCenter(Center):
    def __init__(self, entityid=None):
        super(BattleAllocatorCenter, self).__init__(entityid)
        self.sa_logger = SALoggerManager()

        self.battle_id_2_info = {}  # battle id -> dun info
        self.pending_joins = collections.defaultdict(list)  # battle_id -> params
        # 负载数据
        self._load_metric = BattleCenterMetric()
        self._delay_calls.callback('report_tick', 5, self.send_report_tick, repeat_num=-1, first_delay=10.0)

    def call_server_method(self, remote_mailbox, methodname, parameters=None, callback=None):
        if parameters is None:
            parameters = {}
        return self.call_server_method_direct(remote_mailbox, methodname, parameters=parameters)

    def destroy(self, callback=None, freelocalid=False):
        GameAPI.unregister_entity_from_etcd('%s-bac' % GameServerRepo.hostid)
        super(BattleAllocatorCenter, self).destroy(callback, freelocalid)

    def send_report_tick(self):
        # 定时向战斗分配服务，发送自身负载健康数据
        self._load_metric.refresh()
        report = self._load_metric.make_report()
        args = GameServerRepo.hostid, GameAPI.encode_mailbox(self.get_mailbox()), GameServerRepo.battle_type, report

        def _cb(error, _):
            if error:
                self.logger.error('report metric failed, error=%s', error)
                return

        future = GameAPI.request_mobile_service(service_const.SERVICE_BATTLE, 'battle_server_report', args,
                                                need_reply=True, timeout=2.0)
        future.add_listener(_cb)
        self.logger.info(
            'battle_server_report hostid=%s load=%s n_stubs=%s n_battle=%s n_avt=%s n_rbt=%s, battle_type=%s',
            GameServerRepo.hostid, report['load'], report['n_stubs'],
            report['n_battle'], report['n_avt'], report['n_rbt'], GameServerRepo.battle_type,
        )

    def pending_shutdown(self):
        # 进入预备下线状态
        self._load_metric.pending_shutdown = True

    @rpc_method(SERVER_ONLY, (MailBox(), Dict('m'),))
    def report_metric(self, stub_mb, metric_dict):
        self._load_metric.update_from_stub(stub_mb, metric_dict)