# coding=utf-8
import os
import time
import collections


def cpu_time():
    return sum(os.times()[:2])


def cpu_time_user():
    return os.times()[0]


def cpu_time_system():
    return os.times()[1]


class CPULoad(object):
    # 本进程CPU占比计算
    __slots__ = ('_base_ts', '_base_cpu_time')

    def __init__(self):
        super(CPULoad, self).__init__()
        self.reset()

    def percentage(self):
        return 100.0 * (cpu_time() - self._base_cpu_time) / (time.time() - self._base_ts)

    def reset(self):
        self._base_ts = time.time()
        self._base_cpu_time = cpu_time()


class LoadQueue(object):
    __slots__ = ('_q', '_max_len', 'avg_load')

    def __init__(self, maxlen=5):
        super(LoadQueue, self).__init__()
        self._max_len = maxlen
        self._q = collections.deque()
        self.avg_load = 0

    def append(self, load):
        self._q.append(load)
        self.avg_load += load * 1.0 / self._max_len
        while len(self._q) > self._max_len:
            self.avg_load -= self._q.popleft() * 1.0 / self._max_len


class CompositeLoadQueue(object):
    __slots__ = ('_q_5s', '_q_30s', '_q_90s')

    def __init__(self):
        super(CompositeLoadQueue, self).__init__()
        self._q_5s = LoadQueue(5)
        self._q_30s = LoadQueue(30)
        self._q_90s = LoadQueue(90)

    def append(self, load):
        self._q_5s.append(load)
        self._q_30s.append(load)
        self._q_90s.append(load)

    def extend(self, iterable_load):
        for appender in (self._q_5s.append, self._q_30s.append, self._q_90s.append):
            for l in iterable_load:
                appender(l)

    def load_tuple(self):
        return max(0, self._q_5s.avg_load), min(0, self._q_30s.avg_load), min(0, self._q_90s.avg_load)

    @property
    def avg_load_5s(self):
        return self._q_5s.avg_load

    @property
    def avg_load_30s(self):
        return self._q_30s.avg_load

    @property
    def avg_load_90s(self):
        return self._q_90s.avg_load


class Load(object):
    # 提供类似于uptime的load average统计
    #     将原来的1min、5min、15min修改为更小的时间粒度，5s、30s、90s

    def __init__(self, load_length=1, max_gap_seconds=5):
        super(Load, self).__init__()
        self._max_gap_seconds = max_gap_seconds
        self._last_tick = 0
        self._load_items = [CompositeLoadQueue() for _ in xrange(load_length)]

    def update(self, *loads):
        now = time.time()
        # 一秒一个点计算，需要补多少个点
        points = int(now - self._last_tick)
        self._last_tick = now
        if points <= 0:
            return
        for value, queue in zip(loads, self._load_items):
            if points == 1:
                queue.append(value)
            else:
                if points > self._max_gap_seconds:
                    values = [0] * min(points - self._max_gap_seconds, 90) + [value] * self._max_gap_seconds
                else:
                    values = [value] * points
                queue.extend(values)

    def get_load(self, index):
        return self._load_items[index].load_tuple()

    def get_loads(self):
        return [x.load_tuple() for x in self._load_items]


class PerformanceMetric(object):
    # 性能指标数据
    def __init__(self, index):
        super(PerformanceMetric, self).__init__()
        self._process_index = index

    def refresh(self):
        raise NotImplementedError

    def to_dict(self):
        # 返回单层dict，value必须是数字组成的tuple
        return {
            'index': self._process_index,
        }
