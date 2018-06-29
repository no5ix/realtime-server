#include "realtime_srv/common/RealtimeSrvShared.h"


using namespace realtime_srv;

bool UdpSockInterf::StaticInit() {
#ifdef IS_WIN
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	if ( iResult != NO_ERROR ) {
		ReportError( "Starting Up" );
		return false;
	}
#endif
	return true;
}

void UdpSockInterf::CleanUp() {
#ifdef IS_WIN
	WSACleanup();
#endif
}


void UdpSockInterf::ReportError( const char* inOperationDesc ) {
#ifdef IS_WIN
	LPVOID lpMsgBuf;
	DWORD errorNum = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		( LPTSTR )&lpMsgBuf,
		0, NULL );


	LOG( "Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf );
#else
	LOG( "Error: %hs", inOperationDesc );
#endif
}

int UdpSockInterf::GetLastError() {
#ifdef IS_WIN
	return WSAGetLastError();
#else
	return errno;
#endif

}

UDPSocketPtr UdpSockInterf::CreateUDPSocket() {
	SOCKET s = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	if ( s != INVALID_SOCKET ) {
		return UDPSocketPtr( new UdpSockInterf( s ) );
	} else {
		ReportError( "CreateUDPSocket" );
		return nullptr;
	}
}

int UdpSockInterf::Bind( const SockAddrInterf& inBindAddress ) {
	int error = bind( mSocket, &inBindAddress.mSockAddr, inBindAddress.GetSize() );
	if ( error != 0 ) {
		ReportError( "UDPSocketInterface::Bind" );
		return GetLastError();
	}

	return NO_ERROR;
}

int UdpSockInterf::SendTo( const void* inToSend, int inLength, const SockAddrInterf& inToAddress ) {
	int byteSentCount = sendto( mSocket,
		static_cast< const char* >( inToSend ),
		inLength,
		0, &inToAddress.mSockAddr, inToAddress.GetSize() );

	if ( byteSentCount <= 0 ) {
		ReportError( "UDPSocketInterface::SendTo" );
		return -GetLastError();
	} else {
		return byteSentCount;
	}
}

int UdpSockInterf::ReceiveFrom( void* inToReceive, int inMaxLength, SockAddrInterf& outFromAddress ) {
	socklen_t fromLength = outFromAddress.GetSize();

	int readByteCount = recvfrom( mSocket,
		static_cast< char* >( inToReceive ),
		inMaxLength,
		0, &outFromAddress.mSockAddr, &fromLength );
	if ( readByteCount >= 0 ) {
		return readByteCount;
	} else {
		int error = GetLastError();

		if ( error == WSAEWOULDBLOCK ) {
			return 0;
		} else if ( error == WSAECONNRESET ) {
			LOG( "Connection reset from %s", outFromAddress.ToString().c_str() );
			return -WSAECONNRESET;
		} else {
			ReportError( "UDPSocketInterface::ReceiveFrom" );
			return -error;
		}
	}
}

UdpSockInterf::~UdpSockInterf() {
#ifdef IS_WIN
	closesocket( mSocket );
#else
	close( mSocket );
#endif
}


int UdpSockInterf::SetNonBlockingMode( bool inShouldBeNonBlocking ) {
#ifdef IS_WIN
	u_long arg = inShouldBeNonBlocking ? 1 : 0;
	int result = ioctlsocket( mSocket, FIONBIO, &arg );
#else
	int flags = fcntl( mSocket, F_GETFL, 0 );
	flags = inShouldBeNonBlocking ? ( flags | O_NONBLOCK ) : ( flags & ~O_NONBLOCK );
	int result = fcntl( mSocket, F_SETFL, flags );
#endif

	if ( result == SOCKET_ERROR ) {
		ReportError( "UDPSocketInterface::SetNonBlockingMode" );
		return GetLastError();
	} else {
		return NO_ERROR;
	}
}



int UdpSockInterf::Connect( const SockAddrInterf& inAddress ) {
	int err = connect( mSocket, &inAddress.mSockAddr, inAddress.GetSize() );
	if ( err < 0 ) {
		ReportError( "TCPSocket::Connect" );
		return GetLastError();
	}
	return NO_ERROR;
}



int UdpSockInterf::SetReUse() {

#ifndef IS_WIN
	int reuse = 1;
	int err = setsockopt( mSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( reuse ) );
	if ( err < 0 ) {
		ReportError( "UDPSocketInterface::SetReUse SO_REUSEADDR" );
		return GetLastError();
	}

	err = setsockopt( mSocket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof( reuse ) );
	if ( err < 0 ) {
		ReportError( "UDPSocketInterface::SetReUse SO_REUSEPORT" );
		return GetLastError();
	}
#endif //IS_WIN

	return NO_ERROR;
}




int32_t	UdpSockInterf::Send( const void* inData, size_t inLen ) {
	int byteSentCount = send( mSocket, static_cast< const char* >( inData ), inLen, 0 );

	if ( byteSentCount <= 0 ) {
		ReportError( "UDPSocketInterface::Send" );
		return -GetLastError();
	} else {
		return byteSentCount;
	}
}

int32_t	UdpSockInterf::Recv( void* inData, size_t inLen ) {
	int bytesReceivedCount = recv( mSocket, static_cast< char* >( inData ), inLen, 0 );

	if ( bytesReceivedCount >= 0 ) {
		return bytesReceivedCount;
	} else {
		int error = GetLastError();

		if ( error == WSAEWOULDBLOCK ) {
			return 0;
		} else if ( error == WSAECONNRESET ) {
			//LOG( "Connection reset from %s", outFromAddress.ToString().c_str() );
			return -WSAECONNRESET;
		} else {
			ReportError( "UDPSocketInterface::Recv" );
			return -error;
		}
	}
}