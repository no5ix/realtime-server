# -*- coding: utf-8 -*-

"""
这是一个decorator，把他放到entity的函数上就可以自动变成一个rpc的方法，同时还会
帮你自动从Bson中解析出来对应的参数

def rpc_method(type, args, pub) 参数说明：

type: 类型，  CLIENT_ONLY, SERVER_ONLY, CLIENT_SERVER
args:	GM指令列表，可以是tuple of str(别名)，或者单独一个str:
	例子：('$teleport', '$goto') or '$teleport'
pub: 是debug用，还是公开也可以用

例：如下声明了一个只有客户端可以调用的服务端RPC方法
	它会自动从客户端传来的bson中提取key为"n"的value，然后作为heroname传进来
	参数名和bson中的key不需要相同，bson的key短一些可以节省带宽，而参数名长一些
	可读性更好
@rpc_method(CLIENT_ONLY, (Str('n'), ) )
def select_hero(self, heroname):
	pass
"""

# from ..mobilelog.LogManager import LogManager
# from ..common.RpcMethodArgs import RpcMethodArg, ConvertError, Avatar, MailBox, ClientInfo, GateMailBox, Response
# from ..common.EntityManager import EntityManager
# from ..servercommon.PostmanDelayGuard import PostmanDelayGuard
# from const import server_const
# from common.utils import utility
import functools

from core.common.RpcMethodArgs import ConvertError, RpcMethodArg

CLIENT_ONLY = 0  # client call server
SERVER_ONLY = 1  # server call server
CLIENT_SERVER = 2  # client or server call server
CLIENT_STUB = 3    # server call client

# logger = LogManager.get_logger("server.RpcMethod")
# _delay_guard = PostmanDelayGuard('rpc', server_const.POSTMAN_WARN_LIMIT_RPC)


