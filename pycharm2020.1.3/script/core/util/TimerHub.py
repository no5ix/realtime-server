import asyncio
import collections
import typing
from asyncio import TimerHandle
from time import monotonic, time


class TimerHub:

    def __init__(self, ev_loop=None):
        try:
            self._ev_loop = asyncio.get_running_loop() if ev_loop is None else ev_loop
        except RuntimeError:
            self._ev_loop = asyncio.get_event_loop()

        self._key_2_timer_list_map = collections.defaultdict(list)

    def call_later(
            self, delay_second: typing.Union[int, float],
            delay_callback: typing.Callable, key: str = None, repeat_count: int = 0,
            repeat_interval_sec: typing.Union[int, float] = None) -> (str, TimerHandle):
        """
        可以重复使用一个key, 并不会冲掉之前key的timer, 但是 `cancel_timer` 的时候, 会一次性全部cancel掉
        """
        if key is None:
            key = str(monotonic())

        callback = delay_callback
        if repeat_count > 0:
            def repeat_cb_wrapper(ds=delay_second, k=key, rc=repeat_count, ris=repeat_interval_sec):
                if ris is not None:
                    assert (type(ris) in (int, float))
                    ds = ris
                delay_callback()
                self.call_later(ds, delay_callback, k, rc - 1)

            callback = repeat_cb_wrapper
        elif repeat_count < 0:
            raise Exception("err: TimerHub.call_later repeat_count < 0 !!")

        _ret_timer = self._ev_loop.call_later(delay_second, callback)
        self._key_2_timer_list_map[key].append(_ret_timer)
        return key, _ret_timer

    def call_at(
            self, when_second: typing.Union[int, float],
            delay_callback: typing.Callable, key: str = None) -> (str, TimerHandle):
        return self.call_later(when_second-time(), delay_callback, key)

    def cancel_timer(self, key: str):
        try:
            for _cur_timer in self._key_2_timer_list_map[key]:
                _cur_timer.cancel()
            self._key_2_timer_list_map[key].clear()
            self._key_2_timer_list_map.pop(key)
        except KeyError:
            pass

    def has_timer(self, key: str):
        return key in self._key_2_timer_list_map

    def destroy(self):
        for _, _timer_list in self._key_2_timer_list_map.items():
            for _t in _timer_list:
                _t.cancel()
        self._key_2_timer_list_map.clear()
        self._key_2_timer_list_map = None
        self._ev_loop = None


if __name__ == "__main__":

    hello_str = "hhx"
    EV_LOOP = asyncio.get_event_loop()
    # EV_LOOP.call_later(2, partial(print, hello_str))
    # EV_LOOP.call_later(2, print, hello_str)
    cur_timer = EV_LOOP.call_later(1, lambda h=hello_str: print(h))
    hello_str = "mmd"
    cur_timer.cancel()

    test_timer_key1_str = "test_timer_key1"
    test_timer_key2_str = "test_timer_key2"

    th = TimerHub()
    # th.call_later(
    #     3, lambda h=test_timer_key1_str: print(h), test_timer_key1_str, repeat_count=3, repeat_interval_sec=1)
    # print(f"th.has_timer(test_timer_key1_str): {th.has_timer(test_timer_key1_str)}")
    #
    # th.call_later(
    #     2, lambda h=test_timer_key2_str: print(h), test_timer_key2_str, repeat_count=3, repeat_interval_sec=1)
    # print(f"before cancel, th.has_timer(test_timer_key2_str): {th.has_timer(test_timer_key2_str)}")
    # # th.cancel_timer(test_timer_key2_str)
    # print(f"after cancel, th.has_timer(test_timer_key2_str): {th.has_timer(test_timer_key2_str)}")
    #
    # # th.call_later(2, lambda: th.cancel_timer(test_timer_key2_str), "cancel_test_timer_key2_str_during_repeating")
    # _cur_timer_key, _ = th.call_at(time() + 4, lambda: th.cancel_timer(test_timer_key2_str))
    # th.cancel_timer(_cur_timer_key)

    th.call_later(
        0, lambda h="test_0_sec_timer": print(h), "test_0_sec_timer", repeat_count=2, repeat_interval_sec=0)

    def test_timer_circle():
        print("test_timer_circle")
        th.call_later(
            0, lambda: test_timer_circle(), "test_timer_circle"
            , repeat_count=2, repeat_interval_sec=0
        )
    test_timer_circle()

    print("test reach here")
    # th.destroy()

    EV_LOOP.run_forever()
