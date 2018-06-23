#include <realtime_srv/RealtimeServer.h>


class ExampleSrvForUe4Demo
{
public:
	static std::unique_ptr< ExampleSrvForUe4Demo >	sInst;

	static bool StaticInit();

	void Run();

	GameObjPtr SpawnNewCharacterForPlayer( int inPlayerId );

private:
	ExampleSrvForUe4Demo();
};