class RpcMethod(object):
    """ 被decorate的函数会变成一个RpcMethod对象"""
    def __init__(
            self, func,  rpctype, argtypes,
            pub, cd=-1
    ):
        super(RpcMethod, self).__init__()
        self._has_response = False                        # 标记当前Rpc方法的定义是否包含了Response
        self.func = func
        self.rpctype = rpctype
        self.argtypes = tuple(argtypes)
        # self.pub = pub
        self.need_mailbox = False
        # self.cd = cd
        # self._check_index()

    # def _check_index(self):
    #     """
    #     检测是否有Response参数的定义并保证参数类型的排序:
    #     (MailBox, Response.....)
    #     (MailBox......)
    #     (Response.....)
    #     (.....)
    #     @return:
    #     """
    #     placeholder_index, response_index = -1, -1
    #     for index in range(len(self.arg_types)):
    #         argtype = self.arg_types[index]
    #         if isinstance(argtype, Avatar) or type(argtype) in (MailBox, ClientInfo, GateMailBox):
    #             placeholder_index = index
    #         if isinstance(argtype, Response) or type(argtype) is Response:
    #             response_index = index
    #     if response_index > -1:
    #         self._has_response = True
    #     if placeholder_index > -1:
    #         self.need_mailbox = True
    #     if response_index > 1 or placeholder_index > 0:
    #         raise Exception("argtype index error")

    # def get_placeholder(self, argtype, placeholder):
    #     if not self.need_mailbox:
    #         return None
    #     if isinstance(argtype, Avatar):
    #         avatar = EntityManager.getentity(placeholder)
    #         return avatar
    #
    #     if type(argtype) in (MailBox, ClientInfo, GateMailBox):
    #         return placeholder
    #
    #     return None

    # def call(self, entity, placeholder, parameters):
    def call(self, entity, parameters):
        # if not isinstance(parameters, dict):
        #     print("call: bson parameter decode failed in RPC call %s (%s), ",
        #                  self.func.__name__,  "" .join(str(x) for x in self.arg_types))
        #     return

        args = []
        argtypes = self.argtypes

        for argtype in argtypes:
            try:
                arg = parameters[argtype.getname()]
            except KeyError:
                print(
                    "call: parameter %s not found in RPC call %s, using default value" %
                    argtype.getname(), self.func.__name__)
                arg = argtype.default_val()
            try:
                if arg is None and argtype.get_type() == 'Uuid':
                    # 让uuid类型的参数可以是None
                    arg = None
                else:
                    arg = argtype.convert(arg)
            except ConvertError as e:  # we will call the method if the conversion failed
                print(
                    "call: parameter %s can't convert input %s for RPC call %s exception %s" %
                    argtype.getname(),  str(arg),  self.func.__name__, str(e))
                return
            args.append(arg)
        return self.func(entity, *args)
        # return self.func(entity, *args)


    # def call(self, entity, placeholder, parameters):
    #     """
    #     调用服务端的rpc 方法
    #     :@note game进程的直连通信扩展了带Response的通信模式，为了保持与原API的一致，如果有Response对象，那么placeholder参数
    #         将会传一个tuple进来 (Response, MailBox)
    #     """
    #     #parameters = BSON(bson_string).decode()
    #     if not isinstance(parameters, dict):
    #         # logger.warn("call: bson parameter decode failed in RPC call %s (%s), ",
    #         #              self.func.__name__,  "" .join(str(x) for x in self.arg_types))
    #         return
    #
    #     if self.cd > 0:
    #         now = utility.get_time()
    #         last_call_ts_name = 'last_call_%s_ts' % self.func.__name__
    #         last_call_ts = getattr(entity, last_call_ts_name, 0)
    #         if now - last_call_ts < self.cd:
    #             return
    #         setattr(entity, last_call_ts_name, now)
    #
    #     if not self.arg_types:
    #         return self.func(entity)
    #
    #     # 检测当前调用的rpc方法的Response参数的定义情况是否与调用的情况一致
    #     # 例如被调用的方法定义了Response，但是调用者却没有设置need_reply参数
    #     # 调用者设置了need_reply参数，但是被调用的方法没有定义Response
    #     if not (self._has_response is isinstance(placeholder, tuple)):
    #         if self._has_response:
    #             raise Exception("%s need a response, check you call define" % self.func)
    #         else:
    #             placeholder[0].send_response(error="%s do not have response define" % self.func)
    #             raise Exception("%s do not have response define, check your caller or %s define" % (self.func, self.func))
    #
    #     auto_parameters = parameters.get('_')
    #     if auto_parameters:
    #         parameters = auto_parameters
    #
    #     if isinstance(parameters, dict):
    #         return self.call_with_key(entity, placeholder, parameters)
    #     else:
    #         return self.call_without_key(entity, placeholder, parameters)
    #
    # def call_without_key(self, entity, placeholder, parameters):
    #     response = None
    #     if isinstance(placeholder, tuple):
    #         response = placeholder[0]
    #         placeholder = placeholder[1]
    #
    #     args = []
    #     arg_types = self.arg_types
    #     holder = self.get_placeholder(arg_types[0], placeholder)
    #     if holder:
    #         args.append(holder)
    #         arg_types = arg_types[1:]
    #
    #     if response:
    #         assert isinstance(arg_types[0], Response)
    #         arg_types = arg_types[1:]
    #         args.append(response)
    #
    #     for index in xrange(len(arg_types)):
    #         argtype = arg_types[index]
    #         try:
    #             arg = parameters[index]
    #         except KeyError:
    #             logger.warn("call: parameter %s not found in RPC call %s, using default value",
    #                          argtype.getname(), self.func.__name__)
    #             arg = argtype.default_val()
    #         try:
    #             if arg == None and argtype.get_type() == 'Uuid':
    #                 #让uuid类型的参数可以是None
    #                 arg = None
    #             else:
    #                 arg = argtype.convert(arg)
    #         except ConvertError, e:    # we will call the method if the conversion failed
    #             logger.error("call: parameter %s can't convert input %s for RPC call %s exception %s",
    #                           argtype.getname(),  str(arg),  self.func.__name__, str(e))
    #             return
    #         args.append(arg)
    #     return self.func(entity, *args)
    #
    # def call_with_key(self, entity, placeholder, parameters):
    #     response = None
    #     if isinstance(placeholder, tuple):
    #         response = placeholder[0]
    #         placeholder = placeholder[1]
    #
    #     args = []
    #     arg_types = self.arg_types
    #     # 如果第一个参数是Avatar，则我们把对端调用Entity的Avatar传给方法
    #     holder = self.get_placeholder(arg_types[0], placeholder)
    #     if holder:
    #         args.append(holder)
    #         arg_types = arg_types[1:]
    #
    #     if response:
    #         assert isinstance(arg_types[0], Response)
    #         arg_types = arg_types[1:]
    #         args.append(response)
    #
    #     for argtype in arg_types:
    #         try:
    #             arg = parameters[argtype.getname()]
    #         except KeyError:
    #             logger.warn("call: parameter %s not found in RPC call %s, using default value",
    #                          argtype.getname(), self.func.__name__)
    #             arg = argtype.default_val()
    #         try:
    #             if arg == None and argtype.get_type() == 'Uuid':
    #                 #让uuid类型的参数可以是None
    #                 arg = None
    #             else:
    #                 arg = argtype.convert(arg)
    #         except ConvertError, e: # we will call the method if the conversion failed
    #             logger.error("call: parameter %s can't convert input %s for RPC call %s exception %s",
    #                           argtype.getname(),  str(arg),  self.func.__name__, str(e))
    #             return
    #         args.append(arg)
    #     return self.func(entity, *args)


