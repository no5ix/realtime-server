import asyncio
from typing import Callable
# from TcpServer import ev_loop


# try:
#     ev_loop =
# except RuntimeError:
#     ev_loop = None


# usage: r = await AioApi.async_wrap(lambda: requests.request("GET", 'http://baidu.com', timeout=2))
# lambda关键字不可少
def async_wrap(func: Callable):
    try:
        return asyncio.get_running_loop().run_in_executor(None, func)
    except RuntimeError:
        pass  # 正常情况不可能会发生调用此async_wrap比server启动还要早, 所以直接pass
