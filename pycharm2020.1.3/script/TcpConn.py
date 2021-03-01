import asyncio
from struct import unpack as s_unpack, pack as s_pack

from core.common import MsgpackSupport

HEAD_LEN = 4
MAX_BODY_LEN = 4294967296


class TcpConn(object):

    def __init__(self, addr_str, asyncio_writer, asyncio_reader, entity=None):
        self.addr_str = addr_str
        self.entity = entity
        self.asyncio_writer = asyncio_writer
        self.asyncio_reader = asyncio_reader

        self.send_cnt = 0
        self._recv_cnt = 0

    def set_entity(self, entity):
        self.entity = entity

    def set_asyncio_writer(self, asyncio_writer):
        self.asyncio_writer = asyncio_writer

    def send_msg(self, msg):
        self.asyncio_writer.write(MsgpackSupport.encode(msg))

    async def loop(self):
        while True:
            # self.asyncio_writer
            _data = await self.asyncio_reader.read(8192)
            while True:
                if len(_data) >= HEAD_LEN:
                    _body_len = s_unpack('i', _data[0:HEAD_LEN])
                    if _body_len > MAX_BODY_LEN:
                        print("body too big, Close the connection")
                        self.asyncio_writer.close()
                        return
                    _body_data = _data[HEAD_LEN:]
            self._recv_cnt += 1
            print("self._recv_cnt:" + str(self._recv_cnt))

        # message = _data.decode().strip()
            self.handle_message(_data)

            # message = MsgpackSupport.decode(_data)
            # self.forward(self.asyncio_writer, addr, message)
            # await self.asyncio_writer.drain()
            # if message == "exit":
            #     message = f"{addr!r} wants to close the connection."
            #     print(message)
            #     self.forward(self.asyncio_writer, "Server", message)
            #     break
        # self.asyncio_writer.close()

    def handle_message(self, msg_data):
        try:
            rpc_message = MsgpackSupport.decode(msg_data)
        except:
            pass
        else:
            try:
                self.handle_rpc(rpc_message)
            except:
                pass

    def handle_rpc(self, rpc_msg):
        if not self.entity:
            # self.logger.error("call direct client entity method, but do not bind")
            return
        _entity = self.entity
        _method_name, _parameters = rpc_msg
        _method = getattr(_entity, _method_name, None)

        if not _method:
            # self.logger.error("entity:%s  method:%s not exist", entity, method_name)
            return
        try:
            _method(_parameters)
        except:
            pass

    async def request_rpc(
            # self, address, service_id, method_name, args=[], service_id_type=0, method_name_type=0,
            self, method_name, args=None,
            # method_name_type=0,
            # need_reply=False, timeout=2
    ):
            # need_reply=False, timeout=2, target_con=None):
        """
        向远端的进程发起一个rpc请求，如果当前已经有连接了，那么直接用这个连接发送数据就行，如果没有可用的连接，
        那么建立一个连接，将发送的数据缓存起来，等到连接建立成功之后再发送
        @param address:            (ip，port)
        @param service_id:         远端服务的id  一般情况下是一个字符串表示，可以通过service_id_type来指定
        @param method_name:        调用的服务对象的方法名字
        @param args:               调用的参数，这个必须要是一个list
        @param service_id_type:    服务id类型，默认为0，字符串
        @param method_name_type:   方法名字类型，默认为0，字符串
        @param need_reply:         当前rpc是否需要返回值
        @param timeout:            返回超时，配合need_reply
        @param target_con:         指定使用的连接对象
        """
        # assert isinstance(args, list)

        # message = [RPC_REQUEST, service_id_type, service_id, method_name_type, method_name, args]
        message = [method_name, args]
        future = None
        _task = None
        # if need_reply:
        #     now_rid = self._get_rid()
        #     future = Future(now_rid, time_out=timeout)
        #     message.append(now_rid)
        # use_con = target_con if target_con else self.get_connection(address)
        # if use_con:
        #     if future:
                # use_con.add_future(future)
            # self._send_rpc_message(message, use_con)
        # else:
        #     self.logger.warn("request rpc but no connection exist, will connect: %s", address)
        #     if future:
        #         future.stop_time_out()     # 关闭超时计时，等到真正发送数据的时候再开启
        #         message.append(future)
        #     self.buffer_send_message(address, message)
        #     self._create_connection(address)

        try:
            data = self.do_encode(message)
        except:
            # self.logger.error("encode request message error")
            # self.logger.log_last_except()
            # self.handle_traceback()
            print("encode request message error")
        else:
            # con.send_data_and_count(data)
            # if gr.flow_backups:
            #     gr.flow_msg('[BATTLE] NET UP ', len(data), message)
            await self.send_data_and_count(data)
            # _task = asyncio.create_task(self.send_data_and_count(data))
        return _task

    def do_encode(self, message):
        return MsgpackSupport.encode(message)

    # def send_data_and_count(self, data):
    async def send_data_and_count(self, data):
        self.send_cnt += 1
        # _len = len(data)
        data_len = len(data) if data else 0
        header_data = s_pack("i", data_len)             # 构建数据的头部
        data = header_data + data

        self.asyncio_writer.write(data)
        await self.asyncio_writer.drain()
        # self.asyncio_writer.drain()
