import sys

from TcpServer import TcpServer
# from common import gr
from core.common.RpcMethodArgs import RpcMethodArg, Float
from core.common.RpcSupport import rpc_method, SRV_TO_SRV
from core.mobilelog.LogManager import LogManager
from server_entity.LoadCollector import LoadCollector
from server_entity.ServerEntity import ServerEntity


class LobbyGate(object):

    def __init__(self, server_name):
        # game_server_name = sys.argv[1]

        server_json_conf_path = r"../bin/win/conf/lobby_server.json"
        self._server = TcpServer(server_name, server_json_conf_path)
        self._logger = LogManager.get_logger()

        self._load_collector = LoadCollector.instance()

    def start(self):
        self._server.run()


    # def handle(self):


if __name__ == '__main__':
    game_server_name = sys.argv[1]
    bs = LobbyGate(game_server_name)
    bs.start()
