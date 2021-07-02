import asyncio
import time
from typing import Optional

from core.common import rudp
from core.util.TimerHub import TimerHub
from core.util.UtilApi import wait_or_not

import time

from core.util.UtilApi import wait_or_not

last_ts12 = time.time()


class EchoServerProtocol:

    def __init__(self):
        self.conv = 0
        self.cli_map = {}
        self._kcp = None  # type: Optional[rudp.Kcp]
        self._cli_addr = None
        self._timer_hub = TimerHub()  # type: TimerHub

    def connection_made(self, transport):
        self.transport = transport
        print(f'connection_made, {self.conv=}')

    def datagram_received(self, data, addr):
        # message = data.decode()
        # print('Received %r from %s' % (message, addr))
        # print('Send %r to %s' % (message, addr))
        print(f"Received: {data=}, {addr=}")
        if data == b"Hello World!":
            self.conv += 1
            # self.cli_map[self.conv] = rudp.Kcp(self.conv, self.)
            self._kcp = rudp.Kcp(self.conv, self.send_data_internal)
            self._kcp.set_nodelay(nodelay=True, interval=10, resend=2, nocwnd=True)
            self.tick_kcp_update()

            self._cli_addr = addr
            msg = str(self.conv).encode()
            print("kcp created!")
            self.transport.sendto(msg, addr)
        else:
            _input_res = self._kcp.input(data)
            if _input_res < 0:
                print(f'kcp input error {_input_res=}')
                print("Close the socket")
                self.transport.close()
            elif _input_res == 0:
                while (_recv_data := self._kcp.recv()) is not None:
                    print(f"{_recv_data=}")
                    self.send_msg(_recv_data + b'>>>>')

                self._kcp.update(int(time.time() * 1000))

    def send_msg(self, msg):
        self._kcp.send(msg)  # todo: handle_msg
        # self._kcp.update(int(time.time() * 1000))  # todo: handle_msg
        # self._kcp.flush()  # todo: handle_msg

    def send_data_internal(self, kcp, data):
        assert self.transport
        self.transport.sendto(data, self._cli_addr)

        global last_ts12
        ts_1111112 = time.time() - last_ts12
        print(f'{ ts_1111112=}')
        last_ts12 = time.time()

    # @wait_or_not()
    def tick_kcp_update(self):
        now = time.time()
        self._kcp.update(int(time.time() * 1000))
        wait_sec = self._kcp.check(int(time.time() * 1000)) / 1000
        print(f"tickkkkkkkk, {wait_sec-now=}")

        self._timer_hub.call_at(wait_sec, self.tick_kcp_update)
        # while 1:
        #     self._kcp.update(int(time.time() * 1000))


async def main():
    print("Starting UDP server")

    # Get a reference to the event loop as we plan to use
    # low-level APIs.
    loop = asyncio.get_running_loop()

    # One protocol instance will be created to serve all
    # client requests.
    transport, protocol = await loop.create_datagram_endpoint(
        lambda: EchoServerProtocol(),
        local_addr=('127.0.0.1', 9999))

    try:
        await asyncio.sleep(3600)  # Serve for 1 hour.
    finally:
        transport.close()


asyncio.run(main())





# import asyncio
#
#
# class EchoServerProtocol(asyncio.DatagramProtocol):
#     def connection_made(self, transport):
#         self.transport = transport
#
#     def datagram_received(self, data, addr):
#         message = data.decode()
#         print('Received %r from %s' % (message, addr))
#         print('Send %r to %s' % (message, addr))
#         self.transport.sendto(data, addr)
#
#
# async def main():
#     print("Starting UDP server")
#
#     # Get a reference to the event loop as we plan to use
#     # low-level APIs.
#     loop = asyncio.get_running_loop()
#
#     # One protocol instance will be created to serve all
#     # client requests.
#     transport, protocol = await loop.create_datagram_endpoint(
#         lambda: EchoServerProtocol(),
#         local_addr=('127.0.0.1', 9999))
#
#     try:
#         await asyncio.sleep(3600)  # Serve for 1 hour.
#     finally:
#         transport.close()
#
#
# asyncio.run(main())
