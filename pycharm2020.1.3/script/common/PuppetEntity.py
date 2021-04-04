import inspect

from server_entity.ServerEntity import ServerEntity


class PuppetEntity(ServerEntity):
    
    def __init__(self):
        # super(PuppetEntity, self).__init__()
        ServerEntity.__init__(self)
        self.remote_entity = RemoteEntity(self)
        self.remote_comp = RemoteComp(self)


class RemoteComp:

    def __init__(self, server_entity):
        super(RemoteComp, self).__init__()
        self._server_ent = server_entity

    def __getattr__(self, rpc_name: str):

        def temp_rpc_func(*args, rpc_func_name=rpc_name):
            # print(f"rpc_func_name1: {r}")
            # print(*args, **kwargs)
            # print(*args)

            caller_module = inspect.stack()[1][0]
            # if "self" in caller_module.f_locals:
            _caller_comp_name = caller_module.f_locals["self"].__class__.__name__
            # _caller_comp_name = ""
            final_rpc_name = ".".join((_caller_comp_name, rpc_func_name))
            self._server_ent.call_remote_method(final_rpc_name, [*args])

        return temp_rpc_func


class RemoteEntity:

    def __init__(self, server_entity):
        super().__init__()
        self._server_ent = server_entity
        self._cur_comp_name = ""

    def __getattr__(self, item_name: str):
        # print(f"gett{rpc_func_name}")

        if item_name.startswith("Comp"):
            self._cur_comp_name = item_name
            return self
        else:
            def temp_rpc_func(*args, rpc_func_name=item_name):
                # print(f"rpc_func_name1: {r}")
                # print(*args, **kwargs)
                # print(*args)
                if self._cur_comp_name == "":
                    final_rpc_name = rpc_func_name
                else:
                    final_rpc_name = ".".join((self._cur_comp_name, rpc_func_name))
                    self._cur_comp_name = ''
                self._server_ent.call_remote_method(final_rpc_name, [*args])
            return temp_rpc_func
            # return lambda *args, rpc=item_name: self._server_ent.call_remote_method(rpc, *args)

    # def __await__(self):
