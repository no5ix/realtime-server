class NetworkMgrSrv : public NetworkMgr
{
public:
	static NetworkMgrSrv*	sInstance;


	static bool				StaticInit( uint16_t inPort );
//
	virtual void			ProcessPacket( InputBitStream& inInputStream, const SocketAddressInterface& inFromAddress ) override;
//	virtual void			HandleConnectionReset( const SocketAddress& inFromAddress ) override;
//
	void			SendOutgoingPackets();
//	void			CheckForDisconnects();
//
	void			RegisterGameObject( GameObjectPtr inGameObject );
	inline	GameObjectPtr	RegisterAndReturn( Entity* inGameObject );
	void			UnregisterGameObject( Entity* inGameObject );
	void			SetStateDirty( int inNetworkId, uint32_t inDirtyState );
//
//	void			RespawnCats();
//
	ClientProxyPtr	GetClientProxy( int inPlayerId ) const;
//
private:
	NetworkMgrSrv();
//
	void	HandlePacketFromNewClient( InputBitStream& inInputStream, const SocketAddressInterface& inFromAddress );
	void	ProcessPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );
//
	void	SendWelcomePacket( ClientProxyPtr inClientProxy );
//	void	UpdateAllClients();
//
//	void	AddWorldStateToPacket( OutputMemoryBitStream& inOutputStream );
//	void	AddScoreBoardStateToPacket( OutputMemoryBitStream& inOutputStream );
//
	void	SendStatePacketToClient( ClientProxyPtr inClientProxy );
	void	WriteLastMoveTimestampIfDirty( OutputBitStream& inOutputStream, ClientProxyPtr inClientProxy );
//
	void	HandleInputPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );
//
//	void	HandleClientDisconnected( ClientProxyPtr inClientProxy );
//
	int		GetNewNetworkId();

	typedef unordered_map< int, ClientProxyPtr >	IntToClientMap;
	typedef unordered_map< SocketAddressInterface, ClientProxyPtr >	AddressToClientMap;

	AddressToClientMap		mAddressToClientMap;
	IntToClientMap			mPlayerIdToClientMap;

	int				mNewPlayerId;
	int				mNewNetworkId;

	float			mTimeOfLastSatePacket;
	float			mTimeBetweenStatePackets;
	float			mClientDisconnectTimeout;
	float			mTimeOfLastStatePacket;
};


inline GameObjectPtr NetworkMgrSrv::RegisterAndReturn( Entity* inGameObject )
{
	GameObjectPtr toRet( inGameObject );
	RegisterGameObject( toRet );
	return toRet;
}
