from functools import wraps
from typing import Callable, List, Optional, Tuple

from sanic.request import Request

from sanic_jwt_extended.exceptions import (
    AccessDeniedError,
    ConfigurationConflictError,
    CSRFError,
    FreshTokenRequiredError,
    InvalidHeaderError,
    NoAuthorizationError,
    RevokedTokenError,
    WrongTokenError,
)
from sanic_jwt_extended.jwt_manager import JWT
from sanic_jwt_extended.tokens import Token

try:
    from hmac import compare_digest
except ImportError:  # pragma: no cover

    def compare_digest(a, b):
        if isinstance(a, str):
            a = a.encode("utf-8")
        if isinstance(b, str):
            b = b.encode("utf-8")

        if len(a) != len(b):
            return False

        r = 0
        for x, y in zip(a, b):
            r |= x ^ y

        return not r


jwt_get_function = Callable[[Request, bool], Tuple[str, Optional[str]]]


def _get_request(args) -> Request:
    if isinstance(args[0], Request):
        request = args[0]
    else:
        request = args[1]
    return request


def _get_raw_jwt_from_request(request, is_access=True):
    functions: List[jwt_get_function] = []

    for eligible_location in JWT.config.token_location:
        if eligible_location == "header":
            functions.append(_get_raw_jwt_from_headers)
        if eligible_location == "query":
            functions.append(_get_raw_jwt_from_query_params)
        if eligible_location == "cookies":
            functions.append(_get_raw_jwt_from_cookies)

    raw_jwt = None
    csrf_value = None
    errors = []

    for f in functions:
        try:
            raw_jwt, csrf_value = f(request, is_access)
            break
        except NoAuthorizationError as e:
            errors.append(str(e))

    if not raw_jwt:
        raise NoAuthorizationError(', '.join(errors))

    return raw_jwt, csrf_value


def _get_raw_jwt_from_headers(request, is_access):
    header_key = (
        JWT.config.jwt_header_key if is_access else JWT.config.refresh_jwt_header_key
    )
    header_prefix = JWT.config.jwt_header_prefix

    token_header = request.headers.get(header_key)

    if not token_header:
        raise NoAuthorizationError(f'Missing header "{header_key}"')

    parts: List[str] = token_header.split()

    if parts[0] != header_prefix or len(parts) != 2:
        raise InvalidHeaderError(
            f"Bad {header_key} header. Expected value '{header_prefix} <JWT>'"
        )

    encoded_token: str = parts[1]

    return encoded_token, None


def _get_raw_jwt_from_query_params(request, _):
    encoded_token = request.args.get(JWT.config.jwt_query_param_name)
    if not encoded_token:
        raise NoAuthorizationError(
            f'Missing query parameter "{JWT.config.jwt_query_param_name}"'
        )

    return encoded_token, None


def _get_raw_jwt_from_cookies(request, is_access):
    cookie_key = JWT.config.jwt_cookie if is_access else JWT.config.refresh_jwt_cookie
    csrf_header_key = (
        JWT.config.jwt_csrf_header if is_access else JWT.config.refresh_jwt_csrf_header
    )

    encoded_token = request.cookies.get(cookie_key)
    csrf_value = None

    if not encoded_token:
        raise NoAuthorizationError(f'Missing cookie "{cookie_key}"')

    if JWT.config.csrf_protect and request.method in JWT.config.csrf_request_methods:
        csrf_value = request.headers.get(csrf_header_key)

        if not csrf_value:
            raise CSRFError("Missing CSRF token")

    return encoded_token, csrf_value


def _csrf_check(csrf_from_request, csrf_from_jwt):
    if not csrf_from_jwt or not isinstance(csrf_from_jwt, str):
        raise CSRFError('Can not find valid CSRF data from token')
    if not compare_digest(csrf_from_request, csrf_from_jwt):
        raise CSRFError('CSRF double submit tokens do not match')


def jwt_required(
    function=None, *, allow=None, deny=None, fresh_required=False,
):
    def real(fn):
        @wraps(fn)
        async def wrapper(*args, **kwargs):
            request = _get_request(args)
            raw_jwt, csrf_value = _get_raw_jwt_from_request(request)

            token_obj = Token(raw_jwt)

            if csrf_value:
                _csrf_check(csrf_value, token_obj.csrf)

            if token_obj.type != "access":
                raise WrongTokenError("Only access tokens are allowed")

            if fresh_required and not token_obj.fresh:
                raise FreshTokenRequiredError("Only fresh access tokens are allowed")

            if allow and token_obj.role not in allow:
                raise AccessDeniedError("You are not allowed to access here")

            if deny and token_obj.role in deny:
                raise AccessDeniedError("You are not allowed to access here")

            if JWT.config.use_blacklist and await JWT.blacklist.is_blacklisted(
                token_obj
            ):
                raise RevokedTokenError("Token has been revoked")

            kwargs["token"] = token_obj

            return await fn(*args, **kwargs)

        return wrapper

    if function:
        return real(function)
    else:
        if allow and deny:
            raise ConfigurationConflictError(
                "Can not use 'deny' and 'allow' option together."
            )
        return real


def jwt_optional(function):
    @wraps(function)
    async def wrapper(*args, **kwargs):
        request = _get_request(args)
        token_obj: Optional[Token] = None

        try:
            raw_jwt, csrf_value = _get_raw_jwt_from_request(request)

            token_obj = Token(raw_jwt)

            if csrf_value:
                _csrf_check(csrf_value, token_obj.csrf)

            if token_obj.type != "access":
                raise WrongTokenError("Only access tokens are allowed")
        except (NoAuthorizationError, InvalidHeaderError):
            pass

        kwargs["token"] = token_obj

        return await function(*args, **kwargs)

    return wrapper


def refresh_jwt_required(function=None, *, allow=None, deny=None):
    def real(fn):
        @wraps(fn)
        async def wrapper(*args, **kwargs):
            request = _get_request(args)
            raw_jwt, csrf_value = _get_raw_jwt_from_request(request, is_access=False)

            token_obj = Token(raw_jwt)

            if csrf_value:
                _csrf_check(csrf_value, token_obj.csrf)

            if token_obj.type != "refresh":
                raise WrongTokenError("Only refresh tokens are allowed")

            if allow and token_obj.role not in allow:
                raise AccessDeniedError("You are not allowed to refresh in here")

            if deny and token_obj.role in deny:
                raise AccessDeniedError("You are not allowed to refresh in here")

            if JWT.config.use_blacklist and await JWT.blacklist.is_blacklisted(
                token_obj
            ):
                raise RevokedTokenError("Token has been revoked")

            kwargs["token"] = token_obj

            return await fn(*args, **kwargs)

        return wrapper

    if function:
        return real(function)
    else:
        if allow and deny:
            raise ConfigurationConflictError(
                "Can not use 'deny' and 'allow' option together."
            )
        return real
