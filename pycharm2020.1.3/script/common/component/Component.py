# -*- coding: utf-8 -*-

####
# 作为组件的类(非entity)只需要继承Component，然后再ComponentRegister注册一下自己，就可以作为entity(ComponentSupport)的组件

# Component可以使用修饰符 dependency require 添加与其他Component的关系 （dependency不得出现循环依赖）

# ComponentSupport 使用修饰符components添加组件（目前双端的entity都继承自ComponentSupport，因此双端的entity可以直接添加组件）

###

import typing
from server_entity.ServerEntity import ServerEntity
import copy
import inspect
from common import gr
from common.component import ComponentRegister
# from common.aoi.Property import get_class_properties, is_sync_client, is_sync_other_client, is_persist

# 依赖的component
ClassAllDependency = {}

# require的component
ClassAllRequire = {}


def __add_component(klass, name, component):
    if name in klass.__components__ and klass.__components__[name] != component:
        origin_component = klass.__components__[name]
        # 如果不是子类关系,抛异常
        if origin_component not in component.__bases__:
            raise RuntimeError("The names of Components are conflict, please modify: %s (type %s, name %s)" % (component, type(component), name))

    klass.__components__[name] = component


def __get_dependency(klass):
    dependency = set()

    if klass is object:
        return dependency

    if klass in ClassAllDependency:
        return ClassAllDependency[klass]

    for base in list(klass.__bases__):
        dependency.update(__get_dependency(base))

    if "__dependency__" in klass.__dict__:
        dependency.update(klass.__dependency__)
    ClassAllDependency[klass] = dependency
    klass.__dependency_all__ = dependency

    return dependency


# def __get_require(klass):
#     require = set()
#
#     if klass is object:
#         return require
#
#     if klass in ClassAllRequire:
#         return ClassAllRequire[klass]
#
#     for base in list(klass.__bases__):
#         require.update(__get_require(base))
#
#     if "__require__" in klass.__dict__:
#         require.update(klass.__require__)
#     ClassAllRequire[klass] = require
#     klass.__require_all__ = require
#
#     return require


# 支持component的类装饰器, 声明entity拥有哪些component
# 参数：传入列表，可以是字符串形式的类名，也可以直接类名
def components(*component_list):
    def _components(entity_klass):
        entity_klass.__components__ = {}
        entity_klass.__sorted_components__ = []
        fix_components = list(component_list)

        bases = list(entity_klass.__bases__)
        bases.reverse()

        for base in bases:
            # 如果继承父component, 需要把父component加进来
            if "__components__" in base.__dict__:
                for name, component in base.__components__.iteritems():
                    if name in fix_components or component in fix_components:
                        continue
                    fix_components.insert(0, component)

        for name in fix_components:
            if type(name) == str:
                component = ComponentRegister.get_component(name)
            elif inspect.isclass(name):
                component = name
            else:
                component = None

            if inspect.isclass(component):
                # 取类名的VAR_NAME, 这是component在entity中被引用的名字，通常比原始的Component名字更短
                # 在entity里面用VAR_NAME来引用该component
                if component.VAR_NAME == "Component":
                    component.VAR_NAME = component.__name__

                name = component.VAR_NAME
                # assert name != "Component"
                __add_component(entity_klass, name, component)
            else:
                raise RuntimeError("Can't find component: %s (type %s)." % (name, type(component)))

        for component in entity_klass.__components__.values():
            # 处理component的特殊逻辑
            if hasattr(component, "init_after_bind"):
                component.init_after_bind(entity_klass)

        # dependency
        # 根据依赖关系进行拓扑排序, 生成__sorted_components__作为后面初始化和遍历的顺序
        graph = {}
        for name, component in entity_klass.__components__.items():
            graph[name] = __get_dependency(component)

        sorted_list = toposort(graph, entity_klass.__name__)
        for name in sorted_list:
            entity_klass.__sorted_components__.append((name, entity_klass.__components__[name]))

        # # print 'sorted_components: ', klass.__sorted_components__
        # var_names = set()
        # for name, _ in entity_klass.__sorted_components__:
        #     var_names.add(name)
        #
        # # require
        # all_requires = set()
        # for _, component in entity_klass.__sorted_components__:
        #     all_requires.update(__get_require(component))
        #
        # # print "[chh]!!!!!!!",all_requires
        # # print "[ccc]",ClassAllRequire
        #
        # for name in all_requires:
        #     # print "aaa",name
        #     if type(name) == str:
        #         component = ComponentRegister.get_component(name)
        #     elif inspect.isclass(name):
        #         component = name
        #     else:
        #         component = None
        #
        #     # print "name",name, component
        #     if inspect.isclass(component):
        #         v_name = component.VAR_NAME
        #         if v_name not in var_names:
        #             raise RuntimeError("Entity must have a named component:%s" % v_name)
        #     else:
        #         raise RuntimeError("Can't find component: %s (type %s)." % (name, type(component)))

        # if gr.is_client:
        # 	import Property
        # 	for name, component in klass.__sorted_components__:
        # 		Property.scan_property_changed_listener(klass, name, component)

        return entity_klass

    return _components


