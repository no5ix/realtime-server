#pragma once

namespace realtime_srv
{

class SockAddrInterf
{
public:
	SockAddrInterf( uint32_t inAddress, uint16_t inPort )
	{
		GetAsSockAddrIn()->sin_family = AF_INET;
		GetIP4Ref() = htonl( inAddress );
		GetAsSockAddrIn()->sin_port = htons( inPort );
	}

	SockAddrInterf( const sockaddr& inSockAddr )
	{
		memcpy( &mSockAddr, &inSockAddr, sizeof( sockaddr ) );
	}

	SockAddrInterf()
	{
		GetAsSockAddrIn()->sin_family = AF_INET;
		GetIP4Ref() = INADDR_ANY;
		GetAsSockAddrIn()->sin_port = 0;
	}

	bool operator==( const SockAddrInterf& inOther ) const
	{
		return ( mSockAddr.sa_family == AF_INET &&
			GetAsSockAddrIn()->sin_port == inOther.GetAsSockAddrIn()->sin_port ) &&
			( GetIP4Ref() == inOther.GetIP4Ref() );
	}

	size_t GetHash() const
	{
		return ( GetIP4Ref() ) |
			( ( static_cast< uint32_t >( GetAsSockAddrIn()->sin_port ) ) << 13 ) |
			mSockAddr.sa_family;
	}


	uint32_t				GetSize()			const { return sizeof( sockaddr ); }

	std::string	ToString() const
	{
		const sockaddr_in* s = GetAsSockAddrIn();
		char destinationBuffer[128];
	#ifdef IS_WIN
		InetNtop( s->sin_family, const_cast< in_addr* >( &s->sin_addr ), destinationBuffer, sizeof( destinationBuffer ) );
	#else
		inet_ntop( s->sin_family, &s->sin_addr, destinationBuffer, sizeof( destinationBuffer ) );
	#endif
		return RealtimeSrvHelper::Sprintf( "%s:%d",
			destinationBuffer,
			ntohs( s->sin_port ) );
	}

private:
	friend class UdpSockInterf;
	friend class TCPSocket;

	sockaddr mSockAddr;
#ifdef IS_WIN
	uint32_t&				GetIP4Ref() { return *reinterpret_cast< uint32_t* >( &GetAsSockAddrIn()->sin_addr.S_un.S_addr ); }
	const uint32_t&			GetIP4Ref()			const { return *reinterpret_cast< const uint32_t* >( &GetAsSockAddrIn()->sin_addr.S_un.S_addr ); }
#else
	uint32_t&				GetIP4Ref() { return GetAsSockAddrIn()->sin_addr.s_addr; }
	const uint32_t&			GetIP4Ref()			const { return GetAsSockAddrIn()->sin_addr.s_addr; }
#endif

	sockaddr_in*			GetAsSockAddrIn() { return reinterpret_cast< sockaddr_in* >( &mSockAddr ); }
	const	sockaddr_in*	GetAsSockAddrIn()	const { return reinterpret_cast< const sockaddr_in* >( &mSockAddr ); }

};

typedef shared_ptr< SockAddrInterf > SocketAddressPtr;

}

namespace std
{
using realtime_srv::SockAddrInterf;

template<> struct hash< SockAddrInterf >
{
	size_t operator()( const SockAddrInterf& inAddress ) const
	{
		return inAddress.GetHash();
	}
};
}