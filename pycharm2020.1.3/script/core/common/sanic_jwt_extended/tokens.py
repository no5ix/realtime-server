import datetime
import json
import uuid
from dataclasses import dataclass, field
from typing import Any, Dict, Optional

import jwt
from flatten_dict import unflatten
from jwt.utils import base64url_decode

from sanic_jwt_extended.exceptions import ConfigurationConflictError, JWTDecodeError
from sanic_jwt_extended.jwt_manager import JWT


@dataclass
class Token:
    raw_jwt: str
    raw_data: Dict[str, Any] = field(init=False)

    # Metadata
    type: str = field(init=False)
    role: Optional[str] = field(init=False, default=None)
    fresh: Optional[bool] = field(init=False, default=None)
    identity: str = field(init=False)
    csrf: str = field(init=False, default=None)

    # Registered claims
    iss: Optional[str] = field(init=False, default=None)
    sub: str = field(init=False)
    aud: Optional[str] = field(init=False, default=None)
    exp: Optional[datetime.datetime] = field(init=False, default=None)
    nbf: datetime.datetime = field(init=False)
    iat: datetime.datetime = field(init=False)
    jti: uuid.UUID = field(init=False)

    # Additional claims
    public_claims: Dict[str, Any] = field(init=False, default=None)
    private_claims: Dict[str, Any] = field(init=False, default=None)

    def __post_init__(self):
        self.raw_data = self._decode_jwt()

        self.type = self._get_type()
        self.role = (
            self.raw_data.get(JWT.config.acl_claim) if JWT.config.use_acl else None
        )
        self.fresh = self.raw_data.get("fresh") if self.type == "access" else None
        self.csrf = self.raw_data.get("csrf")

        try:
            self.iss = self.raw_data.get("iss")
            self.sub = self.identity = self.raw_data["sub"]
            self.aud = self.raw_data.get("aud")
            exp = self.raw_data.get("exp")
            nbf = self.raw_data["nbf"]
            iat = self.raw_data["iat"]
            jti = self.raw_data["jti"]
        except KeyError as e:
            raise JWTDecodeError(
                f"Can not get registered claims from payload. missing {e}"
            )

        try:
            self.jti = uuid.UUID(jti)
        except Exception:
            raise JWTDecodeError(f"Wrong jti")

        try:
            self.exp = datetime.datetime.utcfromtimestamp(exp) if exp else None
            self.nbf = datetime.datetime.utcfromtimestamp(nbf)
            self.iat = datetime.datetime.utcfromtimestamp(iat)
        except Exception:
            raise JWTDecodeError(f"Wrong timestamp for 'nbf' or/and 'iat'")

        self.public_claims = (
            self._get_public_claims() if JWT.config.public_claim_namespace else {}
        )
        self.private_claims = self._get_private_claims()

    def _get_private_claims(self):
        private_claims = {
            k: v
            for k, v in self.raw_data.items()
            if k.startswith(JWT.config.private_claim_prefix)
            and k not in ("iss", "sub", "aud", "exp", "nbf", "iat", "jti")
        }
        if JWT.config.private_claim_prefix:
            private_claims = {
                k.replace(f"{JWT.config.private_claim_prefix}_", ""): v
                for k, v in private_claims.items()
            }
        if JWT.config.public_claim_namespace:
            private_claims = {
                k: v
                for k, v in private_claims.items()
                if not k.startswith(JWT.config.public_claim_namespace)
            }

        return private_claims

    def _get_public_claims(self):
        public_claims = {
            k.replace(JWT.config.public_claim_namespace, ""): v
            for k, v in self.raw_data.items()
            if k.startswith(JWT.config.public_claim_namespace)
            and k not in ("iss", "sub", "aud", "exp", "nbf", "iat", "jti")
        }

        return unflatten(public_claims, splitter="path")

    def _get_type(self):
        raw_header = self.raw_jwt.split(".")[0]
        header: Dict[str, str] = json.loads(base64url_decode(raw_header))

        if header.get("class") not in ("access", "refresh"):
            raise JWTDecodeError(
                "Can not resolve token type by JOSE header. missing 'class'"
            )

        return header.get("class")

    def _decode_jwt(self):
        algorithm = JWT.config.algorithm
        secret = (
            JWT.config.secret_key
            if algorithm.startswith("HS")
            else JWT.config.public_key
        )

        jwt_data = jwt.decode(
            self.raw_jwt,
            secret,
            algorithms=[algorithm],
            options={"verify_iss": False, "verify_aud": False},
        )

        return jwt_data

    async def revoke(self):
        if not JWT.config.use_blacklist:
            raise ConfigurationConflictError(
                "To revoke token, you should enable blacklist"
            )

        await JWT.blacklist.register(self)
