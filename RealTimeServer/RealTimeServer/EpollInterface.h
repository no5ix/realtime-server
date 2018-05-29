#pragma once

//#ifndef _WIN32
//#define DEPRECATED_EPOLL_INTERFACE
//#endif

#ifndef _WIN32
#define NEW_EPOLL_INTERFACE
#endif

#ifdef DEPRECATED_EPOLL_INTERFACE

typedef std::unordered_map<SOCKET, UDPSocketPtr> SocketToUDPSocketPtrMap;
typedef std::unordered_map<SOCKET, SocketAddrInterface> SocketToSocketAddrMap;

class EpollInterface
{
public:
	static std::unique_ptr<EpollInterface> sInst;
public:
	static void StaticInit() { sInst.reset( new EpollInterface ); }
	~EpollInterface();

	bool Add( SOCKET inFd );
	void CloseSocket( SOCKET inFd );
	void Wait( float inMaxWait );
	
	void HandleInputEvent( SOCKET inFd );

	void AcceptClient();
	SOCKET UdpConnect( const SocketAddrInterface& inAddress );
	void SetListener( UDPSocketPtr inListener, SocketAddrInterface inSocketAddr );

protected:
	EpollInterface( int inSize = 256 );

private:
	int mEqfd;
	UDPSocketPtr mListener;
	SocketAddrInterface mListenerAddr;
	SocketToUDPSocketPtrMap mSocketToUDPSocketPtrMap;
	SocketToSocketAddrMap mSocketToSocketAddrMap;
};

#endif // DEPRECATED_EPOLL_INTERFACE