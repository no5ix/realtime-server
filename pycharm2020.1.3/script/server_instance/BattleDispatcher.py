import sys

from TcpServer import TcpServer
# from common import gr
from core.common.RpcSupport import rpc_method, SERVER_ONLY
from core.mobilelog.LogManager import LogManager
from server_entity.ServerEntity import ServerEntity


class BattleDispatcher(object):

    def __init__(self, server_name):
        # game_server_name = sys.argv[1]

        server_json_conf_path = r"../bin/win/conf/battle_server.json"
        self._server = TcpServer(server_name, server_json_conf_path)
        self._logger = LogManager.get_logger()

    def start(self):
        self._server.run()


class BattleDispatcherImpl(ServerEntity):

    @rpc_method(SERVER_ONLY)
    def report_battle_server_info(self):
        pass


if __name__ == '__main__':
    game_server_name = sys.argv[1]
    bs = BattleDispatcher(game_server_name)
    bs.start()

