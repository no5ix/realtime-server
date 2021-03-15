import pkgutil
import importlib

from importlib import util


def walk_pkg(path, pkg_name):
    for module_loader, name, is_pkg in pkgutil.iter_modules(path):
        # importlib.import_module('.' + name, __package__)
        _cur_module = importlib.import_module('.' + name, pkg_name)
        if is_pkg:
            pass
            _path = _cur_module.__path__
            walk_pkg(_path, ''.join((pkg_name, '.', name)))


def all_subclasses(cls):
    return set(cls.__subclasses__()).union(
        [s for c in cls.__subclasses__() for s in all_subclasses(c)])


def scan_entity_package(package_name, entity_base_class):
    _spec = importlib.util.find_spec(package_name)
    if _spec is None:
        raise ImportError('Not a package: %r', package_name)
    walk_pkg(_spec.submodule_search_locations, package_name)
    cls_dict = {_cls.__name__: _cls for _cls in all_subclasses(entity_base_class)}
    return cls_dict


if __name__ == '__main__':
    from common.component.Component import Component
    print(scan_entity_package('component', Component))
