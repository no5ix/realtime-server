# -*- coding: utf-8 -*-
import collections
from typing import Union, Optional

import aiohttp
# import requests
import asyncio

import typing

import time

from common import gv
from core.util import UtilApi
# from ..distserver.game import GameServerRepo
# from common import gv
from core.mobilelog.LogManager import LogManager
import urllib.parse
import json
# import gevent
# import gevent.event
import random

from core.util.UtilApi import wait_or_not
from core.util.performance.cpu_load_handler import AvgCpuLoad

_WATCH_TIMEOUT = 60                             # watch执行的超时时间
# _WATCH_TIMEOUT = 0.5                             # watch执行的超时时间  # TODO: del
_GET_TIMEOUT = 5                                # 获取节点值或者refresh ttl的超时
_MAX_FAIL_TIME = 10                             # get 或者 regist, refresh ttl 的时候最多的失败尝试次数


_KEY_TIME_OUT = 29                             # key超时时间
_TTL_INTERVAL = 6                             # ttl refresh间隔时间
_TTL_INTERVAL_RAND_LATENCY = 8                   # ttl refresh间隔时间的随机延迟
# _KEY_TIME_OUT = 22                             # key超时时间  # TODO: del
# _TTL_INTERVAL = 1                             # ttl refresh间隔时间  # TODO: del
# _TTL_INTERVAL_RAND_LATENCY = 1                   # ttl refresh间隔时间的随机延迟  # TODO: del

_TTL_RETRY = 2                                  # ttl刷新失败之后重新尝试的延迟间隔


_ETCD_KEY_PREFIX = "/v2/keys"
_NAME_ENTITY_DOMAIN = "/v2/keys/nameentity/"    # entity的名字与信息将会注册到这个目录下
_TAG_DOMAIN = "/v2/keys/tags/"                  # 注册tag的目录

_NAME_ENTITY_PREFIX = "/nameentity/"

_SERVICE_PREFIX = "/services/"
_SERVICE_DOMAIN = "/v2/keys" + _SERVICE_PREFIX


_HEADER = {"Content-Type": "application/x-www-form-urlencoded"}


# etcd Version: 3.2.11
# Git SHA: 1e1dbb2
# Go Version: go1.8.5
# Go OS/Arch: linux/amd64
class EtcdProcessor(object):
    def __init__(self, etcd_address_list):
        self._fail_time = 0
        self._stop = False
        self._etcd_address_list = self._form_etcd_url(etcd_address_list)         # 构建etcd服务器列表
        self._server_len = len(self._etcd_address_list) - 1
        self._session = None  # type: typing.Optional[aiohttp.ClientSession]

    def _get_server_info(self):
        return random.choice(self._etcd_address_list)

    def _form_etcd_url(self, address_list):
        out = []
        for address in address_list:
            ip, port = address[0], address[1]
            out.append("http://%s:%s" % (ip, port))
        return out

    def check_ok(self):
        if not self._stop and self._fail_time < _MAX_FAIL_TIME:
            return True
        return False

    def stop(self):
        self._stop = True


