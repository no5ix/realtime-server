
// 是否要显示调试打印信息
#define ACTION_SHOW_DEBUG_MESSAGE					true

namespace Utility
{
	string GetCommandLineArg( int inIndex );

	string Sprintf( const char* inFormat, ... );

	void	Log( const char* inFormat );
	void	Log( const char* inFormat, ... );
}

#define LOG( ... ) Utility::Log( __VA_ARGS__ );