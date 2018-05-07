#include "RealTimeSrvPCH.h"


#if !_WIN32
extern const char** __argv;
extern int __argc;
void OutputDebugString( const char* inString )
{
	printf( "%s", inString );
}
#endif

string Utility::GetCommandLineArg( int inIndex )
{
	if (inIndex < __argc)
	{
		return string( __argv[inIndex] );
	}

	return string();
}


string Utility::Sprintf( const char* inFormat, ... )
{
	//not thread safe...
	static char temp[4096];

	va_list args;
	va_start( args, inFormat );

#if _WIN32
	_vsnprintf_s( temp, 4096, 4096, inFormat, args );
#else
	vsnprintf( temp, 4096, inFormat, args );
#endif
	return string( temp );
}

// void Utility::Log( const char* inFormat )
// {
// 	OutputDebugString( inFormat );
// 	OutputDebugString( "\n" );
// }

void Utility::Log( const char* inFormat, ... )
{
	if ( !ACTION_SHOW_DEBUG_MESSAGE )
		return;

	//not thread safe...
	static char temp[4096];

	va_list args;
	va_start( args, inFormat );

#if _WIN32
	_vsnprintf_s( temp, 4096, 4096, inFormat, args );
#else
	vsnprintf( temp, 4096, inFormat, args );
#endif
	OutputDebugString( temp );
	OutputDebugString( "\n" );
}

