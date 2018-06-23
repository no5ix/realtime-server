class ExampleSrvForUe4Demo
{
public:
	static std::unique_ptr< ExampleSrvForUe4Demo >	sInst;

	static bool StaticInit();

	int Run();


	void HandleNewClient( ClientProxyPtr inClientProxy );
	void SpawnCharacterForPlayer( int inPlayerId );

#ifndef IS_LINUX
	void SimulateRealWorld();
#endif

private:
	ExampleSrvForUe4Demo();
	void InitNetworkMgr();

private:

	bool	mShouldKeepRunning;
};