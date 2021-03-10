import asyncio
import functools
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
from core.mobilelog.LogManager import LogManager
from util.SingletonEntityManager import SingletonEntityManager

TCP_SERVER = None


class TcpServer(object):

    def __init__(self):
        self.tcp_conn_map = {}
        self._etcd_service_node = None

        self._logger = LogManager.get_logger('TcpServer')
        self.register_entities()

    def parse_json_conf(self):


    def register_entities(self):
        """
        推荐使用entity_package的形式来自动搜索注册entity类型，通过entity_package配置项指定entity所在，这种方式经过迭代
        后不会污染全局的path
        @note entity_package指定的package，必须是可以直接import成功的
        """
        gameconfig = self.config[self.config_sections.game]
        entity_root = gameconfig.get('entity_root')
        if entity_root:
            entity_root = str(entity_root)
            # store for further usage
            Netease.entity_root = entity_root
            entity_classes = EntityScanner.scan_entity_package(entity_root, ServerEntity.ServerEntity)
        else:
            entity_path = str(gameconfig["entity_path"])
            # store for further usage
            Netease.entity_path = entity_path
            entity_classes = EntityScanner.scan_entity_classes(entity_path, ServerEntity.ServerEntity)
        entity_classes = entity_classes.items()
        # 这里进行排序是为了保证register_entity函数中注册md5的index时是按照一致的顺序
        entity_classes.sort(lambda a, b: cmp(a[0], b[0]))
        for etype, claz in entity_classes:
            self.logger.info(" Entity %s registered with class %s", etype, claz)
            EntityFactory.instance().register_entity(etype, claz)
        # register classes in ServerEntity module
        cls_list = EntityScanner._get_class_list(ServerEntity, ServerEntity.ServerEntity)
        # 这里进行排序是为了保证register_entity函数中注册md5的index时是按照一致的顺序
        cls_list.sort(lambda a, b: cmp(a.__name__, b.__name__))
        for cls in cls_list:
            EntityFactory.instance().register_entity(cls.__name__, cls)

        self.register_local_entity()
        self.register_component()
        RpcIndexer.calculate_recv_rpc_salt()

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
        _pbe = PuppetBindEntity()
        _tcp_conn.set_entity(_pbe)
        _pbe.set_connection(_tcp_conn)
        _pbe.set_puppet(_ppt)
        _ppt.set_puppet_bind_entity(_pbe)
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
