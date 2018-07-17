#include "Character.h"
#include "CharacterLuaBind.h"

#ifdef IS_LINUX

using namespace realtime_srv;

CharacterLuaBind::CharacterLuaBind()
{
	lua_.new_usertype<Character>( "Character"
		, "NewCharacter", []() { return new Character; }
		, "SetLocation", sol::overload( sol::resolve<void( const float x,
			const float y, const float z )>( &Character::SetLocation ) )
		, "SetRotation", sol::overload( sol::resolve<void( const float x,
			const float y, const float z )>( &Character::SetRotation ) )
		);
}

Character* CharacterLuaBind::DoFile()
{
	lua_.script_file( "for_ue4_demo/Character.lua" );
	sol::object keep_alive = lua_["newCharacter"];
	Character* newCharacter = keep_alive.as<Character*>();
	return newCharacter;
}

#endif // IS_LINUX