# [chh] Component基类 请保持Component与VAR_NAME名字的一致
class Component(object):
    VAR_NAME = 'Component'
    __use_descriptor__ = True

    def __init__(self):
        self.entity = None  # type: typing.Type[ServerEntity, None]
        # self.logger = None
        self._client_tick_cache = []

    # 绑定entity到自己. 每个component有个归属的entity
    # 如果有args, 则初始化property, 否则等到后面entity读取数据库后，再主动调用init_properties
    def init(self, entity):
        self.entity = entity
        # self.logger = entity.logger
        if self._client_tick_cache and hasattr(self.entity, 'add_tick'):
            for tick_func in self._client_tick_cache:
                self.add_tick(tick_func)
            self._client_tick_cache = []

        self._do_init()

    def init_properties(self, info_dict):
        if gr.is_client:
            return
        # # TODO 等服务器编译好C++版的AOI_DATA就可以统一了
        # cls = self.__class__
        # all_properties = get_class_properties(cls)
        # for property_name, attr in all_properties.iteritems():
        #     info = info_dict.get(property_name)
        #     default_val = attr['default']
        #     # if gr.is_server:
        #     # 	self.entity.create_property(default_val, self.VAR_NAME + '.' + property_name, info, attr['sync'])
        #     # else:
        #     # 	if info_dict.get(property_name) is None:
        #     # 		setattr(self, property_name, default_val)
        #     # 	else:
        #     # 		setattr(self, property_name, info_dict.get(property_name))
        #     self.entity.create_property(default_val, self.VAR_NAME + '.' + property_name, info, attr['sync'])

    # def get_persistent_properties(self):
    #     return self.get_properties_data(is_persist)
    #
    # def get_client_properties(self):
    #     return self.get_properties_data(is_sync_client)
    #
    # def get_other_properties(self):
    #     return self.get_properties_data(is_sync_other_client)
    #
    # def get_properties_data(self, checker=lambda x: True):
    #     # 获得属性对应的数据
    #     cls = self.__class__
    #     all_properties = get_class_properties(cls)
    #     mp = {}
    #     for var_name, attr in all_properties.iteritems():
    #         if not checker(attr):
    #             continue
    #         v = getattr(self, var_name)
    #         default_val = attr['default']
    #         if v is None:
    #             v = default_val
    #         if isinstance(default_val, list) or isinstance(default_val, dict) or isinstance(default_val, tuple):
    #             if not hasattr(v, 'object'):
    #                 raise AttributeError("'%s' object has no attribute 'object', path=%s.%s" % (
    #                     v.__class__.__name__, cls.__name__, var_name
    #                 ))
    #             mp[var_name] = v.object()
    #         elif isinstance(default_val, type):
    #             if not hasattr(v, 'get_properties_data'):
    #                 raise AttributeError("'%s' object has no attribute 'get_properties_data', path=%s.%s" % (
    #                     v.__class__.__name__, cls.__name__, var_name
    #                 ))
    #             mp[var_name] = v.get_properties_data(checker)
    #         else:
    #             mp[var_name] = v
    #     return mp

    # 初始化后的回调
    def post_init(self):
        self._do_post_init()

    # 销毁处理
    def destroy(self):
        self._do_destroy()

    # 所有Component都会执行。在Destroy之前调用
    def before_destroy(self):
        pass

    # 所有Component都会执行。在Destroy之后调用
    def post_destroy(self):
        self.entity = None
        self._do_post_destroy()

    # 针对一些数据进行转换。例如object对象
    def convert_dict_data(self, bdict):
        if not hasattr(self, "_do_convert_dict_data"):
            return bdict

        if type(bdict) is not dict:
            bdict = {}

        # return self._do_convert_dict_data(copy.deepcopy(bdict))
        return bdict

    def reload_script(self):
        self._do_reload_script()

    def _do_reload_script(self):
        pass

    def _do_init(self):
        pass

    def _do_post_init(self):
        pass

    def _do_destroy(self):
        pass

    def _do_post_destroy(self):
        pass

    def add_tick(self, tick):
        if gr.is_client:
            if self.entity:
                self.entity.add_tick(tick)
            else:
                self._client_tick_cache.append(tick)

    def remove_tick(self, tick):
        if gr.is_client and self.entity:
            self.entity.remove_tick(tick)

    # # 需要的时候显示实现
    # def _do_convert_dict_data(self, bdict):
    # 	return bdict

    def call_client_comp_method(self, component_name, method_name, parameters):
        self.entity.call_client_method(component_name + '.' + method_name, parameters)

    def call_other_client_comp_method(self, component_name, method_name, parameters):
        self.entity.call_other_client_method(component_name + '.' + method_name, parameters)

    def call_all_client_comp_method(self, component_name, method_name, parameters):
        self.entity.call_all_client_method(component_name + '.' + method_name, parameters)

    def call_client_method(self, method_name, parameters):
        self.entity.call_client_method(method_name, parameters)

    def call_other_client_method(self, method_name, parameters):
        self.entity.call_other_client_method(method_name, parameters)

    def call_all_client_method(self, method_name, parameters):
        self.entity.call_all_client_method(method_name, parameters)

    def call_server_comp_method(self, component_name, method_name, parameters):
        self.entity.call_server_method(component_name + '.' + method_name, parameters)

    def call_server_method(self, method_name, parameters):
        self.entity.call_server_method(method_name, parameters)

    def call_server_method_direct(self, remote_mb, method_name, parameters, *args, **kwargs):
        return self.entity.call_server_method_direct(remote_mb, method_name, parameters, *args, **kwargs)

    # def set_logger(self, logger, entity_name, extra):
    #     import logging
    #     from util import LoggerAdapter
    #
    #     lg = logger
    #     extra_for_comp = {'comp': self.VAR_NAME, 'class': self.__class__.__name__}
    #     if isinstance(logger, logging.LoggerAdapter):
    #         lg = logger.logger
    #     extra_for_comp.update(extra)
    #     adapter_class_name = entity_name + 'CompLoggerAdapter'
    #     comp_adapter_class = getattr(LoggerAdapter, adapter_class_name, None)
    #     if comp_adapter_class is None:
    #         return
    #     self.logger = comp_adapter_class(lg, extra_for_comp)


