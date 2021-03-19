# from core.common.RpcMethodArgs import Str, Dict
# from core.common.RpcSupport import rpc_method, CLIENT_STUB
#
#
# class PuppetBindEntity(object):
#
#     def __init__(self):
#         self._bind_ok = False
#         self._connection = None
#         self._puppet = None
#
#     def set_puppet(self, ppt):
#         self._puppet = ppt
#
#     def call_server_method(self, method_name, parameters=None):
#         """
#         @param method_name:    服务端entity方法的名字
#         @param parameters:     参数，与call_server_method方法参数类似
#         :@note 关于方法名，mobileserver本身有RpcIndex的优化方案，将method_name哈希会有一定的开销，对于业务层，一些频繁
#             的rpc方法调用，其实可以简单的将其名字做一些特殊化处理，名字起的短一些，例如就 1,2,3个字符构成方法名字，
#             也能达到减少数据传输的效果
#         """
#         # method_name = RpcInde.send_rpc_index(method_name)
#         if self._bind_ok:
#             # self._connection.request_rpc("", "", args=[method_name, parameters])
#             self._connection.request_rpc(self._puppet.__class__.__name__, method_name, parameters)
#         else:
#             pass # raise Exception("call rpc in a connection do not bind any server entity")
#
#     def set_connection(self, conn):
#         self._connection = conn
#
#     def set_bind_ok(self):
#         self._bind_ok = True
#
#     @rpc_method(CLIENT_STUB, (Str('m'), Dict('p')))
#     def local_entity_message(self, method_name, parameters):
#         # try:
#         #     method_name = RpcIndexer.INDEX2RPC[method_name]
#         # except KeyError:
#         #     self.logger.error('Failed to decode method_name for uid=%s index=%s', self.uid, method_name)
#         #     return
#         puppet = self._puppet
#         if not puppet:
#             # self.logger.error('Failed to get puppet, method_name=%s', method_name)
#             print('Failed to get puppet, method_name=%s', method_name)
#             return
#         method_list = method_name.split('.')
#         if len(method_list) == 1:
#             ent = puppet
#             name = method_name
#         else:
#             ent = puppet.get_component(method_list[0])
#             name = method_list[1]
#         method = getattr(ent, name, None)
#         if method:
#             # optimized after tick，降低客户端延迟，不然需要等下一次tick才做统计
#             # if puppet.delay_calls:
#             #     puppet.delay_calls.callback('opt-at', 0.001, puppet.flush_aoi_data)
#             # method(ent, parameters)
#             method(parameters)