def rpc_method(rpc_type, arg_types=(), pub=True, cd=-1):
    """ decorator """
    assert (
            rpc_type in (CLIENT_ONLY, SERVER_ONLY, CLIENT_SERVER, CLIENT_STUB)),\
        str(type) + ": type must be one of (CLIENT_ONLY, SERVER_ONLY, CLIENT_SERVER) "
    assert (pub in (True, False)), str(pub) + "should be True of False"

    if type(arg_types) is tuple or type(arg_types) is set or type(arg_types) is list:
        for argtype in arg_types:
            assert (isinstance(argtype, RpcMethodArg)), str(argtype) + ": args Type error"
    elif isinstance(arg_types, RpcMethodArg):
        arg_types = (arg_types,)
    else:
        assert (
                type(arg_types) is tuple or
                type(arg_types) is list or
                type(arg_types) is set or
                isinstance(arg_types, RpcMethodArg)), \
            str(arg_types) + ": arg_types error"

    def _rpc_method(func):
        rpc_func = RpcMethod(func, rpc_type, arg_types, pub, cd=cd)
        call_func = rpc_func.call

        # @functools.wraps(func)
        def call_rpc_method_CLIENT_STUB(self, args):
            func_for_reload = func       # do not remove this, it is useful for reload
            return call_func(self, args)

        # @functools.wraps(func)
        def call_rpc_method_Others(self, args):
            func_for_reload = func       # do not remove this, it is useful for reload
            # return call_func(self, *args)
            # with _delay_guard:
            ret = call_func(self, args)
            return ret

        if rpc_type == CLIENT_STUB:
            call_rpc_method = call_rpc_method_CLIENT_STUB
        else:
            call_rpc_method = call_rpc_method_Others

        call_rpc_method.rpc_func = rpc_func
        if rpc_func.need_mailbox:
            call_rpc_method.need_mailbox = True
        else:
            call_rpc_method.need_mailbox = False
        return call_rpc_method
    return _rpc_method


# def rpc_func(rpc_type, arg_types=()):
#     assert (
#             rpc_type in (CLIENT_ONLY, SERVER_ONLY, CLIENT_SERVER, CLIENT_STUB)),\
#         str(type) + ": type must be one of (CLIENT_ONLY, SERVER_ONLY, CLIENT_SERVER) "
#     # assert (pub in (True, False)), str(pub) + "should be True of False"
#
#     if type(arg_types) is tuple or type(arg_types) is set or type(arg_types) is list:
#         for argtype in arg_types:
#             assert (isinstance(argtype, RpcMethodArg)), str(argtype) + ": args Type error"
#     elif isinstance(arg_types, RpcMethodArg):
#         arg_types = (arg_types, )
#     else:
#         assert (
#                 type(arg_types) is tuple or
#                 type(arg_types) is list or
#                 type(arg_types) is set or
#                 isinstance(arg_types, RpcMethodArg)), \
#             str(arg_types) + ": arg_types error"
#
#     def _rpc_method(func):
#         rpc_method = RpcMethod(func, rpc_type, arg_types
#                               # , pub, cd=cd
#                               )
#         call_func = rpc_method.call
#
#         @functools.wraps(func)
#         def call_rpc_method_CLIENT_STUB(self, args):
#             # fun_for_reload = func       # do not remove this, it is usefull for reload
#             return call_func(self, args)
#
#         @functools.wraps(func)
#         def call_rpc_method_Others(self, args):
#             # fun_for_reload = func       # do not remove this, it is usefull for reload
#             # return call_func(self, *args)
#             # with _delay_guard:
#             ret = call_func(self, args)
#             return ret
#
#         if rpc_type == CLIENT_STUB:
#             call_rpc_method = call_rpc_method_CLIENT_STUB
#         else:
#             call_rpc_method = call_rpc_method_Others
#
#         call_rpc_method.rpc_method = rpc_method
#         if rpc_method.need_mailbox:
#             call_rpc_method.need_mailbox = True
#         else:
#             call_rpc_method.need_mailbox = False
#         return call_rpc_method
#     return _rpc_method


def expose_to_client(method):
    try:
        rpctype = method.rpcmethod.rpctype
        if (rpctype == CLIENT_ONLY or rpctype == CLIENT_SERVER):
            return True
        return False
    except AttributeError:
        return False


def expose_to_server(method):
    try:
        rpctype = method.rpcmethod.rpctype
        if (rpctype == SERVER_ONLY or rpctype == CLIENT_SERVER):
            return True
        return False
    except AttributeError:
        return False

