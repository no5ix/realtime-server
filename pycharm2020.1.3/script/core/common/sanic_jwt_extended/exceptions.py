class JWTExtendedException(Exception):
    """
    Base except which all sanic_jwt_extended errors extend
    """

    ...


class JWTDecodeError(JWTExtendedException):
    """
    An error decoding a JWT
    """

    ...


class InvalidHeaderError(JWTExtendedException):
    """
    An error getting header information from a request
    """

    ...


class NoAuthorizationError(JWTExtendedException):
    """
    An error raised when no authorization token was found in a protected endpoint
    """

    ...


class WrongTokenError(JWTExtendedException):
    """
    Error raised when attempting to use a refresh token to access an endpoint
    or vice versa
    """

    ...


class RevokedTokenError(JWTExtendedException):
    """
    Error raised when a revoked token attempt to access a protected endpoint
    """

    ...


class FreshTokenRequiredError(JWTExtendedException):
    """
    Error raised when a valid, non-fresh JWT attempt to access an endpoint
    protected by jwt_required with fresh_required is True
    """

    ...


class AccessDeniedError(JWTExtendedException):
    """
    Error raised when a valid JWT attempt to access an endpoint
    protected by jwt_required with not allowed role
    """

    ...


class ConfigurationConflictError(JWTExtendedException):
    """
    Error raised when trying to use allow and deny option together in jwt_required
    """

    ...


class CSRFError(JWTExtendedException):
    ...
