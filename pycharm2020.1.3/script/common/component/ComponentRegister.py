# -*- coding: utf-8 -*-
# 负责注册Component
import sys
# from common.component import Property
# from core.common.Md5OrIndexCodec import Md5OrIndexDecoder
# from core.common.RpcIndex import RpcIndexer
#
# ComponentClass = {}
#
#
# def register_rpc_class(cls, prefix=None):
#     import inspect
#
#     if hasattr(cls, "VAR_NAME"):
#         prefix = cls.VAR_NAME
#
#     Md5OrIndexDecoder.register_str(cls.__name__)
#     for name, func in inspect.getmembers(cls, inspect.ismethod):
#         if not (name.startswith("__") and name.endswith("__")):
#             if hasattr(func, "rpcmethod"):
#                 if prefix:
#                     RpcIndexer.register_rpc("%s.%s" % (prefix, func.rpcmethod.func.__name__))
#                     Md5OrIndexDecoder.register_str("%s.%s" % (prefix, func.rpcmethod.func.__name__))
#                 else:
#                     RpcIndexer.register_rpc(func.__name__)
#                     Md5OrIndexDecoder.register_str(func.__name__)
#                 # print "RegisterRPCClass ", func.__name__
#             else:
#                 pass
#
#
# def register_by_name(name, component):
#     ComponentClass[name] = component
#     register_rpc_class(component)
# # Property.register_property_class(component)
#
#
# def register(component):
#     if 'VAR_NAME' in component.__dict__:
#         name = component.VAR_NAME
#     else:
#         name = component.__name__
#
#     # print "register", name,component
#     register_by_name(name, component)


def get_component(name, default=None):
    comp = ComponentClass.get(name)

    if not comp:
        return default

    # 不直接用comp是因为可能代码会reload，reload之后新的类会放到ComponentClass中
    # 但实际上代码还是用旧的类对象，只是把function替换成新类的function而已
    return getattr(sys.modules[comp.__module__], comp.__name__)
