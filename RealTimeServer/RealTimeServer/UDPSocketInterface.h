

class UDPSocketInterface
{
public:

	~UDPSocketInterface();



	static bool			StaticInit();
	static void			CleanUp();

	static void			ReportError(const char* inOperationDesc);
	static int			GetLastError();

	static shared_ptr< UDPSocketInterface > CreateUDPSocket();

	int Bind(const SocketAddressInterface& inToAddress);
	int SendTo(const void* inToSend, int inLength, const SocketAddressInterface& inToAddress);
	int ReceiveFrom(void* inToReceive, int inMaxLength, SocketAddressInterface& outFromAddress);

	int SetNonBlockingMode(bool inShouldBeNonBlocking);

private:
	UDPSocketInterface(SOCKET inSocket) : mSocket(inSocket) {}
	SOCKET mSocket;

};

typedef shared_ptr< UDPSocketInterface >	UDPSocketPtr;