class ServiceRegister(EtcdProcessor):
    def __init__(self, etcd_address_list, my_address, service_tag, server_name):
        EtcdProcessor.__init__(self, etcd_address_list)
        self._logger = LogManager.get_logger()
        self._my_address = my_address                                            # 当前节点的监听地址(ip, port)
        self._service_name_address_str = server_name + "|" + my_address[0] + "|" + str(my_address[1])  # 当前节点的监听地址的字符串描述
        self._service_tag = service_tag
        self._server_name = server_name                          # 服务模块的name->module字典
        # self._stop_event = gevent.event.Event()
        self._service_regist_status = dict()
        self._threads = []
        # self._die_event = gevent.event.Event()

        # for name in self._service_module_dict.keys():
            # self._threads.append(gevent.spawn(self._process_regist_and_ttl, name))
        self._threads.append(self._update_cpu_load_n_ttl(self._service_tag))
            # self._threads.append(asyncio.create_task(self._process_regist_and_ttl(name)))

        # gevent.spawn(self._dead_check_process)
        # await asyncio.gather(*self._threads)

        self._avg_load = AvgCpuLoad()

    # async def _dead_check_process(self):
    async def start(self, session: aiohttp.ClientSession):
        # for thread in self._threads:
        #     thread.join()
        # self._die_event.set()
        self._session = session
        await asyncio.gather(*self._threads)

    # def wait_dead(self, timeout):
    #     return self._die_event.wait(timeout=timeout)

    def _get_url(self, service_name):
        path = self._get_server_info() + _SERVICE_DOMAIN + service_name + "/" + self._service_name_address_str
        # self._logger.debug(f'{path=}')
        return path

    def _do_unregist(self, service_name):
        pass

    async def _update_cpu_load(self, service_tag):
        """
        注册service，如果注册成功，返回True
        """
        # service_module = self._service_module_dict[service_tag]
        # singleton = service_module.singleton
        # assert service_tag == service_module.service_tag
        cur_cpu_load = self._avg_load.get_avg_cpu_by_period(_TTL_INTERVAL)
        data = urllib.parse.urlencode(
            {"ttl": _KEY_TIME_OUT, "value": cur_cpu_load})

        # todo: 处理 单例 的情况
        # if singleton:
        #     """
        #     如果是有状态的服务，那么在注册的时候需要确保etcd没有相关的服务信息的注册
        #     :todo 由于没有原子操作的保证，所以还是可能会出现多个的情况
        #     """
        #     while self.check_ok():
        #         try:
        #             now_url = self._get_server_info() + _SERVICE_DOMAIN + service_tag
        #             r = await AioApi.async_wrap(lambda: requests.request("GET", now_url, timeout=2))
        #             res = json.loads(r_text)
        #             self._fail_time = 0
        #             if res.get("action") == "get" and res.get("node", {}).get("nodes", []):
        #                 self.logger.info("service : %s not stateless, but exist, %s", service_tag, r_text)
        #                 return False
        #             break
        #         except:
        #             self._fail_time += 1
        #             self.logger.error("check singleton service : %s error", service_tag)
        #             self.logger.log_last_except()
        #             # if GameServerRepo.game_event_callback is not None:
        #             #     t, v, tb = sys.exc_info()
        #             #     GameServerRepo.game_event_callback.on_traceback(t, v, tb)

        while self.check_ok():
            try:
                now_url = self._get_url(service_tag)
                # if singleton:
                #     now_url = self._get_url(service_tag) + "?prevExist=false"
                # r = requests.request("PUT", now_url, data=data, headers=_HEADER, timeout=2)
                # self._logger.debug(f"_update_cpu_load: now_url: {now_url}")
                # r = await UtilApi.async_wrap(
                #     lambda: requests.request("PUT", now_url, data=data, headers=_HEADER, timeout=2))
                r_text, _ = await UtilApi.async_http_requests(
                    "PUT", now_url, data=data, headers=_HEADER, timeout=2, session=self._session)

                res = json.loads(r_text)
                if res.get("action") in ("create", "set"):
                    self._logger.debug(f"_update_cpu_load {self._server_name=}, {self._my_address}, {cur_cpu_load=}")
                    self._fail_time = 0
                    return True
                else:
                    self._fail_time += 1
                    self._logger.error(
                        "regist service : %s error: %s, fail time: %d", service_tag, r_text, self._fail_time)
            except:
                self._fail_time += 1
                self._logger.error("regist service : %s error", service_tag)
                self._logger.log_last_except()
                # if GameServerRepo.game_event_callback is not None:
                #     t, v, tb = sys.exc_info()
                #     GameServerRepo.game_event_callback.on_traceback(t, v, tb)
        return False

    async def _update_cpu_load_n_ttl(self, service_tag):
        """
        为每一个服务启动一个单独的协程来处理注册与ttl的刷新
        """
        if not await self._update_cpu_load(service_tag):
            self._logger.error("regist service %s fail", service_tag)
            return

        ttl_refresh_data = urllib.parse.urlencode(
            {
                # "value": self._avg_load.get_avg_cpu_by_period(_TTL_INTERVAL),
                "ttl": _KEY_TIME_OUT,
                "refresh": True,
                "prevExist": True
            })
        while self.check_ok():
            # self._stop_event.wait(random.randint(_TTL_INTERVAL, _TTL_INTERVAL + 10))
            await asyncio.sleep(
                random.randint(_TTL_INTERVAL, _TTL_INTERVAL + _TTL_INTERVAL_RAND_LATENCY))

            await self._update_cpu_load(service_tag)

            while self.check_ok():
                try:
                    now_url = self._get_url(service_tag)
                    # self._logger.info(f"outside, {time.time()=}, refresh service {service_tag}, now_url: {now_url}")

                    # def request_etcd_put():
                    #     self._logger.info(f"inside before, {time.time()=}, refresh service {service_tag}, now_url: {now_url}")
                    #     req_res = requests.request("PUT", now_url, data=ttl_refresh_data, headers=_HEADER, timeout=2)
                    #     self._logger.info(
                    #         f"inside after, {time.time()=}, refresh service {service_tag}, now_url: {now_url}")
                    #     return req_res
                    #
                    # r = await UtilApi.async_wrap(
                    #     # lambda: requests.request("PUT", now_url, data=ttl_refresh_data, headers=_HEADER, timeout=2))
                    #     request_etcd_put)  # TODO: sometime PUT ERROR

                    r_text, _ = await UtilApi.async_http_requests(
                        "PUT", now_url, data=ttl_refresh_data, headers=_HEADER, timeout=2, session=self._session)

                    res = json.loads(r_text)
                    if res.get("action", "") in ("set", "update"):
                        # self._logger.debug(
                        #   f"{time.time()=}, refresh service success: {service_tag}: {r_text}, now_url: {now_url}")
                        self._fail_time = 0
                        break
                    else:
                        self._fail_time += 1
                        self._logger.error(f"{time.time()=}, refresh service {service_tag} error: {r_text}, now_url: {now_url}")
                except:
                    self._fail_time += 1
                    self._logger.log_last_except()
                    # if GameServerRepo.game_event_callback is not None:
                    #     t, v, tb = sys.exc_info()
                    #     GameServerRepo.game_event_callback.on_traceback(t, v, tb)
        for _ in range(10):
            """在退出的时候，将注册的信息删除掉，如果删除失败，那就靠ttl自动删除吧"""
            try:
                self._logger.info(f"try delete service: {service_tag}")
                # r = await UtilApi.async_wrap(
                #     lambda: requests.request("DELETE", self._get_url(service_tag)))
                r_text, _ = await UtilApi.async_http_requests(
                    "DELETE", self._get_url(service_tag), session=self._session)
                res = json.loads(r_text)
                if res.get("action", "") == "delete":
                    self._logger.info("delete service : %s success", service_tag)
                    break
                else:
                    self._logger.error(
                        f"delete service {service_tag} error: {r_text}, now_url: {self._get_url(service_tag)}")
            except:
                self._logger.log_last_except()
                # if GameServerRepo.game_event_callback is not None:
                #     t, v, tb = sys.exc_info()
                #     GameServerRepo.game_event_callback.on_traceback(t, v, tb)

    def stop(self):
        if not self._stop:
            self._stop = True
            # self._stop_event.set()
            # for thread in self._threads:
            #     thread.join()
            # self._threads = []


