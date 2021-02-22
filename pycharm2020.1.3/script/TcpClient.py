import asyncio
import msgpack


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


# def handle_rpc():
#     pass
#
#
# def handle_msg():



async def tcp_echo_client(message):
    reader, writer = await asyncio.open_connection(
        '127.0.0.1', 8888)

    print(f'Send: {message!r}')
    # writer.write(message.encode())
    writer.write(encode(message))

    while True:
        data = await reader.read(100)
        # print(f'Received: {data.decode()!r}')
        print(f'Received: {decode(data)!r}')

    print('Close the connection')
    writer.close()

asyncio.run(tcp_echo_client('Hello World!'))