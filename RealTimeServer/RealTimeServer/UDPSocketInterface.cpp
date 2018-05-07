#include "RealTimeServerPCH.h"


bool UDPSocketInterface::StaticInit()
{
#if _WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		ReportError("Starting Up");
		return false;
	}
#endif
	return true;
}

void UDPSocketInterface::CleanUp()
{
#if _WIN32
	WSACleanup();
#endif
}


void UDPSocketInterface::ReportError(const char* inOperationDesc)
{
#if _WIN32
	LPVOID lpMsgBuf;
	DWORD errorNum = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);


	LOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
#else
	LOG("Error: %hs", inOperationDesc);
#endif
}

int UDPSocketInterface::GetLastError()
{
#if _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif

}

UDPSocketPtr UDPSocketInterface::CreateUDPSocket()
{
	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (s != INVALID_SOCKET)
	{
		return UDPSocketPtr(new UDPSocketInterface(s));
	}
	else
	{
		ReportError("CreateUDPSocket");
		return nullptr;
	}
}

int UDPSocketInterface::Bind(const SocketAddressInterface& inBindAddress)
{
	int error = bind(mSocket, &inBindAddress.mSockAddr, inBindAddress.GetSize());
	if (error != 0)
	{
		ReportError("UDPSocket::Bind");
		return GetLastError();
	}

	return NO_ERROR;
}

int UDPSocketInterface::SendTo(const void* inToSend, int inLength, const SocketAddressInterface& inToAddress)
{
	int byteSentCount = sendto(mSocket,
		static_cast<const char*>(inToSend),
		inLength,
		0, &inToAddress.mSockAddr, inToAddress.GetSize());

	if (byteSentCount <= 0)
	{
		//we'll return error as negative number to indicate less than requested amount of bytes sent...
		ReportError("UDPSocket::SendTo");
		return -GetLastError();
	}
	else
	{
		return byteSentCount;
	}
}

int UDPSocketInterface::ReceiveFrom(void* inToReceive, int inMaxLength, SocketAddressInterface& outFromAddress)
{
	socklen_t fromLength = outFromAddress.GetSize();

	int readByteCount = recvfrom(mSocket,
		static_cast<char*>(inToReceive),
		inMaxLength,
		0, &outFromAddress.mSockAddr, &fromLength);
	if (readByteCount >= 0)
	{
		return readByteCount;
	}
	else
	{
		int error = GetLastError();

		if (error == WSAEWOULDBLOCK)
		{
			return 0;
		}
		else if (error == WSAECONNRESET)
		{
			//this can happen if a client closed and we haven't DC'd yet.
			//this is the ICMP message being sent back saying the port on that computer is closed
			LOG("Connection reset from %s", outFromAddress.ToString().c_str());
			return -WSAECONNRESET;
		}
		else
		{
			ReportError("UDPSocket::ReceiveFrom");
			return -error;
		}
	}
}

UDPSocketInterface::~UDPSocketInterface()
{
#if _WIN32
	closesocket(mSocket);
#else
	close(mSocket);
#endif
}


int UDPSocketInterface::SetNonBlockingMode(bool inShouldBeNonBlocking)
{
#if _WIN32
	u_long arg = inShouldBeNonBlocking ? 1 : 0;
	int result = ioctlsocket(mSocket, FIONBIO, &arg);
#else
	int flags = fcntl(mSocket, F_GETFL, 0);
	flags = inShouldBeNonBlocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
	int result = fcntl(mSocket, F_SETFL, flags);
#endif

	if (result == SOCKET_ERROR)
	{
		ReportError("UDPSocket::SetNonBlockingMode");
		return GetLastError();
	}
	else
	{
		return NO_ERROR;
	}
}

