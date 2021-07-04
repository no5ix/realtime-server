from typing import Any, Dict, Optional

import aioredis
import ujson


class RedisConnection:  # pragma: no cover
    redis: Optional[aioredis.Redis] = None
    connection_info: Dict[str, Any]

    @classmethod
    async def _get_redis_connection(cls):
        if cls.redis and not cls.redis.closed:
            return cls.redis

        cls.redis = await aioredis.create_redis_pool(**cls.connection_info)
        return cls.redis

    @classmethod
    async def initialize(cls, connection_info):
        cls.connection_info = connection_info
        await cls._get_redis_connection()

    @classmethod
    async def release(cls):
        if cls.redis:
            cls.redis.close()
            await cls.redis.wait_closed()

        cls.redis = None

    @classmethod
    async def set(cls, key: str, value: Any, **kwargs) -> None:
        redis = await cls._get_redis_connection()

        dumped_value = ujson.dumps(value)
        await redis.set(key, dumped_value, **kwargs)

    @classmethod
    async def get(cls, key: str) -> Any:
        redis = await cls._get_redis_connection()
        value = await redis.get(key)
        value = ujson.loads(value) if value else None

        return value

    @classmethod
    async def delete(cls, *keys: str):
        redis = await cls._get_redis_connection()
        await redis.delete(*keys)
