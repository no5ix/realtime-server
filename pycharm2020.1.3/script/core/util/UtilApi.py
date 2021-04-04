import asyncio
# from asyncio import AbstractEventLoop
from typing import Callable
# from TcpServer import ev_loop
# import typing
from common import gv


def wait_or_not(f):
    def wrapped(*args, **kwargs):
        if asyncio.iscoroutinefunction(f):
            is_running = gv.get_ev_loop().is_running()
            is_closed = gv.get_ev_loop().is_closed()
            return gv.get_ev_loop().create_task(f(*args, **kwargs))
        else:
            return gv.get_ev_loop().run_in_executor(None, f, *args, *kwargs)
    return wrapped


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


def log(text):
    def decorator(func):
        # @functools.wraps(func)
        def wrapper(*args, **kw):
            print('%s %s():' % (text, func.__name__))
            return func(*args, **kw)

        return wrapper

    return decorator
