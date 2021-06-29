import sys

from TcpServer import TcpServer
from common import gv
from common.service_const import ETCD_TAG_DISPATCHER_SERVICE, ETCD_TAG_BATTLE_SRV
from core.mobilelog.LogManager import LogManager
from core.common import EntityScanner
from core.common.EntityFactory import EntityFactory
from core.tool import incremental_reload
from core.util.TimerHub import TimerHub
from server_entity.LoadReporter import LoadReporter


class BattleServer(object):

    def __init__(self, server_name):
        # server_name = sys.argv[1]

        server_json_conf_path = r"../bin/win/conf/battle_server.json"
        self._server = TcpServer(server_name, ETCD_TAG_BATTLE_SRV, server_json_conf_path)
        self._logger = LogManager.get_logger()
        self._timer_hub = TimerHub()

        # self._load_reporter = LoadReporter(ETCD_TAG_DISPATCHER_SERVICE)
        # self.external_task()

    def external_task(self):
        try:
            1/0
        except:
            self._logger.log_last_except()

    def start(self):
        self._server.run()


if __name__ == '__main__':
    game_server_name = sys.argv[1]
    bs = BattleServer(game_server_name)
    bs.start()