# 依赖某些组件，组件Init必然在依赖组件Init之后
def dependency(*component_list):
    def _dependency(klass):
        klass.__dependency__ = set()
        '''
        bases = []
        #bases = Utils.GetClassAllBases(klass)
        for base in klass.__bases__:
            if "__dependency__" in base.__dict__:
                for name in base.__dependency__:
                    klass.__dependency__.add(name)
        '''

        for name in component_list:
            if type(name) == str:
                component = ComponentRegister.get_component(name)
            elif inspect.isclass(name):
                component = name
            else:
                component = None
            if inspect.isclass(component):
                name = component.VAR_NAME
                klass.__dependency__.add(name)
            else:
                raise RuntimeError("Something Strange Slip Into Components: %s (type %s)." % (name, type(component)))

        return klass

    return _dependency


# # 需要某些组件或者其扩展组件
# def require(*component_list):
#     def _require(klass):
#         klass.__require__ = set()
#         '''
#         bases = []
#         #bases = Utils.GetClassAllBases(klass)
#         for base in klass.__bases__:
#             if "__require__" in base.__dict__:
#                 for name in base.__require__:
#                     klass.__require__.add(name)
#         '''
#         for name in component_list:
#             if type(name) == str:
#                 component = ComponentRegister.get_component(name)
#             elif inspect.isclass(name):
#                 component = name
#             else:
#                 component = None
#             if inspect.isclass(component):
#                 name = component.VAR_NAME
#                 klass.__require__.add(name)
#             else:
#                 raise RuntimeError("Something Strange Slip Into Components: %s (type %s)." % (name, type(component)))
#         return klass
#
#     return _require


'''
拓扑排序
graph dict依赖图 {node1:set(depend1, depend2), }
result list排序后的值 [node_a, node_b, ...]

从入度为0的点开始逐个处理
'''


def toposort(graph, class_name):
    graph = copy.deepcopy(graph)
    result = []
    while True:
        # 获取入度为0的值
        zero_node = None
        for node, depends in graph.items():
            if len(depends) == 0:
                zero_node = node
                break

        if zero_node:
            result.append(zero_node)

            graph.pop(zero_node)
            for node, depends in graph.items():
                if zero_node in depends:
                    depends.remove(zero_node)
        elif len(graph) == 0:
            break
        else:
            for node, depends in graph.items():
                print("Node: {0}, Dependency: {1}".format(node, depends))
            raise RuntimeError("toposort graph has loop or has undefined node %s" % class_name)

    return result
