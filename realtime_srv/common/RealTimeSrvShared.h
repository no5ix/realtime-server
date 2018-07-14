#ifndef REALTIME_SRV_SHARED_H
#define REALTIME_SRV_SHARED_H


//#ifdef __linux__
	#define IS_LINUX
//#else
//	#ifdef _WIN32
//		#define IS_WIN
//	#else
//		#define IS_MAC
//	#endif
//#endif

#ifdef IS_WIN
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX

	#include "Windows.h"
	#include "WinSock2.h"
	#include "Ws2tcpip.h"
	typedef int socklen_t;
#else 
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <sys/types.h>
	#include <netdb.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <unistd.h>

	#ifdef IS_LINUX
		#include <signal.h>
		#include <sys/stat.h>
		#include <pthread.h>
		#include <arpa/inet.h>
		#include <sys/epoll.h>
		#include <cstdarg>
	#endif //IS_LINUX

	typedef int SOCKET;
	const int NO_ERROR = 0;
	const int INVALID_SOCKET = -1;
	const int WSAECONNRESET = ECONNRESET;
	const int WSAEWOULDBLOCK = EAGAIN;
	const int SOCKET_ERROR = -1;
#endif

#include <time.h>
#include <functional>
#include <stdint.h>
#include <memory>

#include <cstring>
#include <cmath>

#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <list>
#include <queue>
#include <deque>
#include <set>
#include <unordered_set>
#include <cassert>

using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::vector;
using std::queue;
using std::list;
using std::deque;
using std::unordered_map;
using std::map;
using std::string;
using std::set;
using std::unordered_set;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;


#include "realtime_srv/common/noncopyable.h"
#include "realtime_srv/math/Vector3.h"
#include "realtime_srv/math/Quaternion.h"
#include "realtime_srv/math/Matrix3x3.h"
#include "realtime_srv/math/Vector2.h"

#include "realtime_srv/math/RealtimeSrvMath.h"

#include "realtime_srv/common/RealtimeSrvHelper.h"
#include "realtime_srv/common/RealtimeSrvMacro.h"
#include "realtime_srv/common/RealtimeSrvTiming.h"

#include "realtime_srv/net/SockAddrInterf.h"
#include "realtime_srv/net/UdpSockInterf.h"


#include "realtime_srv/rep/BitStream.h"


#include "realtime_srv/game_obj/GameObj.h"

#include "realtime_srv/rep/ReplicationCmd.h"
#include "realtime_srv/game_obj/World.h"

#include "realtime_srv/rep/InFlightPacket.h"
#include "realtime_srv/rep/AckBitField.h"
#include "realtime_srv/rep/DeliveryNotifyMgr.h"
#include "realtime_srv/rep/ReplicationMgr.h"

#include "realtime_srv/game_obj/InputState.h"
#include "realtime_srv/game_obj/Action.h"
#include "realtime_srv/game_obj/ActionList.h"


#include "realtime_srv/net/ClientProxy.h"
#include "realtime_srv/net/NetworkMgr.h"



#endif // REALTIME_SRV_SHARED_H