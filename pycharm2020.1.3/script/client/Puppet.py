
from common.component.Component import components
from common.component.ComponentSupport import ComponentSupport
from client.component.puppet.CompPuppetTest import CompPuppetTest
from client.BattleEntity import BattleEntity


@components(
    CompPuppetTest
)
class Puppet(BattleEntity, ComponentSupport):
    super().__init__()
    pass
