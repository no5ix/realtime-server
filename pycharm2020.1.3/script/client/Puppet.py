
from common.component.Component import components
from common.component.ComponentSupport import ComponentSupport
from client.component.puppet.CompPuppetTest import CompPuppetTest
from client.BattleEntity import BattleEntity


@components(
    CompPuppetTest
)
class Puppet(BattleEntity, ComponentSupport):

    def __init__(self):
        super().__init__()
        pass

    def init_from_dict(self, bdict):
        ComponentSupport.init_from_dict(self, bdict)
