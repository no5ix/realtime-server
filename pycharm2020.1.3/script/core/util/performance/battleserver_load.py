# coding=utf-8
import time
import operator
from .cpu_load_handler import PerformanceMetric, Load, CPULoad
# from core.distserver.game import GameServerRepo
from core.common.EntityManager import EntityManager


class BattleProcessMetric(PerformanceMetric):
    def __init__(self):
        super(BattleProcessMetric, self).__init__(GameServerRepo.game_server_name)
        # avatar, robot, battle
        self._entity_count = Load(load_length=3)
        self._average_cpu = Load(load_length=1)
        self._last_cpu = CPULoad()

    def refresh(self):
        # called each second
        n_battle = 0
        n_avatar = 0
        n_robot = 0
        for _, _ent in EntityManager.instance().iter_entity():
            if _ent.__class__.__name__ != 'Battle':
                continue
            n_battle += 1
            entity_count = _ent.local_entity_mgr.entity_count
            n_avatar += entity_count['AvatarPuppet']
            n_robot += entity_count['RobotPuppet']

        self._entity_count.update(n_avatar, n_robot, n_battle)

        self._average_cpu.update(self._last_cpu.percentage())
        self._last_cpu.reset()

    def to_dict(self):
        d = super(BattleProcessMetric, self).to_dict()
        d.update({
            'cpu': self._average_cpu.get_load(0),
            'avt': self._entity_count.get_load(0),
            'rbt': self._entity_count.get_load(1),
            'battle': self._entity_count.get_load(2),
        })
        return d


class BattleCenterMetric(PerformanceMetric):
    # BattleAllocateCenter汇总的性能数据
    def __init__(self, ttl_seconds=5.0):
        super(BattleCenterMetric, self).__init__(GameServerRepo.hostid)
        self._ttl_seconds = ttl_seconds
        # {stub_process_name: performance_dict}
        self._stub_dict = {}
        # sorted list for online stubs
        self._sorted_stubs = []
        # 创建战斗导致的负载预估增加
        self._load_punishment = {}
        self._total_load = 0.0
        # 预备下线
        self.pending_shutdown = False

    def _estimate_load(self, stub_name):
        # 估算负载，0~100浮点数
        d = self._stub_dict[stub_name]
        cpu = d['cpu']
        load = cpu[0] * 0.9 + cpu[1] * 0.2 + cpu[2] * 0.1

        punish_start = self._load_punishment.get(stub_name)
        if punish_start:
            punish_length = time.time() - punish_start
            if punish_length <= 1.0:
                load += 50.0
            elif punish_length < 5.0:
                load += 50.0 / punish_length
            else:
                del self._load_punishment[stub_name]

        d['load'] = min(120.0, load)

    def update_from_stub(self, mailbox, performance_dict):
        name = performance_dict['index']
        performance_dict['mb'] = mailbox
        performance_dict['ttl'] = time.time() + self._ttl_seconds
        self._stub_dict[name] = performance_dict

    def refresh(self):
        # build sorted stubs
        alive_stubs = []
        now = time.time()
        sum_load = 0.0
        for stub_name, stub_load in self._stub_dict.iteritems():
            self._estimate_load(stub_name)
            if stub_name == u'battle_0':
                # Center进程，不要分配战斗了
                pass
            elif stub_load['ttl'] > now:
                # is alive
                alive_stubs.append(stub_load)
                # add load according to cpu
                sum_load += stub_load['load']
            else:
                # add full load
                sum_load += 120

        self._sorted_stubs = sorted(alive_stubs, key=operator.itemgetter('load'))
        if self._stub_dict:
            self._total_load = sum_load / len(self._stub_dict)
        else:
            self._total_load = 0

    def get_mb_by_load(self, rank_idx, load_punishment=True):
        if load_punishment:
            self.refresh()
        if not self._sorted_stubs:
            return None, None
        if rank_idx < 0:
            rank_idx = 0
        if rank_idx >= len(self._sorted_stubs):
            rank_idx = len(self._sorted_stubs) - 1
        d = self._sorted_stubs[rank_idx]
        stub_name = d['index']
        if load_punishment:
            # add punishment
            self._load_punishment[stub_name] = time.time()
        return stub_name, d['mb']

    def make_report(self):
        report = {
            'load': self._total_load,
            'stubs': map(operator.itemgetter('load'), self._sorted_stubs),
            'n_stubs': len(self._sorted_stubs),
            'n_battle': int(sum(x['battle'][0] for x in self._sorted_stubs)),
            'n_avt': int(sum(x['avt'][0] for x in self._sorted_stubs)),
            'n_rbt': int(sum(x['rbt'][0] for x in self._sorted_stubs)),
        }
        if self.pending_shutdown:
            report['pending_shutdown'] = True
        return report

