class RealTimeSrv
{
public:
	static std::unique_ptr< RealTimeSrv >	sInstance;

	static bool StaticInit();

	int Run();

	void SimulateRealWorld();

	void HandleNewClient( ClientProxyPtr inClientProxy );
	void SpawnCharacterForPlayer( int inPlayerId );

private:
	RealTimeSrv();
	bool	InitNetworkMgr();

private:

	bool	mShouldKeepRunning;
};