# from typing import Optional
from typing import Union

from client.PuppetBindEntity import PuppetBindEntity
from common.component.Component import components
from common.component.ComponentSupport import ComponentSupport
from client.component.puppet.CompPuppetTest import CompPuppetTest
from client.BattleEntity import BattleEntity


@components(
    CompPuppetTest
)
class Puppet(BattleEntity, ComponentSupport):

    def __init__(self):
        BattleEntity.__init__(self)
        ComponentSupport.__init__(self)
        # self.bind_entity = None  # type: # Optional[PuppetBindEntity]
        # 以下代码与上一行等价
        self.bind_entity = None  # type: Union[PuppetBindEntity, None]

    def init_from_dict(self, bdict: dict):
        ComponentSupport.init_from_dict(self, bdict)

    def set_bind_entity(self, be: PuppetBindEntity):
        self.bind_entity = be
        self.bind_entity.set_bind_ok()
