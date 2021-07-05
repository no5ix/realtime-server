from __future__ import annotations

import typing

import asyncio
import datetime
from asyncio import transports
from typing import Optional

from jwt import ExpiredSignatureError

from core.mobilelog.LogManager import LogManager
from sanic_jwt_extended import JWT
from sanic_jwt_extended.tokens import Token

if typing.TYPE_CHECKING:
    import TcpConn
    from RudpConn import RudpConn


PROTO_TYPE_TCP = 0
PROTO_TYPE_RUDP = 1


class TcpProtocol(asyncio.Protocol):
    def __init__(self, role_type):
        self._role_type = role_type
        self._conn = None  # type: Optional[TcpConn.TcpConn]

    def connection_made(self, transport: transports.BaseTransport) -> None:
        from ConnMgr import ConnMgr
        from ConnMgr import ROLE_TYPE_PASSIVE

        addr = transport.get_extra_info('peername')  # type: typing.Tuple[str, int]
        if self._role_type == ROLE_TYPE_PASSIVE:
            self._conn = ConnMgr.instance().add_incoming_conn(
                PROTO_TYPE_TCP, transport, addr
                # self._rpc_handler
            )
            LogManager.get_logger().info(f"TCP ROLE_TYPE_PASSIVE peer_addr={addr} is connected !!!!")
        else:
            self._conn = ConnMgr.instance().get_conn(addr, PROTO_TYPE_TCP)
            assert self._conn
            LogManager.get_logger().info(f"TCP ROLE_TYPE_ACTIVE peer_addr={addr} is connected !!!!")

    def data_received(self, data: bytes) -> None:
        self._conn.handle_read(data)

    def connection_lost(self, exc: Optional[Exception]) -> None:
        self._conn.handle_close(str(exc))


RUDP_HANDSHAKE_SYN = b'new_byte_hello_its_me'
RUDP_HANDSHAKE_SYN_ACK_PREFIX = b'new_byte_welcome:'
RUDP_HANDSHAKE_ACK_PREFIX = b'new_byte_ack:'
RUDP_HANDSHAKE_RESET_PREFIX = b'new_byte_reset'

RUDP_JWT_EXP = 6
RUDP_CONV = 0


