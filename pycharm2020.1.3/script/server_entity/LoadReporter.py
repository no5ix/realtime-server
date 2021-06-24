import random

from RpcHandler import rpc_func
from common import service_const, gv
# from common.service_const import ETCD_TAG_DISPATCHER_SERVICE
from core.util import UtilApi
from core.util.UtilApi import Singleton
from core.util.performance.cpu_load_handler import AvgCpuLoad
from server_entity.ServerEntity import ServerEntity


# LOAD_REPORT_INTERVAL = 0.01  # todo modify to 8
# LOAD_REPORT_INTERVAL = 0.04  # todo modify to 8
LOAD_REPORT_INTERVAL = 2


class LoadReporter(ServerEntity):

    def __init__(self, load_collector_etcd_tag):
        super().__init__()
        self._load_collector_etcd_tag = load_collector_etcd_tag
        self._avg_load = AvgCpuLoad()
        self.timer_hub.call_later(LOAD_REPORT_INTERVAL, self.report_load, repeat_count=-1)
        # self.timer_hub.call_later(LOAD_REPORT_INTERVAL, self.report_load, repeat_count=8)  # TODO: del

    def report_load(self):
        try:
            if gv.etcd_service_node is None:
                return
            dispatcher_service_addr = UtilApi.get_service_info(self._load_collector_etcd_tag)
            # if self._rpc_handler._conn:
            #     self.logger.debug(f"{self._rpc_handler._conn.get_addr()=}")
            if dispatcher_service_addr:  # todo: 每次都有新ip, 但是还是用self.rpc_handler还是用老conn
                self.call_remote_method(
                    "report_load",
                    [gv.etcd_tag, gv.game_server_name, gv.local_ip, gv.local_port,
                        self._avg_load.get_avg_cpu_by_period(10)],
                    rpc_remote_entity_type="LoadCollector", ip_port_tuple=dispatcher_service_addr)
                # self.logger.info(f"report_server_load: {self._avg_load.get_avg_cpu_by_period(10)}")
                print(f"report_server_load: {self._avg_load.get_avg_cpu_by_period(10)}")  # TODO: DEL
            else:
                self.logger.error("can not find dispatcher_service_addr")
        except:
            self.logger.log_last_except()

    # todo: del
    @rpc_func
    def report_load_pingpong_test(self):
        self.call_remote_method(
            "pick_lowest_load_service_addr",
            [gv.etcd_tag],
            # rpc_remote_entity_type="LoadCollector", ip_port_tuple=dispatcher_service_addr
            # rpc_callback=lambda err, res: self.logger.info(f"pick_lowest_load_service_addr: {err=} {res=}"),
            rpc_callback=lambda err, res: print(f"pick_lowest_load_service_addr: {err=} {res=}"),
            rpc_remote_entity_type="LoadCollector")
        self.report_load()

