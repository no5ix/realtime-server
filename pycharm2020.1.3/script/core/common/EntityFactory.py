# -*- coding: utf-8 -*-
"""
管理所有的Entity创建的工厂类
"""
from ..mobilelog.LogManager import LogManager
# from Md5OrIndexCodec import  Md5OrIndexDecoder
# from mobilecommon import extendabletype
# from RpcIndex import RpcIndexer


class EntityFactory(object):
    # __metaclass__ = extendabletype

    _instance = None

    def __init__( self ):
        # logger for EntityFactory
        self.logger = LogManager.get_logger("server.EntityFactory")
        #registered classed for EntityFactory
        self.entity_classes = {}

    @classmethod
    def instance( cls ):
        if cls._instance == None:
            cls._instance = EntityFactory()
        return cls._instance

    def register_entity( self, entitytype, entityclass):
        """注册entity 类"""
        self.entity_classes[entitytype] = entityclass
        # 把自己的字符串注册到底层
        RpcIndexer.register_rpc(entityclass.__name__)
        Md5OrIndexDecoder.register_str(entityclass.__name__)
        import inspect
        methods  = inspect.getmembers(entityclass, predicate=inspect.ismethod)
        # 排序以保证注册的顺序是一样的
        methods.sort(lambda a, b : cmp(a[0], b[0]))
        for method in methods:
            if not method[0].startswith("_"):
                RpcIndexer.register_rpc(method[0])
                Md5OrIndexDecoder.register_str(method[0])

    def get_entity_class(self, entitytype):
        EntityClass = None
        if isinstance(entitytype, str):
            EntityClass = self.entity_classes.get( entitytype, None)
        elif isinstance(entitytype, type):
            EntityClass = entitytype
        return EntityClass

    def create_entity(self, entitytype,  entityid = None):
        """创建Entity"""
        EntityClass = self.get_entity_class( entitytype )
        if not EntityClass:
            self.logger.error( "failed to create entity for type %s id %s", str(entitytype),  str(entityid))
            return None
        if entityid == None:
            return EntityClass( )
        else:
            return EntityClass( entityid )


