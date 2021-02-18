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


# def default(obj):
#     if isinstance(obj, array.array) and obj.typecode == 'd':
#         return msgpack.ExtType(42, obj.tostring())
#     raise TypeError("Unknown type: %r" % (obj,))
#
#
# def ext_hook(code, data):
#     if code == 42:
#         a = array.array('d')
#         a.frombytes(data)
#         return a
#     return msgpack.ExtType(code, data)
#
#
# # data = array.array('f', [1.2, 3.4])
# data = ['xxd', 'kk']
# packed = msgpack.packb(data, default=default, use_bin_type=True)
# unpacked = msgpack.unpackb(packed, ext_hook=ext_hook, raw=False)
# print(data == unpacked)  # True
#
#
# useful_dict = {
#     "id": 1,
#     "created": datetime.datetime.now(),
# }
#
#
# def decode_datetime(obj):
#     if b'__datetime__' in obj:
#         obj = datetime.datetime.strptime(obj["as_str"], "%Y%m%dT%H:%M:%S.%f")
#     return obj
#
#
# def encode_datetime(obj):
#     if isinstance(obj, datetime.datetime):
#         return {'__datetime__': True, 'as_str': obj.strftime("%Y%m%dT%H:%M:%S.%f")}
#     return obj
#
#
# packed_dict = msgpack.packb(useful_dict, default=encode_datetime, use_bin_type=True)
# this_dict_again = msgpack.unpackb(packed_dict, object_hook=decode_datetime, raw=False)
# print(this_dict_again)


def msgpackext(obj):
    # if IdManager.is_id_type(obj):
    #     return ExtType(42, IdManager.id2bytes(obj))
    # elif isinstance(obj, datetime.datetime):
    #     ts = int(time.mktime(obj.timetuple()) * 1000) + obj.microsecond / 1000
    #     data = struct.pack('<Q', ts)
    #     return ExtType(43, data)
    # elif isinstance(obj, MsgBinData):
    #     return ExtType(45, obj.bin_data)
    return repr(obj)


def ext_hook(code, data):
    # if code == 42:
    #     return IdManager.bytes2id(data)
    # if code == 43:
    #     ts = struct.unpack('<Q', data)[0]
    #     return datetime.datetime.fromtimestamp(ts / 1000.0)
    # if code == 45:
    #     return do_decode(data)
    return msgpack.ExtType(code, data)


encode = lambda p: msgpack.packb(p, use_bin_type=True, default=msgpackext)
decode = lambda p: msgpack.unpackb(p,
                        # encoding='utf-8',
                        ext_hook=ext_hook, use_list=False,
                        max_str_len=16384,
                        max_array_len=1024,
                           max_map_len=1024,
                           max_ext_len=1024)


async def handle_echo(reader, writer):
    data = await reader.read(100)
    # message = data.decode()
    message = decode(data)

    addr = writer.get_extra_info('peername')

    print(f"Received {message!r} from {addr!r}")

    print(f"Send: {message!r}")
    writer.write(data)
    await writer.drain()

    print("Close the connection")
    writer.close()


async def main():
    server = await asyncio.start_server(
        handle_echo, '127.0.0.1', 8888)

    addr = server.sockets[0].getsockname()
    print(f'Server on {addr}')

    async with server:
        await server.serve_forever()


asyncio.run(main())
