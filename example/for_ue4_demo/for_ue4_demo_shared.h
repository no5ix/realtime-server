#include <realtime_srv/common/RealtimeSrvShared.h>



#ifdef IS_LINUX

#define COMMAND_LINE_ARG_DAEMON_INDEX					(1)
#define COMMAND_LINE_ARG_PORT_INDEX						(2)
#define COMMAND_LINE_ARG_LATENCY_INDEX					(3)
#define COMMAND_LINE_ARG_DROP_PACKET_CHANCE_INDEX		(4)	    
#define COMMAND_LINE_ARG_IS_SIMULATED_JITTER_INDEX		(5)	    

#else

#define COMMAND_LINE_ARG_PORT_INDEX						(1)
#define COMMAND_LINE_ARG_LATENCY_INDEX					(2)
#define COMMAND_LINE_ARG_DROP_PACKET_CHANCE_INDEX		(3)	    
#define COMMAND_LINE_ARG_IS_SIMULATED_JITTER_INDEX		(4)	    

#endif //IS_LINUX

//#include "ClientProxy.h"

//#include "NetworkMgr.h"

#include "World.h"
#include "EntityFactory.h"
#include "ExampleSrvForUe4Demo.h"

#include "Character.h"
