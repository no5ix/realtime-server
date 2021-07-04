from sanic.response import json


class Handler:
    default_message_key = "msg"

    no_authorization = staticmethod(
        lambda r, e: json({Handler.default_message_key: str(e)}, 401)
    )
    expired_signature = staticmethod(
        lambda r, e: json({Handler.default_message_key: str(e)}, 401)
    )
    invalid_header = staticmethod(
        lambda r, e: json({Handler.default_message_key: str(e)}, 422)
    )
    invalid_token = staticmethod(
        lambda r, e: json({Handler.default_message_key: str(e)}, 422)
    )
    jwt_decode_error = staticmethod(
        lambda r, e: json({Handler.default_message_key: str(e)}, 422)
    )
    wrong_token = staticmethod(
        lambda r, e: json({Handler.default_message_key: str(e)}, 422)
    )
    revoked_token = staticmethod(
        lambda r, e: json({Handler.default_message_key: str(e)}, 401)
    )
    fresh_token_required = staticmethod(
        lambda r, e: json({Handler.default_message_key: str(e)}, 401)
    )
    access_denied = staticmethod(
        lambda r, e: json({Handler.default_message_key: str(e)}, 403)
    )
