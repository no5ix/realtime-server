import asyncio
from collections import defaultdict
import typing
from time import monotonic, time


class TimerHub:

    def __init__(self, ev_loop=None):
        try:
            self._ev_loop = asyncio.get_running_loop() if ev_loop is None else ev_loop
        except RuntimeError:
            self._ev_loop = asyncio.get_event_loop()

        self._final_key_2_timer_map = {}
        self._original_key_2_final_key_set_map = defaultdict(set)

    def call_later(
            self, delay_second: typing.Union[int, float],
            delay_callback: typing.Callable, key: str = "", repeat_count: int = 0,
            repeat_interval_sec: typing.Union[int, float] = None) -> str:
        """
        可以重复使用一个key, 并不会冲掉之前key的timer, 但是当调用`cancel_timer`的时候, 会一次性全部cancel掉所有
        :param delay_second:
        :param delay_callback:
        :param key:
        :param repeat_count: 当等于 -1 的时候则为无限重复  # TODO
        :param repeat_interval_sec:
        :return: 最终的唯一key
        """
        if repeat_count < 0 and repeat_count != -1:
            raise Exception("err: TimerHub.call_later repeat_count < 0 !!")

        final_key = "_".join((key, str(monotonic())))
        if repeat_count > 0:
            _ret_timer = self._handle_repetitive_timer(
                delay_second, delay_callback, key, final_key,
                repeat_count, repeat_interval_sec)
        elif repeat_count == 0:
            _ret_timer = self._handle_disposable_timer(
                delay_second, delay_callback, key, final_key)
        else:  # repeat_count < 0:
            raise Exception("err: TimerHub.call_later repeat_count < 0 !!")

        self._original_key_2_final_key_set_map[key].add(final_key)
        self._final_key_2_timer_map[final_key] = _ret_timer
        return final_key

    def call_at(
            self, when_second: typing.Union[int, float],
            delay_callback: typing.Callable, key: str = "") -> str:
        return self.call_later(when_second-time(), delay_callback, key)

    def cancel_timer(self, key: str):
        try:
            for _final_key in self._original_key_2_final_key_set_map[key]:
                self._final_key_2_timer_map[_final_key].cancel()
                self._final_key_2_timer_map.pop(_final_key, None)
            self._original_key_2_final_key_set_map[key].clear()
            self._original_key_2_final_key_set_map.pop(key)
        except KeyError:
            pass

    def has_timer(self, key: str):
        if key in self._original_key_2_final_key_set_map:
            return len(self._original_key_2_final_key_set_map[key]) > 0
        return False

    def destroy(self):
        for _, _timer in self._final_key_2_timer_map.items():
            _timer.cancel()
        self._final_key_2_timer_map.clear()
        self._final_key_2_timer_map = None
        for _, _final_key_set in self._original_key_2_final_key_set_map.items():
            _final_key_set.clear()
        self._original_key_2_final_key_set_map.clear()
        self._original_key_2_final_key_set_map = None
        self._ev_loop = None

    def _handle_repetitive_timer(
            self, delay_second: typing.Union[int, float],
            delay_callback: typing.Callable, original_key: str = "", final_key: str = "",
            repeat_count: int = 0, repeat_interval_sec: typing.Union[int, float] = None,
            is_permanent=False):

        def repeat_cb_wrapper(ds=delay_second, rc=repeat_count, ris=repeat_interval_sec):
            if ris is not None:
                assert (type(ris) in (int, float))
                ds = ris
            delay_callback()
            _final_rc = rc - 1
            if _final_rc == 0:
                self._handle_disposable_timer(
                    ds, delay_callback, original_key, final_key)
            elif _final_rc > 0:
                self._handle_repetitive_timer(
                    ds, delay_callback, original_key, final_key, _final_rc)

        _ret_timer = self._ev_loop.call_later(delay_second, repeat_cb_wrapper)
        self._final_key_2_timer_map[final_key] = _ret_timer
        return _ret_timer

    def _handle_disposable_timer(
            self, delay_second: typing.Union[int, float],
            delay_callback: typing.Callable, original_key: str = "", final_key: str = ""):

        def funeral_wrapper(_ok=original_key, _fk=final_key):
            delay_callback()
            # print(f"funeral_wrapper: _ok: {_ok}")
            # print(f"funeral_wrapper: _fk: {_fk}")
            self._final_key_2_timer_map.pop(_fk, None)
            self._original_key_2_final_key_set_map[_ok].discard(_fk)
            pass

        _ret_timer = self._ev_loop.call_later(delay_second, funeral_wrapper)
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
    th.call_later(
        3, lambda h=test_timer_key1_str: print(h), test_timer_key1_str, repeat_count=3, repeat_interval_sec=1)
    print(f"th.has_timer(test_timer_key1_str): {th.has_timer(test_timer_key1_str)}")
    
    th.call_later(
        2, lambda h=test_timer_key2_str: print(h), test_timer_key2_str, repeat_count=3, repeat_interval_sec=1)
    print(f"before cancel, th.has_timer(test_timer_key2_str): {th.has_timer(test_timer_key2_str)}")
    th.cancel_timer(test_timer_key2_str)
    print(f"after cancel, th.has_timer(test_timer_key2_str): {th.has_timer(test_timer_key2_str)}")

    # th.call_later(2, lambda: th.cancel_timer(test_timer_key2_str), "cancel_test_timer_key2_str_during_repeating")
    _cur_timer_key = th.call_at(time() + 3.5, lambda: th.cancel_timer(test_timer_key2_str))
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
