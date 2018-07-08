#pragma once
#include <realtime_srv/RealtimeServer.h>

#ifdef IS_LINUX

#include <lualib/LuaBindMgr.h>


class CharacterLuaBind : public realtime_srv::LuaBindMgr
{
public:
	CharacterLuaBind();
	CharacterPtr DoFile();
};

#endif // IS_LINUX