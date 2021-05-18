import sys

from TcpServer import TcpServer
# from common import gr
from common.service_const import ETCD_TAG_LOBBY_GATE
from core.common.RpcMethodArgs import RpcMethodArg, Float
from core.common.RpcSupport import rpc_method, SRV_TO_SRV
from core.mobilelog.LogManager import LogManager
from server_entity.LoadReporter import LoadReporter
from server_entity.ServerEntity import ServerEntity


class LobbyServer(object):

    def __init__(self, server_name):
        # game_server_name = sys.argv[1]

        server_json_conf_path = r"../bin/win/conf/battle_server.json"
        self._server = TcpServer(server_name, server_json_conf_path)
        self._logger = LogManager.get_logger()

        self._load_reporter = LoadReporter(ETCD_TAG_LOBBY_GATE)

    def start(self):
        self._server.run()


if __name__ == '__main__':
    game_server_name = sys.argv[1]
    bs = LobbyServer(game_server_name)
    bs.start()

    # host = '127.0.0.1'
    # port = 27017
    # database = 'testdb'
    #
    # from pymongo import MongoClient
    #
    # connection = MongoClient(
    #     host,
    #     port
    # )
    # db = connection[database]
    #
    # for doc in db.post.find({}, ['item_id', 'title', 'content']):
    #     db.post.update({'item_id': doc.get('item_id')}, {
    #         '$set': {
    #             'title': doc.get('title'),
    #             'content': doc.get('title')
    #         }
    #     })

    # import time
    # from pymongo import MongoClient
    # import asyncio
    # from motor.motor_asyncio import AsyncIOMotorClient
    #
    # host = '127.0.0.1'
    # port = 27017
    # database = 'testdb'
    #
    # connection = MongoClient(
    #     host,
    #     port
    # )
    # db = connection[database]
    # start = time.time()
    #
    # for doc in db.LiePin_Analysis1.find({}, ['_id', 'JobTitle', 'is_end']):
    #     db.LiePin_Analysis1.update_one({'_id': doc.get('_id')}, {
    #         '$set': {
    #             'is_end': 1
    #         }
    #     })
    #
    # elapsed = (time.time() - start)
    # print("Time used:", elapsed)
    #
    # ######################################
    #
    # connection = AsyncIOMotorClient(
    #     host,
    #     port
    # )
    # db = connection[database]
    #
    # start = time.time()
    #
    # async def run():
    #     async for doc in db.LiePin_Analysis1.find({}, ['_id', 'JobTitle', 'is_end']):
    #         db.LiePin_Analysis1.update_one({'_id': doc.get('_id')}, {'$set': {'is_end': 0}})
    #
    #
    # asyncio.get_event_loop().run_until_complete(run())
    #
    # elapsed = (time.time() - start)
    # print("Time used:", elapsed)
