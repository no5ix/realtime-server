import asyncio
import time
from typing import Optional

from core.common import rudp
from core.util.TimerHub import TimerHub

import time

from core.util.UtilApi import wait_or_not

last_ts1 = time.time()


class EchoClientProtocol:
    def __init__(self, message, on_con_lost):
        self.message = message
        self.on_con_lost = on_con_lost
        self.transport = None

        self._kcp = None  # type: Optional[rudp.Kcp]
        self._timer_hub = TimerHub()  # type: TimerHub

    def connection_made(self, transport):
        self.transport = transport
        print('Send:', self.message)
        # self.transport.sendto(self.message.encode())
        self.transport.sendto(self.message)

    def datagram_received(self, data, addr):
        # print("Received:", data.decode())
        print(f"Received: {data=}, {addr=}")
        if self._kcp is None:
            _conv = int(data.decode())
            self._kcp = rudp.Kcp(int(_conv), self.send_data_internal)
            self._kcp.set_nodelay(nodelay=True, interval=10, resend=2, nocwnd=True)
            self.tick_kcp_update()
            self.send_msg(b"first kcp msg")
        else:
            _input_res = self._kcp.input(data)
            if _input_res < 0:
                print(f'kcp input error {_input_res=}')
                print("Close the socket")
                self.transport.close()
            elif _input_res == 0:
                while (_recv_data := self._kcp.recv()) is not None:
                    print(f"{_recv_data=}")

                    global last_ts1
                    ts_111111 = time.time() - last_ts1
                    print(f'{ ts_111111=}')
                    last_ts1 = time.time()

                    self.send_msg(_recv_data)

                self._kcp.update(int(time.time()*1000))

    # @wait_or_not()
    def tick_kcp_update(self):
        now = time.time()
        self._kcp.update(int(time.time() * 1000))
        wait_sec = self._kcp.check(int(time.time() * 1000)) / 1000
        print(f"tickkkkkkkk, {wait_sec-now=}")

        self._timer_hub.call_at(wait_sec, self.tick_kcp_update)

    def send_msg(self, msg):
        self._kcp.send(msg)  # todo: handle_msg
        # self._kcp.update(int(time.time() * 1000))  # todo: handle_msg
        # self._kcp.flush()  # todo: handle_msg

    def send_data_internal(self, kcp, data):
        assert self.transport
        self.transport.sendto(data)

    def error_received(self, exc):
        print('Error received:', exc)

    def connection_lost(self, exc):
        print("Connection closed")
        self.on_con_lost.set_result(True)


async def main():
    # Get a reference to the event loop as we plan to use
    # low-level APIs.
    loop = asyncio.get_running_loop()

    on_con_lost = loop.create_future()
    message = b"Hello World!"

    transport, protocol = await loop.create_datagram_endpoint(
        lambda: EchoClientProtocol(message, on_con_lost),
        remote_addr=('127.0.0.1', 9999))

    try:
        await on_con_lost
    finally:
        transport.close()


asyncio.run(main())
