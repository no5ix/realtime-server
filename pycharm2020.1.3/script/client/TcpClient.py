import asyncio
import random
# import sys

from TcpConn import TcpConn
from client.Puppet import Puppet
# from client.PuppetBindEntity import PuppetBindEntity
# from common import gr
# from core.common import MsgpackSupport
from core.mobilelog.LogManager import LogManager
# from core.util import UtilApi
from core.util.TimerHub import TimerHub


async def tcp_echo_client():
    LogManager.set_log_tag("TcpClient")
    LogManager.set_log_path("../bin/win/log/")
    local_server_port_tuple = (8888, 8889, 9000)
    # local_server_port_tuple = (8889,)
    port = random.choice(local_server_port_tuple)
    reader, writer = await asyncio.open_connection(
        # '192.168.82.177', port)
        # '192.168.1.4', port)
        '127.0.0.1', port)
    # peer_name = writer.get_extra_info('peername')
    _ppt = Puppet()
    _tcp_conn = TcpConn(
        writer.get_extra_info('peername'), writer, reader, _ppt.get_rpc_handle())
    # _tcp_conn.loop()
    # _pbe = PuppetBindEntity()
    # _tcp_conn.set_entity(_pbe)
    # _tcp_conn.set_entity(_ppt)
    # _ppt.set_rpc_handler(_tcp_conn.get_rpc_handler())
    # _pbe.set_puppet(_ppt)
    # _pbe.set_connection(_tcp_conn)

    _ppt.init_from_dict({})

    # _ppt.set_bind_entity(_pbe)
    # gr.bind_entity = _pbe

    # _ppt.CompPuppetTest.puppet_chat_to_channel({'content': 'test_chat_msg'})
    # _ppt.CompPuppetTest.make_server_reload()

    # return
    #
    # print('return')
    #
    # print(f'Send: {message!r}')
    # # writer.write(message.encode())
    # writer.write(MsgpackSupport.encode(message))

    # _cnt = 1000000
    # while _cnt > 0:
    # _ppt.CompPuppetTest.puppet_chat_to_ppt({'content': 'puppet_chat_to_ppt'})
        # _cnt -= 1
        # print(_cnt)
        # await asyncio.sleep(1)
    th = TimerHub()
    th.call_later(
        2, lambda: _ppt.CompPuppetTest.test_reload())
    th.call_later(
        8, lambda: _ppt.CompPuppetTest.make_server_reload())

    th.call_later(
        10, lambda: _ppt.CompPuppetTest.test_reload())
    # await _tcp_conn.loop()
    # _tcp_conn.loop()
    # while True:
    #     data = await reader.read(100)
    #     # print(f'Received: {data.decode()!r}')
    #     print(f'Received: {MsgpackSupport.decode(data)!r}')

    # print('Close the connection')
    # writer.close()


if __name__ == '__main__':
    # gr.game_server_name = sys.argv[1]
    # tcp_server = TcpServer()
    # TCP_SERVER = tcp_server
    # tcp_server.run()

    asyncio.run(tcp_echo_client())
