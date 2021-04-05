# from common import gr
from common import gv
from core.common.RpcMethodArgs import RpcMethodArg, Float, Str
from core.common.RpcSupport import rpc_method, SRV_TO_SRV, rpc_func
from core.mobilelog.LogManager import LogManager
from server_entity.ServerEntity import ServerEntity
import typing
import redis
import time


class LoadCollector(ServerEntity):

    def __init__(self):
        super().__init__()
        gv.add_server_singleton(self)

        _pool = redis.ConnectionPool(host='localhost', port=6379, decode_responses=True)
        self._redis_cli = redis.Redis(connection_pool=_pool)
        self._pipe = self._redis_cli.pipeline(transaction=False)

    # @rpc_method(SERVER_ONLY, [Str("sn"), Float("l")])
    # @rpc_method(SRV_TO_SRV)
    @rpc_func
    def report_load(self, etcd_tag, server_name, ip, port, load):
        print(f"etcd_tag: {etcd_tag} server_name: {server_name} load: {load}")
        # self._pipe.zadd(etcd_tag, {server_name: load})
        self._redis_cli.zadd(etcd_tag, {"|".join([server_name, ip, str(port)]): load})
        self.timer_hub.call_later(2, lambda: self.pick_lowest_load_service(etcd_tag))

    # @rpc_method(SERVER_ONLY)
    def pick_lowest_load_service(self, etcd_tag) -> typing.Tuple[str, int]:
        # self._pipe.zpopmin()
        # self._pipe.zrange()
        _res_list = self._redis_cli.zrange(etcd_tag, 0, 0)  # type: typing.List[str]
        _ret = None
        if _res_list:
            split_res = _res_list[0].split("|")
            _ret = (split_res[1], int(split_res[2]))
        print(f"pick_lowest_load_service _ret: {_ret}")
        return _ret


if __name__ == "__main__":
    pool = redis.ConnectionPool(host='localhost', port=6379, decode_responses=True)
    redis_redis = redis.Redis(connection_pool=pool)
    _pipe = redis_redis.pipeline(transaction=False)

    # @rpc_method(SERVER_ONLY, [Str("sn"), Float("l")])
    etcd_tag = "battle_server"
    server_name = "battle_0"
    load = 12
    print(f"etcd_tag: {etcd_tag} server_name: {server_name} load: {load}")
    redis_redis.zadd(etcd_tag, {server_name: load})

    etcd_tag = "battle_server"
    server_name = "battle_1"
    load = 14
    redis_redis.zadd(etcd_tag, {server_name: load})

    # self.redis_redis.zpopmin()
    print(redis_redis.zrange(etcd_tag, 0, 1))
    print(redis_redis.zrange(etcd_tag, 1, 1))
    print(redis_redis.keys())
    # s = _pipe.execute()
    # print(s)
