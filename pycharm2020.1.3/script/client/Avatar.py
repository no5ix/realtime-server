# from typing import Optional
from typing import Union

# from client.PuppetBindEntity import PuppetBindEntity
from BattleEntity import BattleEntity
from common.component.Component import components
from common.component.ComponentSupport import ComponentSupport
from client.component.avatar.CompAvatarTest import CompAvatarTest
# from client.BattleEntity import BattleEntity


@components(
    CompAvatarTest
)
class Avatar(BattleEntity, ):

    def __init__(self):
        BattleEntity.__init__(self)
        # ComponentSupport.__init__(self)
        # self.bind_entity = None  # type: # Optional[PuppetBindEntity]
        # 以下代码与上一行等价
        # self.bind_entity = None  # type: Union[PuppetBindEntity, None]

    def init_from_dict(self, bdict: dict):
        ComponentSupport.init_from_dict(self, bdict)

    # def set_bind_entity(self, be: PuppetBindEntity):
    #     self.bind_entity = be
    #     self.bind_entity.set_bind_ok()
