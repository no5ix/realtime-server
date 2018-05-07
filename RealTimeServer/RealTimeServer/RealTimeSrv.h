class RealTimeSrv
{
public:
	static std::unique_ptr< RealTimeSrv >	sInstance;

	static bool StaticInit();

	void DoFrame();

	int Run();

	void HandleNewClient( ClientProxyPtr inClientProxy );
	//void HandleLostClient( ClientProxyPtr inClientProxy );

	//RoboCatPtr	GetCatForPlayer( int inPlayerId );
	void	SpawnCharacterForPlayer( int inPlayerId );

	virtual ~RealTimeSrv();

private:
	RealTimeSrv();

	bool	InitNetworkMgr();

private:

	bool	mShouldKeepRunning;

};