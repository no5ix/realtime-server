import asyncio
import datetime
import msgpack
import json


# import msgpack
from io import BytesIO
import datetime
import msgpack
import msgpack
import array


def default(obj):
    if isinstance(obj, array.array) and obj.typecode == 'd':
        return msgpack.ExtType(42, obj.tostring())
    raise TypeError("Unknown type: %r" % (obj,))


def ext_hook(code, data):
    if code == 42:
        a = array.array('d')
        a.frombytes(data)
        return a
    return msgpack.ExtType(code, data)


# data = array.array('f', [1.2, 3.4])
data = ['xxd', 'kk']
packed = msgpack.packb(data, default=default, use_bin_type=True)
unpacked = msgpack.unpackb(packed, ext_hook=ext_hook, raw=False)
print(data == unpacked)  # True


useful_dict = {
    "id": 1,
    "created": datetime.datetime.now(),
}


def decode_datetime(obj):
    if b'__datetime__' in obj:
        obj = datetime.datetime.strptime(obj["as_str"], "%Y%m%dT%H:%M:%S.%f")
    return obj


def encode_datetime(obj):
    if isinstance(obj, datetime.datetime):
        return {'__datetime__': True, 'as_str': obj.strftime("%Y%m%dT%H:%M:%S.%f")}
    return obj


packed_dict = msgpack.packb(useful_dict, default=encode_datetime, use_bin_type=True)
this_dict_again = msgpack.unpackb(packed_dict, object_hook=decode_datetime, raw=False)
print(this_dict_again)


async def handle_echo(reader, writer):
    data = await reader.read(100)
    message = data.decode()
    addr = writer.get_extra_info('peername')

    print(f"Received {message!r} from {addr!r}")

    print(f"Send: {message!r}")
    writer.write(data)
    await writer.drain()

    print("Close the connection")
    writer.close()




# async def main():
#     server = await asyncio.start_server(
#         handle_echo, '127.0.0.1', 8888)
#
#     addr = server.sockets[0].getsockname()
#     print(f'Server on {addr}')
#
#     async with server:
#         await server.serve_forever()
#
#
# asyncio.run(main())
