from core.common import MsgpackSupport


class TcpConn(object):

    def __init__(self, addr_str, asyncio_writer, asyncio_reader, entity=None):
        self.addr_str = addr_str
        self.entity = entity
        self.asyncio_writer = asyncio_writer
        self.asyncio_reader = asyncio_reader

    def set_entity(self, entity):
        self.entity = entity

    def set_asyncio_writer(self, asyncio_writer):
        self.asyncio_writer = asyncio_writer

    def send_data(self, data):
        self.asyncio_writer.write(MsgpackSupport.encode(data))

    def loop(self):
        while True:
            # self.asyncio_writer
            data = await self.asyncio_reader.read(100)
            # message = data.decode().strip()
            self.handle_message(data)

            # message = MsgpackSupport.decode(data)
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