class RealTimeSrv
{
public:
	static std::unique_ptr< RealTimeSrv >	sInstance;

	static bool StaticInit();

	int Run();

	void HandleNewClient( ClientProxyPtr inClientProxy );

	void	SpawnCharacterForPlayer( int inPlayerId );

	virtual ~RealTimeSrv();

	void SimulateRealWorld();


#ifdef NEW_EPOLL_INTERFACE
	static int BecomeDaemon();
#else
	void DoFrame();
#endif


private:
	RealTimeSrv();
	bool	InitNetworkMgr();

private:

	bool	mShouldKeepRunning;
};