import asyncio
from core.common import MsgpackSupport




async def tcp_echo_client(message):
    reader, writer = await asyncio.open_connection(
        '127.0.0.1', 8888)

    print(f'Send: {message!r}')
    # writer.write(message.encode())
    writer.write(MsgpackSupport.encode(message))

    while True:
        data = await reader.read(100)
        # print(f'Received: {data.decode()!r}')
        print(f'Received: {MsgpackSupport.decode(data)!r}')

    print('Close the connection')
    writer.close()

asyncio.run(tcp_echo_client('Hello World!'))