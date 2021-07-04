import sys

from RudpServer import RudpServer
from TcpServer import TcpServer
# from common import gr
from common.service_const import ETCD_TAG_DISPATCHER_SERVICE
from core.common.RpcMethodArgs import RpcMethodArg, Float
from core.common.RpcSupport import rpc_method, SRV_TO_SRV
from core.mobilelog.LogManager import LogManager
from server_entity.LoadCollector import LoadCollector
from server_entity.ServerEntity import ServerEntity


class DispatcherService(object):

    def __init__(self, server_name):
        # server_name = sys.argv[1]

        server_json_conf_path = r"../bin/win/conf/dispatcher_service.json"
        self._server = TcpServer(server_name, ETCD_TAG_DISPATCHER_SERVICE, server_json_conf_path)
        # self._server = RudpServer(server_name, ETCD_TAG_DISPATCHER_SERVICE, server_json_conf_path)
        self._logger = LogManager.get_logger()

        self._load_collector = LoadCollector.instance()

    def start(self):
        self._server.run()


if __name__ == '__main__':
    game_server_name = sys.argv[1]
    bs = DispatcherService(game_server_name)
    bs.start()
