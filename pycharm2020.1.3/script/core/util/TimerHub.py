import asyncio
from collections import defaultdict
import typing
from time import monotonic_ns, time

from core.util.UtilApi import wait_or_not


class TimerInfo:
    __slots__ = ('timer', 'original_key', 'final_key')

    def __init__(self, timer, original_key, final_key):
        self.timer = timer
        self.original_key = original_key
        self.final_key = final_key


class TimerHub:

    def __init__(self, ev_loop=None):
        try:
            self._ev_loop = asyncio.get_running_loop() if ev_loop is None else ev_loop
        except RuntimeError:
            self._ev_loop = asyncio.get_event_loop()

        self._final_key_2_timer_info_map = {}
        self._original_key_2_final_key_set_map = defaultdict(set)
        self._key_incr = 0
        self._destroyed = False

    def call_later(
            self, delay_second: typing.Union[int, float],
            delay_callback: typing.Callable, key: str = "", repeat_count: int = 0,
            repeat_interval_sec: typing.Union[int, float] = None) -> str:
        """
        可以重复使用一个key, 并不会冲掉之前key的timer, 但是当调用`cancel_timer`的时候, 会一次性全部cancel掉所有
        :param delay_second:  如果小于0, 则相当于等于0, 而等于0并不等同于立即执行(为了避免死循环), 而是在下一轮的loop中尽快执行
        :param delay_callback:
        :param key:
        :param repeat_count: 当等于 -1 的时候则为无限重复
        :param repeat_interval_sec:
        :return: 最终的唯一key
        """
        assert (not self._destroyed)
        assert (repeat_count >= 0 or repeat_count == -1)
        # if repeat_count < 0 and repeat_count != -1:
        #     raise Exception("err: TimerHub.call_later repeat_count < 0 and not equal to -1 !!")

        # final_key = "_".join((key, str(monotonic_ns())))
        final_key = self._get_final_key(key)
        if repeat_count > 0 or repeat_count == -1:
            _ret_timer = self._handle_repetitive_timer(
                delay_second, delay_callback, key, final_key,
                repeat_count, repeat_interval_sec)
        else:  # repeat_count == 0:
            _ret_timer = self._handle_disposable_timer(
                delay_second, delay_callback, key, final_key)

        self._original_key_2_final_key_set_map[key].add(final_key)
        self._final_key_2_timer_info_map[final_key] = TimerInfo(_ret_timer, key, final_key)
        return final_key

    def call_at(
            self, when_second: typing.Union[int, float], delay_callback: typing.Callable, key: str = "",
            repeat_count: int = 0, repeat_interval_sec: typing.Union[int, float] = None) -> str:
        return self.call_later(when_second-time(), delay_callback, key, repeat_count, repeat_interval_sec)

    def cancel_timer(self, key: str):
        # try:
        # print(f"cccccccanc key: {key}")
        # print(f"_original_key_2_final_key_set_map: {self._original_key_2_final_key_set_map}")
        if key in self._original_key_2_final_key_set_map:
            for _final_key in self._original_key_2_final_key_set_map[key]:
                self._final_key_2_timer_info_map[_final_key].timer.cancel()
                self._final_key_2_timer_info_map.pop(_final_key, None)
            self._original_key_2_final_key_set_map[key].clear()
            self._original_key_2_final_key_set_map.pop(key)
        elif key in self._final_key_2_timer_info_map:
            _timer_info = self._final_key_2_timer_info_map[key]
            self._original_key_2_final_key_set_map[_timer_info.original_key].discard(key)
            self._final_key_2_timer_info_map[key].timer.cancel()
            self._final_key_2_timer_info_map.pop(key, None)
        # except KeyError:
        #     pass

    def has_timer(self, key: str):
        if key in self._original_key_2_final_key_set_map:
            return len(self._original_key_2_final_key_set_map[key]) > 0
        return key in self._final_key_2_timer_info_map

    def destroy(self):
        # print("desssssstoryyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy")
        # print(f"{self._final_key_2_timer_info_map}")
        for _, _timer_info in self._final_key_2_timer_info_map.items():
            # print(f"_timer_info.final_key: {_timer_info.final_key}")
            _timer_info.timer.cancel()
        # print("affffff dd")
        self._final_key_2_timer_info_map.clear()
        self._final_key_2_timer_info_map = None
        for _, _final_key_set in self._original_key_2_final_key_set_map.items():
            _final_key_set.clear()
        self._original_key_2_final_key_set_map.clear()
        self._original_key_2_final_key_set_map = None
        self._ev_loop = None
        self._destroyed = True

    def _get_final_key(self, key):
        self._key_incr += 1
        if self._key_incr >= 10000000:
            self._key_incr = 0
        return "_".join((key, str(monotonic_ns() + self._key_incr)))

    def _handle_repetitive_timer(
            self, delay_second: typing.Union[int, float],
            delay_callback: typing.Callable, original_key: str = "", final_key: str = "",
            repeat_count: int = 0, repeat_interval_sec: typing.Union[int, float] = None):
        if self._destroyed:
            return

        @wait_or_not
        async def repeat_cb_wrapper(ds=delay_second, rc=repeat_count, ris=repeat_interval_sec):
            if ris is not None:
                assert (type(ris) in (int, float))
                ds = ris
            _cb_res = delay_callback()
            if asyncio.iscoroutine(_cb_res):
                await _cb_res
            if rc == 1:
                self._handle_disposable_timer(
                    ds, delay_callback, original_key, final_key)
            elif rc > 1 or rc == -1:
                self._handle_repetitive_timer(
                    ds, delay_callback, original_key, final_key, rc-1 if rc > 1 else rc)

        _ret_timer = self._ev_loop.call_later(delay_second, repeat_cb_wrapper)
        self._final_key_2_timer_info_map[final_key] = TimerInfo(_ret_timer, original_key, final_key)
        return _ret_timer

    def _handle_disposable_timer(
            self, delay_second: typing.Union[int, float],
            delay_callback: typing.Callable, original_key: str = "", final_key: str = ""):

        @wait_or_not
        async def funeral_cb_wrapper(_ok=original_key, _fk=final_key):
            _cb_res = delay_callback()
            if asyncio.iscoroutine(_cb_res):
                await _cb_res
            # print(f"funeral_cb_wrapper: _ok: {_ok}")
            # print(f"funeral_cb_wrapper: _fk: {_fk}")
            if self._final_key_2_timer_info_map is not None and self._original_key_2_final_key_set_map is not None:
                self._final_key_2_timer_info_map.pop(_fk, None)
                self._original_key_2_final_key_set_map[_ok].discard(_fk)

        _ret_timer = self._ev_loop.call_later(delay_second, funeral_cb_wrapper)
        return _ret_timer


