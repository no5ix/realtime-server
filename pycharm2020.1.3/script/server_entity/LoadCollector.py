# import functools
import asyncio

from RpcHandler import rpc_func
# from common import gv
from core.util.UtilApi import async_wrap, Singleton
from server_entity.ServerEntity import ServerEntity
import typing
import redis
import aioredis  # TODO


@Singleton
class LoadCollector(ServerEntity):

    def __init__(self):
        super().__init__()

        _pool = redis.ConnectionPool(host='localhost', port=6379, decode_responses=True)
        self._redis_cli = redis.Redis(connection_pool=_pool)
        self._pipe = self._redis_cli.pipeline(transaction=False)

    @rpc_func
    def report_load(self, etcd_tag, server_name, ip, port, load):
        # self.logger.debug(f"_etcd_tag: {etcd_tag} server_name: {server_name} load: {load}")
        print(f"_etcd_tag: {etcd_tag} server_name: {server_name} load: {load}")  # TODO: DEL
        # self._pipe.zadd(_etcd_tag, {server_name: load})
        async_wrap(lambda: self._redis_cli.zadd(etcd_tag, {"|".join([server_name, ip, str(port)]): load}))
        # self.call_remote_method("report_load_pingpong_test")

    @rpc_func
    async def pick_lowest_load_service_addr(self, etcd_tag: str) -> typing.Tuple[str, str, int]:
        _res_list = await async_wrap(lambda: self._redis_cli.zrange(etcd_tag, 0, 0))  # type: typing.List[str]
        _ret = None
        if _res_list:
            split_res = _res_list[0].split("|")
            _ret = (split_res[0], split_res[1], int(split_res[2]))
            # self.logger.debug(f"pick_lowest_load_service server_name: {split_res[0]}, addr: {_ret}")
            print(f"pick_lowest_load_service server_name: {split_res[0]}, addr: {_ret}")
        return _ret

        # # todo: del
        # await asyncio.sleep(10)
        # self.logger.debug(f"pick_lowest_load_service server_name: fake, addr: fake")
        # return "", 1

# if __name__ == "__main__":
#     pool = redis.ConnectionPool(host='localhost', port=6379, decode_responses=True)
#     redis_redis = redis.Redis(connection_pool=pool)
#     _pipe = redis_redis.pipeline(transaction=False)
#
#     # @rpc_method(SERVER_ONLY, [Str("sn"), Float("l")])
#     _etcd_tag = "battle_server"
#     server_name = "battle_0"
#     load = 12
#     print(f"_etcd_tag: {_etcd_tag} server_name: {server_name} load: {load}")
#     redis_redis.zadd(_etcd_tag, {server_name: load})
#
#     _etcd_tag = "battle_server"
#     server_name = "battle_1"
#     load = 14
#     redis_redis.zadd(_etcd_tag, {server_name: load})
#
#     # self.redis_redis.zpopmin()
#     print(redis_redis.zrange(_etcd_tag, 0, 1))
#     print(redis_redis.zrange(_etcd_tag, 1, 1))
#     print(redis_redis.keys())
#     # s = _pipe.execute()
    # print(s)
