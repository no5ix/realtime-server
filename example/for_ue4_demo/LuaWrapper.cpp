#include "LuaWrapper.h"



LuaWrapper::LuaWrapper( const std::string &filename, ToluappOpenCallback toluappOpenCb )
	: lua_( nullptr )
{
	// lua5.3
	//this->Lua = luaL_newstate();
	//luaL_openlibs( this->Lua );

	//if ( this->Lua && ( filename.length() > 0 ) )
	//{
	//	luaL_loadfile( this->Lua, filename.c_str() );
	//	lua_pcall( this->Lua, 0, 0, 0 );
	//}


	// lua5.1
	this->lua_ = lua_open();
	luaL_openlibs( this->lua_ );
	toluappOpenCb( this->lua_ );

	if ( this->lua_ && ( filename.length() > 0 ) )
	{
		luaL_dofile( this->lua_, filename.c_str() );
	}
}

bool LuaWrapper::GetGlobal( const char *name )
{
	if ( ( !this->lua_ ) || ( !name ) )
		return false;
	lua_getglobal( this->lua_, name );
	return true;
}

void LuaWrapper::Close()
{
	if ( !this->lua_ )
		return;
	lua_close( this->lua_ );
	this->lua_ = nullptr;
}

LuaWrapper::~LuaWrapper() { this->Close(); }
