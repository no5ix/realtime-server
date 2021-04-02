import os
import time
import collections

from core.util.TimerHub import TimerHub


class CpuLoad:

    def __init__(self):
        super(CpuLoad, self).__init__()
        self._last_ts = None
        self._last_cpu_time = None
        self._reset()

    @staticmethod
    def _cpu_time():
        return sum(os.times()[:2])

    def get_usage_ratio(self):
        jj = (self._cpu_time() - self._last_cpu_time)
        # print(f"jj: {jj}")
        # print(f"_last_cpu_time: {self._last_cpu_time}")

        ss = (time.time() - self._last_ts)
        # print(f"ss: {ss}")

        _ret = 100.0 * (self._cpu_time() - self._last_cpu_time) / (time.time() - self._last_ts)
        # print(f"ret: {_ret}")
        self._reset()
        return _ret

    def _reset(self):
        self._last_ts = time.time()
        self._last_cpu_time = self._cpu_time()


class AvgCpuLoad:

    def __init__(self):
        self._cpu_load_deque = collections.deque(maxlen=90)
        self._cpu_load = CpuLoad()
        self._timer_hub = TimerHub()

        self._timer_hub.call_later(
            1, lambda: self._cpu_load_deque.appendleft(self._cpu_load.get_usage_ratio()),
            repeat_count=-1)

    def get_avg_cpu_by_period(self, period_second: int):
        _sum_load = 0
        for _i, _load in enumerate(self._cpu_load_deque):
            _sum_load += _load
            if _i >= period_second - 1:
                break
        _len = len(self._cpu_load_deque)
        _ret = _sum_load / (_len if _len < period_second else period_second)
        return _ret


# class LoadQueue(object):
#     __slots__ = ('_q', '_max_len', 'avg_load')
#
#     def __init__(self, maxlen=5):
#         super(LoadQueue, self).__init__()
#         self._max_len = maxlen
#         self._q = collections.deque()
#         self.avg_load = 0
#
#     def append(self, load):
#         self._q.append(load)
#         self.avg_load += load * 1.0 / self._max_len
#         while len(self._q) > self._max_len:
#             self.avg_load -= self._q.popleft() * 1.0 / self._max_len
#
#
# class CompositeLoadQueue(object):
#     __slots__ = ('_q_5s', '_q_30s', '_q_90s')
#
#     def __init__(self):
#         super(CompositeLoadQueue, self).__init__()
#         self._q_5s = LoadQueue(5)
#         self._q_30s = LoadQueue(30)
#         self._q_90s = LoadQueue(90)
#
#     def append(self, load):
#         self._q_5s.append(load)
#         self._q_30s.append(load)
#         self._q_90s.append(load)
#
#     def extend(self, iterable_load):
#         for appender in (self._q_5s.append, self._q_30s.append, self._q_90s.append):
#             for l in iterable_load:
#                 appender(l)
#
#     def load_tuple(self):
#         return max(0, self._q_5s.avg_load), min(0, self._q_30s.avg_load), min(0, self._q_90s.avg_load)
#
#     @property
#     def avg_load_5s(self):
#         return self._q_5s.avg_load
#
#     @property
#     def avg_load_30s(self):
#         return self._q_30s.avg_load
#
#     @property
#     def avg_load_90s(self):
#         return self._q_90s.avg_load
#
#
# class Load(object):
#     # 提供类似于uptime的load average统计
#     #     将原来的1min、5min、15min修改为更小的时间粒度，5s、30s、90s
#
#     def __init__(self, load_length=1, max_gap_seconds=5):
#         super(Load, self).__init__()
#         self._max_gap_seconds = max_gap_seconds
#         self._last_tick = 0
#         self._load_items = [CompositeLoadQueue() for _ in range(load_length)]
#
#     def update(self, *loads):
#         now = time.time()
#         # 一秒一个点计算，需要补多少个点
#         points = int(now - self._last_tick)
#         self._last_tick = now
#         if points <= 0:
#             return
#         for value, queue in zip(loads, self._load_items):
#             if points == 1:
#                 queue.append(value)
#             else:
#                 if points > self._max_gap_seconds:
#                     values = [0] * min(points - self._max_gap_seconds, 90) + [value] * self._max_gap_seconds
#                 else:
#                     values = [value] * points
#                 queue.extend(values)
#
#     def get_load(self, index):
#         return self._load_items[index].load_tuple()
#
#     def get_loads(self):
#         return [x.load_tuple() for x in self._load_items]
#
#
# class PerformanceMetric(object):
#     # 性能指标数据
#     def __init__(self, index):
#         super(PerformanceMetric, self).__init__()
#         self._process_index = index
#
#     def refresh(self):
#         raise NotImplementedError
#
#     def to_dict(self):
#         # 返回单层dict，value必须是数字组成的tuple
#         return {
#             'index': self._process_index,
#         }
