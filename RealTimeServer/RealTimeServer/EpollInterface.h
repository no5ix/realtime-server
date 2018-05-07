#pragma once

#ifndef _WIN32
#define HAS_EPOLL
#endif


#ifdef HAS_EPOLL


typedef std::unordered_map<SOCKET, UDPSocketPtr> SocketToUDPSocketPtrMap;
typedef std::unordered_map<SOCKET, SocketAddressInterface> SocketToSocketAddrMap;

class EpollInterface
{
public:
	static void StaticInit() { sInst.reset( new EpollInterface ); }
	~EpollInterface();

	bool Add( SOCKET inFd );
	void Wait(float inMaxWait);
	
	void HandleInputEvent( SOCKET inFd );
	void HandleOutputEvent( SOCKET inFd );
	void AcceptClient();
	SOCKET UdpConnect( const SocketAddressInterface& inAddress );

	void SetListener( UDPSocketPtr inListener, SocketAddressInterface inSocketAddr );
	//UDPSocketPtr GetListener() const { return mListener; }

	//SocketToUDPSocketPtrMap* GetUDPSocketPtrs() const { return &mSocketToUDPSocketPtrMap; }

protected:
	EpollInterface( int inSize = 256 );
public:
	static std::unique_ptr<EpollInterface> sInst;
private:
	int mEqfd;
	UDPSocketPtr mListener;
	SocketAddressInterface mListenerAddr;
	SocketToUDPSocketPtrMap mSocketToUDPSocketPtrMap;
	SocketToSocketAddrMap mSocketToSocketAddrMap;
};

#endif // HAS_EPOLL