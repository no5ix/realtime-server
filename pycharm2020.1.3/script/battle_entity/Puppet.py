from BattleEntity import BattleEntity
from common.component.Component import components
from common.component.ComponentSupport import ComponentSupport
from component.puppet.CompPuppetTest import CompPuppetTest


@components(
    CompPuppetTest
)
class Puppet(BattleEntity, ComponentSupport):
    super().__init__()
    pass

