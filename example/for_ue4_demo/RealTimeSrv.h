class RealTimeSrv
{
public:
	static std::unique_ptr< RealTimeSrv >	sInstance;

	static bool StaticInit();

	int Run();


	void HandleNewClient( ClientProxyPtr inClientProxy );
	void SpawnCharacterForPlayer( int inPlayerId );

#ifndef IS_LINUX
	void SimulateRealWorld();
#endif

private:
	RealTimeSrv();
	void InitNetworkMgr();

private:

	bool	mShouldKeepRunning;
};