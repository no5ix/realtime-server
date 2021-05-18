import asyncio
from motor.motor_asyncio import AsyncIOMotorClient

from core.util.UtilApi import wait_or_not


@wait_or_not()
async def save_entity(entity):
    db_save_dict = {"entity_type": entity.__class__.__name__}
    db_save_dict.update(entity.get_db_save_dict())

    host = '127.0.0.1'
    port = 27017
    database = 'testdb'

    connection = AsyncIOMotorClient(
        host,
        port
    )
    db = connection[database]

    # async def _save():
    #     async for doc in db.LiePin_Analysis1.find({}, ['_id', 'JobTitle', 'is_end']):
    #         db.LiePin_Analysis1.update_one({'_id': doc.get('_id')}, {'$set': {'is_end': 0}})

    await db.entity.update_one({'_id': entity.id}, {'$set': db_save_dict})
