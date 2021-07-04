import datetime
from typing import Any, ContextManager, Dict, Optional

from sanic import Sanic

from sanic_jwt_extended.blacklist import BlacklistABC
from sanic_jwt_extended.config import Config
from sanic_jwt_extended.handler import Handler
from sanic_jwt_extended.tokens import Token

class JWT:
    config: Config = ...
    handler: Handler = ...
    blacklist: Optional[BlacklistABC] = ...
    @classmethod
    def initialize(cls, app: Sanic) -> ContextManager[JWT]: ...
    @classmethod
    def _setup_blacklist(cls): ...
    @classmethod
    def _validate_config(cls): ...
    @classmethod
    def _set_error_handlers(cls, app: Sanic) -> None: ...
    @classmethod
    def _encode_jwt(
        cls,
        token_type: str,
        payload: Dict[str, Any],
        expires_delta: datetime.timedelta,
    ) -> str: ...
    @classmethod
    def create_access_token(
        cls,
        identity: str,
        role: str = ...,
        fresh: bool = ...,
        *,
        expires_delta: datetime.timedelta = ...,
        public_claims: Dict[str, Any] = ...,
        private_claims: Dict[str, Any] = ...,
        iss: str = ...,
        aud: str = ...,
        nbf: datetime.datetime = ...,
    ) -> str: ...
    @classmethod
    def create_refresh_token(
        cls,
        identity: str,
        role: str = ...,
        *,
        expires_delta: datetime.timedelta = ...,
        public_claims: Dict[str, Any] = ...,
        private_claims: Dict[str, Any] = ...,
        iss: str = ...,
        aud: str = ...,
        nbf: datetime.datetime = ...,
    ) -> str: ...
