#pragma once

#include <lualib/sol.hpp>

namespace realtime_srv
{

class LuaBindMgr
{
public:
	LuaBindMgr() { lua_.open_libraries(); }
	virtual ~LuaBindMgr() {}

	sol::state& GetOwner() { return lua_; }

protected:
	sol::state lua_;


};

}