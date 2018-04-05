

class UDPSocket
{
public:

	~UDPSocket();



	static bool			StaticInit();
	static void			CleanUp();

	static void			ReportError(const char* inOperationDesc);
	static int			GetLastError();

	static shared_ptr< UDPSocket > CreateUDPSocket();

	int Bind(const SocketAddress& inToAddress);
	int SendTo(const void* inToSend, int inLength, const SocketAddress& inToAddress);
	int ReceiveFrom(void* inToReceive, int inMaxLength, SocketAddress& outFromAddress);

	/*
	int SendTo( const MemoryOutputStream& inMOS, const SocketAddress& inToAddress );
	int ReceiveFrom( MemoryInputStream& inMIS, SocketAddress& outFromAddress );
	*/

	int SetNonBlockingMode(bool inShouldBeNonBlocking);

private:
	friend class SocketUtil;
	UDPSocket(SOCKET inSocket) : mSocket(inSocket) {}
	SOCKET mSocket;

};

typedef shared_ptr< UDPSocket >	UDPSocketPtr;