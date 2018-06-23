#include "ExampleSrvForUe4Demo.h"


int main( int argc, const char** argv )
{
	RealtimeSrvHelper::SaveCommandLineArg( argc, argv );

	if (ExampleSrvForUe4Demo::StaticInit())
	{
		ExampleSrvForUe4Demo::sInst->Run();
		return 0;
	}
	else
	{
		//error
		return -1;
	}
}