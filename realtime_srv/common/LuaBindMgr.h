#pragma once

#include <lualib/sol.hpp>	

namespace realtime_srv
{


//remember to add ` add_definitions( -std = c++1y ) ` to ur CMakeLists.txt
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