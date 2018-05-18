#include "RealTimeSrvPCH.h"




#ifdef HAS_EPOLL

std::unique_ptr<EpollInterface> EpollInterface::sInst;

EpollInterface::EpollInterface( int inSize ) :
	mEqfd( epoll_create( inSize ) )
{
	if ( mEqfd == -1 )
	{
		LOG( "Error: %hs", "epoll_create failed" );
	}
}

EpollInterface::~EpollInterface()
{
	if ( mEqfd != -1 )
	{
		close( mEqfd );
	}
}

bool EpollInterface::Add( SOCKET inFd )
{
	struct epoll_event ev;
	memset( &ev, 0, sizeof( ev ) );

	ev.data.fd = inFd;
	ev.events = EPOLLIN | EPOLLET;

	if ( epoll_ctl( mEqfd, EPOLL_CTL_ADD, inFd, &ev ) < 0 )
	{
		LOG( "Error: %hs", "epoll_ctl failed" );
		return false;
	}
	return true;
}

void EpollInterface::CloseSocket(SOCKET inFd)
{
	mSocketToUDPSocketPtrMap.erase( inFd );
	mSocketToSocketAddrMap.erase( inFd );
	if ( close( inFd ) == -1 )
	{
		LOG( "EpollInterface::CloseSocket, Error: %hs", "close socket" );
	}
}

void EpollInterface::Wait( float inMaxWait )
{
	const int MAX_EVENTS = 10;
	struct epoll_event events[MAX_EVENTS];
	int maxWaitInMilliseconds = inMaxWait == -1.f ? -1 : int( ceil( inMaxWait * 1000 ) );

	int nfds = epoll_wait( mEqfd, events, MAX_EVENTS, maxWaitInMilliseconds );

	for ( int i = 0; i < nfds; ++i )
	{
		if ( events[i].events & EPOLLIN )
		{
			HandleInputEvent( events[i].data.fd );
		}
		else if ( events[i].events & ( EPOLLERR | EPOLLHUP ) )
		{
			CloseSocket( events[i].data.fd );
		}
	}
}

void EpollInterface::HandleInputEvent( SOCKET inFd )
{
	if ( inFd == mListener->GetSocket() )
	{
		AcceptClient();
	}
	else
	{
		SocketToUDPSocketPtrMap::iterator iter = mSocketToUDPSocketPtrMap.find( inFd );
		if ( iter == mSocketToUDPSocketPtrMap.end() )
		{
			return;
		}
		SocketToSocketAddrMap::iterator it = mSocketToSocketAddrMap.find( inFd );
		if ( it == mSocketToSocketAddrMap.end() )
		{
			return;
		}
		NetworkMgrSrv::sInst->RecvIncomingPacketsIntoQueue( iter->second, it->second );
	}
}

void EpollInterface::AcceptClient()
{
	char packetMem[MAX_PACKET_BYTE_LENGTH];
	int packetSize = sizeof( packetMem );
	SocketAddrInterface fromAddress;

	if ( mListener->ReceiveFrom( packetMem, packetSize, fromAddress ) >= 0 )
	{
		UdpConnect( fromAddress );
	}
}

SOCKET EpollInterface::UdpConnect( const SocketAddrInterface& inAddress )
{
	UDPSocketPtr usp = UDPSocketInterface::CreateUDPSocket();
	if (usp)
	{
		if ( usp->SetReUse() == NO_ERROR )
		{
			if ( usp->Bind( mListenerAddr ) == NO_ERROR  )
			{
				if ( usp->Connect( inAddress ) == NO_ERROR  )
				{
					if ( usp->SetNonBlockingMode( true ) == NO_ERROR )
					{
						if ( Add( usp->GetSocket() ) )
						{
							mSocketToUDPSocketPtrMap[usp->GetSocket()] = usp;
							mSocketToSocketAddrMap[usp->GetSocket()] = inAddress;

							return usp->GetSocket();
						}
					}
				}
			}
		}
	}
	return -1;
}

void EpollInterface::SetListener( UDPSocketPtr inListener, SocketAddrInterface inSocketAddr )
{
	mListener = inListener;
	mListenerAddr = inSocketAddr;
}

#endif // HAS_EPOLL

