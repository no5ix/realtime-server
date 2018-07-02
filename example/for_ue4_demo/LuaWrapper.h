#pragma once

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "luaconf.h"  
}

#include <lua.hpp>
#include <string>

#include "realtime_srv/common/RealtimeSrvShared.h"



class LuaWrapper
{

public:

	typedef std::function<int( lua_State* )> ToluappOpenCallback;

	LuaWrapper( const std::string &filename, ToluappOpenCallback toluappOpenCb );

	void Close();

	~LuaWrapper();

private:

	bool GetGlobal( const char *name );

	lua_State *lua_;
};
