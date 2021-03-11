# -*- coding: utf-8 -*-
"""
主要功能就是扫描一个目录 , 并把下面所有python模块中属于指定类子类的类找出来
方便我们把服务端的entity进行注册
"""
import os
import sys
import pkgutil
# import importlib

# import syspathhelper
# from _importlib_modulespec import ModuleSpec


def _get_module_files(module_dir):
    module_name_set = set()
    try:
        files = os.listdir(module_dir)
    except:
        print(
        "error in generate_module_list  for directory:", module_dir)
        return ()
    for fileName in files:
        list = fileName.split('.')
        if len(list) == 2:
            # module_name = list[0]
            extension = list[1]
            if extension in ("py", "pyc"):
                module_name_set.add(list[0])
    module_name_set.discard('__init__')
    return module_name_set


def _get_module_list(module_dir):
    """得到目录下所有模块的列表"""
    module_name_set = _get_module_files(module_dir)
    module_list = []
    # syspathhelper.addpath(module_dir)
    sys.path.append(module_dir)
    for moduleName in module_name_set:
        try:
            mod = __import__(moduleName, fromlist=[''])
            if mod:
                module_list.append(mod)
        except:
            print(
            "error in generate_module_list .", moduleName)
            import traceback

            traceback.print_exc()
            continue
    print(
    "generate_module_list ", module_list)
    return module_list


def _load_all_modules_from_dir(dirname):
    module_set = set()
    # syspathhelper.addpath(dirname)
    sys.path.append(dirname)
    for importer, package_name, _ in pkgutil.walk_packages([dirname]):
        if package_name not in sys.modules:
            mod = importer.find_module(package_name)
            module = mod.load_module(package_name)
            module_set.add(module)
        else:
            module_set.add(sys.modules[package_name])
    return module_set


def _get_class_list(module, entity_base_class):
    """得到模块里面所有属于指定类子类的类"""
    class_list = []
    for name in dir(module):
        attr = getattr(module, name)
        if isinstance(attr, type) and issubclass(attr, entity_base_class):
            class_list.append(attr)
    return class_list


def scan_entity_classes(module_dir, entity_base_class):
    """主要功能就是扫描一个目录 , 并把下面所有python模块中属于指定类子类的类找出来"""
    class_dict = {}
    for module in _load_all_modules_from_dir(module_dir):
        clist = _get_class_list(module, entity_base_class)
        for claz in clist:
            class_dict[claz.__name__] = claz
    return class_dict


def walk_packages(path=None, prefix='', pkg_filter=None):
    # copy from pkgutil
    def seen(p, m={}):
        if p in m:
            return True
        m[p] = True

    for importer, name, ispkg in pkgutil.iter_modules(path, prefix):
        if pkg_filter and not pkg_filter(name):
            continue
        yield importer, name, ispkg

        if ispkg:
            try:
                __import__(name)
            except ImportError:
                pass
            except Exception:
                raise
            else:
                path = getattr(sys.modules[name], '__path__', None) or []

                # don't traverse path items we've seen before
                path = [p for p in path if not seen(p)]

                for item in walk_packages(path, name + '.', pkg_filter):
                    yield item


def _load_all_submodules(package_name):
    # import imp
    from importlib import util
    import importlib
    # file_obj, pathname, description = imp.find_module(package_name)
    _spec = importlib.util.find_spec(package_name)
    if _spec is None:
    # if file_obj:
        raise ImportError('Not a package: %r', package_name)
    module_set = set()
    # for _sub_m in _spec.submodule_search_locations:


    name_prefix = package_name + '.'
    for outer_importer in pkgutil.iter_importers():
        if outer_importer is None:
            continue
        module = outer_importer.find_module(package_name)
        if not module:
            continue
        # load base module
        if package_name not in sys.modules:
            outer_importer.find_module(package_name).load_module(package_name)
        # if not hasattr(outer_importer, 'path'):
        #     continue
        # for importer, name, _ in walk_packages(outer_importer.path, pkg_filter=lambda n: n.startswith(package_name)):
        for importer, name, _ in walk_packages(_spec.submodule_search_locations, pkg_filter=lambda n: n.startswith(package_name)):
            if not name.startswith(name_prefix):
                continue
            if name not in sys.modules:
                module = importer.find_module(name).load_module(name)
                module_set.add(module)
            else:
                module_set.add(sys.modules[name])
    return module_set


def scan_entity_package(package_name, entity_base_class):
    """从给定package下面搜寻模块，并注册类型"""
    class_dict = {}
    for _cls in entity_base_class.__subclasses__():
        class_dict[_cls.__name__] = _cls
    # for module in _load_all_submodules(package_name):
    #     clist = _get_class_list(module, entity_base_class)
    #     for claz in clist:
    #         class_dict[claz.__name__] = claz
    return class_dict


if __name__ == '__main__':
    from common.component.Component import Component
    from component import avatar

    # scan_entity_package('component', Component)
    # clist = _get_class_list(avatar, Component)
    from component.avatar import CompAvatarTest
    clist = _get_class_list(CompAvatarTest, Component)
    ss = Component.__subclasses__()
    pass