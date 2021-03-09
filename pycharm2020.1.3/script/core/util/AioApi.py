import asyncio
from typing import Callable

try:
    ev_loop = asyncio.get_running_loop()
except RuntimeError:
    ev_loop = asyncio.get_event_loop()
    pass


# usage: r = await AioApi.async_wrap(lambda: requests.request("GET", 'http://baidu.com', timeout=2))
# lambda关键字不可少
def async_wrap(func: Callable):
    return ev_loop.run_in_executor(None, func)