class ServiceFinder(EtcdProcessor):
    def __init__(self, etcd_address_list):
        """
        通过watch机制监听etcd上注册的信息更改
        （1）通过recursive=true首先一次性全量拉etcd上所有数据，并保存当前etcd服务器的index
        （2）watch配合index做增量更新，并及时更改index
        :param etcd_address_list:    [(ip, port), ()....]
        """
        EtcdProcessor.__init__(self, etcd_address_list)
        self._logger = LogManager.get_logger("ServiceFinder")
        self._server_len = len(self._etcd_address_list) - 1
        self._tag_to_addr_2_load = \
            collections.defaultdict(  # just like: _tag_to_addr_2_load[service_tag][address] = (server_name, cpu_load)
                dict)  # type: typing.DefaultDict[str, typing.Dict[typing.Tuple[str, int], typing.Tuple[str, float]]]
        self._es = dict()
        self._etcd_index = 0  # 最近一次get获取的etcd服务器所处的index
        self._watch_index = 0  # 当前watch所在的index

    async def start(self, session: aiohttp.ClientSession):
        self._session = session
        # await asyncio.sleep(5)
        if await self._init_info():
            self._logger.info(
                "init service info from etcd success, will going to watch, at etcd index -> %s", self._etcd_index)
            # gevent.spawn(self._watch_process)
            # await asyncio.create_task(self._watch_process())
            await self._watch_process()
        else:
            self.stop()
            raise Exception("init service info from etcd fail")

    @staticmethod
    def _get_node_name(prefix, key_path):
        if prefix in key_path:
            return key_path[len(prefix):]
        raise Exception("what")

    def _add_service_info(self, service_tag, server_name, address: typing.Tuple[str, int], cpu_load: Union[float, str]):
        """
        可能会存在某个服务进程异常退出了，之后重启，之前的注册信息还没有失效，那么存在重复的情况
        """
        # if service_tag not in self._tag_to_addr_2_load:
        #     self._tag_to_addr_2_load[service_tag] = []
        # if address in self._tag_to_addr_2_load[service_tag]:
        #     self._logger.info("duplicate service info  -> %s:%s", service_tag, address)
        #     return
        # self._tag_to_addr_2_load[service_tag].add(address)
        self._tag_to_addr_2_load[service_tag][address] = (server_name, cpu_load)
        # self._logger.debug(f'{service_tag=}, {server_name=}, {address=}, {cpu_load=}')

    def _delete_service_info(self, service_tag, address: typing.Tuple[str, int]):
        try:
            self._tag_to_addr_2_load[service_tag].pop(address, None)
        except:
            self._logger.log_last_except()
            # if GameServerRepo.game_event_callback is not None:
            #     t, v, tb = sys.exc_info()
            #     GameServerRepo.game_event_callback.on_traceback(t, v, tb)

    def _init_node_info(self, nodes):
        """
        从服务节点列表中初始化服务信息
        """
        for node in nodes:
            # node just like:
            # `{'key': '/services/dispatcher_service', 'dir': True, 'modifiedIndex': 4, 'createdIndex': 4}`
            service_tag, service_nodes = self._get_node_name("/services/", node["key"]), node.get("nodes", [])
            prefix = node["key"] + "/"
            for service_node in service_nodes:
                address_str = self._get_node_name(prefix, service_node["key"])
                server_name, ip, port = \
                    str(address_str.split("|")[0]), str(address_str.split("|")[1]), int(address_str.split("|")[2])
                self._add_service_info(service_tag, server_name, (ip, port), float(service_node["value"]))
                self._logger.info("add service node info--> %s: %s", service_tag, (ip, port))

    def _process_add_entity_info(self, node):
        value = node.get("value")
        if value:
            entity_info = json.loads(value)
            entity_info["ip"] = str(entity_info["ip"])
            entity_info["key"] = node["key"]
            self._logger.info("set entity info -> %s", entity_info)
            self._es[entity_info["name"]] = entity_info
        elif "nodes" in node:
            for item in node.get("nodes", []):
                self._process_add_entity_info(item)

    def _process_delete_enttiy_info(self, key):
        entity_name = key[len(_NAME_ENTITY_PREFIX):]
        try:
            self._logger.info("delete enttiy info -> %s", self._es[entity_name])
            del self._es[entity_name]
        except:
            pass

    async def _init_info(self):
        """
        从etcd服务器一次性拉取所有的注册信息来初始化
        """
        while self.check_ok():
            try:
                now_url = self._get_server_info() + _ETCD_KEY_PREFIX + "?recursive=true"
                self._logger.info(f"_init_info, now_url: {now_url}")
                # r = await UtilApi.async_wrap(lambda: requests.request("GET", now_url))
                r_text, r_headers = await UtilApi.async_http_requests("GET", now_url, session=self._session)
                res = json.loads(r_text)
                self._fail_time = 0
                self._etcd_index = int(r_headers["x-etcd-index"])
                self._watch_index = self._etcd_index
                self._tag_to_addr_2_load = collections.defaultdict(dict)  # service的信息重置
                self._es = dict()            # MobileServer注册的Entity信息重置
                root_nodes = res.get("node", {}).get("nodes", [])
                for node in root_nodes:
                    if _SERVICE_PREFIX.startswith(node.get("key")):        # service的相关注册信息
                        self._init_node_info(node.get("nodes", []))
                    elif node.get("key", "") == _NAME_ENTITY_PREFIX[:-1]:         # mobileserver注册的entity
                        self._process_add_entity_info(node)
                return True
            except:
                self._fail_time += 1
                self._logger.log_last_except()
                # if GameServerRepo.game_event_callback is not None:
                #     t, v, tb = sys.exc_info()
                #     GameServerRepo.game_event_callback.on_traceback(t, v, tb)

    def _form_etcd_url(self, address_list):
        out = []
        for address in address_list:
            ip, port = address[0], address[1]
            out.append("http://%s:%s" % (ip, port))
        return out

    @staticmethod
    def _get_service_node_info(key_path):
        """
        /services/fjs/192.168.82.177|8080
        """
        key_path = key_path[len(_SERVICE_PREFIX):]
        service_tag, address_str = key_path.split("/")[0], key_path.split("/")[1]
        server_name, ip, port = \
            str(address_str.split("|")[0]), str(address_str.split("|")[1]), int(address_str.split("|")[2])
        # ip, port = str(address_str.split("|")[0]), int(address_str.split("|")[1])
        return service_tag, server_name, (ip, port)

    async def _watch_process_impl(self):
        # self.logger.info(f"_watch_process_impl1, now_url: {self._get_server_info() + _ETCD_KEY_PREFIX}")

        # r = await UtilApi.async_wrap(lambda: requests.request("GET", self._get_server_info() + _ETCD_KEY_PREFIX))
        _, r_headers = await UtilApi.async_http_requests(
            "GET", self._get_server_info() + _ETCD_KEY_PREFIX, session=self._session)
        self._etcd_index = int(r_headers["x-etcd-index"])
        now_url = self._get_server_info() + _ETCD_KEY_PREFIX + "?wait=true&recursive=true"
        if self._watch_index:
            now_url += "&waitIndex=" + str(self._watch_index + 1)
            # self.logger.info(f"waiting _watch_process_impl2 {now_url}")
        # self._logger.info("bbb ttt test wait longpoll")  # TODO: DEL

        # r = await UtilApi.async_wrap(lambda: requests.request("GET", now_url))
        r_text, _ = await UtilApi.async_http_requests("GET", now_url, session=self._session)

        res = json.loads(r_text)

        action = res.get("action", "")
        key_path = res.get("node", {}).get("key", "")
        cpu_load = float(res.get("node", {}).get("value", "0.0"))  # type: float
        etcd_modify_index = res.get("node", {}).get("modifiedIndex", 0)
        if etcd_modify_index:
            self._watch_index = etcd_modify_index
        # return  # TODO: DEL no

        if action in ("create", "set"):
            """key的创建"""
            if key_path.startswith(_SERVICE_PREFIX):
                # return  # TODO: DEL no
                server_tag, service_name, address = self._get_service_node_info(key_path)
                self._logger.debug(
                    f"recv watch {action=} service node info--> {service_name=}: {address=} - {cpu_load=}")
                self._add_service_info(server_tag, service_name, address, cpu_load)
            elif key_path.startswith(_NAME_ENTITY_PREFIX):
                self._logger.info("recv watch %s entity info -> %s", action, res)
                self._process_add_entity_info(res.get("node", {}))
        elif action in ("expire", "delete"):
            """key超时销毁或删除"""
            if key_path.startswith(_SERVICE_PREFIX):
                server_tag, service_name, address = self._get_service_node_info(key_path)
                self._logger.info(
                    "recv watch %s, info--> %s: %s - [%s]", action, service_name, address, self._watch_index)
                self._delete_service_info(server_tag, address)
            elif key_path.startswith(_NAME_ENTITY_PREFIX):
                self._logger.info("recv watch %s entity info -> %s", action, res)
                self._process_delete_enttiy_info(key_path)
        elif res.get("errorCode", 0) == 401:
            """
            当前etcd的watch index已经失效了，也就是index已经落后当前etcd服务器的index超过了1000次更改
            由于现在是全量更新，所以这种情况出现的可能性应该很小很小，除非同时启动或者停止大量的服务，不过如果
            出现了这种情况，还是重置一下数据吧
            """
            self._logger.info("recv watch index out of date, %s", r_text)
            if not await self._init_info():
                self._logger.error("reset etcd data error, will stop")
                return
        else:
            self._logger.error("recv unknown watch info, %s", r_text)
        self._fail_time = 0

    async def _watch_process(self):
        """
        在一个单独的协程中执行watch的操作，这里watch的路径是/v2/keys，表示跟踪所有数据的变动，做全量的数据的监听
        根据watch收到的数据类型，包括set，create，delete，expire做数据的增量更新即可

        @note: 因为ttl的刷新也会引起etcd服务器index的更新，为了避免因为ttl的刷新导致watch的index失效，每次在watch先get一次
        /v2/keys，获取当前etcd服务器的index状态，如果本次watch超时了，那么直接将watch的index更新之前get获取时候的
        x-etcd-index值即可，这样可以减少因为ttl的刷新导致watch index失效带来的数据全量更新
        """
        while self.check_ok():
            # now_timeout = gevent.Timeout(random.randint(_WATCH_TIMEOUT, _WATCH_TIMEOUT + 10))
            # now_timeout.start()
            try:
                await asyncio.wait_for(
                    self._watch_process_impl(),
                    random.randint(_WATCH_TIMEOUT, _WATCH_TIMEOUT + 10))
                    # _WATCH_TIMEOUT)  # TODO: revert
            # except gevent.Timeout, e:
            except asyncio.TimeoutError:
                # if e is now_timeout:
                    # watch的timeout，将watch的index更新到上一次get更新的是时候etcd服务器的index
                self._fail_time = 0
                self._logger.info("etcd watch timeout, rest watch index %s -> %s", self._watch_index, self._etcd_index)
                self._watch_index = self._etcd_index
                # else:
                #     raise
            except:
                self._fail_time += 1
                self._logger.log_last_except()
                # if GameServerRepo.game_event_callback is not None:
                #     t, v, tb = sys.exc_info()
                #     GameServerRepo.game_event_callback.on_traceback(t, v, tb)
            # finally:
            #     now_timeout.cancel()

    def get_lowest_load_service(self, service_tag) -> Optional[typing.Tuple[str, str, int]]:
        """
        :param service_tag:
        :return: 一个元组 (server_name, ip, port)
        """
        _addr_2_load = self._tag_to_addr_2_load.get(service_tag, None)
        if _addr_2_load is None:
            return None

        _addr_tuple = min(_addr_2_load, key=lambda k: _addr_2_load[k][1])
        server_name, _ = _addr_2_load[_addr_tuple]
        return server_name, _addr_tuple[0], _addr_tuple[1]

    def get_entity_info(self, entity_name):
        """
        根据entity的名字获取其注册信息
        """
        return self._es.get(entity_name)

    def get_all_entity_info(self):
        return dict(self._es)


class ServiceNode(object):
    """
    对于一个service节点，既需要进行服务的注册，也同时需要服务的发现
    """
    def __init__(self, etcd_address_list, my_address, etcd_tag, server_name):
        self._logger = LogManager.get_logger("ServiceNode")
        self._register = ServiceRegister(etcd_address_list, my_address, etcd_tag, server_name)
        self._logger.info("we have create ServiceRegister")
        # gevent.sleep(5)
        self._finder = ServiceFinder(etcd_address_list)
        self._logger.info("we have create ServiceFinder")

    async def start(self):
        async with aiohttp.ClientSession() as session:
            # html = await fetch(session, 'http://python.org')
            register_task = gv.get_ev_loop().create_task(self._register.start(session))
            await asyncio.sleep(3)
            finder_task = gv.get_ev_loop().create_task(self._finder.start(session))
            await register_task
            await finder_task

    def stop(self):
        self._register.stop()
        self._finder.stop()

    def get_lowest_load_service_addr(self, service_tag):
        return self._finder.get_lowest_load_service(service_tag)

    def get_entity_info(self, entity_name):
        return self._finder.get_entity_info(entity_name)
