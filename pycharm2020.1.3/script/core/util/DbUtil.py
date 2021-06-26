import asyncio
from motor.motor_asyncio import AsyncIOMotorClient

from common import gv
from core.mobilelog.LogManager import LogManager
from core.util.UtilApi import wait_or_not

# import aiotask_context as context
# import asyncio
# from sanic.log import logger
# from pymongo import MongoClient
# from umongo import Instance
# from umongo.frameworks import MotorAsyncIOInstance
from motor.motor_asyncio import AsyncIOMotorClient


db_inst = None
# instance = None


def get_db_inst():
    global db_inst
    try:
        if db_inst is not None:
            return db_inst
        raise Exception('DB connection not found.')
    except Exception as e:
        # return connect('localhost', 27017, asyncio.get_event_loop())
        db_host = '127.0.0.1'
        db_port = 27017
        mongo_database = 'new_byte_db'

        LogManager.get_logger().info("[DB] Establishing DB connection to: %s:%s ", db_host, db_port)
        db_inst = AsyncIOMotorClient(db_host, db_port, io_loop=gv.get_ev_loop())[mongo_database]
        # global instance
        # instance = MotorAsyncIOInstance(db_inst)
        # return instance
        return db_inst
        # return connect('localhost', 27017, gv.get_ev_loop())


@wait_or_not()
async def save_entity(entity):
    try:
        db_save_dict = {"entity_type": entity.__class__.__name__}
        db_save_dict.update(entity.get_db_save_dict())

        # host = '127.0.0.1'
        # port = 27017
        # database = 'testdb'
        #
        # connection = AsyncIOMotorClient(
        #     host,
        #     port
        # )
        # db = connection[database]

        # async def _save():
        #     async for doc in db_inst.LiePin_Analysis1.find({}, ['_id', 'JobTitle', 'is_end']):
        #         db_inst.LiePin_Analysis1.update_one({'_id': doc.get('_id')}, {'$set': {'is_end': 0}})
        db = get_db_inst()
        # document = {'key': 'value'}
        # result = await db.test_collection.insert_one(document)
        # print('result %s' % repr(result.inserted_id))
        # await db.entity.update_one({'_id': entity.id}, {'$set': db_save_dict}, )
        await db.entity.replace_one({'_id': entity.id}, db_save_dict, upsert=True)
    except:
        LogManager.get_logger().log_last_except()

