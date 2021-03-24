# -*- coding: utf-8 -*-


"""
目前
增加组件的方式直接在类中增加修饰符components即可
"""

# from core.mobilelog.LogManager import LogManager
# from common.utils import tb_helper
# from common import gr

# 让entity支持Component框架
# 需要支持component框架的entity继承这个类


class ComponentSupport(object):
    def __init__(self, local_id=0):
        self._is_use_aoi = False
        self._is_aoi_property = False
        self._components = None

        # self.logger = LogManager.get_logger(self.__class__.__name__ + "." + self.__class__.__name__)

        if hasattr(self, "__sorted_components__"):
            self._components = {}
            self._create_components(local_id)
            self._init_components()

        self.is_destroying = False
        self.is_destroyed = False

    def _create_components(self, local_id):
        for name, component in self.__sorted_components__:
            # if local_id and name in gr.CPP_ECS_CLASS_LIST:
            if local_id:
                # print 'name', name, component
                com_obj = component(local_id)
            else:
                com_obj = component()
            setattr(self, name, com_obj)
            self._components[name] = com_obj

    def _init_components(self):
        # 逐个调用component的Init方法
        for name, _ in self.__sorted_components__:
            getattr(self, name).init(self)

    def _init_component_finished(self):
        # 完成所有component的初始化
        for name, _ in self.__sorted_components__:
            getattr(self, name).post_init()

    def __str__(self):
        nn = "Noname"
        if hasattr(self, "name"):
            nn = self.name
        return "<%s:%s>"%(self.__class__.__name__, nn)

    # def on_entity_destroy(self):
    #     for name, _ in reversed(self.__sorted_components__):
    #         component_obj = getattr(self, name, None)
    #         if component_obj:
    #             try:
    #                 component_obj.before_destroy()
    #             except:
    #                 tb_helper.upload(self.logger)
    #
    #     for name, _ in reversed(self.__sorted_components__):
    #         component_obj = getattr(self, name, None)
    #         if component_obj:
    #             try:
    #                 component_obj.destroy()
    #             except:
    #                 tb_helper.upload(self.logger)
    #
    #     for name, _ in reversed(self.__sorted_components__):
    #         component_obj = getattr(self, name, None)
    #         if component_obj:
    #             try:
    #                 component_obj.post_destroy()
    #             except:
    #                 tb_helper.upload(self.logger)

    def init_from_dict(self, info_dict):
        self.init_component_data_from_dict(info_dict)

    def init_component_data_from_dict(self, bdict):
        # 初始化component的数据.
        # component数据放在entity存盘bdict下的一个子document
        for name, _ in self.__sorted_components__:
            component_obj = getattr(self, name, None)
            if component_obj and hasattr(component_obj, "init_properties"):
                section_data = bdict.get(name, None)
                if hasattr(component_obj, "convert_dict_data"):
                    section_data = component_obj.convert_dict_data(section_data)
                if section_data:
                    component_obj.init_properties(section_data)
                else:
                    component_obj.init_properties({})

        self._init_component_finished()

    def reload_component_script(self):
        for name, _ in self.__sorted_components__:
            component_obj = getattr(self, name, None)
            if component_obj:
                component_obj.reload_script()

    def _traverse_component_dict(self, visitor_name):
        # 遍历component组合出一个dict
        mp_ret = {}
        for name, _ in self.__sorted_components__:
            component_obj = getattr(self, name, None)
            if component_obj is not None:
                visitor = getattr(component_obj, visitor_name, None)
                if visitor is not None:
                    ret = visitor()
                    if ret:
                        mp_ret[name] = ret
        return mp_ret

    def get_component_persistent_dict(self):
        return self._traverse_component_dict("get_persistent_properties")

    def get_component_client_dict(self):
        return self._traverse_component_dict("get_client_properties")

    def get_component_other_dict(self):
        return self._traverse_component_dict("get_other_properties")

    def get_component_info_dict(self, info_dict):
        visitor_name = "get_client_dict"
        mp_ret = info_dict
        for name, _ in self.__sorted_components__:
            component_obj = getattr(self, name, None)
            if component_obj is not None:
                visitor = getattr(component_obj, visitor_name, None)
                if visitor is not None:
                    ret = visitor()
                    if not ret:
                        continue
                    if name in info_dict:
                        mp_ret[name].update(ret)
                    else:
                        mp_ret[name] = ret
        return mp_ret

    def get_component_puppet_dict(self):
        visitor_name = "get_puppet_dict"
        mp_ret = {}
        for name, _ in self.__sorted_components__:
            component_obj = getattr(self, name, None)
            if component_obj is not None:
                visitor = getattr(component_obj, visitor_name, None)
                if visitor is not None:
                    ret = visitor()
                    if not ret:
                        continue
                    for k, v in ret.iteritems():
                        if k not in mp_ret:
                            mp_ret[k] = v
                        else:
                            mp_ret[k].update(v)
        return mp_ret

    def has_component(self):
        return self._components is not None and len(self._components) > 0

    def get_component(self, name):
        if self._components is None:
            return None
        return self._components.get(name, None)

    def destroy(self):
        self.is_destroying = True
        if hasattr(self, "__sorted_components__"):
            self.on_entity_destroy()
            self._components = None
        self.is_destroying = False
        self.is_destroyed = True

    # def set_logger(self, logger, adapter_name, extra):
    #     self.logger = logger
    #     for name, _ in self.__sorted_components__:
    #         getattr(self, name).set_logger(logger, adapter_name, extra)

    def is_like_destroy(self):
        return self.is_destroyed or self.is_destroying
