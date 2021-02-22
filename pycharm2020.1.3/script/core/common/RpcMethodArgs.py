# -*- coding: utf-8 -*-

"""
见rpcdecorator，这个是rpcdecorator用到，用来转换参数的类
 例：@rpc_method(CLIENT_ONLY, (Str('n'), ) ) 中的Str
"""
# import sys
# reload(sys)
# sys.setdefaultencoding( "utf-8" )

# from ..common.IdManager import IdManager

class ConvertError(StandardError):
    pass

class RpcMethodArg(object):
    def __init__(self, name):
        """参数"""
        super(RpcMethodArg, self).__init__()
        self.name = name

    def getname(self):
        return self.name

    def convert(self, _data):
        raise StandardError('Not implemented!')

    def get_type(self):
        raise StandardError('Not implemented!')

    def genametype(self):
        return '%s(%s)' % (self.getname(), self.get_type())

    def default_val(self):
        raise StandardError('Not implemented!')

    def errstr(self, data):
        return '%s is not valid %s' % (data, self.get_type())

    def tostr(self, value):
        return str(value)

    def __str__(self):
        return self.genametype()

class NoLimit(object):
    def isvalide(self, _data):
        return True

    def __str__(self):
        return ""

#pylint: disable=W0622
class NumeralLimit(object):
    def __init__(self, min=None, max=None, range=None):
        super(NumeralLimit, self).__init__()
        self.min = min
        self.max = max
        self.range = range

    def isvalide(self, data):
        if self.min != None and data < self.min:
            return False
        if self.max != None and data > self.max:
            return False
        if self.range != None and data not in self.range:
            return False
        return True

    def __str__(self):
        extra = ''
        if self.min != None or self.max != None:
            extra = '['
            if self.min != None:
                extra += str(self.min)
            extra += '-'
            if self.max != None:
                extra += str(self.max)
            extra += ']'
        elif self.range != None:
            extra = repr(list(self.range)).replace(' ', '')
        return extra


class Int(RpcMethodArg):
    def __init__(self, name, min=None, max=None, range=None):
        super(Int, self).__init__(name)
        if (min==None and max==None and range==None):
            self.limit = NoLimit()
        else:
            self.limit = NumeralLimit(min, max, range)

    def convert(self, data):
        try:
            d = int(data)
        except:
            raise ConvertError(self.errstr(data))
        if self.limit and not self.limit.isvalide(data):
            raise ConvertError(self.errstr(data))
        return d

    def get_type(self):
        return 'Int' + str(self.limit)

    def default_val(self):
        return 0

class Long(RpcMethodArg):
    def __init__(self, name, min=None, max=None, range=None):
        super(Long, self).__init__(name)
        if (min==None and max==None and range==None):
            self.limit = NoLimit()
        else:
            self.limit = NumeralLimit(min, max, range)

    def convert(self, data):
        try:
            d = long(data)
        except:
            raise ConvertError(self.errstr(data))
        if self.limit and not self.limit.isvalide(data):
            raise ConvertError(self.errstr(data))
        return d

    def get_type(self):
        return 'Long' + str(self.limit)

    def default_val(self):
        return 0

class Float(RpcMethodArg):
    def __init__(self, name, min=None, max=None, range=None):
        super(Float, self).__init__(name)
        if (min==None and max==None and range==None):
            self.limit = NoLimit()
        else:
            self.limit = NumeralLimit(min, max, range)

    def convert(self, data):
        try:
            d = float(data)
        except:
            raise ConvertError(self.errstr(data))
        if self.limit and not self.limit.isvalide(data):
            raise ConvertError(self.errstr(data))
        return d

    def get_type(self):
        return 'Float' + str(self.limit)

    def default_val(self):
        return 0

class Str(RpcMethodArg):
    def __init__(self, name):
        super(Str, self).__init__(name)

    def convert(self, data):
        if not type(data) in (str, unicode):
            raise ConvertError(self.errstr(data))
        return str(data)

    def get_type(self):
        return 'Str'

    def default_val(self):
        return ''

class BinData(RpcMethodArg):
    def __init__(self, name):
        super(BinData, self).__init__(name)

    def convert(self, data):
        return data

    def get_type(self):
        return 'BinData'

    def default_val(self):
        return ''

class Avatar(RpcMethodArg):
    """客户端调用服务端的时候，自动把对应的Avatar对象找出来"""
    def __init__(self, name="Avatar"):
        super(Avatar, self).__init__(name)

class MailBox(RpcMethodArg):
    """服务端调用服务端的时候，传递远程的proxy对象"""
    def __init__(self, name="MailBox"):
        super(MailBox, self).__init__(name)

class Response(RpcMethodArg):
    """ Response对象"""
    def __init__(self, name="Response"):
        super(Response, self).__init__(name)

class ClientInfo(RpcMethodArg):
    """客户端调用service的时候，传到service的路由信息"""
    def __init__(self, name="ClientInfo"):
        super(ClientInfo, self).__init__(name)

class GateMailBox(RpcMethodArg):
    """客户端调用service的时候，传到service的路由信息"""
    def __init__(self, name="GateMailBox"):
        super(GateMailBox, self).__init__(name)


class List(RpcMethodArg):
    """在msgpack下，list可能会被转换成tuple类型，考虑到他们的行为是一样的，rpc传递的参数也不应该直接更改"""
    def __init__(self, name):
        super(List, self).__init__(name)

    def convert(self, data):
        if not isinstance(data, (list, tuple)):
            raise ConvertError(self.errstr(data))
        return data

    def get_type(self):
        return 'List'

    def default_val(self):
        return []



class Dict(RpcMethodArg):
    def __init__(self, name):
        super(Dict, self).__init__(name)

    def convert(self, data):
        if type(data) != dict:
            raise ConvertError(self.errstr(data))
        return data

    def get_type(self):
        return 'Dict'

    def default_val(self):
        return {}


class Bool(RpcMethodArg):
    def __init__(self, name):
        super(Bool, self).__init__(name)

    def convert(self, data):
        if type(data) != bool:
            raise ConvertError(self.errstr(data))
        return data

    def get_type(self):
        return 'Bool'

    def default_val(self):
        return False

class Uuid(RpcMethodArg):
    def __init__(self, name):
        super(Uuid, self).__init__(name)

    def convert(self, data):
        if IdManager.is_id_type(data):
            return data
        elif type(data) in (str, unicode):
            if type(data) == unicode:
                data = str(data)
            try:
                return IdManager.bytes2id(data)
            except:
                return IdManager.str2id(data)
        raise ConvertError(self.errstr(data))

    def get_type(self):
        return 'Uuid'

    def default_val(self):
        return None
