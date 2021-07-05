from __future__ import annotations
import sys

import typing

from ConnMgr import ConnMgr, ROLE_TYPE_PASSIVE
from core.common.protocol_def import PROTO_TYPE_TCP, TcpProtocol
from ServerBase import ServerBase

if typing.TYPE_CHECKING:
    from RpcHandler import RpcHandler

from common import gv

# TCP_SERVER = None


class TcpServer(ServerBase):

    async def _start_server_task(self):
        try:
            # _ev_loop = gv.get_ev_loop()
            server = await self._ev_loop.create_server(
                lambda: TcpProtocol(ROLE_TYPE_PASSIVE),
                gv.local_ip, gv.local_port)

            addr = server.sockets[0].getsockname()
            self._logger.info(f'TCP Server on {addr}')
            async with server:
                await server.serve_forever()
        except KeyboardInterrupt:
            self._logger.info(f"\nShutting Down Server: {gv.server_name}...\n")
            # _loop = asyncio.get_running_loop()
            # _loop.stop()
            # _loop.close()
            # server.close()

            return
        except:
            # self.logger.info("Unexpected error:", sys.exc_info()[0])
            self._logger.log_last_except()
            raise


if __name__ == '__main__':
    game_server_name = sys.argv[1]
    server_json_conf_path = r"../bin/win/conf/battle_server.json"
    tcp_server = TcpServer(game_server_name, server_json_conf_path)
    # TCP_SERVER = tcp_server
    tcp_server.run()

    # loop = asyncio.get_event_loop()
    # loop.run_until_complete(main())
