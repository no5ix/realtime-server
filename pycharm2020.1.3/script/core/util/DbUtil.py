import asyncio
from motor.motor_asyncio import AsyncIOMotorClient


def save_entity(entity):
    db_save_dict = {"entity_type": entity.__class__.__name__}
    db_save_dict.update(entity.get_db_save_dict())

