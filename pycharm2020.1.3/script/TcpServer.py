import asyncio
import functools
import json
import signal
import platform
from random import random
import socket

from PuppetBindEntity import PuppetBindEntity
from battle_entity.Puppet import Puppet
# from core.common import MsgpackSupport
from TcpConn import TcpConn
from common import gr
from core.EtcdSupport import ServiceNode
from core.common import EntityScanner
from core.common.EntityFactory import EntityFactory
from core.mobilelog.LogManager import LogManager
from util.SingletonEntityManager import SingletonEntityManager

TCP_SERVER = None


class TcpServer(object):

    def __init__(self):
        self.tcp_conn_map = {}
        self._etcd_service_node = None

        self._logger = LogManager.get_logger('TcpServer')
        # self.register_entities()
        self._config = self.parse_json_conf()

        self.register_battle_entities()
        self.register_component()

    @staticmethod
    def parse_json_conf():
        file_name = '../bin/win/conf/battle_server.json'
        conf_file = open(file_name)
        json_conf = json.load(conf_file)
        conf_file.close()
        gr.game_json_conf = json_conf
        return json_conf

    def register_component(self):
        from common.component.Component import Component
        from common.component import ComponentRegister
        # gameconfig = self.config[self.config_sections.game]
        component_root = self._config.get('component_root')
        if component_root is None:
            self._logger.error('conf file has no component_root!')
            return
        component_classes = EntityScanner.scan_entity_package(component_root, Component)
        component_classes = component_classes.items()
        # component_classes.sort(lambda a, b: cmp(a[0], b[0]))
        for comp_type, comp_cls in component_classes:
            ComponentRegister.register(comp_cls)

    def register_battle_entities(self):
        from BattleEntity import BattleEntity
        _ber = self._config.get('battle_entity_root', None)
        if _ber is None:
            self._logger.error('conf file has no battle_entity_root!')
            return
        entity_classes = EntityScanner.scan_entity_package(_ber, BattleEntity)
        entity_classes = entity_classes.items()

        # def cmp(x, y):
        #     if x < y:
        #         return -1
        #     elif x == y:
        #         return 0
        #     else:
        #         return 1
        #
        # entity_classes.sort(lambda a, b: cmp(a[0], b[0]))
        for cls_name, cls in entity_classes:
            EntityFactory.instance().register_entity(cls_name, cls)

    def forward(self, addr, message):
        for _addr, _tcp_conn in self.tcp_conn_map.items():
            # if w != writer:
                # w.write(f"{addr!r}: {message!r}\n".encode())
                # w.write(MsgpackSupport.encode(f"{addr!r}: {message!r}\n"))
            _tcp_conn.send_msg(f"{addr!r}: {message!r}\n")

    async def handle_client_connected(self, reader, writer):
        # self.writers.append(writer)
        addr = writer.get_extra_info('peername')
        _tcp_conn = TcpConn(addr, writer, reader)

        _ppt = Puppet()

        # _tcp_conn.set_entity(_ppt)
        # _pbe = PuppetBindEntity()
        # _tcp_conn.set_entity(_pbe)
        _tcp_conn.set_entity(_ppt)
        _ppt.set_connection(_tcp_conn)
        # _pbe.set_connection(_tcp_conn)
        # _pbe.set_puppet(_ppt)
        # _ppt.set_puppet_bind_entity(_pbe)
        _ppt.init_from_dict({})

        self.tcp_conn_map[addr] = _tcp_conn
        message = f"{addr!r} is connected !!!!"
        print(message)
        await _tcp_conn.loop()
        # self.forward(writer, addr, message)
        # while True:
        #     data = await reader.read(100)
        #     # message = data.decode().strip()
        #     message = MsgpackSupport.decode(data)
        #     self.forward(writer, addr, message)
        #     await writer.drain()
        #     if message == "exit":
        #         message = f"{addr!r} wants to close the connection."
        #         print(message)
        #         self.forward(writer, "Server", message)
        #         break
        # self.writers.remove(writer)
        # writer.close()

    async def start_server_task(self):
        # server = await asyncio.start_server(
        #     handle_echo, '127.0.0.1', 8888)
        server = await asyncio.start_server(self.handle_client_connected, '127.0.0.1', 8888)
        # _start_srv_task = asyncio.create_task(asyncio.start_server(self.handle_client_connected, '127.0.0.1', 8888))
        # await _etcd_support_task
        # server = await _start_srv_task

        addr = server.sockets[0].getsockname()
        print(f'Server on {addr}')

        async with server:
            await server.serve_forever()

    async def main(self):

        self.handle_sig()

        etcd_addr_list = [('127.0.0.1', '2379'),]
        my_addr = ('127.0.0.1', '12001')
        self._etcd_service_node = ServiceNode(
            etcd_addr_list, my_addr, {}
        )
        gr.etcd_service_node = self._etcd_service_node
        asyncio.get_running_loop().call_later(4, self._check_game_start)

        _etcd_support_task = asyncio.create_task(self._etcd_service_node.start())
        _start_srv_task = asyncio.create_task(self.start_server_task())
        # await self._etcd_service_node.start()
        # await self.start_server_task()

        # server = await asyncio.start_server(
        #     handle_echo, '127.0.0.1', 8888)
        # server = await asyncio.start_server(self.handle_client_connected, '127.0.0.1', 8888)
        # _start_srv_task = asyncio.create_task(asyncio.start_server(self.handle_client_connected, '127.0.0.1', 8888))
        # await _start_srv_task

        await _etcd_support_task
        await _start_srv_task
        # server = await _start_srv_task

        # addr = server.sockets[0].getsockname()
        # print(f'Server on {addr}')
        #
        # async with server:
        #     await server.serve_forever()

    @staticmethod
    def handle_sig():

        def ask_exit(sig_name, loop):
            print('got signal %s: exit' % sig_name)
            loop.stop()

        if platform.system() != 'Linux':
            return
        _loop = asyncio.get_running_loop()
        for _sig_name in {'SIGINT', 'SIGTERM'}:
            _loop.add_signal_handler(
                getattr(signal, _sig_name), functools.partial(ask_exit, _sig_name, _loop))

    def run(self):
        asyncio.run(self.main())
        # ev_loop.run_until_complete(self.main())

    def _check_game_start(self):
        # 随机种子
        # random.seed()
        # 得到本机ip
        try:
            gr.local_ip = socket.gethostbyname(socket.gethostname())
        except socket.gaierror:
            gr.local_ip = '127.0.0.1'
        # self._register_singleton()

    # def _register_singleton(self):
        # 创建各种server/cluster singleton
        SingletonEntityManager.instance().register_centers_and_stubs(
            gr.game_server_name,
            lambda flag: self._register_centers_and_stubs_cb(flag))

    def _register_centers_and_stubs_cb(self, flag):
        pass


if __name__ == '__main__':
    tcp_server = TcpServer()
    TCP_SERVER = tcp_server
    tcp_server.run()

    # loop = asyncio.get_event_loop()
    # loop.run_until_complete(main())
