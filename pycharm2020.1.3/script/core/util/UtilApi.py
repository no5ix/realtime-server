import asyncio
# from asyncio import AbstractEventLoop
import functools
from typing import Callable
# from TcpServer import ev_loop
# import typing
from common import gv


class Singleton:
    """
    ```
    @Singleton
    class Foo:
        def __init__(self):
            print('Foo created')

    f = Foo() # Error, this isn't how you get the instance of a singleton
    f = Foo.instance() # Good. Being explicit is in line with the Python Zen
    g = Foo.instance() # Returns already created instance
    print(f is g) # True
    ```

    ------------------------
    A non-thread-safe helper class to ease implementing singletons.
    This should be used as a decorator -- not a metaclass -- to the
    class that should be a singleton.

    The decorated_cls class can define one `__init__` function that
    takes only the `self` argument. Also, the decorated_cls class cannot be
    inherited from. Other than that, there are no restrictions that apply
    to the decorated_cls class.

    To get the singleton instance, use the `instance` method. Trying
    to use `__call__` will result in a `TypeError` being raised.
    """

    def __init__(self, decorated_cls):
        self._decorated_cls = decorated_cls

    def instance(self):
        """
        Returns the singleton instance. Upon its first call, it creates a
        new instance of the decorated_cls class and calls its `__init__` method.
        On all subsequent calls, the already created instance is returned.

        """
        try:
            return self._instance
        except AttributeError:
            self._instance = self._decorated_cls()
            return self._instance

    def __call__(self):
        raise TypeError('Singletons must be accessed through `instance()`.')

    def __instancecheck__(self, inst):
        return isinstance(inst, self._decorated_cls)


def wait_or_not(concurrency_limit=88):
    # Bind the default event loop
    # print("a bousennnnnn")
    sem = asyncio.BoundedSemaphore(concurrency_limit)
    # print(f"b bousennnnnn{id(sem._loop)=}")

    def wait_or_not_without_limit(f):
        def _wrapped(*args, **kwargs):
            return gv.get_ev_loop().create_task(f(*args, **kwargs))
            # if asyncio.iscoroutinefunction(f):
            #     # is_running = gv.get_ev_loop().is_running()
            #     # is_closed = gv.get_ev_loop().is_closed()
            #     return gv.get_ev_loop().create_task(f(*args, **kwargs))
            # else:
            #     return gv.get_ev_loop().run_in_executor(None, f, *args, *kwargs)
        return _wrapped

    def executor(f):
        @functools.wraps(f)
        @wait_or_not_without_limit
        async def wrapped(*args, **kwargs):
            async with sem:
                # _res = f(*args, **kwargs)
                # if asyncio.iscoroutine(_res):
                #     return await _res
                # return _res
                return f(*args, **kwargs)
        return wrapped
    return executor


# def request_concurrency_limit(limit=3):
#     # Bind the default event loop
#     sem = asyncio.BoundedSemaphore(limit)
#
#     def executor(func):
#         @functools.wraps(func)
#         async def wrapper(*args, **kwargs):
#             async with sem:
#                 if asyncio.iscoroutinefunction(func):
#                     # is_running = gv.get_ev_loop().is_running()
#                     # is_closed = gv.get_ev_loop().is_closed()
#                     return await gv.get_ev_loop().create_task(func(*args, **kwargs))
#                 else:
#                     return await gv.get_ev_loop().run_in_executor(None, func, *args, *kwargs)
#
#         return wrapper
#     return executor


def async_wrap(func: Callable):
    """
    usage: r = await AioApi.async_wrap(lambda: requests.request("GET", 'http://baidu.com', timeout=2))
    lambda关键字不可少
    """
    return gv.get_ev_loop().run_in_executor(None, func)


def get_global_entity_mailbox(entity_unique_name):
    return gv.etcd_service_node.get_entity_info(entity_unique_name)


def get_service_info(service_name):
    return gv.etcd_service_node.get_service_info(service_name)


def register_entity_globally():
    pass


def register_entity_to_etcd(entity, name, tag=None):
    # ip = gr.local_ip
    # port = gr.local_port
    pass


def unregister_entity_from_etcd(name):
    pass


