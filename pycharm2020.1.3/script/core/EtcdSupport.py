# -*- coding: utf-8 -*-
import collections

import requests
import asyncio

import typing

from core.util import UtilApi
# from ..distserver.game import GameServerRepo
from common import gv
from core.mobilelog.LogManager import LogManager
import urllib.parse
import json
# import gevent
# import gevent.event
import random

_WATCH_TIMEOUT = 60                             # watch执行的超时时间
_GET_TIMEOUT = 5                                # 获取节点值或者refresh ttl的超时
_MAX_FAIL_TIME = 10                             # get 或者 regist, refresh ttl 的时候最多的失败尝试次数


_KEY_TIME_OUT = 180                             # key超时时间
_TTL_INTERVAL = 100                             # ttl refresh间隔时间
_TTL_RETRY = 2                                  # ttl刷新失败之后重新尝试的延迟间隔


_ETCD_KEY_PREFIX = "/v2/keys"
_NAME_ENTITY_DOMAIN = "/v2/keys/nameentity/"    # entity的名字与信息将会注册到这个目录下
_TAG_DOMAIN = "/v2/keys/tags/"                  # 注册tag的目录

_NAME_ENTITY_PREFIX = "/nameentity/"

_MOBILE_SERVICE_PREFIX = "/services/"
_MOBILE_SERVICE_DOMAIN = "/v2/keys" + _MOBILE_SERVICE_PREFIX


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
    def __init__(self, etcd_address_list, my_address, service_tag):
        EtcdProcessor.__init__(self, etcd_address_list)
        self._logger = LogManager.get_logger()
        self._my_address = my_address                                            # 当前节点的监听地址(ip, port)
        self._address_str = my_address[0] + "|" + str(my_address[1])             # 当前节点的监听地址的字符串描述
        self._service_tag = service_tag                          # 服务模块的name->module字典
        # self._stop_event = gevent.event.Event()
        self._service_regist_status = dict()
        self._threads = []
        # self._die_event = gevent.event.Event()

        # for name in self._service_module_dict.keys():
            # self._threads.append(gevent.spawn(self._process_regist_and_ttl, name))
        self._threads.append(self._process_regist_and_ttl(self._service_tag))
            # self._threads.append(asyncio.create_task(self._process_regist_and_ttl(name)))

        # gevent.spawn(self._dead_check_process)
        # await asyncio.gather(*self._threads)

    # async def _dead_check_process(self):
    async def start(self):
        # for thread in self._threads:
        #     thread.join()
        # self._die_event.set()
        await asyncio.gather(*self._threads)

    # def wait_dead(self, timeout):
    #     return self._die_event.wait(timeout=timeout)

    def _get_url(self, service_name):
        path = self._get_server_info() + _MOBILE_SERVICE_DOMAIN + service_name + "/" + self._address_str
        return path

    def _do_unregist(self, service_name):
        pass

    async def _do_regist(self, service_name):
        """
        注册service，如果注册成功，返回True
        """
        # service_module = self._service_module_dict[service_name]
        # singleton = service_module.singleton
        # assert service_name == service_module.service_name
        data = urllib.parse.urlencode({"ttl": _KEY_TIME_OUT})

        # if singleton:
        #     """
        #     如果是有状态的服务，那么在注册的时候需要确保etcd没有相关的服务信息的注册
        #     :todo 由于没有原子操作的保证，所以还是可能会出现多个的情况
        #     """
        #     while self.check_ok():
        #         try:
        #             now_url = self._get_server_info() + _MOBILE_SERVICE_DOMAIN + service_name
        #             r = await AioApi.async_wrap(lambda: requests.request("GET", now_url, timeout=2))
        #             res = json.loads(r.text)
        #             self._fail_time = 0
        #             if res.get("action") == "get" and res.get("node", {}).get("nodes", []):
        #                 self.logger.debug("service : %s not stateless, but exist, %s", service_name, r.text)
        #                 return False
        #             break
        #         except:
        #             self._fail_time += 1
        #             self.logger.error("check singleton service : %s error", service_name)
        #             self.logger.log_last_except()
        #             # if GameServerRepo.game_event_callback is not None:
        #             #     t, v, tb = sys.exc_info()
        #             #     GameServerRepo.game_event_callback.on_traceback(t, v, tb)

        while self.check_ok():
            try:
                now_url = self._get_url(service_name)
                # if singleton:
                #     now_url = self._get_url(service_name) + "?prevExist=false"
                # r = requests.request("PUT", now_url, data=data, headers=_HEADER, timeout=2)
                self._logger.debug(f"_do_regist: now_url: {now_url}")
                r = await UtilApi.async_wrap(
                    lambda: requests.request("PUT", now_url, data=data, headers=_HEADER, timeout=2))
                res = json.loads(r.text)
                if res.get("action") in ("create", "set"):
                    self._logger.debug("regist service : %s success, %s", service_name, r.text)
                    self._fail_time = 0
                    return True
                else:
                    self._fail_time += 1
                    self._logger.error(
                        "regist service : %s error: %s, fail time: %d", service_name, r.text, self._fail_time)
            except:
                self._fail_time += 1
                self._logger.error("regist service : %s error", service_name)
                self._logger.log_last_except()
                # if GameServerRepo.game_event_callback is not None:
                #     t, v, tb = sys.exc_info()
                #     GameServerRepo.game_event_callback.on_traceback(t, v, tb)
        return False

    async def _process_regist_and_ttl(self, service_name):
        """
        为每一个服务启动一个单独的协程来处理注册与ttl的刷新
        """
        if not await self._do_regist(service_name):
            self._logger.error("regist service %s fail", service_name)
            return

        ttl_refresh_data = urllib.parse.urlencode({"ttl": _KEY_TIME_OUT, "refresh": True})
        while self.check_ok():
            # self._stop_event.wait(random.randint(_TTL_INTERVAL, _TTL_INTERVAL + 10))
            await asyncio.sleep(random.randint(_TTL_INTERVAL, _TTL_INTERVAL + 10))
            while self.check_ok():
                try:
                    now_url = self._get_url(service_name)
                    self._logger.debug(f"refresh service {service_name}, now_url: {now_url}")
                    r = await UtilApi.async_wrap(
                        lambda: requests.request("PUT", now_url, data=ttl_refresh_data, headers=_HEADER, timeout=2))
                    res = json.loads(r.text)
                    if res.get("action", "") == "set":
                        self._logger.debug(f"refresh service success: {service_name}: {r.text}, now_url: {now_url}")
                        self._fail_time = 0
                        break
                    else:
                        self._fail_time += 1
                        self._logger.error(f"refresh service {service_name} error: {r.text}, now_url: {now_url}")
                except:
                    self._fail_time += 1
                    self._logger.log_last_except()
                    # if GameServerRepo.game_event_callback is not None:
                    #     t, v, tb = sys.exc_info()
                    #     GameServerRepo.game_event_callback.on_traceback(t, v, tb)
        for _ in range(10):
            """在退出的时候，将注册的信息删除掉，如果删除失败，那就靠ttl自动删除吧"""
            try:
                self._logger.debug(f"try delete service: {service_name}")
                r = await UtilApi.async_wrap(
                    lambda: requests.request("DELETE", self._get_url(service_name)))
                res = json.loads(r.text)
                if res.get("action", "") == "delete":
                    self._logger.debug("delete service : %s success", service_name)
                    break
                else:
                    self._logger.error(
                        f"delete service {service_name} error: {r.text}, now_url: {self._get_url(service_name)}")
            except:
                self._logger.log_last_except()
                # if GameServerRepo.game_event_callback is not None:
                #     t, v, tb = sys.exc_info()
                #     GameServerRepo.game_event_callback.on_traceback(t, v, tb)

    def stop(self):
        if not self._stop:
            self._stop = True
            # self._stop_event.set()
            for thread in self._threads:
                thread.join()
            self._threads = []


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
        self._services = \
            collections.defaultdict(set)  # type: typing.DefaultDict[str, typing.Set[typing.Tuple[str, int]]]
        self._es = dict()
        self._etcd_index = 0  # 最近一次get获取的etcd服务器所处的index
        self._watch_index = 0  # 当前watch所在的index

    async def start(self):
        # await asyncio.sleep(5)
        if await self._init_info():
            self._logger.debug(
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

    def _add_service_info(self, service_name, address):
        """
        可能会存在某个服务进程异常退出了，之后重启，之前的注册信息还没有失效，那么存在重复的情况
        """
        # if service_name not in self._services:
        #     self._services[service_name] = []
        if address in self._services[service_name]:
            self._logger.debug("duplicate service info  -> %s:%s", service_name, address)
            return
        self._services[service_name].add(address)

    def _delete_service_info(self, service_name, address):
        try:
            self._services[service_name].discard(address)
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
            service_name, service_nodes = self._get_node_name("/services/", node["key"]), node.get("nodes", [])
            prefix = node["key"] + "/"
            for service_node in service_nodes:
                address_str = self._get_node_name(prefix, service_node["key"])
                ip, port = str(address_str.split("|")[0]), int(address_str.split("|")[1])
                self._add_service_info(service_name, (ip, port))
                self._logger.debug("add service node info--> %s: %s", service_name, (ip, port))

    def _process_add_entity_info(self, node):
        value = node.get("value")
        if value:
            entity_info = json.loads(value)
            entity_info["ip"] = str(entity_info["ip"])
            entity_info["key"] = node["key"]
            self._logger.debug("set entity info -> %s", entity_info)
            self._es[entity_info["name"]] = entity_info
        elif "nodes" in node:
            for item in node.get("nodes", []):
                self._process_add_entity_info(item)

    def _process_delete_enttiy_info(self, key):
        entity_name = key[len(_NAME_ENTITY_PREFIX):]
        try:
            self._logger.debug("delete enttiy info -> %s", self._es[entity_name])
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
                self._logger.debug(f"_init_info, now_url: {now_url}")
                r = await UtilApi.async_wrap(lambda: requests.request("GET", now_url))
                res = json.loads(r.text)
                self._fail_time = 0
                self._etcd_index = int(r.headers["x-etcd-index"])
                self._watch_index = self._etcd_index
                self._services = collections.defaultdict(set)  # service的信息重置
                self._es = dict()            # MobileServer注册的Entity信息重置
                root_nodes = res.get("node", {}).get("nodes", [])
                for node in root_nodes:
                    if _MOBILE_SERVICE_PREFIX.startswith(node.get("key")):        # service的相关注册信息
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

    def _get_service_node_info(self, key_path):
        """
        /services/fjs/192.168.82.177|8080
        """
        key_path = key_path[len(_MOBILE_SERVICE_PREFIX):]
        service_name, address_str = key_path.split("/")[0], key_path.split("/")[1]
        ip, port = str(address_str.split("|")[0]), int(address_str.split("|")[1])
        return service_name, (ip, port)

    async def _watch_process_impl(self):
        # self.logger.debug(f"_watch_process_impl1, now_url: {self._get_server_info() + _ETCD_KEY_PREFIX}")

        r = await UtilApi.async_wrap(lambda: requests.request("GET", self._get_server_info() + _ETCD_KEY_PREFIX))
        self._etcd_index = int(r.headers["x-etcd-index"])
        now_url = self._get_server_info() + _ETCD_KEY_PREFIX + "?wait=true&recursive=true"
        if self._watch_index:
            now_url += "&waitIndex=" + str(self._watch_index + 1)
            # self.logger.debug(f"waiting _watch_process_impl2 {now_url}")

        r = await UtilApi.async_wrap(lambda: requests.request("GET", now_url))
        res = json.loads(r.text)
        action = res.get("action", "")
        key_path = res.get("node", {}).get("key", "")
        etcd_modify_index = res.get("node", {}).get("modifiedIndex", 0)
        if etcd_modify_index:
            self._watch_index = etcd_modify_index
        if action in ("create", "set"):
            """key的创建"""
            if key_path.startswith(_MOBILE_SERVICE_PREFIX):
                service_name, address = self._get_service_node_info(key_path)
                self._logger.debug(
                    "recv watch %s service node info--> %s: %s - [%s]",
                    action, service_name, address, self._watch_index)
                self._add_service_info(service_name, address)
            elif key_path.startswith(_NAME_ENTITY_PREFIX):
                self._logger.debug("recv watch %s entity info -> %s", action, res)
                self._process_add_entity_info(res.get("node", {}))
        elif action in ("expire", "delete"):
            """key超时销毁或删除"""
            if key_path.startswith(_MOBILE_SERVICE_PREFIX):
                service_name, address = self._get_service_node_info(key_path)
                self._logger.debug(
                    "recv watch %s, info--> %s: %s - [%s]", action, service_name, address, self._watch_index)
                self._delete_service_info(service_name, address)
            elif key_path.startswith(_NAME_ENTITY_PREFIX):
                self._logger.debug("recv watch %s entity info -> %s", action, res)
                self._process_delete_enttiy_info(key_path)
        elif res.get("errorCode", 0) == 401:
            """
            当前etcd的watch index已经失效了，也就是index已经落后当前etcd服务器的index超过了1000次更改
            由于现在是全量更新，所以这种情况出现的可能性应该很小很小，除非同时启动或者停止大量的服务，不过如果
            出现了这种情况，还是重置一下数据吧
            """
            self._logger.debug("recv watch index out of date, %s", r.text)
            if not await self._init_info():
                self._logger.error("reset etcd data error, will stop")
                return
        else:
            self._logger.error("recv unknown watch info, %s", r.text)
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
            # except gevent.Timeout, e:
            except asyncio.TimeoutError:
                # if e is now_timeout:
                    # watch的timeout，将watch的index更新到上一次get更新的是时候etcd服务器的index
                self._fail_time = 0
                self._logger.debug("etcd watch timeout, rest watch index %s -> %s", self._watch_index, self._etcd_index)
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

    def get_service_info(self, service_name) -> typing.Tuple[str, int]:
        # """
        # 获取service_name的服务地址列表，如果没有信息的话，返回的是一个空的列表，这里是返回的一个随机的list副本,
        # 用于实现一个简单的负载均衡
        #
        # :todo 本身这里是应该有很多策略和控制的，类似于更具延迟，吞吐来返回一个优先权的排序列表
        # """
        # if service_name != 'all_service':
        #     out_list = list(self._services.get(service_name, []))
        # else:
        #     out_list = []
        #     for info_list in self._services.values():
        #         out_list.extend(info_list)
        # random.shuffle(out_list)
        # return out_list
        return random.choice(tuple(self._services.get(service_name, {None})) or (None, ))

    def get_entity_info(self, entity_name):
        """
        根据entity的名字获取其注册信息
        """
        return self._es.get(entity_name)

    def get_all_entity_info(self):
        return dict(self._es)

    # def get_service_map(self):
    #     service_dict = {}
    #     for service_name, service_infos in self._services.iteritems():
    #         for service_info in service_infos:
    #             service_dict[service_info] = service_name
    #
    #     return service_dict


class ServiceNode(object):
    """
    对于一个service节点，既需要进行服务的注册，也同时需要服务的发现
    """
    def __init__(self, etcd_address_list, my_address, service_module_dict):
        self._logger = LogManager.get_logger("ServiceNode")
        self._register = ServiceRegister(etcd_address_list, my_address, service_module_dict)
        self._logger.debug("we have create ServiceRegister, wait 5 seconds")
        # gevent.sleep(5)
        self._finder = ServiceFinder(etcd_address_list)
        self._logger.debug("we have create ServiceFinder")

    async def start(self):
        # await self._register.start()
        # await asyncio.sleep(5)
        # await self._finder.start()

        register_task = asyncio.create_task(self._register.start())
        finder_task = asyncio.create_task(self._finder.start())
        await register_task
        # await asyncio.sleep(5)
        await finder_task

    def stop(self):
        self._register.stop()
        self._finder.stop()

    # def wait_dead(self, timeout):
    #     return self._register.wait_dead(timeout)

    def get_service_info(self, service_name):
        return self._finder.get_service_info(service_name)

    def get_entity_info(self, entity_name):
        return self._finder.get_entity_info(entity_name)
