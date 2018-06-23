#include "for_ue4_demo_shared.h"

//
//#if _WIN32
//int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow )
//{
//	UNREFERENCED_PARAMETER( hPrevInstance );
//	UNREFERENCED_PARAMETER( lpCmdLine );
//
//	if (Server::StaticInit())
//	{
//		return Server::sInstance->Run();
//	}
//	else
//	{
//		//error
//		return 1;
//	}
//
//}
//#else


#ifndef _WIN32
const char** __argv;
int __argc;
#endif


int main( int argc, const char** argv )
{
#ifndef _WIN32
	__argc = argc;
	__argv = argv;
#endif
	if (ExampleSrvForUe4Demo::StaticInit())
	{
		return ExampleSrvForUe4Demo::sInst->Run();
	}
	else
	{
		//error
		return -1;
	}
}