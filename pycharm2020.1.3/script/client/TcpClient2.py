import asyncio
import random
import sys
# from asyncio import events

import TcpConn
from client.Avatar import Avatar
from client.Puppet import Puppet
# from client.PuppetBindEntity import PuppetBindEntity
# from common import gr
# from core.common import MsgpackSupport
from common import gv
from core.mobilelog.LogManager import LogManager
# from core.util import UtilApi
from core.util import UtilApi
from core.util.TimerHub import TimerHub
from server_entity.ServerEntity import ServerEntity


async def tcp_echo_client(cli_index):
    LogManager.set_log_tag("tcp_client_" + str(cli_index))
    LogManager.set_log_path("../bin/win/log/")

    cli_log = LogManager.get_logger()

    json_conf_path = r"../bin/win/conf/lobby_server.json"
    UtilApi.parse_json_conf(json_conf_path)
    rand_dispatcher_service_addr = None
    for _svr_name, _svr_info in gv.game_json_conf.items():
        if type(_svr_info) is dict and _svr_info.get("etcd_tag", None) == "dispatcher_service":
            rand_dispatcher_service_addr = (_svr_info["ip"], _svr_info["port"])
            break

    temp_se = ServerEntity()
    _err, _res = await temp_se.call_remote_method(
        "pick_lowest_load_service_addr",
        # [gv.etcd_tag],
        # ["battle_server"],
        ["lobby_server"],
        # rpc_reply_timeout=None,
        # rpc_remote_entity_type="LoadCollector", ip_port_tuple=dispatcher_service_addr
        # rpc_callback=lambda err, res: self.logger.info(f"pick_lowest_load_service_addr: {err=} {res=}"),
        rpc_remote_entity_type="LoadCollector", ip_port_tuple=rand_dispatcher_service_addr)
    if _err:
        cli_log.error(f"{_err=}")
        return


    # local_server_port_tuple = (8888, 8889, 9000, 9001, 9002, 9003, 9004)
    local_server_port_tuple = (8888,)
    port = random.choice(local_server_port_tuple)

    cli_log.debug(f"bs: {_res}")
    reader, writer = await asyncio.open_connection(
        _res[1], _res[2])
        # '192.168.82.177', port)
        # '192.168.1.4', port)
        # '127.0.0.1', port)
    # peer_name = writer.get_extra_info('peername')
    # _ppt = Puppet()
    _ppt = Avatar()
    _tcp_conn = TcpConn.TcpConn(
        TcpConn.ROLE_TYPE_ACTIVE,
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
    th = TimerHub()
    th.call_later(
        # 1,
        # 10,
        0.016,  # 极限
        # 0.01,  # 基本已经处理不过来
        lambda: _ppt.CompAvatarTest.puppet_chat_to_ppt({'content': 'puppet_chat_to_ppt'}),
        repeat_count=-1, repeat_interval_sec=10
    )
        # _cnt -= 1
        # print(_cnt)
        # await asyncio.sleep(1)
    # th = TimerHub()
    # th.call_later(
    #     2, lambda: _ppt.CompPuppetTest.test_reload())
    # th.call_later(
    #     8, lambda: _ppt.CompPuppetTest.make_server_reload())
    #
    # th.call_later(
    #     10, lambda: _ppt.CompPuppetTest.test_reload())

    # msg = "calll test_timer_async"
    # _ppt.CompPuppetTest.test_response_rpc(msg)
    # msg = "calll test_timer_async nonono"
    # _ppt.CompPuppetTest.test_response_rpc(msg)
    # await _tcp_conn.loop()
    # await _tcp_conn.loop()
    # while True:
    #     data = await reader.read(100)
    #     # print(f'Received: {data.decode()!r}')
    #     print(f'Received: {MsgpackSupport.decode(data)!r}')
    await asyncio.sleep(888888888)
    # print('Close the connection')
    # writer.close()


if __name__ == '__main__':
    # gr.game_server_name = sys.argv[1]
    # tcp_server = TcpServer()
    # TCP_SERVER = tcp_server
    # tcp_server.run()
    # _loop = asyncio.get_event_loop()

    # _ev_loop = events.new_event_loop()
    # events.set_event_loop(_ev_loop)
    # _ev_loop.run_until_complete(tcp_echo_client())
    # _ev_loop.run_forever()
    gv.get_ev_loop().run_until_complete(tcp_echo_client(sys.argv[1]))
    # asyncio.run(tcp_echo_client(sys.argv[1]))
