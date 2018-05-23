class RealTimeSrv
{
public:
	static std::unique_ptr< RealTimeSrv >	sInstance;

	static bool StaticInit();

	void DoFrame();

	int Run();

	void HandleNewClient( ClientProxyPtr inClientProxy );

	void	SpawnCharacterForPlayer( int inPlayerId );

	virtual ~RealTimeSrv();

	void Simulate();

#ifndef _WIN32
	static int BecomeDaemon();
#endif

private:
	RealTimeSrv();

	bool	InitNetworkMgr();

private:

	bool	mShouldKeepRunning;

};