if __name__ == "__main__":

    # hello_str = "hhx"
    EV_LOOP = asyncio.get_event_loop()
    # # EV_LOOP.call_later(2, partial(print, hello_str))
    # # EV_LOOP.call_later(2, print, hello_str)
    # cur_timer = EV_LOOP.call_later(1, lambda h=hello_str: print(h))
    # hello_str = "mmd"
    # cur_timer.cancel()

    test_timer_key1_str = "test_timer_key1"
    test_timer_key2_str = "test_timer_key2"

    th = TimerHub()

    async def test_lambda_async_func():
        await asyncio.sleep(1)
        print("test_lambda_async_func success")

    th.call_later(1, lambda: test_lambda_async_func())

    def print_key1():
        print(test_timer_key1_str)

    th.call_later(
        3, lambda: print_key1(), test_timer_key1_str, repeat_count=-1, repeat_interval_sec=2)
    print(f"th.has_timer(test_timer_key1_str): {th.has_timer(test_timer_key1_str)}")

    th.call_later(
        2, lambda h=test_timer_key2_str: print(h), test_timer_key2_str, repeat_count=3, repeat_interval_sec=1)
    print(f"before cancel, th.has_timer(test_timer_key2_str): {th.has_timer(test_timer_key2_str)}")
    th.cancel_timer(test_timer_key2_str)
    print(f"after cancel, th.has_timer(test_timer_key2_str): {th.has_timer(test_timer_key2_str)}")

    th.call_later(2, lambda: th.cancel_timer(test_timer_key2_str), "cancel_test_timer_key2_str_during_repeating")
    _cur_timer_key = th.call_at(time() + 4, lambda: th.cancel_timer(test_timer_key1_str))

    # _cur_timer_key = th.call_later(4, lambda: th.cancel_timer("zamehui ?"))
    # th.call_later(4, lambda: th.cancel_timer("zamehui ?"), "dfadfa")
    # th.call_later(4, lambda: th.cancel_timer("zddd ?"))
    # th.call_later(4, lambda: print("ooi"))
    # print(f"_cur_timer_key: {_cur_timer_key}")
    print(f"before cancel, th.has_timer(_cur_timer_key): {th.has_timer(_cur_timer_key)}")
    th.cancel_timer(_cur_timer_key)
    print(f"after cancel, th.has_timer(_cur_timer_key): {th.has_timer(_cur_timer_key)}")

    # th.call_later(
    #     0, lambda h="test_0_sec_timer": print(h), "test_0_sec_timer", repeat_count=2, repeat_interval_sec=0)

    # def test_timer_circle():
    #     print("test_timer_circle")
    #     th.call_later(
    #         0.1, lambda: test_timer_circle(), "test_timer_circle"
    #         # , repeat_count=2, repeat_interval_sec=0.1
    #     )
    # test_timer_circle()

    # print(f"abef daffinal: {th._final_key_2_timer_info_map}")
    th.call_at(
        time() + 4.5, lambda: print("test call at repeat"), repeat_count=2, repeat_interval_sec=1)

    print("test reach here")
    # des_key = th.call_later(0, lambda: th.destroy())
    # print(f"des_key: {des_key}")

    # print(f"daffinal: {th._final_key_2_timer_info_map}")
    # th.destroy()

    EV_LOOP.run_forever()
