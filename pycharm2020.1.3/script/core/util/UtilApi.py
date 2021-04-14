from __future__ import annotations
import asyncio
# from asyncio import AbstractEventLoop
import functools
from typing import Callable
# from TcpServer import ev_loop
# import typing
import aiohttp
import typing

if typing.TYPE_CHECKING:
    from ConnMgr import ConnMgr
    from TcpServer import TcpServer

from common import gv
# from core.mobilelog.LogManager import LogManager
from core.util import EnhancedJson
# from concurrent.futures.thread import ThreadPoolExecutor


server_singletons = {}


def add_server_singleton(entity, postfix=''):
    """添加一个GameServer内唯一的entity"""
    server_singletons[entity.__class__.__name__ + postfix] = entity


def get_server_singleton(entity_name):
    return server_singletons.get(entity_name, None)


class Singleton:
    def __init__(self, decorated_cls):
        self._decorated_cls = decorated_cls

    def instance(self):
        try:
            return self._instance
        except AttributeError:
            self._instance = self._decorated_cls()
            add_server_singleton(self._instance)
            return self._instance

    def __call__(self):
        raise TypeError('Singletons must be accessed through `instance()`.')

    def __instancecheck__(self, inst):
        return isinstance(inst, self._decorated_cls)


def async_lock(f):

    lock = asyncio.Lock()

    @functools.wraps(f)
    async def _wrapped(*args, **kwargs):
        async with lock:
            return await f(*args, **kwargs)
    return _wrapped


def wait_or_not(concurrency_limit=888):
    # Bind the default event loop
    # print("a bousennnnnn")
    # sem = asyncio.BoundedSemaphore(concurrency_limit)
    sem = asyncio.BoundedSemaphore(concurrency_limit, loop=gv.get_ev_loop())

    # LogManager.set_log_tag("tcp_client_" + str(1))
    # LogManager.set_log_path("../bin/win/log/")
    # logger = LogManager.get_logger()
    # logger.debug(f"b bousennnnnn {id(sem._loop)=}")
    # logger.debug(f"b bousennnnnn{id(sem)=}")

    def wait_or_not_without_limit(f):
        @functools.wraps(f)
        def _wrapped(*args, **kwargs):
            # logger.debug(f"b withold {id(gv.get_ev_loop())=}")

            return gv.get_ev_loop().create_task(f(*args, **kwargs))
        return _wrapped

    def executor(f):
        assert(asyncio.iscoroutinefunction(f))

        @functools.wraps(f)
        @wait_or_not_without_limit
        async def wrapped(*args, **kwargs):
            async with sem:
                return await f(*args, **kwargs)
        return wrapped
    return executor


# _async_wrap_tp_executor = ThreadPoolExecutor()


def async_wrap(func: Callable):  # TODO: 貌似和long polling 不和, 不适合用在 cpu bound 之处
    """
    usage: r = await AioApi.async_wrap(lambda: requests.request("GET", 'http://baidu.com', timeout=2))
    lambda关键字不可少
    """
    if not callable(func):
        raise Exception(f"{func=} is not callable")
    # dv_loop = gv.get_ev_loop()
    # print(f"{id(gv.get_ev_loop())=}")
    # print(f"{(gv.get_ev_loop()._default_executor)=}")
    # print(f"{id(gv.get_ev_loop()._default_executor)=}")
    return gv.get_ev_loop().run_in_executor(None, func)


async def async_http_requests(method: str, url: str, session: aiohttp.ClientSession = None, **kwargs) -> (str, dict):
    method = method.lower()
    assert(method in ("get", "put", "post", "delete"))

    async def _async_http_requests_impl(_method, _url, _session, **_kwargs):
        async with getattr(_session, _method)(_url, **_kwargs) as response:  # type: aiohttp.ClientResponse
            return await response.text(), response.headers

    if session is None:
        async with aiohttp.ClientSession() as session:
            return await _async_http_requests_impl(method, url, session, **kwargs)
    else:
        return await _async_http_requests_impl(method, url, session, **kwargs)


def get_global_entity_mailbox(entity_unique_name):
    return gv.etcd_service_node.get_entity_info(entity_unique_name)


def get_service_info(service_name):
    if gv.etcd_service_node is None:
        return None
    return gv.etcd_service_node.get_service_info(service_name)


def register_entity_globally():
    pass


def register_entity_to_etcd(entity, name, tag=None):
    # ip = gr.local_ip
    # port = gr.local_port
    pass


def unregister_entity_from_etcd(name):
    pass


def parse_json_conf(json_conf_path):
    # with open(r"../bin/win/conf/battle_server.json") as conf_file:
    with open(json_conf_path) as conf_file:
        # data = file.read()
        # _name = r'../bin/win/conf/battle_server.json'
        # file_name = r'D:\Documents\github\realtime-server\pycharm2020.1.3\bin\win\conf\battle_server.json'
        # file_name = r'C:\Users\b\Documents\github\realtime-server\pycharm2020.1.3\bin\win\conf\battle_server.json'
        # conf_file = open(file_name)
        json_conf = EnhancedJson.load(conf_file)
        # conf_file.close()
        gv.game_json_conf = json_conf

    # file_name = r'../bin/win/conf/battle_server.json'
    # # file_name = r'D:\Documents\github\realtime-server\pycharm2020.1.3\bin\win\conf\battle_server.json'
    # # file_name = r'C:\Users\b\Documents\github\realtime-server\pycharm2020.1.3\bin\win\conf\battle_server.json'
    # conf_file = open(file_name)
    # json_conf = json.load(conf_file)
    # conf_file.close()
    # gr.game_json_conf = json_conf
    return json_conf


def get_cur_server() -> TcpServer:
    return get_server_singleton("TcpServer")


def get_conn_mgr() -> ConnMgr:
    return get_server_singleton("ConnMgr")
