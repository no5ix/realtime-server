

class PuppetBindEntity(object):

    def __init__(self):
        self._bind_ok = False
        self._connection = None

    def call_server_method(self, method_name, parameters=None):
        """
        @param method_name:    服务端entity方法的名字
        @param parameters:     参数，与call_server_method方法参数类似
        :@note 关于方法名，mobileserver本身有RpcIndex的优化方案，将method_name哈希会有一定的开销，对于业务层，一些频繁
            的rpc方法调用，其实可以简单的将其名字做一些特殊化处理，名字起的短一些，例如就 1,2,3个字符构成方法名字，
            也能达到减少数据传输的效果
        """
        # method_name = RpcInde.send_rpc_index(method_name)
        if self._bind_ok:
            # self._connection.request_rpc("", "", args=[method_name, parameters])
            self._connection.request_rpc(method_name, parameters)
        else:
            pass # raise Exception("call rpc in a connection do not bind any server entity")

    def set_connection(self, conn):
        self._connection = conn
