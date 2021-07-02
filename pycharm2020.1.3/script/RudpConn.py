from __future__ import annotations

import time
import asyncio
import struct
import typing
from asyncio import transports
from asyncio.exceptions import CancelledError

from ConnBase import HEARTBEAT_TIMEOUT, HEARTBEAT_INTERVAL, ROLE_TYPE_ACTIVE, ConnBase
from common import gv
from core.common.IdManager import IdManager
from core.util.TimerHub import TimerHub
from core.util.UtilApi import wait_or_not

if typing.TYPE_CHECKING:
    from RpcHandler import RpcHandler

# from common import gr
# from core.common import MsgpackSupport
# from core.common.EntityFactory import EntityFactory
from core.mobilelog.LogManager import LogManager


HEAD_LEN = 4
RPC_HANDLER_ID_LEN = 12
STRUCT_PACK_FORMAT = '12s'
MAX_BODY_LEN = 4294967296


class RudpConn(ConnBase):
    pass
