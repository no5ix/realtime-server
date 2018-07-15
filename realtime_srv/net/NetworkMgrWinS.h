public:
		virtual ~NetworkMgr() { UdpSockInterf::CleanUp(); }
	
		void	SetDropPacketChance( float inChance )
		{ mDropPacketChance = inChance; isSimilateRealWorld_ = true; }
		void	SetSimulatedLatency( float inLatency )
		{ mSimulatedLatency = inLatency; isSimilateRealWorld_ = true; }
		void	SetIsSimulatedJitter( bool inIsSimulatedJitter )
		{ mSimulateJitter = inIsSimulatedJitter; isSimilateRealWorld_ = true; }
		void	SetIsSimilateRealWorld( bool inIsSimilateRealWorld )
		{ isSimilateRealWorld_ = inIsSimilateRealWorld; }
	
		void SendOutgoingPackets();
	
		void	SendPacket( const OutputBitStream& inOutputStream,
			const SockAddrInterf& inSockAddr );
	
		void	HandleConnectionReset( const SockAddrInterf& inFromAddress );
	private:
		void	ProcessQueuedPackets();
	
		void	SendGamePacket( ClientProxyPtr inClientProxy,
			const uint32_t inConnFlag );
	
		void	ReadIncomingPacketsIntoQueue();
	
		virtual void ProcessPacket( InputBitStream& inInputStream,
			const SockAddrInterf& inFromAddress,
			const UDPSocketPtr& inUDPSocket );
	
		void	HandlePacketFromNewClient( InputBitStream& inInputStream,
			const SockAddrInterf& inFromAddress,
			const UDPSocketPtr& inUDPSocket );
	
	private:
		class ReceivedPacket
		{
		public:
			ReceivedPacket(
				float inReceivedTime,
				InputBitStream& inInputMemoryBitStream,
				const SockAddrInterf& inFromAddress,
				UDPSocketPtr  inUDPSocket = nullptr )
				:
				mReceivedTime( inReceivedTime ),
				mFromAddress( inFromAddress ),
				mPacketBuffer( inInputMemoryBitStream ),
				mUDPSocket( inUDPSocket )
			{}
	
			const	SockAddrInterf&			GetFromAddress()	const { return mFromAddress; }
			float					GetReceivedTime()	const { return mReceivedTime; }
			InputBitStream&	GetPacketBuffer() { return mPacketBuffer; }
			UDPSocketPtr	GetUDPSocket() const { return mUDPSocket; }
	
		private:
	
			float					mReceivedTime;
			InputBitStream			mPacketBuffer;
			SockAddrInterf			mFromAddress;
			UDPSocketPtr			mUDPSocket;
		};
		queue< ReceivedPacket, list< ReceivedPacket > >	mPacketQueue;
	
	private:
		UDPSocketPtr				mSocket;
		static int				kNewNetId;
		typedef unordered_map< SockAddrInterf, ClientProxyPtr >	AddrToClientMap;
		AddrToClientMap		addrToClientMap_;
	
		float						mTimeOfLastStatePacket;
		float						mDropPacketChance;
		float						mSimulatedLatency;
		bool						mSimulateJitter;
		bool						isSimilateRealWorld_;
		float						mLastCheckDCTime;