class RudpProtocol(asyncio.DatagramProtocol):
    def __init__(self):
        with JWT.initialize() as manager:
            manager.config.secret_key = "new_byte"

        self.transport = None
        self.logger = LogManager.get_logger()

    def send_access_jwt(self, addr, send_handshake_type):
        global RUDP_CONV
        RUDP_CONV += 1
        access_token_jwt: bytes = JWT.create_access_token(
            identity=str(RUDP_CONV), expires_delta=datetime.timedelta(seconds=RUDP_JWT_EXP)).encode()

        # exp = int((Token(access_token_jwt.decode()).exp - datetime.datetime.utcnow()).total_seconds())
        # print(f'aaa {exp=}')

        self.transport.sendto(send_handshake_type + access_token_jwt, addr)

    def get_token_obj(self, recv_data, recv_handshake_type) -> Token:
        parts = recv_data.split(recv_handshake_type, 2)
        if len(parts) != 2:
            raise Exception(f"Expected value '{recv_handshake_type}<JWT>'")

        raw_jwt = parts[1].decode()
        try:
            token_obj = Token(raw_jwt)
            # exp = int((token_obj.exp - datetime.datetime.utcnow()).total_seconds())
            # print(f'xxx {exp=}')
        except ExpiredSignatureError as e:
            self.logger.warning(str(e))
            return
        except:
            self.logger.log_last_except()
            return
        if token_obj.type != "access":
            raise Exception("Only access tokens are allowed")
        # self.transport.sendto(RUDP_HANDSHAKE_ACK_PREFIX + raw_jwt.encode(), addr)
        return token_obj

    def connection_made(self, transport):
        self.transport = transport

    def datagram_received(self, data: bytes, addr):
        global RUDP_CONV
        global RUDP_JWT_EXP
        global RUDP_HANDSHAKE_SYN
        global RUDP_HANDSHAKE_SYN_ACK_PREFIX
        global RUDP_HANDSHAKE_ACK_PREFIX

        from ConnMgr import ConnMgr

        # print(f'datagram_received: {data=}, {addr=}')
        if data == RUDP_HANDSHAKE_SYN:
            # RUDP_CONV += 1
            # access_token_jwt: bytes = JWT.create_access_token(
            #     identity=str(RUDP_CONV), expires_delta=datetime.timedelta(seconds=RUDP_JWT_EXP)).encode()
            #
            # # exp = int((Token(access_token_jwt.decode()).exp - datetime.datetime.utcnow()).total_seconds())
            # # print(f'aaa {exp=}')
            #
            # self.transport.sendto(RUDP_HANDSHAKE_SYN_ACK_PREFIX + access_token_jwt, addr)
            self.send_access_jwt(addr, RUDP_HANDSHAKE_SYN_ACK_PREFIX)
        elif data.startswith(RUDP_HANDSHAKE_ACK_PREFIX):
            # i = randint(1, 5)
            # print(f'{i=}')
            # if i >= 3:
            #     return  # todo: del
            # print('passssssssss')
            # parts = data.split(RUDP_HANDSHAKE_ACK_PREFIX, 2)
            # if len(parts) != 2:
            #     raise Exception(f"Expected value '{RUDP_HANDSHAKE_ACK_PREFIX}<JWT>'")
            # # sleep(2)
            # raw_jwt = parts[1].decode()
            # try:
            #     token_obj = Token(raw_jwt)
            #     # exp = int((token_obj.exp - datetime.datetime.utcnow()).total_seconds())
            #     # print(f'xxx {exp=}')
            # except ExpiredSignatureError as e:
            #     self.logger.warning(str(e))
            #     return
            # except:
            #     self.logger.log_last_except()
            #     return
            #
            # if token_obj.type != "access":
            #     raise Exception("Only access tokens are allowed")
            token_obj = self.get_token_obj(data, RUDP_HANDSHAKE_ACK_PREFIX)
            conv = int(token_obj.identity)
            ConnMgr.instance().add_incoming_conn(PROTO_TYPE_RUDP, self.transport, addr, rudp_conv=conv)

            self.logger.info(f"RUDP ROLE_TYPE_PASSIVE peer_addr={addr} is connected !!!!")
        elif data.startswith(RUDP_HANDSHAKE_SYN_ACK_PREFIX):
            # return  # todo: del
            # parts = data.split(RUDP_HANDSHAKE_SYN_ACK_PREFIX, 2)
            # if len(parts) != 2:
            #     raise Exception(f"Expected value '{RUDP_HANDSHAKE_SYN_ACK_PREFIX}<JWT>'")
            #
            # raw_jwt = parts[1].decode()
            # try:
            #     token_obj = Token(raw_jwt)
            #     # exp = int((token_obj.exp - datetime.datetime.utcnow()).total_seconds())
            #     # print(f'xxx {exp=}')
            # except ExpiredSignatureError as e:
            #     self.logger.warning(str(e))
            #     return
            # except:
            #     self.logger.log_last_except()
            #     return
            # if token_obj.type != "access":
            #     raise Exception("Only access tokens are allowed")

            token_obj = self.get_token_obj(data, RUDP_HANDSHAKE_SYN_ACK_PREFIX)
            self.transport.sendto(RUDP_HANDSHAKE_ACK_PREFIX + token_obj.raw_jwt.encode(), addr)
            conv = int(token_obj.identity)
            ConnMgr.instance().set_fut_result(addr, conv)

            self.logger.info(f"RUDP ROLE_TYPE_ACTIVE peer_addr={addr} is connected !!!!")
        elif data.startswith(RUDP_HANDSHAKE_RESET_PREFIX):
            _conn: RudpConn = ConnMgr.instance().get_conn(addr, PROTO_TYPE_RUDP)
            if _conn is None:
                self.logger.debug(
                    f"RUDP ROLE_TYPE_PASSIVE peer_addr={addr} RESET sending RUDP_HANDSHAKE_SYN_ACK_PREFIX!!!!")
                self.send_access_jwt(addr, RUDP_HANDSHAKE_SYN_ACK_PREFIX)
                return
            else:
                self.logger.debug(
                    f"RUDP ROLE_TYPE_PASSIVE peer_addr={addr} RESET reinit kcp!!!!")
                token_obj = self.get_token_obj(data, RUDP_HANDSHAKE_RESET_PREFIX)
                conv = int(token_obj.identity)
                _conn.init_kcp(conv)
                self.transport.sendto(RUDP_HANDSHAKE_ACK_PREFIX + token_obj.raw_jwt.encode(), addr)
        else:
            _cur_conn = ConnMgr.instance().get_conn(addr, PROTO_TYPE_RUDP)
            if _cur_conn:
                _cur_conn.handle_read(data)
            else:
                self.logger.debug(f"RUDP ROLE_TYPE_PASSIVE peer_addr={addr} RESET !!!!")
                self.send_access_jwt(addr, RUDP_HANDSHAKE_RESET_PREFIX)
                # RUDP_CONV += 1
                # access_token_jwt: bytes = JWT.create_access_token(
                #     identity=str(RUDP_CONV), expires_delta=datetime.timedelta(seconds=RUDP_JWT_EXP)).encode()
                #
                # # exp = int((Token(access_token_jwt.decode()).exp - datetime.datetime.utcnow()).total_seconds())
                # # print(f'aaa {exp=}')
                #
                # self.transport.sendto(RUDP_HANDSHAKE_RESET_PREFIX + access_token_jwt, addr)
