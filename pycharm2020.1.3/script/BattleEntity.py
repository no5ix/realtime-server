# from typing import Union

# from PuppetBindEntity import PuppetBindEntity
from common.PuppetEntity import PuppetEntity
from core.common.RpcMethodArgs import Str, Dict
from core.common.RpcSupport import rpc_method, CLI_TO_SRV
from server_entity.ServerEntity import ServerEntity


class BattleEntity(PuppetEntity):

    def __init__(self):
        PuppetEntity.__init__(self)

    # def call_server_method(self, method_name, parameters):
    # def call_server_method(self, method_name, *args):
    #     self.call_remote_method(
    #         # 'battle_entity_message', {'m': method_name, 'p': parameters})
    #         'battle_entity_message', [method_name, *args])
    #
    # def call_client_method(self, method_name, *args):
    #     self.call_remote_method(
    #         'battle_entity_message', [method_name, *args])

    # @rpc_method(CLI_TO_SRV, (Str('m'), Dict('p')))
    # def battle_entity_message(self, method_name, parameters):
    #     # try:
    #     #     method_name = RpcIndexer.INDEX2RPC[method_name]
    #     # except KeyError:
    #     #     self.logger.error('Failed to decode method_name for uid=%s index=%s', self.uid, method_name)
    #     #     return
    #     # puppet = self._puppet
    #     # if not puppet:
    #     #     # self.logger.error('Failed to get puppet, method_name=%s', method_name)
    #     #     print('Failed to get puppet, method_name=%s', method_name)
    #     #     return
    #     method_list = method_name.split('.')
    #     if len(method_list) == 1:
    #         ent = self
    #         name = method_name
    #     else:
    #         ent = self.get_component(method_list[0])
    #         name = method_list[1]
    #     method = getattr(ent, name, None)
    #     if method:
    #         # optimized after tick，降低客户端延迟，不然需要等下一次tick才做统计
    #         # if puppet.delay_calls:
    #         #     puppet.delay_calls.callback('opt-at', 0.001, puppet.flush_aoi_data)
    #         # method(ent, parameters)
    #         method(parameters)
