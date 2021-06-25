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
        self._server = TcpServer(server_name, ETCD_TAG_LOBBY_GATE, server_json_conf_path)
        self._logger = LogManager.get_logger()

        # self._load_collector = LoadCollector.instance()
        self._load_reporter = LoadReporter(ETCD_TAG_DISPATCHER_SERVICE)


        try:
            if gv.etcd_service_node is None:
                return
            lobby_gate_addr = UtilApi.get_service_info(ETCD_TAG_LOBBY_SRV)
            # if self._rpc_handler._conn:
            #     self.logger.debug(f"{self._rpc_handler._conn.get_addr()=}")

            if lobby_gate_addr:  # todo: 每次都有新ip, 但是还是用self.rpc_handler还是用老conn
                self.call_remote_method(
                    "report_load",
                    [gv.etcd_tag, gv.game_server_name, gv.local_ip, gv.local_port,
                        self._avg_load.get_avg_cpu_by_period(10)],
                    rpc_remote_entity_type="LoadCollector", ip_port_tuple=lobby_gate_addr)
                # self.logger.info(f"report_server_load: {self._avg_load.get_avg_cpu_by_period(10)}")
                print(f"report_server_load: {self._avg_load.get_avg_cpu_by_period(10)}")  # TODO: DEL
            else:
                self.logger.error("can not find dispatcher_service_addr")
        except:
            self.logger.log_last_except()

    def start(self):
        self._server.run()


if __name__ == '__main__':
    game_server_name = sys.argv[1]
    bs = LobbyGate(game_server_name)
    bs.start()
