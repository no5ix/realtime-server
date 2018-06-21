#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX

	#include "Windows.h"
	#include "WinSock2.h"
	#include "Ws2tcpip.h"
	typedef int socklen_t;
	//typedef char* receiveBufer_t;
#else
	#include <signal.h>
	#include <sys/stat.h>
	#include <pthread.h>
	#include <arpa/inet.h>
	#include <sys/epoll.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <sys/types.h>
	#include <netdb.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <cstdarg>
	//typedef void* receiveBufer_t;
	typedef int SOCKET;
	const int NO_ERROR = 0;
	const int INVALID_SOCKET = -1;
	const int WSAECONNRESET = ECONNRESET;
	const int WSAEWOULDBLOCK = EAGAIN;
	const int SOCKET_ERROR = -1;
#endif

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
using std::vector;
using std::queue;
using std::list;
using std::deque;
using std::unordered_map;
using std::map;
using std::string;
using std::set;
using std::unordered_set;

#include "realtime_srv/math/Vector3.h"
#include "realtime_srv/math/Quaternion.h"
#include "realtime_srv/math/Matrix3x3.h"
#include "realtime_srv/math/Vector2.h"

#include "realtime_srv/math/RealTimeSrvMath.h"

#include "realtime_srv/common/RealTimeSrvHelper.h"
#include "realtime_srv/common/Macro.h"
#include "realtime_srv/common/RealTimeSrvTiming.h"

#include "realtime_srv/net/SocketAddrInterface.h"
#include "realtime_srv/net/UDPSocketInterface.h"


#include "realtime_srv/rep/BitStream.h"


#include "realtime_srv/entity/Entity.h"

#include "realtime_srv/net/NetworkMgr.h"

#include "realtime_srv/rep/ReplicationCmd.h"
#include "realtime_srv/rep/InFlightPacket.h"
#include "realtime_srv/rep/AckBitField.h"
#include "realtime_srv/rep/DeliveryNotifyMgr.h"
#include "realtime_srv/rep/ReplicationMgr.h"

#include "realtime_srv/entity/InputState.h"
#include "realtime_srv/entity/Action.h"
#include "realtime_srv/entity/ActionList.h"
