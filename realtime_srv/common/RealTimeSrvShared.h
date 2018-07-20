#ifndef REALTIME_SRV_SHARED_H
#define REALTIME_SRV_SHARED_H


#include "realtime_srv/common/RealtimeSrvSharedMin.h"


#include "realtime_srv/net/Packet.h"
#include "realtime_srv/net/PktHandler.h"

#include "realtime_srv/math/Vector3.h"
#include "realtime_srv/math/Quaternion.h"
#include "realtime_srv/math/Matrix3x3.h"
#include "realtime_srv/math/Vector2.h"

#include "realtime_srv/math/RealtimeSrvMath.h"

#include "realtime_srv/common/RealtimeSrvHelper.h"
#include "realtime_srv/common/RealtimeSrvMacro.h"
#include "realtime_srv/common/RealtimeSrvTiming.h"

#include "realtime_srv/rep/BitStream.h"

#include "realtime_srv/game_obj/GameObj.h"

#include "realtime_srv/rep/ReplicationCmd.h"
#include "realtime_srv/game_obj/World.h"

#include "realtime_srv/rep/InflightPacket.h"
#include "realtime_srv/rep/AckBitField.h"
#include "realtime_srv/rep/DeliveryNotifyMgr.h"
#include "realtime_srv/rep/ReplicationMgr.h"

#include "realtime_srv/game_obj/InputState.h"
#include "realtime_srv/game_obj/Action.h"
#include "realtime_srv/game_obj/ActionList.h"

#include "realtime_srv/net/ClientProxy.h"
#include "realtime_srv/net/NetworkMgr.h"


#endif // REALTIME_SRV_SHARED_H