#pragma once
#include <realtime_srv/RealtimeServer.h>

#include <lualib/sol.hpp>

class Character;

class CharacterLuaBind
{

public:
	CharacterLuaBind();
	Character* DoFile();
	sol::state& GetOwner() { return lua_; }

private:
	sol::state lua_;

};