# import functools
import asyncio

import time

from RpcHandler import rpc_func
# from common import gv
from core.util.UtilApi import Singleton, wait_or_not
from server_entity.ServerEntity import ServerEntity
import typing
# import redis
import aioredis  # TODO


@Singleton
class LoadCollector(ServerEntity):

    def __init__(self):
        super().__init__()

        # _pool = redis.ConnectionPool(host='localhost', port=6379, decode_responses=True)
        # self._redis_cli = redis.Redis(connection_pool=_pool)
        # self._pipe = self._redis_cli.pipeline(transaction=False)

        self._redis_cli = None  # type: typing.Optional[aioredis.commands.Redis]
        self.start()

    @wait_or_not()
    async def start(self):
        self._redis_cli = await aioredis.create_redis_pool(('127.0.0.1', 6379), encoding="utf-8")
        # print(f"{self._redis_cli.__class__.__name__=}")
        # await self._redis_cli.set("my-key", "valuemmp")
        # value = await self._redis_cli.get("my-key")
        # print(f"vvvvvvvvvv {value=}")

    async def stop(self):
        self._redis_cli.close()
        await self._redis_cli.wait_closed()

    @rpc_func
    def report_load(self, etcd_tag, server_name, ip, port, load):
        # self.logger.info(f"_etcd_tag: {etcd_tag} server_name: {server_name} load: {load}")
        # print(f"_etcd_tag: {etcd_tag} server_name: {server_name} load: {load}")  # TODO: DEL
        # self._pipe.zadd(_etcd_tag, {server_name: load})
        # async_wrap(lambda: self._redis_cli.zadd(etcd_tag, {"|".join([server_name, ip, str(port)]): load}))
        self._redis_cli.zadd(etcd_tag, load, "|".join([server_name, ip, str(port)]))
        # self.call_remote_method("report_load_pingpong_test")

    @rpc_func
    async def pick_lowest_load_service_addr(self, etcd_tag: str) -> typing.Tuple[str, str, int]:
        # _res_list = await async_wrap(lambda: self._redis_cli.zrange(etcd_tag, 0, 0))  # type: typing.List[str]

        self.logger.info(f"pick_lowest_load_service_addr: {etcd_tag=}")

        start_time = time.time()
        # print(f'pick_lowest_load_service_addr start: {start_time=}')
        _res_list = await self._redis_cli.zrange(etcd_tag, 0, 0)
        _ret = None
        if _res_list:
            split_res = _res_list[0].split("|")
            _ret = (split_res[0], split_res[1], int(split_res[2]))
            # self.logger.info(f"pick_lowest_load_service_addr server_name: {split_res[0]}, addr: {_ret}")
            self.logger.info(f"pick_lowest_load_service_addr server_name: {split_res[0]}, addr: {_ret}")

        end_time = time.time()
        offset = end_time - start_time
        # self.logger.info(f'pick_lowest_load_service_addr end: {offset=}')

        return _ret

        # # todo: del
        # await asyncio.sleep(10)
        # self.logger.info(f"pick_lowest_load_service_addr server_name: fake, addr: fake")
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
