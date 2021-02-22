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


def encode(p):
    return msgpack.packb(p, use_bin_type=True, default=msgpackext)


def decode(p):
    return msgpack.unpackb(
        p,
        # encoding='utf-8',
        ext_hook=ext_hook,
        use_list=False,
        max_str_len=16384,
        max_array_len=1024,
        max_map_len=1024,
        max_ext_len=1024)


def handle_message(msg_data):
    try:
        rpc_message = decode(msg_data)
    except:
        pass
    else:
        try:
            handle_rpc(rpc_message)
        except:
            pass


def handle_rpc(connection, rpc_msg):
    if not connection.entity:
        # self.logger.error("call direct client entity method, but do not bind")
        return
    _entity = connection.entity
    _method_name, _parameters = rpc_msg
    _method = getattr(_entity, _method_name, None)

    if not _method:
        # self.logger.error("entity:%s  method:%s not exist", entity, method_name)
        return
    try:
        _method(_parameters)
    except:
        pass


# async def handle_echo(reader, writer):
#     data = await reader.read(100)
#     # message = data.decode()
#     message = decode(data)
#
#     addr = writer.get_extra_info('peername')
#
#     print(f"Received {message!r} from {addr!r}")
#
#     print(f"Send: {message!r}")
#     writer.write(data)
#     await writer.drain()
#
#     print("Close the connection")
#     writer.close()


writers = []


def forward(writer, addr, message):
    for w in writers:
        if w != writer:
            # w.write(f"{addr!r}: {message!r}\n".encode())
            w.write(encode(f"{addr!r}: {message!r}\n"))


async def handle_client_connected(reader, writer):
    writers.append(writer)
    addr = writer.get_extra_info('peername')
    message = f"{addr!r} is connected !!!!"
    print(message)
    forward(writer, addr, message)
    while True:
        data = await reader.read(100)
        # message = data.decode().strip()
        message = decode(data)
        forward(writer, addr, message)
        await writer.drain()
        if message == "exit":
            message = f"{addr!r} wants to close the connection."
            print(message)
            forward(writer, "Server", message)
            break
    writers.remove(writer)
    writer.close()


async def main():
    # server = await asyncio.start_server(
    #     handle_echo, '127.0.0.1', 8888)
    server = await asyncio.start_server(
        handle_client_connected, '127.0.0.1', 8888)

    addr = server.sockets[0].getsockname()
    print(f'Server on {addr}')

    async with server:
        await server.serve_forever()


asyncio.run(main())
