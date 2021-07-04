import warnings
from datetime import datetime

from abc import ABC, abstractmethod
from sanic_jwt_extended.redis import RedisConnection


class BlacklistABC(ABC):  # pragma: no cover
    @abstractmethod
    async def register(self, token):
        pass

    @abstractmethod
    async def is_blacklisted(self, token):
        pass


class InMemoryBlacklist(BlacklistABC):
    def __init__(self):
        self.blacklist = []
        warnings.warn(
            "Using in-memory blacklist is not recommended for production environment"
        )

    async def register(self, token):
        self.blacklist.append(token.jti)

    async def is_blacklisted(self, token):
        return token.jti in self.blacklist


class RedisBlacklist(BlacklistABC):  # pragma: no cover
    def __init__(self, connection_info):
        self.connection_info = connection_info

    async def register(self, token):
        if not RedisConnection.redis:
            await RedisConnection.initialize(self.connection_info)

        kwargs = {}

        if token.exp:
            kwargs["expire"] = int((token.exp - datetime.utcnow()).total_seconds())

        await RedisConnection.set(token.jti.hex, token.raw_jwt, **kwargs)

    async def is_blacklisted(self, token):
        if not RedisConnection.redis:
            await RedisConnection.initialize(self.connection_info)

        return bool(await RedisConnection.get(token.jti.hex))
