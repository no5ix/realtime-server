import inspect

import typing
from asyncio.futures import Future

from RpcHandler import rpc_func
from common.component.ComponentSupport import ComponentSupport
from server_entity.ServerEntity import ServerEntity
from RpcHandler import RpcHandler


class PuppetEntity(ServerEntity, ComponentSupport):
    
    def __init__(self):
        # super(PuppetEntity, self).__init__()
        self.remote_entity = RemoteEntity(self)
        ServerEntity.__init__(self)
        ComponentSupport.__init__(self)

    # @rpc_method(CLI_TO_SRV, (Str('m'), Dict('p')))
    @rpc_func
    async def puppet_entity_message(self, method_name, *args, **kwargs):
        # try:
        #     method_name = RpcIndexer.INDEX2RPC[method_name]
        # except KeyError:
        #     self.logger.error('Failed to decode method_name for uid=%s index=%s', self.uid, method_name)
        #     return
        # puppet = self._puppet
        # if not puppet:
        #     # self.logger.error('Failed to get puppet, method_name=%s', method_name)
        #     print('Failed to get puppet, method_name=%s', method_name)
        #     return

        _comp_method_list = method_name.split(".")
        if len(_comp_method_list) == 1:
            _method = getattr(self, method_name, None)
        else:
            _method = getattr(
                self.get_component(_comp_method_list[0]),
                _comp_method_list[1], None)
        method_res = await RpcHandler.handle_request_notify_rpc(
            _method, method_name, args, kwargs)
        return method_res
        # method_list = method_name.split('.')
        # if len(method_list) == 1:
        #     ent = self
        #     name = method_name
        # else:
        #     ent = self.get_component(method_list[0])
        #     name = method_list[1]
        # method = getattr(ent, name, None)
        # if method is None:
            # optimized after tick，降低客户端延迟，不然需要等下一次tick才做统计
            # if puppet.delay_calls:
            #     puppet.delay_calls.callback('opt-at', 0.001, puppet.flush_aoi_data)
            # method(ent, parameters)
            # return method(parameters)


class RemoteComp:

    def __init__(self, comp_name: str, server_entity: PuppetEntity):
        super(RemoteComp, self).__init__()
        self._server_ent = server_entity  # type: PuppetEntity
        self._comp_name = comp_name

    def __getattr__(self, rpc_name: str):

        def temp_rpc_func(
                *args, rpc_callback=None, rpc_need_reply=True, rpc_reply_timeout=2,
                rpc_func_name=rpc_name, **kwargs) -> Future:
            # print(f"rpc_func_name1: {r}")
            # print(*args, **kwargs)
            # print(*args)

            # caller_module = inspect.stack()[1][0]
            # _caller_comp_name = caller_module.f_locals["self"].__class__.__name__
            # final_rpc_name = ".".join((_caller_comp_name, rpc_func_name))
            final_rpc_name = ".".join((self._comp_name, rpc_func_name))
            puppet_rpc_name = "puppet_entity_message"
            args = list(args)
            args.insert(0, final_rpc_name)
            return self._server_ent.call_remote_method(
                # final_rpc_name, args, kwargs, need_reply, reply_timeout)
                puppet_rpc_name, args, kwargs, rpc_callback, rpc_need_reply, rpc_reply_timeout)

        return temp_rpc_func


class RemoteEntity:

    def __init__(self, server_entity):
        self._server_ent = server_entity
        self._cur_comp_name = ""

    def __getattr__(self, rpc_name: str):
        # print(f"gett{rpc_func_name}")

        if rpc_name.startswith("Comp"):
            self._cur_comp_name = rpc_name
            return self
        else:
            def temp_rpc_func(
                    *args, rpc_callback=None, rpc_need_reply=True, rpc_reply_timeout=2,
                    rpc_func_name=rpc_name, **kwargs) -> Future:
                # print(f"rpc_func_name1: {r}")
                # print(*args, **kwargs)
                # print(*args)
                if self._cur_comp_name == "":
                    final_rpc_name = rpc_func_name
                else:
                    final_rpc_name = ".".join((self._cur_comp_name, rpc_func_name))
                    self._cur_comp_name = ''
                # return self._server_ent.call_remote_method(
                #     final_rpc_name, args, kwargs, need_reply, reply_timeout)

                # final_rpc_name = ".".join((self._comp_name, rpc_func_name))
                puppet_rpc_name = "puppet_entity_message"
                args = list(args)
                args.insert(0, final_rpc_name)
                return self._server_ent.call_remote_method(
                    # final_rpc_name, args, kwargs, need_reply, reply_timeout)
                    puppet_rpc_name, args, kwargs, rpc_callback, rpc_need_reply, rpc_reply_timeout)
            return temp_rpc_func
            # return lambda *args, rpc=item_name: self._server_ent.call_remote_method(rpc, *args)

