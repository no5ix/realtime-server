class NetworkMgrSrv : public NetworkMgr
{
public:
	static NetworkMgrSrv*	sInst;


	static bool				StaticInit( uint16_t inPort );
	virtual void			ProcessPacket( InputBitStream& inInputStream, const SocketAddrInterface& inFromAddress, const UDPSocketPtr& inUDPSocket  ) override;
	virtual void			HandleConnectionReset( const SocketAddrInterface& inFromAddress ) override;
	void					SendOutgoingPackets();
	void					RegisterGameObject( GameObjectPtr inGameObject );
	inline	GameObjectPtr	RegisterAndReturn( Entity* inGameObject );
	void					UnregisterGameObject( Entity* inGameObject );
	void					SetStateDirty( int inNetworkId, uint32_t inDirtyState );
	virtual void			CheckForDisconnects();
	ClientProxyPtr			GetClientProxy( int inPlayerId ) const;
	void SendResetPacket	( const SocketAddrInterface& inFromAddress );

private:
	NetworkMgrSrv();

	void	HandlePacketFromNewClient( InputBitStream& inInputStream, const SocketAddrInterface& inFromAddress, const UDPSocketPtr& inUDPSocket );
	void	ProcessPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );

	void	SendWelcomePacket( ClientProxyPtr inClientProxy );


	void	SendStatePacketToClient( ClientProxyPtr inClientProxy );
	void	WriteLastMoveTimestampIfDirty( OutputBitStream& inOutputStream, ClientProxyPtr inClientProxy );

	void	HandleInputPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );

	int		GetNewNetworkId();

	typedef unordered_map< int, ClientProxyPtr >	IntToClientMap;
	typedef unordered_map< SocketAddrInterface, ClientProxyPtr >	AddressToClientMap;

	AddressToClientMap		mAddressToClientMap;
	IntToClientMap			mPlayerIdToClientMap;

	int				mNewPlayerId;
	int				mNewNetworkId;

	float			mTimeOfLastSatePacket;
	float			mTimeBetweenStatePackets;
	float			mTimeOfLastStatePacket;

	float			mLastCheckDCTime;
};


inline GameObjectPtr NetworkMgrSrv::RegisterAndReturn( Entity* inGameObject )
{
	GameObjectPtr toRet( inGameObject );
	RegisterGameObject( toRet );
	return toRet;
}
