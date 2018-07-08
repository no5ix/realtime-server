#include "Character.h"
#include "CharacterLuaBind.h"

using namespace realtime_srv;

CharacterLuaBind::CharacterLuaBind()
{
	lua_.open_libraries();

	lua_.new_usertype<Character>( "Character"
		//, sol::constructors<Character()>()
		//, "GetMaxSpeed", &Character::GetMaxSpeed
		//, "GetCameraRotation", &Character::GetCameraRotation
		//, "GetVelocity", &Character::GetVelocity
		//, "SetPlayerId", &Character::SetPlayerId
		, "StaticCreate", &Character::StaticCreate
		//, "Write", &Character::Write
		//, "GetAllStateMask", &Character::GetAllStateMask
		, "SetLocation", sol::overload( sol::resolve<void( const float x, const float y, const float z )>( &Character::SetLocation ) )
			//SetLocation( const Vector3& inLocation ))
		, "SetRotation", sol::overload( sol::resolve<void( const float x, const float y, const float z )>( &Character::SetRotation ) )
		//, "SetRotation", &Character::SetRotation
		);

	//// make usertype metatable
	//lua.new_usertype<player>( "player",

	//	// 3 constructors
	//	sol::constructors<player(), player( int ), player( int, int )>(),

	//	// typical member function that returns a variable
	//	"call_shoot", &player::call_shoot,
	//	// typical member function
	//	"boost", &player::boost,

	//	"test_call_parent_func", &player::test_call_parent_func,

	//	// gets or set the value using member variable syntax
	//	"hp", sol::property( &player::get_hp, &player::set_hp ),

	//	// read and write variable
	//	"speed", &player::speed,
	//	// can only read from, not write to
	//	"bullets", sol::readonly( &player::bullets )

	//	);



}

Character* CharacterLuaBind::DoFile()
{
	lua_.script_file( "Character.lua" );
	//Character newCharacter = lua_["newCharacter"];

	return  new Character( lua_["newCharacter"] ) ;
}

