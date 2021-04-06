from LobbyEntity import LobbyEntity
from common.component.ComponentSupport import ComponentSupport
from common.component.Component import components
from component.avatar.CompAvatarTest import CompAvatarTest


# from core.servercommon.persistentdecorator import Persistent


# @Persistent
@components(
    # CompAvatarBase,
    CompAvatarTest,
    # CompDungeon, CompStatistic, CompAppearance,
    # CompStore, CompPay, CompFriend, CompAchieve, CompInventory,
    # CompChat, CompTrainAbility, CompSpriteSystem, CompTeam, CompMessage,
    # CompAvatarGMCommand, CompSDK, CompRoom, CompActivity, CompReward,
    # CompMail, CompCompensation, CompQuestionnaire, CompNewbie, CompRank,
    # CompCounter, CompRewardRestrict, CompBattleItem, CompPrepare,
    # CompGuild, CompNameCard, CompFunctionUnlock, CompItemManager, CompHero,
    # CompCompetition, CompGradeLine, CompMedal, CompReport, CompAuthority, CompNewbieTask,
    # CompTitle, CompCCLive, CompBattlePass, CompSpecialItem, CompMasterValue, CompMonthCard,
    # CompFlashSale, CompExchangeCode, CompBft
)
class Avatar(LobbyEntity):

    def __init__(self):
        # super(BattleEntity).__init__()
        # super(ComponentSupport).__init__()

        LobbyEntity.__init__(self)
        # ComponentSupport.__init__(self)
