import asyncio
import typing


class TimerHub:

    def __init__(self, ev_loop=None):
        try:
            self._ev_loop = asyncio.get_running_loop() if ev_loop is None else ev_loop
        except RuntimeError:
            self._ev_loop = asyncio.get_event_loop()
        self._key_2_timer_map = {}

    def add_timer(
            self, key: str, delay_second: typing.Union[int, float],
            delay_callback: typing.Callable, repeat_count: int = 0):
        """
        pass
        """

        def repeat_cb_wrapper(cb=delay_callback, rc=repeat_count):
            cb()
            self.add_timer(key, delay_second, cb, rc - 1)

        callback = delay_callback
        if repeat_count > 0:
            callback = repeat_cb_wrapper
            # self._key_2_timer_map[key] = self._ev_loop.call_later(delay_second, callback)
        elif repeat_count < 0:
            raise Exception("err: TimerHub.add_timer repeat_count < 0 !!")

        self._key_2_timer_map[key] = self._ev_loop.call_later(delay_second, callback)

    def cancel_timer(self, key: str):
        try:
            _cur_timer = self._key_2_timer_map.pop(key)
            _cur_timer.cancel()
        except KeyError:
            pass

    def has_timer(self, key: str):
        return key in self._key_2_timer_map


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
    th.add_timer("test_timer_key1", 2, lambda h=test_timer_key1_str: print(h), repeat_count=2)
    print(f"th.has_timer(test_timer_key1_str): {th.has_timer(test_timer_key1_str)}")

    th.add_timer(test_timer_key2_str, 2, lambda h=test_timer_key2_str: print(h), repeat_count=3)
    print(f"before cancel, th.has_timer(test_timer_key2_str): {th.has_timer(test_timer_key2_str)}")
    # th.cancel_timer(test_timer_key2_str)
    print(f"after cancel, th.has_timer(test_timer_key2_str): {th.has_timer(test_timer_key2_str)}")

    th.add_timer("cancel_test_timer_key2_str_during_repeating", 2, lambda: th.cancel_timer(test_timer_key2_str))

    # EV_LOOP.run_until_complete()
    EV_LOOP.run_forever()
