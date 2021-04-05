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

CLI_TO_SRV = 0  # client call server
SRV_TO_SRV = 1  # server call server
CLI_SRV_TO_SRV = 2  # client or server call server
SRV_TO_CLI = 3    # server call client

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
        if type(parameters) is dict:
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
        else:  # type(parameters) is list:
            args = parameters
        return self.func(entity, *args)
        # return self.func(entity, *args)


def rpc_method(rpc_type, arg_types=(), pub=True, cd=-1):
    """ decorator """
    assert (
            rpc_type in (CLI_TO_SRV, SRV_TO_SRV, CLI_SRV_TO_SRV, SRV_TO_CLI)),\
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

        if rpc_type == SRV_TO_CLI:
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


def rpc_func(func):
    def wrapper(*args, **kwargs):
        func_for_reload = func
        return func(*args, **kwargs)
    wrapper.is_rpc_func = True
    return wrapper


def expose_to_client(method):
    try:
        rpctype = method.rpcmethod.rpctype
        if (rpctype == CLI_TO_SRV or rpctype == CLI_SRV_TO_SRV):
            return True
        return False
    except AttributeError:
        return False


def expose_to_server(method):
    try:
        rpctype = method.rpcmethod.rpctype
        if (rpctype == SRV_TO_SRV or rpctype == CLI_SRV_TO_SRV):
            return True
        return False
    except AttributeError:
        return False

