class RealTimeServer
{
public:
	static std::unique_ptr< RealTimeServer >	sInstance;

	static bool StaticInit();

	void DoFrame();

	int Run();

	void HandleNewClient( ClientProxyPtr inClientProxy );
	//void HandleLostClient( ClientProxyPtr inClientProxy );

	//RoboCatPtr	GetCatForPlayer( int inPlayerId );
	void	SpawnCharacterForPlayer( int inPlayerId );

	virtual ~RealTimeServer();

private:
	RealTimeServer();

	bool	InitNetworkManager();

private:

	bool	mShouldKeepRunning;

};