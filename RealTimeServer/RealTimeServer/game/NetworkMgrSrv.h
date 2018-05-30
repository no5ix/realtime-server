// class UdpConnection;

class NetworkMgrSrv : public NetworkMgr
{
public:
	static std::unique_ptr<NetworkMgrSrv>	sInst;

	static bool				StaticInit( uint16_t inPort );

	virtual void			ProcessPacket( 
		InputBitStream& inInputStream, 
		const SocketAddrInterface& inFromAddress, 
		const UDPSocketPtr& inUDPSocket
		// ,
		// const std::shared_ptr<UdpConnection>& inUdpConnetction
	) override;

	virtual void			HandleConnectionReset( const SocketAddrInterface& inFromAddress ) override;
	virtual void			SendOutgoingPackets();
	void					RegisterGameObject( EntityPtr inGameObject );
	inline	EntityPtr		RegisterAndReturn( Entity* inGameObject );
	void					UnregisterGameObject( Entity* inGameObject );
	void					SetStateDirty( int inNetworkId, uint32_t inDirtyState );
	virtual void			CheckForDisconnects();
	ClientProxyPtr			GetClientProxy( int inPlayerId ) const;

	void					SendResetPacket ( ClientProxyPtr inClientProxy );
	uint32_t				HandleServerReset( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );

private:
	NetworkMgrSrv();

	void	HandlePacketFromNewClient( 
		InputBitStream& inInputStream, 
		const SocketAddrInterface& inFromAddress, 
		const UDPSocketPtr& inUDPSocket
		// ,
		// const std::shared_ptr<UdpConnection>& inUdpConnetction
	);

	void	DoProcessPacket( ClientProxyPtr inClientProxy, InputBitStream& inInputStream );

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


inline EntityPtr NetworkMgrSrv::RegisterAndReturn( Entity* inGameObject )
{
	EntityPtr toRet( inGameObject );
	RegisterGameObject( toRet );
	return toRet;
}
