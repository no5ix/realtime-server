import datetime
import uuid
import warnings
from contextlib import contextmanager

import jwt
from flatten_dict import flatten
from jwt import ExpiredSignatureError, InvalidTokenError

from sanic_jwt_extended.blacklist import InMemoryBlacklist
from sanic_jwt_extended.config import Config
from sanic_jwt_extended.exceptions import (
    AccessDeniedError,
    ConfigurationConflictError,
    FreshTokenRequiredError,
    InvalidHeaderError,
    JWTDecodeError,
    NoAuthorizationError,
    RevokedTokenError,
    WrongTokenError,
)
from sanic_jwt_extended.handler import Handler


class JWT:
    config = None
    handler = None
    blacklist = None

    @classmethod
    @contextmanager
    def initialize(cls, app):
        cls.config = Config()
        cls.handler = Handler()

        yield JWT

        cls.config.read_only = True
        cls.handler.read_only = True
        cls._validate_config()
        cls._setup_blacklist()
        cls._set_error_handlers(app)

    @classmethod
    def _setup_blacklist(cls):
        if cls.config.use_blacklist is True:
            blacklist_cls = (
                cls.config.blacklist_class
                if cls.config.blacklist_class
                else InMemoryBlacklist
            )

            if cls.config.blacklist_init_kwargs:
                cls.blacklist = blacklist_cls(**cls.config.blacklist_init_kwargs)
            else:
                cls.blacklist = blacklist_cls()

    @classmethod
    def _validate_config(cls):
        if cls.config.algorithm.startswith("HS") and not cls.config.secret_key:
            raise ConfigurationConflictError(
                "HS* algorithm needs secret key to encode token"
            )
        elif cls.config.algorithm.startswith("RS"):
            if not cls.config.private_key:
                raise ConfigurationConflictError(
                    "RS* algorithm needs private key to encode token"
                )
            if not cls.config.public_key:
                raise ConfigurationConflictError("RS* algorithm needs public key")

        if cls.config.use_blacklist:
            if not cls.config.blacklist_class:
                warnings.warn(
                    "Blacklist enabled but blacklist class was not specified. "
                    "Falling back to default in-memory blacklist"
                )

    @classmethod
    def _set_error_handlers(cls, app):
        app.error_handler.add(NoAuthorizationError, cls.handler.no_authorization)
        app.error_handler.add(ExpiredSignatureError, cls.handler.expired_signature)
        app.error_handler.add(InvalidHeaderError, cls.handler.invalid_header)
        app.error_handler.add(InvalidTokenError, cls.handler.invalid_token)
        app.error_handler.add(JWTDecodeError, cls.handler.jwt_decode_error)
        app.error_handler.add(WrongTokenError, cls.handler.wrong_token)
        app.error_handler.add(RevokedTokenError, cls.handler.revoked_token)
        app.error_handler.add(FreshTokenRequiredError, cls.handler.fresh_token_required)
        app.error_handler.add(AccessDeniedError, cls.handler.access_denied)

    @classmethod
    def _encode_jwt(cls, token_type, payload, expires_delta):
        algorithm = cls.config.algorithm
        secret = (
            cls.config.secret_key
            if algorithm.startswith("HS")
            else cls.config.private_key
        )

        iss = payload.pop("iss") if payload.get("iss") else cls.config.default_iss
        aud = payload.pop("aud") if payload.get("aud") else cls.config.default_aud
        iat = datetime.datetime.utcnow()
        nbf = payload.pop("nbf") if payload.get("nbf") else iat
        jti = uuid.uuid4().hex

        reserved_claims = {"iss": iss, "aud": aud, "jti": jti, "iat": iat, "nbf": nbf}

        if isinstance(expires_delta, datetime.timedelta):
            reserved_claims["exp"] = iat + expires_delta

        if "cookies" in cls.config.token_location and cls.config.csrf_protect:
            reserved_claims["csrf"] = uuid.uuid4().hex

        payload.update(reserved_claims)
        payload = {k: v for k, v in payload.items() if v is not None}

        header = {"class": token_type}

        token = jwt.encode(
            payload, secret, algorithm, header, cls.config.json_encoder
        ).decode("utf-8")

        return token

    @classmethod
    def create_access_token(
        cls,
        identity,
        role=None,
        fresh=None,
        *,
        expires_delta=None,
        public_claims=None,
        private_claims=None,
        iss=None,
        aud=None,
        nbf=None,
    ):
        payload = {"iss": iss, "sub": identity, "aud": aud, "nbf": nbf}

        if role:
            if not cls.config.use_acl:
                raise ConfigurationConflictError("You should enable ACL to use.")
            payload[cls.config.acl_claim] = role

        if fresh is not None and isinstance(fresh, bool):
            payload["fresh"] = fresh

        if public_claims:
            if not cls.config.public_claim_namespace:
                raise ConfigurationConflictError(
                    "You should specify namespace to use public claims. "
                    "\n find more at: https://auth0.com/docs/tokens/concepts/claims-namespacing"
                )

            public_claims = flatten(public_claims, reducer="path")
            for k, v in public_claims.items():
                payload[cls.config.public_claim_namespace + k] = v

        if private_claims:
            private_claim_prefix = (
                f"{cls.config.private_claim_prefix}_"
                if cls.config.private_claim_prefix
                else ""
            )

            for k, v in private_claims.items():
                payload[private_claim_prefix + k] = v

        if expires_delta is None:
            expires_delta = cls.config.access_token_expires

        access_token = cls._encode_jwt("access", payload, expires_delta)

        return access_token

    @classmethod
    def create_refresh_token(
        cls,
        identity,
        role=None,
        *,
        expires_delta=None,
        public_claims=None,
        private_claims=None,
        iss=None,
        aud=None,
        nbf=None,
    ):
        payload = {"iss": iss, "sub": identity, "aud": aud, "nbf": nbf}

        if role:
            if not cls.config.use_acl:
                raise ConfigurationConflictError("You should enable ACL to use.")
            payload[cls.config.acl_claim] = role

        if public_claims:
            if not cls.config.public_claim_namespace:
                raise ConfigurationConflictError(
                    "You should specify namespace to use public claims. "
                    "\n find more at: https://auth0.com/docs/tokens/concepts/claims-namespacing"
                )

            public_claims = flatten(public_claims, reducer="path")
            for k, v in public_claims.items():
                payload[cls.config.public_claim_namespace + k] = v

        if private_claims:
            private_claim_prefix = (
                f"{cls.config.private_claim_prefix}_"
                if cls.config.private_claim_prefix
                else ""
            )

            for k, v in private_claims.items():
                payload[f"{private_claim_prefix}.{k}"] = v

        if expires_delta is None:
            expires_delta = cls.config.refresh_token_expires

        refresh_token = cls._encode_jwt("refresh", payload, expires_delta)

        return refresh_token
