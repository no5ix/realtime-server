#include "ActionServerPCH.h"




string GetCommandLineArg( int inIndex );



#if _WIN32
int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow )
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );

	string portString = GetCommandLineArg( 1 );
	uint16_t port = stoi( portString );

	NetworkManager *serverInst = new NetworkManager();
	serverInst->Init( port );

	while (1)
	{
		serverInst->ProcessIncomingPackets();
	}
	UDPSocket::CleanUp();

	return 0;

}
#else
const char** __argv;
int __argc;

int main( int argc, const char** argv )
{

	__argc = argc;
	__argv = argv;

	string portString = GetCommandLineArg( 1 );
	uint16_t port = stoi( portString );

	NetworkManager *serverInst = new NetworkManager();
	serverInst->Init( port );

	while (1)
	{
		serverInst->ProcessIncomingPackets();
	}

	return 0;
}
#endif


string GetCommandLineArg( int inIndex )
{
	if (inIndex < __argc)
	{
		return string( __argv[inIndex] );
	}

	return string();
}
