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
from util.SingletonEntityManager import SingletonEntityManager

TCP_SERVER = None


class TcpServer(object):

    def __init__(self):
        self.tcp_conn_map = {}
        # self.writers = []

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

    async def main(self):

        self.handle_sig()

        asyncio.get_event_loop().call_later(4, self._check_game_start)

        # server = await asyncio.start_server(
        #     handle_echo, '127.0.0.1', 8888)
        server = await asyncio.start_server(
            self.handle_client_connected, '127.0.0.1', 8888)

        addr = server.sockets[0].getsockname()
        print(f'Server on {addr}')

        async with server:
            await server.serve_forever()

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

    def _check_game_start(self):
        # 随机种子
        # random.seed()
        # 得到本机ip
        try:
            gr.local_ip = socket.gethostbyname(socket.gethostname())
        except socket.gaierror:
            gr.local_ip = '127.0.0.1'
        self._register_singleton()

    def _register_singleton(self):
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
