#include "ActionServerPCH.h"

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


	// for test start

	//Character* c = new Character;
	//float f = 0.00123f;
	//float x = 107.582947f;
	//float y = 426.617615f;
	//float z = 0.000000f;
	//int count = 10000;
	//while ( count-- )
	//{
	//	
	//	c->ActionControlInputVector = Vector3( x, y, z );
	//	c->ApplyControlInputToVelocity( f );
	//	LOG( "( c->GetVelocity() ).X << %f,  << ( c->GetVelocity() ).Y << %f,  << ( c->GetVelocity() ).Z << %f",
	//		( c->GetVelocity() ).X, ( c->GetVelocity() ).Y, ( c->GetVelocity() ).Z
	//	);
	//	x++;
	//	y++;
	//	z++;
	//}
	//delete c;

	//return 1;

	// for test end

	if (Server::StaticInit())
	{
		return Server::sInstance->Run();
	}
	else
	{
		//error
		return 1;
	}
}
//#endif
