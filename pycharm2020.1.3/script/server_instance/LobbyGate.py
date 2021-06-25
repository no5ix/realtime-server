import sys

from TcpServer import TcpServer
# from common import gr
from common import gv
from common.service_const import ETCD_TAG_LOBBY_GATE, ETCD_TAG_LOBBY_SRV, ETCD_TAG_DISPATCHER_SERVICE
from core.common.RpcMethodArgs import RpcMethodArg, Float
from core.common.RpcSupport import rpc_method, SRV_TO_SRV
from core.mobilelog.LogManager import LogManager
from core.util import UtilApi
from server_entity.LoadReporter import LoadReporter
from server_entity.ServerEntity import ServerEntity


class LobbyGate(object):

    def __init__(self, server_name):
        # game_server_name = sys.argv[1]

        server_json_conf_path = r"../bin/win/conf/lobby_server.json"
        self._server = TcpServer(server_name, ETCD_TAG_LOBBY_GATE, server_json_conf_path, is_proxy=True)
        self._logger = LogManager.get_logger()

        # self._load_collector = LoadCollector.instance()
        self._load_reporter = LoadReporter(ETCD_TAG_DISPATCHER_SERVICE)

    def start(self):
        self._server.run()


if __name__ == '__main__':
    game_server_name = sys.argv[1]
    bs = LobbyGate(game_server_name)
    bs.start()
